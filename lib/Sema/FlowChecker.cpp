/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Typechecker for Flow-typed JS.
///
/// Variable type resolution:
/// When entering a block, DeclareScopeTypes and AnnotateScopeDecls are called.
/// DeclareScopeTypes creates Type* for all the bindings which create new types
/// in the scope (type aliases and class declarations) and stores them in
/// the binding table.
/// AnnotateScopeDecls then annotates variable bindings with their types,
/// either by using the explicitly annotated type or inferring them by visiting
/// initializers when possible. The corresponding Decls are associated with the
/// types when possible, 'any' if there was no better type that could be
/// assigned.
/// Once these two functions are done, the visitor is called on the AST nodes in
/// the scope, skipping any initializers visited in AnnotateScopeDecls.
/// When IdentifierNode is encountered, the corresponding Decl is looked up and
/// the type is returned if it exists. Otherwise, any is returned.
///
/// Classes:
/// All classes are handled in two phases: ParseClassType and the typechcking
/// AST visitor.
/// ParseClassType determines the fields and methods of the class as well as
/// their types, resolves the super class, and ensures all the signatures on the
/// fields/methods are valid.
/// The bodies of methods are descended into during the AST visitor on
/// ClassDeclarationNode.
///
/// Generic functions:
/// Generic function declarations are detected during AnnotateScopeDecls and
/// their Decl is marked as \c generic.
/// The generic is registered as a GenericInfo in the FlowChecker, and nothing
/// further is done.
/// When a generic function is invoked, the AST is cloned (or an existing clone
/// is used), the cloned AST is typechecked, and the type of the CallExpression
/// is set to the result type of the cloned AST now that it has been annotated.
///
/// Generic classes:
/// Generic classes can also introduce new types into the scope,
/// so they have to be handled differently from generic functions.
/// The ParseClassType phase has to be deferred until the end of
/// DeclareScopeTypes, because fields in a generic class may refer to fields in
/// other generic classes which haven't been declared yet.
/// This is done by the deferredParseGenerics_ field on FlowChecker.
/// Deferring ParseClassType is only necessary during DeclareScopeTypes;
/// if a class is specialized via a constructor call during the typechecking
/// AST traversal phase, the scope types will already be declared so we can
/// ParseClassType immediately.
/// However, the bodies of the generic class's methods shouldn't be typechecked
/// immediately. Instead they are put into a typecheckQueue_.
/// \code
///    class A<T> extends B<T> {
///      constructor() {
///        super();
///      }
///      foo(): void {}
///    }
///
///    class B<T> {
///      constructor() {}
///      method(): void {
///        new A<boolean>().foo();
///      }
///    }
///
///    new A<number>();
/// \endcode
/// If we eagerly typechecked:
///   A<number>
///   B<number> (via superClass)
///   A<boolean> (via NewExpression)
///   B<boolean> (via superClass)
///   A<boolean> <- ERROR: A<boolean> not initialized yet
/// Queueing the typechecking of the bodies until the very end means that we can
/// use the ClassType immediately after ParseClassType, without having to deal
/// with the creation of potentially more generic specializations recursively.
///
/// Generic type aliases:
/// These are handled similarly to generic classes, but they don't require a new
/// AST node (because IRGen doesn't read type aliases at all).
/// The specialization is stored as a \c Type instead, and the resolution can be
/// called from within DeclareScopeTypes or afterwards from the typechecking
/// visitor.
//===----------------------------------------------------------------------===//

#include "FlowChecker.h"

#include "ESTreeClone.h"
#include "hermes/Support/Conversions.h"

#include "llvh/ADT/MapVector.h"
#include "llvh/ADT/ScopeExit.h"
#include "llvh/ADT/SetVector.h"
#include "llvh/ADT/Twine.h"
#include "llvh/Support/SaveAndRestore.h"

#define DEBUG_TYPE "FlowChecker"

namespace hermes {
namespace flow {

FlowChecker::FlowChecker(
    Context &astContext,
    sema::SemContext &semContext,
    FlowContext &flowContext,
    sema::DeclCollectorMapTy &declCollectorMap,
    bool compile)
    : astContext_(astContext),
      sm_(astContext.getSourceErrorManager()),
      bufferMessages_(&sm_),
      semContext_(semContext),
      flowContext_(flowContext),
      declCollectorMap_(declCollectorMap),
      kw_(semContext.kw),
      declTypes_(flowContext.declTypeMap(FlowContext::ForUpdate())),
      compile_(compile) {}

bool FlowChecker::run(ESTree::ProgramNode *rootNode) {
  if (sm_.getErrorCount())
    return false;

  assert(
      astContext_.isStrictMode() && semContext_.getGlobalFunction()->strict &&
      "Types can only be used in strict mode");
  // Defensive programming.
  if (!semContext_.getGlobalFunction()->strict) {
    sm_.error(rootNode->getStartLoc(), "ft: strict mode required");
    return false;
  }

  FunctionContext globalFunc(*this, rootNode, nullptr, flowContext_.getAny());
  ScopeRAII scope(*this);
  declareNativeTypes(rootNode->getScope());
  if (!resolveScopeTypesAndAnnotate(rootNode, rootNode->getScope()))
    return false;
  visitESTreeNodeNoReplace(*this, rootNode);
  drainTypecheckQueue();
  return sm_.getErrorCount() == 0;
}

void FlowChecker::declareNativeTypes(sema::LexicalScope *rootScope) {
  Type *const number = flowContext_.getNumber();
  Type *const cptr = flowContext_.getCPtr();

  /// Declare a native type as an alias for \p type.
  auto declare = [this, rootScope](
                     llvh::StringRef name, NativeCType ctype, Type *type) {
    UniqueString *ident =
        astContext_.getIdentifier(name).getUnderlyingPointer();
    bindingTable_.try_emplace(ident, TypeDecl(type, rootScope, nullptr));
    nativeTypes_.try_emplace(ident, ctype);
  };

#define NATIVE_TYPE(name, str) \
  declare(llvh::StringLiteral("c_" #name), NativeCType::name, number);
#define NATIVE_PTR(name, str) \
  declare(llvh::StringLiteral("c_" #name), NativeCType::name, cptr);
#define NATIVE_CTYPE(name, str) \
  declare(llvh::StringLiteral("c_" #name), NativeCType::c_##name, number);
#include "hermes/AST/NativeTypes.def"
}

void FlowChecker::recursionDepthExceeded(ESTree::Node *n) {
  sm_.error(
      n->getEndLoc(),
      "ft: too many nested expressions/statements/declarations");
}

void FlowChecker::visit(ESTree::ProgramNode *node) {
  visitESTreeChildren(*this, node);
}

void FlowChecker::visitNonGenericFunctionDeclaration(
    ESTree::FunctionDeclarationNode *node,
    sema::Decl *decl) {
  Type *declType = getDeclType(decl);

  // If this declaration is of a global property, then it doesn't have a
  // function type, because that would be unsound. So, we have to parse the
  // function type here, to make it available inside the function.
  auto *ftype = llvh::dyn_cast<BaseFunctionType>(declType->info);
  if (!ftype) {
    declType = parseFunctionType(
        node->_params, node->_returnType, node->_async, node->_generator);
    ftype = llvh::cast<BaseFunctionType>(declType->info);
  }

  Type *thisType = nullptr;
  if (auto *typed = llvh::dyn_cast<TypedFunctionType>(ftype))
    thisType = typed->getThisParam();

  setNodeType(node, declType);
  FunctionContext functionContext(*this, node, declType, thisType);
  visitFunctionLike(node, node->_body, node->_params);
}

void FlowChecker::visit(ESTree::FunctionDeclarationNode *node) {
  sema::Decl *decl = getDecl(llvh::cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "function declaration must have been resolved");

  // If it has type parameters, it's either a generic declaration
  // or a specialization which should be typechecked directly at clone time
  // by calling visitNonGenericFunctionDeclaration.
  // e.g.
  //   function foo() { bar<number() }
  //   function bar<T>() {}
  // will create bar<number> in after bar<T>, but we don't want to visit after
  // visiting foo.
  if (node->_typeParameters) {
    return;
  }

  visitNonGenericFunctionDeclaration(node, decl);
}

void FlowChecker::visit(ESTree::FunctionExpressionNode *node) {
  if (node->_typeParameters) {
    sm_.error(
        node->_typeParameters->getStartLoc(),
        "ft: type parameters not supported on function expressions");
    return;
  }

  Type *ftype = parseFunctionType(
      node->_params, node->_returnType, node->_async, node->_generator);
  setNodeType(node, ftype);

  // If there is an id, resolve its type.
  if (auto id = llvh::cast_or_null<ESTree::IdentifierNode>(node->_id)) {
    sema::Decl *decl = getDecl(id);
    assert(decl && "function expression id must be resolved");
    assert(
        declTypes_.count(decl) == 0 &&
        "function expression id type already resolved");
    recordDecl(decl, ftype, id, node);
  }

  Type *thisType = nullptr;
  if (auto *typed = llvh::dyn_cast<TypedFunctionType>(ftype->info))
    thisType = typed->getThisParam();

  FunctionContext functionContext(*this, node, ftype, thisType);
  visitFunctionLike(node, node->_body, node->_params);
}

void FlowChecker::visit(ESTree::ArrowFunctionExpressionNode *node) {
  if (node->_typeParameters) {
    sm_.error(
        node->_typeParameters->getStartLoc(),
        "ft: type parameters not supported on function expressions");
    return;
  }

  Type *ftype = parseFunctionType(
      node->_params,
      node->_returnType,
      node->_async,
      false /*node->_generator*/);
  setNodeType(node, ftype);

  FunctionContext functionContext(
      *this, node, ftype, curFunctionContext_->thisParamType);
  visitFunctionLike(node, node->_body, node->_params);
}

class FlowChecker::ParseClassType {
  FlowChecker &outer_;

  llvh::SmallDenseMap<UniqueString *, ESTree::Node *> fieldNames{};
  llvh::SmallVector<ClassType::Field, 4> fields{};

  llvh::SmallDenseMap<UniqueString *, ESTree::Node *> methodNames{};
  llvh::SmallVector<ClassType::Field, 4> methods{};

  Type *constructorType = nullptr;
  Type *superClassType;

  size_t nextFieldLayoutSlotIR = 0;
  size_t nextMethodLayoutSlotIR = 0;

 public:
  ParseClassType(
      FlowChecker &outer,
      ESTree::Node *superClass,
      ESTree::Node *superTypeParameters,
      ESTree::Node *body,
      Type *classType)
      : outer_(outer),
        superClassType(
            resolveSuperClass(outer, superClass, superTypeParameters)) {
    LLVM_DEBUG(
        llvh::dbgs()
        << "ParseClassType for: "
        << llvh::cast<ClassType>(classType->info)->getClassNameOrDefault().str()
        << '\n');

    ClassType *superClassTypeInfo = nullptr;
    if (superClassType) {
      superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
      // Offset based on the superclass if necessary, to avoid overwriting
      // existing fields.
      assert(
          superClassTypeInfo->isInitialized() &&
          "superClass must be initialized");
      nextFieldLayoutSlotIR = superClassTypeInfo->getFieldNameMap().size();
      nextMethodLayoutSlotIR =
          superClassTypeInfo->getHomeObjectTypeInfo()->getFieldNameMap().size();
    }

    auto *classBody = llvh::cast<ESTree::ClassBodyNode>(body);
    for (ESTree::Node &node : classBody->_body) {
      if (auto *prop = llvh::dyn_cast<ESTree::ClassPropertyNode>(&node)) {
        Type *fieldType = parseClassProperty(prop);
        outer_.setNodeType(&node, fieldType);
      } else if (
          auto *method = llvh::dyn_cast<ESTree::MethodDefinitionNode>(&node)) {
        Type *methodType = parseMethodDefinition(classType, method);
        // Methods have FunctionExpression values.
        // Associate the same type with both the outer and inner nodes.
        outer_.setNodeType(method, methodType);
        outer_.setNodeType(method->_value, methodType);
      } else {
        outer_.sm_.error(
            node.getSourceRange(),
            "ft: unsupported class member " + node.getNodeName());
      }
    }

    Type *homeObjectType = outer_.flowContext_.createType(
        outer_.flowContext_.createClass(Identifier{}));
    llvh::cast<ClassType>(homeObjectType->info)
        ->init(
            methods,
            /* constructorType */ nullptr,
            /* homeObjectType */ nullptr,
            superClassTypeInfo ? superClassTypeInfo->getHomeObjectType()
                               : nullptr);
    llvh::cast<ClassType>(classType->info)
        ->init(fields, constructorType, homeObjectType, superClassType);
  }

 private:
  Type *resolveSuperClass(
      FlowChecker &outer_,
      ESTree::Node *superClass,
      ESTree::Node *superTypeParameters) {
    if (!superClass)
      return nullptr;
    auto *superClassIdent = llvh::dyn_cast<ESTree::IdentifierNode>(superClass);
    if (!superClassIdent) {
      outer_.sm_.error(
          superClass->getStartLoc(),
          "ft: only identifiers may be extended as superclasses");
      return nullptr;
    }

    sema::Decl *superDecl = outer_.getDecl(superClassIdent);
    if (superDecl->generic) {
      // Superclass type parameters aren't stored in _superClass,
      // but rather in a separate field in the class declaration itself,
      // so we need to handle it separately here.
      // Importantly, we can't defer the parsing of the superclass
      // because we need to be able to read the type of the superclass
      // to extend it.
      llvh::SaveAndRestore<std::vector<DeferredGenericClass> *>
          savedDeferredGenerics{outer_.deferredParseGenerics_, nullptr};
      auto *superTypeParams =
          llvh::cast_or_null<ESTree::TypeParameterInstantiationNode>(
              superTypeParameters);
      if (!superTypeParams) {
        outer_.sm_.error(
            superClassIdent->getSourceRange(),
            "ft: missing type arguments for superclass");
        return nullptr;
      }
      outer_.resolveGenericClassSpecialization(
          superClassIdent, superTypeParams, superDecl);
    } else if (superTypeParameters) {
      outer_.sm_.error(
          superTypeParameters->getStartLoc(),
          "ft: type arguments are not allowed for non-generic classes");
      return nullptr;
    }

    Type *superType = outer_.flowContext_.findNodeType(superClass);
    if (!superType) {
      outer_.sm_.error(superClass->getStartLoc(), "ft: super type unknown");
      return nullptr;
    }
    auto *superClassConsType =
        llvh::dyn_cast<ClassConstructorType>(superType->info);
    if (!superClassConsType) {
      outer_.sm_.error(
          superClass->getStartLoc(), "ft: super type must be a class");
      return nullptr;
    }
    return superClassConsType->getClassType();
  }

  Type *parseClassProperty(ESTree::ClassPropertyNode *prop) {
    if (prop->_computed || prop->_static || prop->_declare) {
      outer_.sm_.error(
          prop->getSourceRange(), "ft: unsupported property attributes");
      return outer_.flowContext_.getAny();
    }

    if (!llvh::isa<ESTree::IdentifierNode>(prop->_key)) {
      outer_.sm_.error(
          prop->getSourceRange(), "ft: property name must be an identifier");
      return outer_.flowContext_.getAny();
    }

    auto *id = llvh::cast<ESTree::IdentifierNode>(prop->_key);

    Type *fieldType;
    if (prop->_typeAnnotation) {
      fieldType = outer_.parseTypeAnnotation(
          llvh::cast<ESTree::TypeAnnotationNode>(prop->_typeAnnotation)
              ->_typeAnnotation);
    } else {
      fieldType = outer_.flowContext_.getAny();
    }

    // Check if the field is inherited, and reuse the index.
    const ClassType::Field *superField = nullptr;
    if (superClassType) {
      auto *superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
      auto superIt =
          superClassTypeInfo->findField(Identifier::getFromPointer(id->_name));
      if (superIt) {
        // Field is inherited.
        superField = superIt->getField();
        // Fields must be the same for class properties.
        if (!fieldType->info->equals(superField->type->info)) {
          outer_.sm_.error(
              prop->getStartLoc(), "ft: incompatible field type for override");
        }
      }
    }

    // Check if the field is already declared.
    auto [it, inserted] = fieldNames.try_emplace(id->_name, prop);
    if (!inserted) {
      outer_.sm_.error(
          id->getStartLoc(),
          "ft: field " + id->_name->str() + " already declared");
      outer_.sm_.note(
          it->second->getSourceRange(),
          "ft: previous declaration of " + id->_name->str());
      return outer_.flowContext_.getAny();
    }

    if (superField) {
      fields.emplace_back(
          Identifier::getFromPointer(id->_name),
          fieldType,
          superField->layoutSlotIR);
    } else {
      fields.emplace_back(
          Identifier::getFromPointer(id->_name),
          fieldType,
          nextFieldLayoutSlotIR++);
    }

    return fieldType;
  }

  Type *parseMethodDefinition(
      Type *classType,
      ESTree::MethodDefinitionNode *method) {
    auto *fe = llvh::cast<ESTree::FunctionExpressionNode>(method->_value);

    if (fe->_typeParameters) {
      outer_.sm_.error(
          fe->_typeParameters->getSourceRange(),
          "ft: type parameters not supported directly on methods");
      return outer_.flowContext_.getAny();
    }

    if (method->_kind == outer_.kw_.identConstructor) {
      // Constructor
      if (fe->_returnType) {
        outer_.sm_.error(
            fe->_returnType->getSourceRange(),
            "ft: constructor cannot declare a return type");
      }
      // Check for disallowed attributes. Note that the parser already
      // prevents some of these, but it doesn't hurt to check again.
      if (method->_static || fe->_async || fe->_generator) {
        outer_.sm_.error(
            method->getStartLoc(),
            "constructor cannot be static, a generator or async");
      }

      constructorType = outer_.parseFunctionType(
          fe->_params,
          nullptr,
          false,
          false,
          outer_.flowContext_.getVoid(),
          classType);

      return constructorType;
    } else {
      // Non-constructor method

      if (method->_computed) {
        outer_.sm_.error(
            method->getStartLoc(),
            "ft: computed property names in classes are unsupported");
        return outer_.flowContext_.getAny();
      }
      if (method->_static || fe->_async || fe->_generator) {
        outer_.sm_.error(
            method->getStartLoc(),
            "ft: static/async/generator methods unsupported");
        return outer_.flowContext_.getAny();
      }

      auto *id = llvh::cast<ESTree::IdentifierNode>(method->_key);
      Type *methodType = outer_.parseFunctionType(
          fe->_params,
          fe->_returnType,
          fe->_async,
          fe->_generator,
          nullptr,
          classType);

      // Check if the method is inherited, and reuse the index.
      const ClassType::Field *superMethod = nullptr;
      if (superClassType) {
        auto *superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
        auto superIt = superClassTypeInfo->getHomeObjectTypeInfo()->findField(
            Identifier::getFromPointer(id->_name));
        if (superIt) {
          // Field is inherited.
          superMethod = superIt->getField();
          // Mark the super field as overridden.
          superIt->getFieldMut()->overridden = true;
          // Actually check the type of the method later,
          // because we have to wait for the class to be initialized.
        }
      }

      // Check if the method is already declared.
      auto [it, inserted] = methodNames.try_emplace(id->_name, method);
      if (!inserted) {
        outer_.sm_.error(
            id->getStartLoc(),
            "ft: method " + id->_name->str() + " already declared");
        outer_.sm_.note(
            it->second->getSourceRange(),
            "ft: previous declaration of " + id->_name->str());
        return outer_.flowContext_.getAny();
      }

      if (superMethod) {
        methods.emplace_back(
            Identifier::getFromPointer(id->_name),
            methodType,
            superMethod->layoutSlotIR,
            method);
      } else {
        methods.emplace_back(
            Identifier::getFromPointer(id->_name),
            methodType,
            nextMethodLayoutSlotIR++,
            method);
      }

      return methodType;
    }
  }
};

void FlowChecker::parseClassType(
    ESTree::Node *superClass,
    ESTree::Node *superTypeParameters,
    ESTree::Node *body,
    Type *classType) {
  ParseClassType(*this, superClass, superTypeParameters, body, classType);
}

void FlowChecker::visitClassNode(
    ESTree::Node *classNode,
    ESTree::ClassBodyNode *body,
    Type *classType) {
  assert(
      llvh::cast<ClassType>(classType->info)->isInitialized() &&
      "trying to typecheck uninitialized class");
  ClassContext classContext(*this, classType);
  visitESTreeChildren(*this, body);
}

void FlowChecker::visit(ESTree::ClassExpressionNode *node) {
  visitExpression(node->_superClass, node, nullptr);

  auto *id = llvh::cast_or_null<ESTree::IdentifierNode>(node->_id);
  Type *classType = flowContext_.createType(flowContext_.createClass(
      id ? Identifier::getFromPointer(id->_name) : Identifier{}));

  unsigned errorsBefore = sm_.getErrorCount();
  parseClassType(
      node->_superClass, node->_superTypeParameters, node->_body, classType);
  if (sm_.getErrorCount() != errorsBefore) {
    // Failed to parse class.
    return;
  }

  Type *consType =
      flowContext_.createType(flowContext_.createClassConstructor(classType));

  setNodeType(node, consType);

  // A new scope for the class expression name.
  ScopeRAII scope(*this);
  // If there was a class id T, in the new scope declare class type T and
  // declaration for class constructor T.
  if (id) {
    bindingTable_.try_emplace(
        id->_name, TypeDecl(classType, node->getScope(), node));

    sema::Decl *decl = getDecl(id);
    assert(decl && "class expression id must be resolved");
    assert(
        declTypes_.count(decl) == 0 &&
        "class expression id type already resolved");
    recordDecl(decl, consType, id, node);
  }

  visitClassNode(
      node, llvh::cast<ESTree::ClassBodyNode>(node->_body), classType);
}

void FlowChecker::visit(ESTree::ClassDeclarationNode *node) {
  sema::Decl *decl = getDecl(llvh::cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "class declaration must have been resolved");

  // If it is generic, don't typecheck because typechecking will be done
  // directly by draining the typecheck queue.
  if (node->_typeParameters) {
    return;
  }

  auto *classType =
      llvh::cast<ClassConstructorType>(getDeclType(decl)->info)->getClassType();

  visitClassNode(
      node, llvh::cast<ESTree::ClassBodyNode>(node->_body), classType);
}

void FlowChecker::visit(ESTree::MethodDefinitionNode *node) {
  auto *fe = llvh::cast<ESTree::FunctionExpressionNode>(node->_value);

  if (fe->_typeParameters) {
    sm_.error(
        fe->_typeParameters->getSourceRange(),
        "ft: type parameters not supported directly on methods");
    return;
  }

  if (node->_kind == kw_.identConstructor) {
    FunctionContext functionContext(
        *this,
        fe,
        curClassContext_->getClassTypeInfo()->getConstructorType(),
        curClassContext_->classType);
    visitFunctionLike(fe, fe->_body, fe->_params);
  } else if (node->_key) {
    auto *id = llvh::cast<ESTree::IdentifierNode>(node->_key);
    // Cast must be valid because all methods were registered as
    // FunctionType.
    auto optField = curClassContext_->getClassTypeInfo()
                        ->getHomeObjectTypeInfo()
                        ->findField(Identifier::getFromPointer(id->_name));
    if (!optField.hasValue()) {
      // This only happens if the class failed to parse, avoid assertion
      // failures (defensive programming).
      sm_.error(
          id->getSourceRange(),
          llvh::Twine("ft: cannot find method: ") + id->_name->str());
      return;
    }
    Type *funcType = optField->getField()->type;

    // Typecheck overriding methods.
    Type *superClassType =
        curClassContext_->getClassTypeInfo()->getSuperClass();
    if (superClassType) {
      auto *superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
      auto superIt = superClassTypeInfo->getHomeObjectTypeInfo()->findField(
          Identifier::getFromPointer(id->_name));
      if (superIt) {
        auto *superMethod = superIt->getField();
        // Overriding method's function type must flow into the overridden
        // method's function type.
        bool canOverride = canAOverrideB(
            llvh::cast<BaseFunctionType>(funcType->info),
            llvh::cast<BaseFunctionType>(superMethod->type->info));
        if (!canOverride) {
          sm_.error(
              node->getStartLoc(), "ft: incompatible method type for override");
        }
      }
    }

    FunctionContext functionContext(
        *this, fe, funcType, curClassContext_->classType);
    visitFunctionLike(fe, fe->_body, fe->_params);
  }
}

void FlowChecker::visit(ESTree::TypeAnnotationNode *node) {
  // Do nothing.
}
void FlowChecker::visit(ESTree::IdentifierNode *identifierNode) {
  // Do nothing.
}

void FlowChecker::visit(ESTree::ExpressionStatementNode *node) {
  visitExpression(node->_expression, node, nullptr);
}

void FlowChecker::visit(ESTree::IfStatementNode *node) {
  visitExpression(node->_test, node, nullptr);
  visitESTreeNode(*this, node->_consequent, node);
  visitESTreeNode(*this, node->_alternate, node);
}
void FlowChecker::visit(ESTree::SwitchStatementNode *node) {
  ScopeRAII scope(*this);
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  visitExpression(node->_discriminant, node, nullptr);
  visitESTreeNodeList(*this, node->_cases, node);
}
void FlowChecker::visit(ESTree::SwitchCaseNode *node) {
  visitExpression(node->_test, node, nullptr);
  visitESTreeNodeList(*this, node->_consequent, node);
}
void FlowChecker::visit(ESTree::WhileStatementNode *node) {
  visitExpression(node->_test, node, nullptr);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::DoWhileStatementNode *node) {
  visitESTreeNode(*this, node->_body, node);
  visitExpression(node->_test, node, nullptr);
}
void FlowChecker::visit(ESTree::ForOfStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (llvh::isa<ESTree::VariableDeclarationNode>(node->_left))
    visitESTreeNode(*this, node->_left, node);
  else
    visitExpression(node->_left, node, nullptr);
  visitExpression(node->_right, node, nullptr);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::ForInStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (llvh::isa<ESTree::VariableDeclarationNode>(node->_left))
    visitESTreeNode(*this, node->_left, node);
  else
    visitExpression(node->_left, node, nullptr);
  visitExpression(node->_right, node, nullptr);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::ForStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (node->_init) {
    if (llvh::isa<ESTree::VariableDeclarationNode>(node->_init))
      visitESTreeNode(*this, node->_init, node);
    else
      visitExpression(node->_init, node, nullptr);
  }
  visitExpression(node->_test, node, nullptr);
  visitExpression(node->_update, node, nullptr);
  visitESTreeNode(*this, node->_body, node);
}

void FlowChecker::visit(ESTree::ReturnStatementNode *node) {
  auto *ftype = llvh::dyn_cast<TypedFunctionType>(
      curFunctionContext_->functionType->info);
  visitExpression(
      node->_argument, node, ftype ? ftype->getReturnType() : nullptr);

  // Untyped function, can't check the return type.
  if (!ftype) {
    return;
  }

  // Return without an argument and "void" return type is OK.
  if (!node->_argument && llvh::isa<VoidType>(ftype->getReturnType()->info))
    return;

  Type *argType = node->_argument ? getNodeTypeOrAny(node->_argument)
                                  : flowContext_.getVoid();

  auto [retTypeNarrow, cf] = tryNarrowType(argType, ftype->getReturnType());
  if (!cf.canFlow) {
    // TODO: pretty print types.
    sm_.error(
        node->getSourceRange(),
        "ft: return value incompatible with return type");
  }
  node->_argument = implicitCheckedCast(node->_argument, retTypeNarrow, cf);
}

void FlowChecker::visit(ESTree::BlockStatementNode *node) {
  ScopeRAII scope(*this);
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  visitESTreeChildren(*this, node);
}

void FlowChecker::visit(ESTree::VariableDeclarationNode *node) {
  for (ESTree::Node &n : node->_declarations) {
    auto *declarator = llvh::cast<ESTree::VariableDeclaratorNode>(&n);
    // Avoid visiting the initializer if it was already visited while inferring
    // the type of the declaration. Remove the entry from the set.
    auto it = visitedInits_.find(declarator->_init);
    if (it != visitedInits_.end()) {
      visitedInits_.erase(it);
    } else {
      Type *constraint = nullptr;
      if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
        sema::Decl *decl = getDecl(id);
        constraint = flowContext_.findDeclType(decl);
      }
      visitExpression(declarator->_init, declarator, constraint);
    }
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
      if (!declarator->_init)
        continue;

      sema::Decl *decl = getDecl(id);
      Type *lt = getDeclType(decl);
      Type *rt = getNodeTypeOrAny(declarator->_init);
      auto [rtNarrow, cf] = tryNarrowType(rt, lt);
      if (!cf.canFlow) {
        sm_.error(
            declarator->getSourceRange(),
            "ft: incompatible initialization type");
      } else {
        declarator->_init =
            implicitCheckedCast(declarator->_init, rtNarrow, cf);
      }
    } else {
      Type *lt = getNodeTypeOrAny(declarator->_id);
      Type *rt = getNodeTypeOrAny(declarator->_init);

      CanFlowResult cf = canAFlowIntoB(rt, lt);
      if (!cf.canFlow) {
        sm_.error(
            declarator->getSourceRange(),
            "ft: incompatible initialization type");
      } else {
        declarator->_init = implicitCheckedCast(declarator->_init, lt, cf);
      }
    }
  }
}

void FlowChecker::visit(ESTree::ClassPropertyNode *node) {
  if (node->_value) {
    visitExpression(node->_value, node, nullptr);
  }
}

void FlowChecker::visit(ESTree::CatchClauseNode *node) {
  ScopeRAII scope(*this);
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  visitESTreeNode(*this, node->_param, node);
  // Process body's declarations, skip visiting it, visit its children.
  visitESTreeChildren(*this, node->_body);
}

void FlowChecker::visitFunctionLike(
    ESTree::FunctionLikeNode *node,
    ESTree::Node *body,
    ESTree::NodeList &params) {
  assert(node->getSemInfo()->strict && "Types can only be used in strict mode");
  ScopeRAII scope(*this);

  for (auto &param : params) {
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(&param)) {
      sema::Decl *decl = getDecl(id);
      assert(decl && "unresolved parameter");
      declTypes_.try_emplace(
          decl, parseOptionalTypeAnnotation(id->_typeAnnotation));
    }
  }

  if (!resolveScopeTypesAndAnnotate(
          node, node->getSemInfo()->getFunctionScope()))
    return;
  visitESTreeNode(*this, body, node);
  checkImplicitReturnType(node);
}

void FlowChecker::checkImplicitReturnType(ESTree::FunctionLikeNode *node) {
  // Function body isn't known to terminate.
  // Typecheck the implicit 'return;' at the end of the block.
  auto *ftype = llvh::dyn_cast<TypedFunctionType>(
      curFunctionContext_->functionType->info);

  // Untyped function, can't check the return type.
  if (!ftype) {
    return;
  }

  // No implicit return, no need to check anything.
  if (!node->getSemInfo()->mayReachImplicitReturn) {
    return;
  }

  // Check that the implicit "undefined" (void type) can flow into the return
  // type.
  CanFlowResult cf =
      canAFlowIntoB(flowContext_.getVoid(), ftype->getReturnType());
  if (!cf.canFlow) {
    sm_.error(
        ESTree::getReturnType(node)->getSourceRange(),
        "ft: implicitly-returned 'undefined' incompatible with return type");
  }
}

/// AnnotateScopeDecls needs to not report errors when visiting a closure
/// to infer a variable declaration's type:
/// \code
///   let func = () => { let a = b; }
///   let b = 10;
/// \endcode
///
/// So we do the following:
/// 1. Iterate variable declarations with annotations/literals to record types,
///   but do not visit their initializers.
/// 2. Iterate all declarations and visit variable initializers to do inference,
///   descending into functions along the way.
///
/// While visiting an identifier in step 2, if the variable has no type yet,
/// it means that it has no type annotation AND its type hasn't been inferred.
/// Assume "any", because:
/// * If it's 'var': then it's untyped and may be init'ed later, assume "any".
/// * If it's 'let':
///   * If we're in the same function, it's declared late, assume "any",
///     and IRGen will report a TDZ error early when possible.
///   * If we're in a different function the variable could be initialized in
///     the other function, assume "any".
class FlowChecker::AnnotateScopeDecls {
  FlowChecker &outer;

 public:
  AnnotateScopeDecls(
      FlowChecker &outer,
      const sema::ScopeDecls &decls,
      ESTree::Node *scopeNode)
      : outer(outer) {
    setTypesForAnnotatedVariables(decls);
    annotateAllScopeDecls(decls, scopeNode);
  }

 private:
  /// Step 1 above.
  /// Iterate all variable declarations with type annotations or literal
  /// initializers and register them.
  void setTypesForAnnotatedVariables(const sema::ScopeDecls &decls) {
    for (ESTree::Node *declNode : decls) {
      if (auto *declaration =
              llvh::dyn_cast<ESTree::VariableDeclarationNode>(declNode)) {
        for (ESTree::Node &n : declaration->_declarations) {
          auto *declarator = llvh::cast<ESTree::VariableDeclaratorNode>(&n);
          if (auto *id =
                  llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
            sema::Decl *decl = outer.getDecl(id);
            // Global properties don't have sound types, let the
            // annotateVariableDeclaration call handle it later.
            if (sema::Decl::isKindGlobal(decl->kind)) {
              continue;
            }
            if (id->_typeAnnotation) {
              // Found a type annotation on a local variable declaration.
              Type *type = outer.parseTypeAnnotation(
                  llvh::cast<ESTree::TypeAnnotationNode>(id->_typeAnnotation)
                      ->_typeAnnotation);
              outer.recordDecl(decl, type, id, declarator);
              continue;
            }
            // Check for literal initializers.
            if (declarator->_init &&
                (llvh::isa<ESTree::NullLiteralNode>(declarator->_init) ||
                 llvh::isa<ESTree::NumericLiteralNode>(declarator->_init) ||
                 llvh::isa<ESTree::BooleanLiteralNode>(declarator->_init) ||
                 llvh::isa<ESTree::StringLiteralNode>(declarator->_init) ||
                 llvh::isa<ESTree::RegExpLiteralNode>(declarator->_init) ||
                 llvh::isa<ESTree::BigIntLiteralNode>(declarator->_init))) {
              outer.visitExpression(declarator->_init, declarator, nullptr);
              outer.recordDecl(
                  outer.getDecl(id),
                  outer.getNodeTypeOrAny(declarator->_init),
                  id,
                  declarator);
              continue;
            }
          } else if (
              auto *arr =
                  llvh::dyn_cast<ESTree::ArrayPatternNode>(declarator->_id)) {
            if (arr->_typeAnnotation) {
              // Found a type annotation on a local variable declaration that
              // destructures into an array pattern.
              Type *type = outer.parseTypeAnnotation(
                  llvh::cast<ESTree::TypeAnnotationNode>(arr->_typeAnnotation)
                      ->_typeAnnotation);
              annotateDestructuringTarget(declarator, arr, type);
              continue;
            }
          }
        }
      }
    }
  }

  /// Step 2 above.
  /// Visit all declarations and attempt to infer variable types.
  void annotateAllScopeDecls(
      const sema::ScopeDecls &decls,
      ESTree::Node *scopeNode) {
    for (ESTree::Node *declNode : decls) {
      if (auto *declaration =
              llvh::dyn_cast<ESTree::VariableDeclarationNode>(declNode)) {
        // VariableDeclaration.
        //
        annotateVariableDeclaration(declaration);
      } else if (
          auto *funcDecl =
              llvh::dyn_cast<ESTree::FunctionDeclarationNode>(declNode)) {
        // FunctionDeclaration.
        //
        ESTree::Node *parent = nullptr;
        if (auto *program = llvh::dyn_cast<ESTree::ProgramNode>(scopeNode)) {
          parent = program;
        } else if (
            auto *func = llvh::dyn_cast<ESTree::FunctionLikeNode>(scopeNode)) {
          parent = ESTree::getBlockStatement(func);
        } else {
          parent = scopeNode;
          assert(
              llvh::isa<ESTree::BlockStatementNode>(parent) ||
              llvh::isa<ESTree::SwitchStatementNode>(parent));
        }
        annotateFunctionDeclaration(funcDecl, parent);
      } else if (
          auto *id = llvh::dyn_cast<ESTree::ImportDeclarationNode>(declNode)) {
        // ImportDeclaration.
        //
        outer.sm_.error(id->getStartLoc(), "ft: import not implemented yet");
      } else if (
          auto *catchClause =
              llvh::dyn_cast<ESTree::CatchClauseNode>(declNode)) {
        // CatchClause.
        //
        annotateCatchClause(catchClause);
      } else {
        // All the rest.
        //
        continue;
      }
    }
  }

  void annotateVariableDeclaration(
      ESTree::VariableDeclarationNode *declaration) {
    /// Attempt to infer the RHS of the declarator by calling the typecheck
    /// visitor on it to see if it's able to associate a type with the init
    /// node.
    /// \return the inferred type, or nullptr if no inference was possible.
    auto tryInferInitExpression =
        [this](ESTree::VariableDeclaratorNode *declarator) -> Type * {
      outer.visitExpression(declarator->_init, declarator, nullptr);
      outer.visitedInits_.insert(declarator->_init);
      if (Type *inferred = outer.flowContext_.findNodeType(declarator->_init)) {
        return inferred;
      }
      return nullptr;
    };

    for (ESTree::Node &n : declaration->_declarations) {
      auto *declarator = llvh::cast<ESTree::VariableDeclaratorNode>(&n);
      if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
        sema::Decl *decl = outer.getDecl(id);
        if (outer.flowContext_.findDeclType(decl)) {
          // This decl was already handled in setTypesForAnnotatedVariables.
          continue;
        }
        Type *type = outer.parseOptionalTypeAnnotation(id->_typeAnnotation);

        // Global properties don't have sound types, since they can be
        // overwritten without our knowledge and control.
        if (decl->kind == sema::Decl::Kind::GlobalProperty) {
          type = outer.flowContext_.getAny();
          if (id->_typeAnnotation) {
            outer.sm_.warning(
                id->_typeAnnotation->getSourceRange(),
                "ft: global property type annotations are unsound and are ignored");
          }
        } else if (!id->_typeAnnotation && declarator->_init) {
          if (Type *inferred = tryInferInitExpression(declarator))
            type = inferred;
        }

        outer.recordDecl(decl, type, id, declarator);
      } else if (
          auto *arr =
              llvh::dyn_cast<ESTree::ArrayPatternNode>(declarator->_id)) {
        if (outer.flowContext_.findNodeType(arr)) {
          // This array pattern was already handled in
          // setTypesForAnnotatedVariables.
          continue;
        }
        Type *type = outer.flowContext_.getAny();
        if (declarator->_init) {
          // It's possible to not have an _init if this declarator is part of a
          // for loop:
          // for ([x, y] of iterable) {}
          if (Type *inferred = tryInferInitExpression(declarator))
            type = inferred;
        }
        annotateDestructuringTarget(declarator, arr, type);
      } else {
        outer.sm_.warning(
            declarator->_id->getSourceRange(),
            "ft: typing of object declarators not implemented, :any assumed");
      }
    }
  }

  /// Annotate the elements of the destructuring pattern given that the pattern
  /// itself has type \p type.
  /// Associates the target node and nested patterns with their respective
  /// types, and records the types of decls that were declared by the pattern.
  void annotateDestructuringTarget(
      ESTree::VariableDeclaratorNode *declarator,
      ESTree::PatternNode *pattern,
      Type *patternType) {
    // Use a worklist to avoid recursion.
    llvh::SmallVector<std::pair<ESTree::Node *, Type *>, 4> worklist{};
    worklist.emplace_back(pattern, patternType);

    while (!worklist.empty()) {
      auto [node, t] = worklist.pop_back_val();
      if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(node)) {
        // If this is an identifier, then we just record its type.
        if (id->_typeAnnotation) {
          outer.setNodeType(id, outer.flowContext_.getAny());
          outer.sm_.error(
              node->getSourceRange(),
              "ft: type annotations not supported inside destructuring, "
              "annotate the whole pattern instead");
          continue;
        }
        sema::Decl *decl = outer.getDecl(id);
        outer.setNodeType(id, t);
        outer.recordDecl(decl, t, id, declarator);
      } else if (auto *arr = llvh::dyn_cast<ESTree::ArrayPatternNode>(node)) {
        // If we have an array pattern, then we need to visit each element and
        // annotate them accordingly.
        if (auto *tuple = llvh::dyn_cast<TupleType>(t->info)) {
          // Setting the type to the tuple allows IRGen to conveniently
          // query the kind of destructuring to run by checking the annotated
          // type on the array pattern.
          // It also allows us to avoid rerunning annotation in subsequent
          // phases of AnnotateScopeDecls.
          outer.setNodeType(arr, t);
          size_t i = 0;
          // Whether we had to stop early due to not enough tuple elements.
          bool tooFewTupleElements = false;
          for (ESTree::Node &element : arr->_elements) {
            if (i >= tuple->getTypes().size()) {
              tooFewTupleElements = true;
              break;
            }
            worklist.emplace_back(&element, tuple->getTypes()[i]);
            ++i;
          }
          if (tooFewTupleElements || i != tuple->getTypes().size()) {
            outer.sm_.error(
                pattern->getSourceRange(),
                llvh::Twine("ft: cannot destructure tuple, expected ") +
                    llvh::Twine(tuple->getTypes().size()) +
                    " elements, found " + llvh::Twine(arr->_elements.size()));
            return;
          }
        } else if (llvh::isa<AnyType>(t->info)) {
          outer.setNodeType(arr, outer.flowContext_.getAny());
          // Propagate the 'any' type to all children.
          // Records that we've seen the declaration for every variable
          // declared in this pattern, so the IdentifierNode visitor knows not
          // to emit a warning during typechecking for use before declaration.
          // The iterator protocol will be used to populate the elements when
          // the code is generated.
          for (ESTree::Node &element : arr->_elements) {
            worklist.emplace_back(&element, t);
          }
          continue;
        } else {
          outer.setNodeType(arr, outer.flowContext_.getAny());
          outer.sm_.error(
              arr->getSourceRange(),
              "ft: incompatible type for array pattern, expected tuple");
          continue;
        }
      } else if (llvh::isa<ESTree::ObjectPatternNode>(node)) {
        outer.setNodeType(arr, outer.flowContext_.getAny());
        outer.sm_.warning(
            node->getSourceRange(),
            "ft: typing of object declarators not implemented, :any assumed");
        continue;
      }
    }

    return;
  }

  void annotateFunctionDeclaration(
      ESTree::FunctionDeclarationNode *funcDecl,
      ESTree::Node *parent) {
    auto *id = llvh::cast<ESTree::IdentifierNode>(funcDecl->_id);
    sema::Decl *decl = outer.getDecl(id);
    if (funcDecl->_typeParameters) {
      decl->generic = true;
      outer.registerGeneric(
          decl, funcDecl, parent, outer.bindingTable_.getCurrentScope());
      return;
    }

    Type *type = outer.parseFunctionType(
        funcDecl->_params,
        funcDecl->_returnType,
        funcDecl->_async,
        funcDecl->_generator);

    // Global properties don't have sound types, since they can be
    // overwritten without our knowledge and control.
    if (decl->kind == sema::Decl::Kind::GlobalProperty) {
      if (llvh::isa<TypedFunctionType>(type->info)) {
        outer.sm_.warning(
            funcDecl->getStartLoc(),
            "ft: global property type annotations are unsound and are ignored");
      }
      type = outer.flowContext_.getAny();
    }

    outer.recordDecl(decl, type, id, funcDecl);
  }

  /// Annotate the type of the catch parameter if it exists.
  void annotateCatchClause(ESTree::CatchClauseNode *catchClause) {
    if (!catchClause->_param)
      return;

    if (auto *id =
            llvh::dyn_cast<ESTree::IdentifierNode>(catchClause->_param)) {
      sema::Decl *decl = outer.getDecl(id);
      Type *type = outer.parseOptionalTypeAnnotation(id->_typeAnnotation);
      if (!type)
        type = outer.flowContext_.getAny();

      if (!llvh::isa<AnyType>(type->info) &&
          !llvh::isa<MixedType>(type->info)) {
        outer.sm_.error(
            catchClause->_param->getSourceRange(),
            "ft: catch parameter must be 'any' or 'mixed'");
      }

      outer.recordDecl(decl, type, id, catchClause);
    } else {
      outer.sm_.warning(
          catchClause->_param->getSourceRange(),
          "ft: typing of pattern declarators not implemented, :any assumed");
    }
  }
};

bool FlowChecker::resolveScopeTypesAndAnnotate(
    ESTree::Node *scopeNode,
    sema::LexicalScope *scope) {
  const sema::ScopeDecls *decls =
      curFunctionContext_->declCollector->getScopeDeclsForNode(scopeNode);
  if (!decls || decls->empty())
    return true;

  assert(scope && "declarations found but no lexical scope");
  unsigned errorsBefore = sm_.getErrorCount();

  declareScopeTypes(*decls, scope, scopeNode);
  AnnotateScopeDecls(*this, *decls, scopeNode);

  // If there were any errors during annotation, fail.
  // Don't count errors that happened during typechecking other nodes.
  return sm_.getErrorCount() == errorsBefore;
}

void FlowChecker::drainTypecheckQueue() {
  auto savedScope = bindingTable_.getCurrentScope();

  while (!typecheckQueue_.empty()) {
    DeferredGenericClass deferred{std::move(typecheckQueue_.front())};
    typecheckQueue_.pop_front();
    ESTree::ClassDeclarationNode *specialization = deferred.specialization;
    LLVM_DEBUG(
        llvh::dbgs()
        << "Typechecking deferred generic class: "
        << llvh::cast<ESTree::IdentifierNode>(specialization->_id)->_name->str()
        << "\n");
    bindingTable_.activateScope(deferred.scope);
    visitClassNode(
        deferred.specialization,
        llvh::cast<ESTree::ClassBodyNode>(specialization->_body),
        deferred.classType);
  }

  bindingTable_.activateScope(savedScope);
}

/// Record the declaration's type and declaring AST node, while checking for
/// and reporting re-declarations.
bool FlowChecker::recordDecl(
    sema::Decl *decl,
    Type *type,
    ESTree::IdentifierNode *id,
    ESTree::Node *astDeclNode) {
  assert(decl && "unresolved identifier");
  auto [it, inserted] = declNodes_.try_emplace(decl, astDeclNode);
  if (inserted) {
    declTypes_.try_emplace(decl, type);
    return true;
  } else {
    sm_.error(id->getStartLoc(), "ft: redeclaration of " + id->_name->str());
    sm_.note(
        it->second->getSourceRange(),
        "ft: first declaration of " + id->_name->str());
    return false;
  }
};

Type *FlowChecker::parseFunctionType(
    ESTree::NodeList &params,
    ESTree::Node *optReturnTypeAnnotation,
    bool isAsync,
    bool isGenerator,
    Type *defaultReturnType,
    Type *defaultThisType) {
  llvh::SmallVector<TypedFunctionType::Param, 4> paramsList{};

  // If the default return type is expected, then we are parsing a typed
  // function, even if it doesn't have any explicit type annotations.
  bool isTyped = (defaultReturnType != nullptr);

  for (ESTree::Node &n : params) {
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(&n)) {
      paramsList.emplace_back(
          Identifier::getFromPointer(id->_name),
          parseOptionalTypeAnnotation(id->_typeAnnotation));
      isTyped |= (id->_typeAnnotation != nullptr);
    } else {
      sm_.warning(
          n.getSourceRange(),
          "ft: typing of pattern parameters not implemented, :any assumed");
      paramsList.emplace_back(Identifier(), flowContext_.getAny());
    }
  }

  Type *returnType =
      parseOptionalTypeAnnotation(optReturnTypeAnnotation, defaultReturnType);
  isTyped |= (optReturnTypeAnnotation != nullptr);

  Type *thisParamType = defaultThisType;
  llvh::ArrayRef<TypedFunctionType::Param> paramsRef(paramsList);

  // Check if the first parameter is "this", since it is treated specially.
  if (!paramsRef.empty() &&
      paramsRef.front().first.getUnderlyingPointer() == kw_.identThis) {
    // User is allowed to specify a "this" on a method where it's already known,
    // but it must be the same type as the given type.
    if (thisParamType &&
        !thisParamType->info->equals(paramsRef.front().second->info)) {
      sm_.error(
          paramsRef.front().second->node->getSourceRange(),
          "ft: incompatible 'this' type annotation");
    }

    thisParamType = paramsRef.front().second;
    paramsRef = paramsRef.drop_front();
  }

  BaseFunctionType *res = isTyped
      ? static_cast<BaseFunctionType *>(flowContext_.createFunction(
            returnType, thisParamType, paramsRef, isAsync, isGenerator))
      : flowContext_.createUntypedFunction(isAsync, isGenerator);
  return flowContext_.createType(res);
}

Type *FlowChecker::parseOptionalTypeAnnotation(
    ESTree::Node *optAnnotation,
    Type *defaultType) {
  if (!optAnnotation)
    return defaultType ? defaultType : flowContext_.getAny();
  return parseTypeAnnotation(
      llvh::cast<ESTree::TypeAnnotationNode>(optAnnotation)->_typeAnnotation);
}

Type *FlowChecker::parseTypeAnnotation(ESTree::Node *node) {
  assert(
      node &&
      "parseTypeAnnotation requires an annotation, "
      "use parseOptionalTypeAnnotation otherwise");

  switch (node->getKind()) {
    case ESTree::NodeKind::VoidTypeAnnotation:
      return flowContext_.getVoid();
    case ESTree::NodeKind::NullLiteralTypeAnnotation:
      return flowContext_.getNull();
    case ESTree::NodeKind::BooleanTypeAnnotation:
      return flowContext_.getBoolean();
    case ESTree::NodeKind::StringTypeAnnotation:
      return flowContext_.getString();
    case ESTree::NodeKind::NumberTypeAnnotation:
      return flowContext_.getNumber();
    case ESTree::NodeKind::BigIntTypeAnnotation:
      return flowContext_.getBigInt();
    case ESTree::NodeKind::AnyTypeAnnotation:
      return flowContext_.getAny();
    case ESTree::NodeKind::MixedTypeAnnotation:
      return flowContext_.getMixed();
    case ESTree::NodeKind::UnionTypeAnnotation:
      return parseUnionTypeAnnotation(
          llvh::cast<ESTree::UnionTypeAnnotationNode>(node));
    case ESTree::NodeKind::NullableTypeAnnotation:
      return parseNullableTypeAnnotation(
          llvh::cast<ESTree::NullableTypeAnnotationNode>(node));
    // TODO: function, etc.
    case ESTree::NodeKind::ArrayTypeAnnotation:
      return parseArrayTypeAnnotation(
          llvh::cast<ESTree::ArrayTypeAnnotationNode>(node));
    case ESTree::NodeKind::TupleTypeAnnotation:
      return parseTupleTypeAnnotation(
          llvh::cast<ESTree::TupleTypeAnnotationNode>(node));
    case ESTree::NodeKind::ObjectTypeAnnotation:
      return parseObjectTypeAnnotation(
          llvh::cast<ESTree::ObjectTypeAnnotationNode>(node));
    case ESTree::NodeKind::GenericTypeAnnotation:
      return parseGenericTypeAnnotation(
          llvh::cast<ESTree::GenericTypeAnnotationNode>(node));
    case ESTree::NodeKind::FunctionTypeAnnotation:
      return parseFunctionTypeAnnotation(
          llvh::cast<ESTree::FunctionTypeAnnotationNode>(node));

    default:
      sm_.error(
          node->getSourceRange(),
          "ft: unimplemented type annotation " + node->getNodeName());
      return flowContext_.getAny();
  }
}

Type *FlowChecker::parseUnionTypeAnnotation(
    ESTree::UnionTypeAnnotationNode *node) {
  llvh::SmallVector<Type *, 4> types{};
  for (auto &n : node->_types)
    types.push_back(parseTypeAnnotation(&n));
  return flowContext_.createType(flowContext_.maybeCreateUnion(types), node);
}

Type *FlowChecker::parseNullableTypeAnnotation(
    ESTree::NullableTypeAnnotationNode *node) {
  return flowContext_.createType(
      flowContext_.createPopulatedNullable(
          parseTypeAnnotation(node->_typeAnnotation)),
      node);
}

Type *FlowChecker::parseArrayTypeAnnotation(
    ESTree::ArrayTypeAnnotationNode *node) {
  return flowContext_.createType(
      flowContext_.createArray(parseTypeAnnotation(node->_elementType)), node);
}

Type *FlowChecker::parseTupleTypeAnnotation(
    ESTree::TupleTypeAnnotationNode *node) {
  return processTupleTypeAnnotation(
      node, [this](ESTree::Node *annotation) -> Type * {
        return parseTypeAnnotation(annotation);
      });
}

Type *FlowChecker::parseObjectTypeAnnotation(
    ESTree::ObjectTypeAnnotationNode *node) {
  return processObjectTypeAnnotation(
      node, [this](ESTree::Node *annotation) -> Type * {
        return parseTypeAnnotation(annotation);
      });
}

Type *FlowChecker::parseGenericTypeAnnotation(
    ESTree::GenericTypeAnnotationNode *node) {
  auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(node->_id);

  if (!id) {
    sm_.error(node->getSourceRange(), "ft: unsupported type annotation");
    return flowContext_.getAny();
  }

  TypeDecl *td = bindingTable_.find(id->_name);

  if (!td) {
    sm_.error(id->getSourceRange(), "ft: undefined type " + id->_name->str());
    return flowContext_.getAny();
  }

  if (!td->type) {
    if (!node->_typeParameters) {
      sm_.error(
          node->getSourceRange(),
          llvh::Twine("ft: missing generic arguments for '") +
              id->_name->str() + "'");
      return flowContext_.getAny();
    }
    if (td->genericClassDecl) {
      // Resolve to the specialized class.
      return resolveGenericClassSpecializationForType(
          node, td->genericClassDecl);
    } else {
      // This is a generic type alias.
      return resolveGenericTypeAlias(node, td);
    }
  }

  return td->type;
}

Type *FlowChecker::parseFunctionTypeAnnotation(
    ESTree::FunctionTypeAnnotationNode *node) {
  return processFunctionTypeAnnotation(
      node, [this](ESTree::Node *annotation) -> Type * {
        return parseTypeAnnotation(annotation);
      });
}

FlowChecker::CanFlowResult FlowChecker::canAFlowIntoB(
    TypeInfo *a,
    TypeInfo *b) {
  if (a == b)
    return {.canFlow = true};

  // _ -> any
  // _ -> mixed
  if (llvh::isa<AnyType>(b) || llvh::isa<MixedType>(b))
    return {.canFlow = true};
  // any -> _
  if (llvh::isa<AnyType>(a))
    return {.canFlow = true, .needCheckedCast = true};

  // if `a` is union, all of its arms must be able to flow into `b`.
  if (UnionType *unionA = llvh::dyn_cast<UnionType>(a)) {
    bool needCheckedCast = false;
    for (auto *aType : unionA->getTypes()) {
      CanFlowResult tmp = canAFlowIntoB(aType->info, b);
      if (!tmp.canFlow)
        return tmp;
      needCheckedCast |= tmp.needCheckedCast;
    }
    return {.canFlow = true, .needCheckedCast = needCheckedCast};
  }

  // if `b` is union, `a` must be able to flow into at least one arm.
  if (UnionType *unionB = llvh::dyn_cast<UnionType>(b)) {
    // Shortcut the check if `b` has "all-accepting" arms.
    if (unionB->hasAny() || unionB->hasMixed())
      return {.canFlow = true};

    // Check if `a` can flow into at least one of b's arms.
    // Note that we know that `a` is not `any`, so there is no need for a
    // checked cast.
    for (auto *bType : unionB->getTypes())
      if (canAFlowIntoB(a, bType->info).canFlow)
        return {.canFlow = true};

    return {};
  }

  // Arrays are invariant, so if `a` is an array, `b` must be an array with the
  // same element type.
  if (auto *arrayA = llvh::dyn_cast<ArrayType>(a)) {
    auto *arrayB = llvh::dyn_cast<ArrayType>(b);
    if (!arrayB)
      return {};
    assert(
        arrayA->getElement()->info && arrayB->getElement()->info &&
        "uninitialized elements");
    if (arrayA->getElement()->info->compare(arrayB->getElement()->info) == 0)
      return {.canFlow = true};
    return {};
  }

  // Tuples are invariant, so if `a` is an tuple, `b` must be an tuple with the
  // same types.
  if (auto *tupleA = llvh::dyn_cast<TupleType>(a)) {
    auto *tupleB = llvh::dyn_cast<TupleType>(b);
    if (!tupleB)
      return {};
    return canAFlowIntoB(tupleA, tupleB);
  }

  // Objects are invariant, so if `a` is an object, `b` must be an object with
  // the same types.
  if (auto *objectA = llvh::dyn_cast<ExactObjectType>(a)) {
    auto *objectB = llvh::dyn_cast<ExactObjectType>(b);
    if (!objectB)
      return {};
    return canAFlowIntoB(objectA, objectB);
  }

  if (ClassType *classA = llvh::dyn_cast<ClassType>(a)) {
    ClassType *classB = llvh::dyn_cast<ClassType>(b);
    if (!classB)
      return {};
    return canAFlowIntoB(classA, classB);
  }

  if (BaseFunctionType *funcA = llvh::dyn_cast<BaseFunctionType>(a)) {
    BaseFunctionType *funcB = llvh::dyn_cast<BaseFunctionType>(b);
    if (!funcB)
      return {};
    return canAFlowIntoB(funcA, funcB);
  }

  return {};
}

FlowChecker::CanFlowResult FlowChecker::canAFlowIntoB(
    ClassType *a,
    ClassType *b) {
  // It can flow into any superclass.
  ClassType *cur = a;
  while (cur) {
    // `b` is in the inheritance chain of `a`.
    if (cur == b)
      return {.canFlow = true};
    cur = cur->getSuperClassInfo();
  }
  return {};
}

FlowChecker::CanFlowResult FlowChecker::canAFlowIntoB(
    TupleType *a,
    TupleType *b) {
  auto aTypes = a->getTypes();
  auto bTypes = b->getTypes();
  if (aTypes.size() != bTypes.size()) {
    return {};
  }

  for (size_t i = 0, e = aTypes.size(); i < e; ++i) {
    // TODO: This will be more complex when we allow variance in tuple types.
    if (!aTypes[i]->info->equals(bTypes[i]->info)) {
      return {};
    }
  }

  return {.canFlow = true};
}

FlowChecker::CanFlowResult FlowChecker::canAFlowIntoB(
    ExactObjectType *a,
    ExactObjectType *b) {
  auto aFields = a->getFields();
  auto bFields = b->getFields();
  if (aFields.size() != bFields.size()) {
    return {};
  }

  for (size_t i = 0, e = aFields.size(); i < e; ++i) {
    // TODO: This will be more complex when we allow variance in object types.
    if (aFields[i].name != bFields[i].name)
      return {};
    if (!aFields[i].type->info->equals(bFields[i].type->info))
      return {};
  }

  return {.canFlow = true};
}

FlowChecker::CanFlowResult FlowChecker::canAFlowIntoB(
    BaseFunctionType *a,
    BaseFunctionType *b,
    ThisFlowDirection thisFlow) {
  // Function a can flow into b when:
  // * they're the same kind of function (async, generator, etc)
  // * all parameters of b can flow into parameters of a
  // * return type of a can flow into return type of b
  // * no checked casts are needed (can't flow into an 'any' parameter)

  if (a->isAsync() != b->isAsync())
    return {};
  if (a->isGenerator() != b->isGenerator())
    return {};

  // Typed, untyped and native functions are incompatible types.
  if (a->getKind() != b->getKind())
    return {};

  if (llvh::isa<UntypedFunctionType>(a)) {
    assert(
        llvh::isa<UntypedFunctionType>(b) &&
        "we already checked that the kinds are the same!");
    // Untyped functions can flow into each other.
    return {.canFlow = true};
  }

  if (llvh::isa<NativeFunctionType>(a)) {
    auto *nativeA = llvh::cast<NativeFunctionType>(a);
    auto *nativeB = llvh::cast<NativeFunctionType>(b);
    // Native functions can flow into each other only if their signatures are
    // the same.
    if (nativeA->getSignature() != nativeB->getSignature())
      return {};
    return {.canFlow = true};
  }

  auto *aType = llvh::cast<TypedFunctionType>(a);
  auto *bType = llvh::cast<TypedFunctionType>(b);

  if (!aType->getThisParam() != !bType->getThisParam()) {
    // Only one of the functions is missing `this`, can't flow.
    return {};
  }
  if (aType->getThisParam() && bType->getThisParam()) {
    // Both functions have `this`, it must be checked.
    CanFlowResult flowRes = thisFlow == ThisFlowDirection::Default
        ? canAFlowIntoB(bType->getThisParam(), aType->getThisParam())
        : canAFlowIntoB(aType->getThisParam(), bType->getThisParam());
    if (!flowRes.canFlow || flowRes.needCheckedCast)
      return {};
  }

  {
    // TODO: Handle default arguments, which will allow for a changing number of
    // parameters. Example:
    //   let funcB : (a: number) => number;
    //   function funcA(a: number, bType: number = 10) : number {
    //     return 0;
    //   }
    //   funcB = funcA;
    // funcA flows into funcA here because the default argument allows the
    // caller to change the number of parameters.
    if (aType->getParams().size() != bType->getParams().size())
      return {};

    for (size_t i = 0, e = aType->getParams().size(); i < e; ++i) {
      Type *paramA = aType->getParams()[i].second;
      Type *paramB = bType->getParams()[i].second;
      CanFlowResult flowRes = canAFlowIntoB(paramB, paramA);
      if (!flowRes.canFlow || flowRes.needCheckedCast)
        return {};
    }
  }

  {
    CanFlowResult flowRes =
        canAFlowIntoB(aType->getReturnType(), bType->getReturnType());
    if (!flowRes.canFlow || flowRes.needCheckedCast)
      return {};
  }

  // If these are native functions, their signatures must match exactly.
  if (auto *nativeA = llvh::dyn_cast<NativeFunctionType>(a)) {
    auto *nativeB = llvh::cast<NativeFunctionType>(b);
    if (nativeA->getSignature()->compare(nativeB->getSignature()) != 0)
      return {};
  }

  return {.canFlow = true};
}

Type *FlowChecker::getNonOptionalSingleType(Type *exprType) {
  if (auto *unionTy = llvh::dyn_cast<UnionType>(exprType->info)) {
    Type *singleType = nullptr;
    // There is only one useful type to narrow to.
    for (Type *unionArm : unionTy->getTypes()) {
      if (llvh::isa<VoidType>(unionArm->info))
        continue;
      if (llvh::isa<NullType>(unionArm->info))
        continue;
      // Found a second type.
      if (singleType) {
        return nullptr;
      }
      singleType = unionArm;
    }
    return singleType;
  }

  return nullptr;
}

std::pair<Type *, FlowChecker::CanFlowResult> FlowChecker::tryNarrowType(
    Type *exprType,
    Type *targetType) {
  // If the types are already compatible, no need to do anything.
  auto cf = canAFlowIntoB(exprType, targetType);
  if (cf.canFlow) {
    // Signal a cast to the targetType in the caller if necessary.
    return {cf.needCheckedCast ? targetType : exprType, cf};
  }

  // Try to narrow the expression type to a single type.
  Type *narrowType = getNonOptionalSingleType(exprType);
  if (!narrowType)
    return {exprType, cf};

  auto cfCast = canAFlowIntoB(narrowType, targetType);
  if (!cfCast.canFlow) {
    return {exprType, cf};
  }

  // Collapse the two chained checked casts into a single one.
  if (cfCast.needCheckedCast) {
    return {targetType, cfCast};
  }

  // Otherwise, going from narrowType to targetType doesn't require a checked
  // cast, but we do need to cast to the narrowType.
  cfCast.needCheckedCast = true;
  return {narrowType, cfCast};
}

ESTree::Node *FlowChecker::implicitCheckedCast(
    ESTree::Node *argument,
    Type *toType,
    CanFlowResult canFlow) {
  if (!canFlow.needCheckedCast || !compile_)
    return argument;
  auto *cast = new (astContext_) ESTree::ImplicitCheckedCastNode(argument);
  cast->copyLocationFrom(argument);
  setNodeType(cast, toType);
  return cast;
}

bool FlowChecker::validateAndBindTypeParameters(
    ESTree::TypeParameterDeclarationNode *params,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    llvh::ArrayRef<Type *> typeArgTypes,
    sema::LexicalScope *scope) {
  size_t i = 0;
  // Whether we had to stop early due to not enough generic type arguments.
  bool tooFewTypeArgs = false;
  for (ESTree::Node &tparam : params->_params) {
    if (i >= typeArgTypes.size()) {
      // Not enough type arguments provided, break and error.
      tooFewTypeArgs = true;
      break;
    }
    if (auto *paramName = llvh::dyn_cast<ESTree::TypeParameterNode>(&tparam)) {
      if (paramName->_bound) {
        sm_.warning(
            paramName->_bound->getSourceRange(),
            "type parameter bounds not yet supported");
      }
      if (paramName->_variance) {
        sm_.warning(
            paramName->_variance->getSourceRange(),
            "type parameter variance not yet supported");
      }
      bindingTable_.try_emplace(
          paramName->_name, TypeDecl{typeArgTypes[i], scope, &tparam});
    } else {
      sm_.error(
          tparam.getSourceRange(),
          "only named type parameters supported in generics");
    }
    ++i;
  }

  // Check that there aren't too many (or too few) type arguments provided.
  if (tooFewTypeArgs || i != typeArgTypes.size()) {
    sm_.error(
        typeArgsNode->getSourceRange(),
        llvh::Twine("type argument mismatch, expected ") +
            llvh::Twine(params->_params.size()) + ", found " +
            llvh::Twine(typeArgTypes.size()));
    return false;
  }

  return true;
}

sema::Decl *FlowChecker::specializeGeneric(
    sema::Decl *oldDecl,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    sema::LexicalScope *scope) {
  // Parse and populate the type arguments.
  llvh::SmallVector<Type *, 2> typeArgTypes{};
  for (ESTree::Node &arg : typeArgsNode->_params) {
    Type *type = parseTypeAnnotation(&arg);
    typeArgTypes.push_back(type);
  }

  return specializeGenericWithParsedTypes(
      oldDecl, typeArgsNode, typeArgTypes, scope);
}

sema::Decl *FlowChecker::specializeGenericWithParsedTypes(
    sema::Decl *oldDecl,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    llvh::ArrayRef<Type *> typeArgTypes,
    sema::LexicalScope *scope) {
  // Extract info from types.
  GenericInfo<ESTree::Node>::TypeArgsVector typeArgs{};
  for (Type *type : typeArgTypes) {
    assert(type->info && "missing type info for generic specialization");
    typeArgs.push_back(type->info);
  }

  // Retrieve generic info and specialization if it exists.
  GenericInfo<ESTree::Node> &generic = getGenericInfoMustExist(oldDecl);
  GenericInfo<ESTree::Node>::TypeArgsRef typeArgsRef = typeArgs;
  ESTree::Node *specialization = generic.getSpecialization(typeArgsRef);

  // Whether to clone a new specialization in this run of the function.
  bool doClone = !specialization;

  /// \return the NodeList in which to insert new nodes into \p n.
  auto getNodeList = [](ESTree::Node *n) -> ESTree::NodeList & {
    switch (n->getKind()) {
      case ESTree::NodeKind::Program:
        return llvh::cast<ESTree::ProgramNode>(n)->_body;
      case ESTree::NodeKind::BlockStatement:
        return llvh::cast<ESTree::BlockStatementNode>(n)->_body;
      default:
        hermes_fatal("invalid node list");
    }
  };

  // Create specialization if it doesn't exist.
  // TODO: Determine if the actual clone can be deferred to avoid potential
  // stack overflow by cloning from deep in the AST.
  if (doClone) {
    LLVM_DEBUG(
        llvh::dbgs() << "Creating specialization for: " << oldDecl->name.str()
                     << "\n");
    specialization = cloneNode(
        astContext_,
        semContext_,
        declCollectorMap_,
        generic.originalNode,
        scope->parentFunction,
        scope);
    if (!specialization) {
      sm_.error(
          typeArgsNode->getSourceRange(),
          "failed to create specialization for generic function");
      return nullptr;
    }
    auto &nodeList = getNodeList(generic.parent);
    nodeList.insert(generic.originalNode->getIterator(), *specialization);
    typeArgsRef =
        generic.addSpecialization(*this, std::move(typeArgs), specialization);
  }

  // The new Decl for the specialization of the function declaration.
  sema::Decl *newDecl = nullptr;
  if (llvh::isa<ESTree::FunctionDeclarationNode>(specialization)) {
    newDecl = getDecl(llvh::cast<ESTree::IdentifierNode>(
        llvh::cast<ESTree::FunctionDeclarationNode>(specialization)->_id));
  } else if (

      llvh::isa<ESTree::ClassDeclarationNode>(specialization)) {
    newDecl = getDecl(llvh::cast<ESTree::IdentifierNode>(
        llvh::cast<ESTree::ClassDeclarationNode>(specialization)->_id));
  }

  newDecl->generic = false;

  // Perform the typechecking of the specialization if it was newly created.
  if (doClone) {
    TypeBindingTableScopePtrTy savedScope = bindingTable_.getCurrentScope();

    // Activate the scope of the specialization..
    bindingTable_.activateScope(generic.bindingTableScope);

    auto onExit = llvh::make_scope_exit([this, &savedScope]() {
      // Restore the previous scope.
      bindingTable_.activateScope(savedScope);
    });

    {
      // Put the parameter types in the binding table.
      ESTree::TypeParameterDeclarationNode *typeParamsNode = nullptr;
      if (auto *func =
              llvh::dyn_cast<ESTree::FunctionDeclarationNode>(specialization)) {
        typeParamsNode = llvh::cast<ESTree::TypeParameterDeclarationNode>(
            ESTree::getTypeParameters(func));
      } else if (
          auto *classDecl =
              llvh::dyn_cast<ESTree::ClassDeclarationNode>(specialization)) {
        typeParamsNode = llvh::cast<ESTree::TypeParameterDeclarationNode>(
            classDecl->_typeParameters);
      }

      assert(typeParamsNode);

      ScopeRAII paramScope{*this};
      bool populated = validateAndBindTypeParameters(
          typeParamsNode, typeArgsNode, typeArgTypes, oldDecl->scope);
      if (!populated) {
        LLVM_DEBUG(llvh::dbgs() << "Failed to bind type parameters\n");
        return nullptr;
      }

      if (auto *func =
              llvh::dyn_cast<ESTree::FunctionDeclarationNode>(specialization)) {
        // TODO: Determine if the actual function typecheck can be deferred to
        // avoid potential stack overflow by cloning from deep in the AST.
        typecheckGenericFunctionSpecialization(
            func, typeArgsNode, typeArgTypes, oldDecl, newDecl);
      } else if (
          auto *classDecl =
              llvh::dyn_cast<ESTree::ClassDeclarationNode>(specialization)) {
        typecheckGenericClassSpecialization(
            classDecl, typeArgsNode, typeArgTypes, oldDecl, newDecl);
      }
    }

    assert(flowContext_.findDeclType(newDecl) && "expected valid type");
  }

  return newDecl;
}

void FlowChecker::resolveCallToGenericFunctionSpecialization(
    ESTree::CallExpressionNode *node,
    ESTree::IdentifierNode *callee,
    sema::Decl *oldDecl) {
  assert(oldDecl && "expected valid oldDecl");

  sema::Decl *newDecl = specializeGeneric(
      oldDecl,
      llvh::cast<ESTree::TypeParameterInstantiationNode>(node->_typeArguments),
      oldDecl->scope);

  if (newDecl) {
    semContext_.setExpressionDecl(callee, newDecl);
    setNodeType(callee, getDeclType(newDecl));
  }
}

void FlowChecker::typecheckGenericFunctionSpecialization(
    ESTree::FunctionDeclarationNode *specialization,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    llvh::ArrayRef<Type *> typeArgTypes,
    sema::Decl *oldDecl,
    sema::Decl *newDecl) {
  LLVM_DEBUG(
      llvh::dbgs() << "Typechecking generic function specialization: "
                   << newDecl->name.str() << "\n");

  // Record the type of the specialization.
  Type *ftype = parseFunctionType(
      ESTree::getParams(specialization),
      ESTree::getReturnType(specialization),
      ESTree::isAsync(specialization),
      ESTree::isGenerator(specialization));
  setNodeType(specialization, ftype);
  // Specializations of function declarations always have an Identifier name.
  auto *ident =
      llvh::cast<ESTree::IdentifierNode>(ESTree::getIdentifier(specialization));
  recordDecl(newDecl, ftype, ident, specialization);

  // Run typechecker.
  visitNonGenericFunctionDeclaration(specialization, newDecl);
}

void FlowChecker::typecheckGenericClassSpecialization(
    ESTree::ClassDeclarationNode *specialization,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    llvh::ArrayRef<Type *> typeArgTypes,
    sema::Decl *oldDecl,
    sema::Decl *newDecl) {
  // Create the new type of the specialization.
  auto *id = llvh::cast<ESTree::IdentifierNode>(specialization->_id);
  assert(newDecl == getDecl(id) && "expected same decl");
  Type *classType = flowContext_.createType(
      flowContext_.createClass(Identifier::getFromPointer(id->_name)),
      specialization);
  Type *classConsType = flowContext_.createType(
      flowContext_.createClassConstructor(classType), specialization);

  bool recorded = recordDecl(newDecl, classConsType, id, specialization);
  assert(recorded && "class constructor unexpectedly re-declared");
  (void)recorded;

  // Visit the super class first so that it gets enqueued/deferred before the
  // child class.
  visitExpression(specialization->_superClass, specialization, nullptr);

  if (deferredParseGenerics_) {
    LLVM_DEBUG(
        llvh::dbgs() << "Deferring parsing of generic class specialization: "
                     << newDecl->name.str() << "\n");
    deferredParseGenerics_->emplace_back(
        specialization, bindingTable_.getCurrentScope(), classType);
  } else {
    parseClassType(
        specialization->_superClass,
        specialization->_superTypeParameters,
        specialization->_body,
        classType);
    typecheckQueue_.emplace_back(
        specialization, bindingTable_.getCurrentScope(), classType);
  }
}

Type *FlowChecker::resolveGenericClassSpecialization(
    ESTree::IdentifierNode *nameNode,
    ESTree::TypeParameterInstantiationNode *typeArgsNode,
    sema::Decl *oldDecl) {
  assert(oldDecl && "expected valid oldDecl");

  sema::Decl *newDecl =
      specializeGeneric(oldDecl, typeArgsNode, oldDecl->scope);

  if (!newDecl)
    return flowContext_.getAny();

  Type *classConsType = getDeclType(newDecl);
  // Set these unconditionally, but it's not actually necessary if this is
  // called from a GenericTypeAnnotation.
  semContext_.setExpressionDecl(nameNode, newDecl);
  setNodeType(nameNode, classConsType);
  return llvh::cast<ClassConstructorType>(classConsType->info)->getClassType();
}

Type *FlowChecker::resolveGenericClassSpecializationForType(
    ESTree::GenericTypeAnnotationNode *genericTypeNode,
    sema::Decl *oldDecl) {
  assert(oldDecl && "expected valid oldDecl");

  auto *typeArgsNode = llvh::cast<ESTree::TypeParameterInstantiationNode>(
      genericTypeNode->_typeParameters);

  sema::Decl *newDecl =
      specializeGeneric(oldDecl, typeArgsNode, oldDecl->scope);

  if (!newDecl)
    return flowContext_.getAny();

  Type *classConsType = getDeclType(newDecl);
  return llvh::cast<ClassConstructorType>(classConsType->info)->getClassType();
}

void FlowChecker::registerGeneric(
    sema::Decl *decl,
    ESTree::Node *node,
    ESTree::Node *parent,
    const TypeBindingTableScopePtrTy &bindingScope) {
  assert(decl->generic && "expected a generic decl");
  GenericInfo<ESTree::Node> &info =
      generics_.emplace_back(node, parent, bindingScope);
  auto [it, inserted] = genericsMap_.try_emplace(decl, &info);
  assert(inserted && "duplicate generic decl");
  (void)it;
}

void FlowChecker::registerGenericAlias(
    ESTree::TypeAliasNode *node,
    ESTree::Node *parent,
    const TypeBindingTableScopePtrTy &bindingScope) {
  GenericInfo<Type> &info =
      genericAliases_.emplace_back(node, parent, bindingScope);
  auto [it, inserted] = genericAliasesMap_.try_emplace(node, &info);
  assert(inserted && "duplicate generic decl");
  (void)it;
}

/// \return the generic info associated with \p decl.
FlowChecker::GenericInfo<ESTree::Node> &FlowChecker::getGenericInfoMustExist(
    sema::Decl *decl) {
  auto it = genericsMap_.find(decl);
  assert(it != genericsMap_.end() && "generic was never registered");
  return *it->second;
}

/// \return the generic info associated with \p decl.
FlowChecker::GenericInfo<Type> &FlowChecker::getGenericAliasInfoMustExist(
    const ESTree::TypeAliasNode *decl) {
  auto it = genericAliasesMap_.find(decl);
  assert(it != genericAliasesMap_.end() && "generic was never registered");
  return *it->second;
}

UniqueString *FlowChecker::propertyKeyAsIdentifier(ESTree::Node *Key) {
  // Handle String Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-string-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::StringLiteralNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Lit->_value << "\"\n");
    return Lit->_value;
  }

  // Handle identifiers as if they are String Literals.
  if (auto *Iden = llvh::dyn_cast<ESTree::IdentifierNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading String Literal \"" << Iden->_name << "\"\n");
    return Iden->_name;
  }

  // Handle Number Literals.
  // http://www.ecma-international.org/ecma-262/6.0/#sec-literals-numeric-literals
  if (auto *Lit = llvh::dyn_cast<ESTree::NumericLiteralNode>(Key)) {
    LLVM_DEBUG(
        llvh::dbgs() << "Loading Numeric Literal \"" << Lit->_value << "\"\n");
    char buf[NUMBER_TO_STRING_BUF_SIZE];
    auto len = numberToString(Lit->_value, buf, sizeof(buf));
    return astContext_.getIdentifier(llvh::StringRef{buf, len})
        .getUnderlyingPointer();
  }

  return nullptr;
}

} // namespace flow
} // namespace hermes
