/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST/ES6Class.h"
#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Parser/JSLexer.h"

namespace hermes {
struct ClassMemberKey {
    Identifier identifier;
    bool isStatic;

    ClassMemberKey(Identifier identifier, bool isStatic): identifier(identifier), isStatic(isStatic) {}

    bool operator==(const ClassMemberKey &other) const {
        return identifier == other.identifier && isStatic == other.isStatic;
    }

    bool operator!=(const ClassMemberKey &other) const {
        return !(*this == other);
    }
};

}

// Enable using ClassMemberKey in DenseMap.
namespace llvh {

template <>
struct DenseMapInfo<hermes::ClassMemberKey> {
  static inline hermes::ClassMemberKey getEmptyKey() {
      return hermes::ClassMemberKey(hermes::Identifier::getFromPointer(DenseMapInfo<hermes::UniqueString *>::getEmptyKey()), false);
  }
  static inline hermes::ClassMemberKey getTombstoneKey() {
      return hermes::ClassMemberKey(hermes::Identifier::getFromPointer(
        DenseMapInfo<hermes::UniqueString *>::getTombstoneKey()), false);
  }
  static inline unsigned getHashValue(hermes::ClassMemberKey memberKey) {
      auto identifierHash = DenseMapInfo<hermes::UniqueString *>::getHashValue(memberKey.identifier.getUnderlyingPointer());
      return static_cast<unsigned>(llvh::hash_combine(identifierHash, memberKey.isStatic));
  }

  static inline bool isEqual(hermes::ClassMemberKey a, hermes::ClassMemberKey b) {
    return a == b;
  }
};

} // namespace llvh

namespace hermes {

enum class ClassMemberKind {
    Constructor,
    Method,
    PropertyGetter,
    PropertySetter
};

static ClassMemberKind getClassMemberKind(ESTree::MethodDefinitionNode *methodDefinition) {
    const auto &str = methodDefinition->_kind->str();
    if (str == "constructor") {
        return ClassMemberKind::Constructor;
    } else if (str == "method") {
        return ClassMemberKind::Method;
    } else if (str == "get") {
        return ClassMemberKind::PropertyGetter;
    } else if (str == "set") {
        return ClassMemberKind::PropertySetter;
    }
    abort();
}

class NodeVector {
public:
    using Storage = llvh::SmallVector<ESTree::Node *, 8>;

    NodeVector() = default;
    NodeVector(std::initializer_list<ESTree::Node *> nodes) {
        for (auto &node: nodes) {
            _storage.push_back(node);
        }
    }

    NodeVector(ESTree::NodeList &list) {
        for (auto &node: list) {
            _storage.push_back(&node);
        }
    }

    ~NodeVector() = default;

    size_t size() const {
        return _storage.size();
    }

    Storage::const_iterator begin() const {
        return _storage.begin();
    }

    Storage::const_iterator end() const {
        return _storage.end();
    }

    void append(ESTree::Node *node) {
        _storage.emplace_back(node);
    }

    void prepend(ESTree::Node *node) {
        _storage.insert(_storage.begin(), node);
    }

    ESTree::NodeList toNodeList() const {
        ESTree::NodeList nodeList;
        for (auto &node: _storage) {
            nodeList.push_back(*node);
        }

        return nodeList;
    }

private:
    Storage _storage;
};

struct VisitedClass {
    UniqueString *className = nullptr;
    UniqueString *parentClassName = nullptr;

    VisitedClass(ESTree::NodePtr className, ESTree::NodePtr superClass) {
        if (className != nullptr) {
            this->className = llvh::cast<ESTree::IdentifierNode>(className)->_name;
        }
        if (superClass != nullptr) {
            this->parentClassName = llvh::cast<ESTree::IdentifierNode>(superClass)->_name;
        }
    }
};

struct ResolvedClassMember {
    ESTree::StringLiteralNode *name;
    bool isStatic;
    ESTree::MethodDefinitionNode *method = nullptr;
    ESTree::MethodDefinitionNode *getter = nullptr;
    ESTree::MethodDefinitionNode *setter = nullptr;

    ResolvedClassMember(ESTree::StringLiteralNode *name, bool isStatic): name(name), isStatic(isStatic) {}
};

struct ResolvedClassMembers {
    ESTree::MethodDefinitionNode *constructor = nullptr;

    llvh::SmallVector<ResolvedClassMember, 8> members;
};

class ES6ClassesTransformations {
public:
    ES6ClassesTransformations(Context &context): context_(context),
        identVar_(context.getIdentifier("var").getUnderlyingPointer()) {}

    ESTree::VisitResult visit(ESTree::ClassDeclarationNode *classDecl) {
        auto *classBody = llvh::dyn_cast<ESTree::ClassBodyNode>(classDecl->_body);
        if (classBody == nullptr) {
            return ESTree::Unmodified;
        }

        auto classMembers = resolveClassMembers(classBody);
        auto *ctorAsFunction = createClassCtor(classDecl->_id, classBody, classDecl->_superClass, classMembers.constructor);

        auto *oldProcessingClass = _currentProcessingClass;
        VisitedClass currentProcessingClass(classDecl->_id, classDecl->_superClass);
        _currentProcessingClass = &currentProcessingClass;

        ESTree::Node *superClassIdentifier = nullptr;
        if (classDecl->_superClass != nullptr) {
            superClassIdentifier = copyIdentifier(classDecl->_superClass);
        } else {
            superClassIdentifier = new (context_) ESTree::NullLiteralNode();
        }

        auto *defineClassResult = makeHermesES6InternalCall("defineClass", {copyIdentifier(ctorAsFunction->_id), superClassIdentifier});

        NodeVector statements;
        statements.append(ctorAsFunction);
        statements.append(toStatement(defineClassResult));

        appendMethods(classDecl->_id, classMembers, statements);

        // Wrap into an immediately invoked function
        auto *expressionResult = blockToExpression(statements, ctorAsFunction->_id);

        auto result = makeSingleVariableDecl(copyIdentifier(classDecl->_id), expressionResult);

        _currentProcessingClass = oldProcessingClass;

        return result;
    }

    ESTree::VisitResult visit(ESTree::CallExpressionNode *callExpression) {
        // Convert super.method(...args) calls to ParentClass.prototype.method.call(this, ...args);
        auto *memberExpressionNode = llvh::dyn_cast<ESTree::MemberExpressionNode>(callExpression->_callee);
        if (memberExpressionNode == nullptr || memberExpressionNode->_object->getKind() != ESTree::NodeKind::Super) {
            return ESTree::Unmodified;
        }

        auto *topClass = _currentProcessingClass;
        if (topClass == nullptr || topClass->parentClassName == nullptr) {
            // Should not happen
            return ESTree::Unmodified;
        }

        return createSuperMethodCall(topClass->parentClassName, memberExpressionNode->_property, NodeVector(callExpression->_arguments));
    }

    ESTree::VisitResult visit(ESTree::MemberExpressionNode *memberExpression) {
        // Convert super.property into Reflect.get(ParentClass.prototype, 'property', this);
        if (memberExpression->_object->getKind() != ESTree::NodeKind::Super) {
            return ESTree::Unmodified;
        }

        auto *topClass = _currentProcessingClass;
        if (topClass == nullptr || topClass->parentClassName == nullptr) {
            // Should not happen
            return ESTree::Unmodified;
        }

        return createGetSuperProperty(topClass->parentClassName, memberExpression->_property);
    }

    void visit(ESTree::Node *node) {
      visitESTreeChildren(*this, node);
    }

    bool incRecursionDepth(ESTree::Node *) {
      return true;
    }

    void decRecursionDepth() {}

private:
    Context& context_;
    UniqueString *const identVar_;
//    llvh::SmallVector<VisitedClass, 2> _classStack;
    const VisitedClass *_currentProcessingClass = nullptr;
    const ResolvedClassMember *_currentClassMember = nullptr;

    void popClass(const VisitedClass **oldProcessingClass) {
        _currentProcessingClass = *oldProcessingClass;
    }

    ESTree::StatementNode *toStatement(ESTree::Node *expression) {
        return new (context_) ESTree::ExpressionStatementNode(expression, nullptr);
    }

    ESTree::IdentifierNode *copyIdentifier(ESTree::Node *identifer) {
        auto *typedIdentifier = llvh::cast<ESTree::IdentifierNode>(identifer);
        return new (context_) ESTree::IdentifierNode(typedIdentifier->_name, nullptr, false);
    }

    ESTree::Node *makeSingleVariableDecl(ESTree::Node *identifier, ESTree::Node *value) {
        auto *variableDeclarator = new (context_) ESTree::VariableDeclaratorNode(value, identifier);
        ESTree::NodeList variableList;
        variableList.push_back(*variableDeclarator);
        return new (context_) ESTree::VariableDeclarationNode(identVar_, std::move(variableList));
    }

    ESTree::Node *makeHermesES6InternalCall(llvh::StringRef methodName, const NodeVector &parameters) {
        auto hermesInternalIdentifier = makeIdentifierNode("HermesES6Internal");
        auto methodIdentifier = makeIdentifierNode(methodName);

        auto *getPropertyNode = new (context_) ESTree::MemberExpressionNode(hermesInternalIdentifier, methodIdentifier, false);
        return new (context_) ESTree::CallExpressionNode(getPropertyNode, nullptr, parameters.toNodeList());
    }

    ESTree::IdentifierNode *makeIdentifierNode(UniqueString *name) {
        return new (context_) ESTree::IdentifierNode(name, nullptr, false);
    }

    ESTree::IdentifierNode *makeIdentifierNode(llvh::StringRef name) {
        return makeIdentifierNode(context_.getIdentifier(name).getUnderlyingPointer());
    }

    ESTree::Node *makeUndefinedNode() {
        return makeIdentifierNode("undefined");
    }

    ESTree::Node *createCallWithForwardedThis(ESTree::Node *object, NodeVector parameters) {
        auto *this_ = new (context_) ESTree::ThisExpressionNode();

        parameters.prepend(this_);

        auto methodIdentifier = makeIdentifierNode("call");

        auto *getPropertyNode = new (context_) ESTree::MemberExpressionNode(object, methodIdentifier, false);
        return new (context_) ESTree::CallExpressionNode(getPropertyNode, nullptr, parameters.toNodeList());
    }

    ESTree::Node *createSuperCall(ESTree::Node *superClass, NodeVector parameters) {
        return createCallWithForwardedThis(copyIdentifier(superClass), std::move(parameters));
    }

    ESTree::Node *createGetSuperProperty(UniqueString *superClassName, ESTree::Node *propertyName) {
        auto *reflectGet = new (context_) ESTree::MemberExpressionNode(makeIdentifierNode("Reflect"), makeIdentifierNode("get"), false);

        ESTree::NodeList parameters;
        if (_currentClassMember && _currentClassMember->isStatic) {
            // Reflect.get(ParentClass, 'property', this);
            parameters.push_back(*makeIdentifierNode(superClassName));
        } else {
            // Reflect.get(ParentClass.prototype, 'property', this);
            auto *getParentClassPrototype = new (context_) ESTree::MemberExpressionNode(makeIdentifierNode(superClassName), makeIdentifierNode("prototype"), false);
            parameters.push_back(*getParentClassPrototype);
        }

        auto *propertyStringLiteral = new (context_) ESTree::StringLiteralNode(llvh::cast<ESTree::IdentifierNode>(propertyName)->_name);
        auto *this_ = new (context_) ESTree::ThisExpressionNode();

        parameters.push_back(*propertyStringLiteral);
        parameters.push_back(*this_);

        return new (context_) ESTree::CallExpressionNode(reflectGet, nullptr, std::move(parameters));
    }

    ESTree::Node *createSuperMethodCall(UniqueString *superClassName, ESTree::NodePtr property, NodeVector parameters) {
        ESTree::Node *getMethodNodeParameter = nullptr;
        if (_currentClassMember && _currentClassMember->isStatic) {
            // Convert super.method(...args) calls to ParentClass.method.call(this, ...args);
            getMethodNodeParameter = makeIdentifierNode(superClassName);
        } else {
            // Convert super.method(...args) calls to ParentClass.prototype.method.call(this, ...args);
            auto prototypeIdentifier = makeIdentifierNode("prototype");

            getMethodNodeParameter = new (context_) ESTree::MemberExpressionNode(makeIdentifierNode(superClassName), prototypeIdentifier, false);
        }

        auto *getMethodNode = new (context_) ESTree::MemberExpressionNode(getMethodNodeParameter, property, false);

        return createCallWithForwardedThis(getMethodNode, std::move(parameters));
    }

    ESTree::Node *blockToExpression(const NodeVector &statements, ESTree::Node *returnVariableName) {
        auto stmtList = statements.toNodeList();

        auto *returnStmt = new (context_) ESTree::ReturnStatementNode(copyIdentifier(returnVariableName));

        stmtList.push_back(*returnStmt);

        auto *body = new (context_) ESTree::BlockStatementNode(std::move(stmtList));

        auto *immediateInvokedFunction = new (context_) ESTree::FunctionExpressionNode(
                                                                                       nullptr,
                                                                                       {},
                                                                                       body,
                                                                                       nullptr,
                                                                                       nullptr,
                                                                                       nullptr,
                                                                                       false,
                                                                                       false);

        return new (context_) ESTree::CallExpressionNode(immediateInvokedFunction, nullptr, {});
    }

    ESTree::FunctionDeclarationNode *createClassCtor(ESTree::Node *identifier, ESTree::ClassBodyNode *classBody, ESTree::Node *superClass, ESTree::MethodDefinitionNode *existingCtor) {
        ESTree::NodeList paramList;
        ESTree::Node *superCall = nullptr;
        NodeVector userStmts;

        if (existingCtor != nullptr) {
            auto *ctorExpression = llvh::dyn_cast<ESTree::FunctionExpressionNode>(existingCtor->_value);
            paramList = std::move(ctorExpression->_params);

            auto *block = llvh::dyn_cast<ESTree::BlockStatementNode>(ctorExpression->_body);
            NodeVector tmpStatements(block->_body);
            for (auto *stmt: tmpStatements) {
                visitESTreeChildren(*this, stmt);

                if (isSuperCtorCall(stmt)) {
                    auto *call = llvh::cast<ESTree::CallExpressionNode>(llvh::cast<ESTree::ExpressionStatementNode>(stmt)->_expression);

                    superCall = createSuperCall(superClass, NodeVector(call->_arguments));
                } else {
                    userStmts.append(stmt);
                }
            }
        }

         if (superCall == nullptr && superClass != nullptr) {
             superCall = createSuperCall(superClass, {});
         }

        ESTree::NodeList stmtList;
        // Call super as the first statement
        if (superCall != nullptr) {
            stmtList.push_back(*superCall);
        }

        // Append initializers of class properties
        appendPropertyInitializers(classBody, stmtList);

        // Append user statements that were found in the user provided constructor
        for (auto *statement: userStmts) {
            stmtList.push_back(*statement);
        }

        auto *body = new (context_) ESTree::BlockStatementNode(std::move(stmtList));

        return new (context_) ESTree::FunctionDeclarationNode(
                identifier,
                std::move(paramList),
                body,
                nullptr,
                nullptr,
                nullptr,
                false,
                false);
    }

    void appendPropertyInitializers(ESTree::ClassBodyNode *classBody, ESTree::NodeList &stmtList) {
        for (auto &entry: classBody->_body) {
            if (auto *classProperty = llvh::dyn_cast<ESTree::ClassPropertyNode>(&entry)) {
                if (classProperty->_value != nullptr) {
                    visitESTreeNode(*this, classProperty->_value);
                    auto *initializer = createThisPropertyInitializer(classProperty->_key, classProperty->_value);
                    stmtList.push_back(*initializer);
                }
            }
        }
    }

    ResolvedClassMembers resolveClassMembers(ESTree::ClassBodyNode *classBody) {
        ResolvedClassMembers resolvedClassMembers;
        llvh::DenseMap<ClassMemberKey, size_t> classMemberIndexByIdentifier;

        for (auto &entry: classBody->_body) {
            if (auto *methodDefinition = llvh::dyn_cast<ESTree::MethodDefinitionNode>(&entry)) {
                auto memberKind = getClassMemberKind(methodDefinition);

                if (memberKind == ClassMemberKind::Constructor) {
                    resolvedClassMembers.constructor = methodDefinition;
                    continue;
                }

                auto *identifierNode = llvh::cast<ESTree::IdentifierNode>(methodDefinition->_key);
                auto memberKey = ClassMemberKey(Identifier::getFromPointer(identifierNode->_name), methodDefinition->_static);

                // Group all the MethodDefinition by the member name
                // e.g.:
                /**
                 Group all the MethodDefinition by the member name.
                 e.g.: class Animal {
                 get hello() {
                   return this._hello;
                 }

                 set hello(value) {
                     this._hello = value;
                 }

                 constructor(name) {
                   this.name = name
                 }

                 sayHello() {}
                 }`

                 Would be turned into an array containing the `hello` getter/setter, the `constructor`, and the `sayHello` method
                 */
                ResolvedClassMember *resolvedClassMember = nullptr;
                auto it = classMemberIndexByIdentifier.find(memberKey);
                if (it != classMemberIndexByIdentifier.end()) {
                    resolvedClassMember = &resolvedClassMembers.members[it->second];
                } else {
                    auto index = resolvedClassMembers.members.size();
                    classMemberIndexByIdentifier[memberKey] = index;
                    resolvedClassMembers.members.emplace_back(new (context_) ESTree::StringLiteralNode(identifierNode->_name), memberKey.isStatic);
                    resolvedClassMember = &resolvedClassMembers.members.back();
                }

                switch (memberKind) {
                    case ClassMemberKind::Method:
                        resolvedClassMember->method = methodDefinition;
                        break;
                    case ClassMemberKind::PropertyGetter:
                        resolvedClassMember->getter = methodDefinition;
                        break;
                    case ClassMemberKind::PropertySetter:
                        resolvedClassMember->setter = methodDefinition;
                        break;
                    default:
                        std::abort();
                }
            }
        }

        return resolvedClassMembers;
    }

    void visitMethodESTreeChildren(const ResolvedClassMember &classMember, ESTree::Node *node) {
        auto *previousClassMember = _currentClassMember;
        _currentClassMember = &classMember;
        visitESTreeChildren(*this, node);
        _currentClassMember = previousClassMember;
    }

    void appendMethods(ESTree::Node *className, const ResolvedClassMembers &classMembers, NodeVector &stmtList) {
        for (const auto &classMember: classMembers.members) {
            NodeVector parameters;
            parameters.append(copyIdentifier(className));
            parameters.append(classMember.name);

            llvh::StringRef hermesCallName;

            if (classMember.method != nullptr) {
                visitMethodESTreeChildren(classMember, classMember.method);

                hermesCallName = classMember.isStatic ? "defineStaticClassMethod" : "defineClassMethod";

                parameters.append(classMember.method->_value);
            } else {
                hermesCallName = classMember.isStatic ? "defineStaticClassProperty" : "defineClassProperty";

                if (classMember.getter != nullptr) {
                    visitMethodESTreeChildren(classMember, classMember.getter);
                    parameters.append(classMember.getter->_value);
                } else {
                    parameters.append(makeUndefinedNode());
                }

                if (classMember.setter != nullptr) {
                    visitMethodESTreeChildren(classMember, classMember.setter);
                    parameters.append(classMember.setter->_value);
                } else {
                    parameters.append(makeUndefinedNode());
                }
            }

            auto *call = makeHermesES6InternalCall(hermesCallName, parameters);

            stmtList.append(toStatement(call));
        }
    }

    ESTree::Node *createThisPropertyInitializer(ESTree::Node *identifier, ESTree::Node *initialValue) {
        auto *this_ = new (context_) ESTree::ThisExpressionNode();

        auto *getPropertyNode = new (context_) ESTree::MemberExpressionNode(this_, identifier, false);
        auto *assignmentExpression = new (context_) ESTree::AssignmentExpressionNode(getIdentifierForTokenKind(parser::TokenKind::equal), getPropertyNode, initialValue);

        return toStatement(assignmentExpression);
    }

    UniqueString *getIdentifierForTokenKind(parser::TokenKind tokenKind) const {
        return context_.getStringTable().getIdentifier(hermes::parser::tokenKindStr(tokenKind)).getUnderlyingPointer();
    }

    bool isSuperCtorCall(ESTree::Node *node) {
        auto *stmt = llvh::dyn_cast<ESTree::ExpressionStatementNode>(node);
        if (stmt == nullptr) {
            return false;
        }
        
        auto *call = llvh::dyn_cast<ESTree::CallExpressionNode>(stmt->_expression);
        if (call == nullptr) {
            return false;
        }
        auto *callee = ESTree::getCallee(call);
        return callee->getKind() == ESTree::NodeKind::Super;
    }
};

void transformES6Classes(Context &context, ESTree::Node *node) {
  ES6ClassesTransformations transformations(context);
  visitESTreeNode(transformations, node);
}

} // namespace hermes
