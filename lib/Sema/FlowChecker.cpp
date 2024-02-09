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
  visitExpression(node->_superClass, node);

  auto *id = llvh::cast_or_null<ESTree::IdentifierNode>(node->_id);
  Type *classType = flowContext_.createType();
  ParseClassType(
      *this,
      node->_superClass,
      node->_superTypeParameters,
      node->_body,
      classType);

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
    assert(optField.hasValue() && "method must have been registered");
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

class FlowChecker::ExprVisitor {
  FlowChecker &outer_;

 public:
  explicit ExprVisitor(FlowChecker &outer) : outer_(outer) {}

  bool incRecursionDepth(ESTree::Node *n) {
    return outer_.incRecursionDepth(n);
  }
  void decRecursionDepth() {
    return outer_.decRecursionDepth();
  }

  /// Default case for all ignored nodes, we still want to visit their children.
  void visit(ESTree::Node *node) {
    if (0) {
      LLVM_DEBUG(
          llvh::dbgs() << "Unsupported node " << node->getNodeName()
                       << " in expr context\n");
      llvm_unreachable("invalid node in expression context");
    } else {
      visitESTreeChildren(*this, node);
    }
  }

  void visit(ESTree::FunctionExpressionNode *node) {
    return outer_.visit(node);
  }
  void visit(ESTree::ArrowFunctionExpressionNode *node) {
    return outer_.visit(node);
  }
  void visit(ESTree::ClassExpressionNode *node) {
    return outer_.visit(node);
  }

  void visit(ESTree::IdentifierNode *node, ESTree::Node *parent) {
    // Skip cases where the identifier isn't a variable.
    // TODO: these should be dealt with by the parent node.
    if (auto *prop = llvh::dyn_cast<ESTree::PropertyNode>(parent)) {
      if (!prop->_computed && prop->_key == node) {
        // { identifier: ... }
        return;
      }
    }

    if (auto *mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(parent)) {
      if (!mem->_computed && mem->_property == node) {
        // expr.identifier
        return;
      }
    }

    // Identifiers that aren't variables.
    if (llvh::isa<ESTree::MetaPropertyNode>(parent) ||
        llvh::isa<ESTree::BreakStatementNode>(parent) ||
        llvh::isa<ESTree::ContinueStatementNode>(parent) ||
        llvh::isa<ESTree::LabeledStatementNode>(parent)) {
      return;
    }

    // typeof
    if (auto *unary = llvh::dyn_cast<ESTree::UnaryExpressionNode>(parent)) {
      if (unary->_operator == outer_.kw_.identTypeof) {
        // FIXME: handle typeof identifier
        return;
      }
    }

    auto *decl = outer_.getDecl(node);
    assert(decl && "unresolved identifier in expression context");

    if (sema::Decl::isKindGlobal(decl->kind) &&
        decl->name.getUnderlyingPointer() == outer_.kw_.identUndefined) {
      outer_.setNodeType(node, outer_.flowContext_.getVoid());
      return;
    }

    if (decl->generic) {
      bool isValid = false;
      if (auto *call = llvh::dyn_cast<ESTree::CallExpressionLikeNode>(parent)) {
        if (ESTree::getCallee(call) == node)
          isValid = true;
      }
      if (auto *newExpr = llvh::dyn_cast<ESTree::NewExpressionNode>(parent)) {
        if (newExpr->_callee == node)
          isValid = true;
      }
      if (auto *classDecl =
              llvh::dyn_cast<ESTree::ClassDeclarationNode>(parent)) {
        if (classDecl->_id == node)
          isValid = true;
        if (classDecl->_superClass == node)
          isValid = true;
      }
      if (!isValid) {
        // Unspecialized generic functions are only allowed in calls.
        // They can't be stored directly because they need type parameters.
        outer_.sm_.error(
            node->getSourceRange(),
            "ft: invalid use of generic function outside of call");
        return;
      }
    }

    // The type is either the type of the identifier or "any".
    Type *type = outer_.flowContext_.findDeclType(decl);

    // Generic decls don't have types set because they aren't real values.
    if (!type && !sema::Decl::isKindGlobal(decl->kind) && !decl->generic) {
      // Assume "any" during the call to setNodeType below.
      // If we're in the same function as decl was declared,
      // then IRGen can report TDZ violations early when applicable.
      // See FlowChecker::AnnotateScopeDecls doc-comment.

      // Report a warning because this is likely unintended.
      // The following code errors in Flow but not in untyped JS:
      //   x = 10;
      //   var x = x + 1;
      // So we don't error to maintain compatibility when there's no
      // annotations.
      outer_.sm_.warning(
          node->getSourceRange(),
          "local variable may be used prior to declaration, assuming 'any'");
    }

    outer_.setNodeType(node, type ? type : outer_.flowContext_.getAny());
  }

  void visit(ESTree::ThisExpressionNode *node) {
    outer_.setNodeType(
        node,
        outer_.curFunctionContext_->thisParamType
            ? outer_.curFunctionContext_->thisParamType
            : outer_.flowContext_.getAny());
  }

  void visit(ESTree::MemberExpressionNode *node) {
    // TODO: types
    visitESTreeNode(*this, node->_object, node);
    if (node->_computed)
      visitESTreeNode(*this, node->_property, node);

    Type *objType = outer_.getNodeTypeOrAny(node->_object);
    Type *resType = outer_.flowContext_.getAny();

    // Attempt to narrow object type if it doesn't currently support member
    // access.
    if (Type *narrowedObjType = outer_.getNonOptionalSingleType(objType)) {
      objType = narrowedObjType;
      node->_object = outer_.implicitCheckedCast(
          node->_object,
          narrowedObjType,
          {.canFlow = true, .needCheckedCast = true});
    }

    if (auto *classType = llvh::dyn_cast<ClassType>(objType->info)) {
      if (node->_computed) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: computed access to class instances not supported");
      } else {
        bool found = false;
        auto id = llvh::cast<ESTree::IdentifierNode>(node->_property);
        auto optField =
            classType->findField(Identifier::getFromPointer(id->_name));
        if (optField) {
          resType = optField->getField()->type;
          found = true;
        } else {
          auto optMethod = classType->getHomeObjectTypeInfo()->findField(
              Identifier::getFromPointer(id->_name));
          if (optMethod) {
            resType = optMethod->getField()->type;
            found = true;
            assert(
                llvh::isa<BaseFunctionType>(resType->info) &&
                "methods must be functions");
          }
        }
        if (!found) {
          // TODO: class declaration location.
          outer_.sm_.error(
              node->_property->getSourceRange(),
              "ft: property " + id->_name->str() + " not defined in class " +
                  classType->getClassNameOrDefault());
        }
      }
    } else if (auto *arrayType = llvh::dyn_cast<ArrayType>(objType->info)) {
      if (node->_computed) {
        resType = arrayType->getElement();
        Type *indexType = outer_.getNodeTypeOrAny(node->_property);
        if (!llvh::isa<NumberType>(indexType->info) &&
            !llvh::isa<AnyType>(indexType->info)) {
          outer_.sm_.error(
              node->_property->getSourceRange(),
              "ft: array index must be a number");
        }
      } else {
        auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
        if (id->_name == outer_.kw_.identLength) {
          resType = outer_.flowContext_.getNumber();
        } else if (id->_name == outer_.kw_.identPush) {
          // TODO: Represent .push as a real function.
          resType = outer_.flowContext_.getAny();
        } else {
          outer_.sm_.error(
              node->_property->getSourceRange(), "ft: unknown array property");
        }
      }
    } else if (auto *tupleType = llvh::dyn_cast<TupleType>(objType->info)) {
      if (node->_computed) {
        if (auto *idx =
                llvh::dyn_cast<ESTree::NumericLiteralNode>(node->_property)) {
          double d = idx->_value;
          if (0 <= d && d < tupleType->getTypes().size()) {
            // d is in bounds of the valid integer indices so the cast is safe.
            if ((uint32_t)d == d) {
              // ulen can only compare equal to d when d is a valid uint32
              // integer.
              resType = tupleType->getTypes()[(uint32_t)d];
            } else {
              outer_.sm_.error(
                  node->_property->getSourceRange(),
                  "ft: tuple index must be a non-negative integer");
            }
          } else {
            outer_.sm_.error(
                node->_property->getSourceRange(),
                "ft: tuple index out of bounds");
          }
        } else {
          outer_.sm_.error(
              node->_property->getSourceRange(),
              "ft: tuple property access requires an number literal index");
        }
      }
    } else if (!llvh::isa<AnyType>(objType->info)) {
      if (node->_computed) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            llvh::Twine(
                "ft: indexed access only allowed on array and tuple, found ") +
                objType->info->getKindName());
      } else {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            llvh::Twine(
                "ft: named property access only allowed on objects, found ") +
                objType->info->getKindName());
      }
    }

    outer_.setNodeType(node, resType);
  }
  void visit(ESTree::OptionalMemberExpressionNode *node) {
    // TODO: types
    outer_.sm_.warning(
        node->getSourceRange(),
        "ft: optional member expression not implemented");
    visitESTreeNode(*this, node->_object, node);
    if (node->_computed)
      visitESTreeNode(*this, node->_property, node);
  }

  void visitExplicitCast(
      ESTree::Node *node,
      ESTree::Node *expression,
      ESTree::Node *typeAnnotation) {
    auto *resTy = outer_.parseTypeAnnotation(typeAnnotation);
    // Populate the type of this node before visiting the expression, since it
    // is already known. This also allows the result type to be used as context
    // while we are visiting the expression being cast. For instance, if we are
    // casting an empty array literal, the resulting type of the cast can be
    // used to set the element type of the array.
    outer_.setNodeType(node, resTy);
    visitESTreeNode(*this, expression, node);

    auto *expTy = outer_.getNodeTypeOrAny(expression);
    auto cf = canAFlowIntoB(expTy->info, resTy->info);
    if (!cf.canFlow) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: cast from incompatible type");
    }
  }

  void visit(ESTree::TypeCastExpressionNode *node) {
    visitExplicitCast(
        node,
        node->_expression,
        llvh::cast<ESTree::TypeAnnotationNode>(node->_typeAnnotation)
            ->_typeAnnotation);
  }

  void visit(ESTree::AsExpressionNode *node) {
    visitExplicitCast(node, node->_expression, node->_typeAnnotation);
  }

  void visit(ESTree::ArrayExpressionNode *node, ESTree::Node *parent) {
    visitESTreeChildren(*this, node);

    /// Given an element in the array expression, determine its type.
    auto getElementType = [this](const ESTree::Node *elem) -> Type * {
      if (auto *spread = llvh::dyn_cast<ESTree::SpreadElementNode>(elem)) {
        // The type of a spread element depends on its argument.
        auto *spreadTy = outer_.getNodeTypeOrAny(spread->_argument);
        // If we are spreading an array, use the type of the array.
        if (auto *spreadArrTy = llvh::dyn_cast<ArrayType>(spreadTy->info))
          return spreadArrTy->getElement();
        // TODO: Handle spread of non-arrays.
        outer_.sm_.error(
            spread->_argument->getSourceRange(),
            "ft: spread argument must be an array");
        return outer_.flowContext_.getAny();
      }
      return outer_.flowContext_.getNodeTypeOrAny(elem);
    };

    /// Try using the given element type \p elTy for this array. If any elements
    /// are incompatible with the given type, report an error, otherwise, set
    /// the type of the array.
    auto tryArrayElementType = [node, this, getElementType](Type *elTy) {
      auto &elements = node->_elements;
      size_t i = 0;
      for (auto it = elements.begin(); it != elements.end(); ++it) {
        ESTree::Node *arg = &*it;
        Type *argTy = getElementType(arg);
        auto cf = canAFlowIntoB(argTy->info, elTy->info);
        if (!cf.canFlow) {
          outer_.sm_.error(
              arg->getSourceRange(),
              llvh::Twine("ft: incompatible array element type at index: ") +
                  llvh::Twine(i));
          return;
        }
        // Add a checked cast if needed. Skip spread elements, since we need to
        // cast each element they produce, rather than the spread itself.
        if (cf.needCheckedCast && !llvh::isa<ESTree::SpreadElementNode>(arg)) {
          auto newIt =
              elements.insert(it, *outer_.implicitCheckedCast(arg, elTy, cf));
          elements.erase(it);
          it = newIt;
        }

        ++i;
      }
      auto *arrTy = outer_.flowContext_.createArray(elTy);
      outer_.setNodeType(node, outer_.flowContext_.createType(arrTy));
    };

    /// Make the tuple type of the array expression based on its elements.
    auto tryTupleType = [node, this, getElementType](
                            Type *target, TupleType *targetTuple) -> void {
      auto &elements = node->_elements;
      size_t i = 0;
      // Whether we had to stop early due to not enough tuple elements.
      bool fail = false;
      for (ESTree::Node &arg : elements) {
        if (i >= targetTuple->getTypes().size()) {
          fail = true;
          break;
        }

        Type *targetTy = targetTuple->getTypes()[i];
        Type *argTy = getElementType(&arg);
        auto cf = canAFlowIntoB(argTy->info, targetTy->info);
        if (!cf.canFlow) {
          outer_.sm_.error(
              arg.getSourceRange(),
              llvh::Twine("ft: incompatible tuple element type at index: ") +
                  llvh::Twine(i));
          return;
        }
        ++i;
      }
      if (fail || i != targetTuple->getTypes().size()) {
        outer_.sm_.error(
            node->getSourceRange(),
            llvh::Twine("ft: incompatible tuple type, expected ") +
                llvh::Twine(targetTuple->getTypes().size()) +
                " elements, found " + llvh::Twine(elements.size()));
        outer_.setNodeType(node, outer_.flowContext_.getAny());
        return;
      }
      outer_.setNodeType(node, target);
    };

    // First, try to determine the desired element type from surrounding
    // context. For instance, this lets us determine the type of empty array
    // literals. If the type of an element is incompatible, it is okay to fail
    // here, since we can point to the exact element that is incompatible.

    // If this array expression initializes a variable, try the type of that
    // variable.
    if (auto *declarator =
            llvh::dyn_cast<ESTree::VariableDeclaratorNode>(parent)) {
      if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
        sema::Decl *decl = outer_.getDecl(id);
        // It's possible we're just trying to infer the type of the declarator
        // right now, so it's possible `findDeclType` returns nullptr.
        if (Type *declType = outer_.flowContext_.findDeclType(decl)) {
          if (auto *arrTy = llvh::dyn_cast<ArrayType>(declType->info)) {
            tryArrayElementType(arrTy->getElement());
            return;
          }
          if (auto *tupleTy = llvh::dyn_cast<TupleType>(declType->info)) {
            tryTupleType(declType, tupleTy);
            return;
          }
        }
      }
    }

    // If this array expression is immediately cast to something else, try using
    // the type we are casting to.
    if (llvh::isa<ESTree::TypeCastExpressionNode>(parent) ||
        llvh::isa<ESTree::AsExpressionNode>(parent)) {
      auto *resTy = outer_.getNodeTypeOrAny(parent);
      if (auto *arrTy = llvh::dyn_cast<ArrayType>(resTy->info)) {
        tryArrayElementType(arrTy->getElement());
        return;
      }
      if (auto *tupleTy = llvh::dyn_cast<TupleType>(resTy->info)) {
        tryTupleType(resTy, tupleTy);
        return;
      }
    }

    // If this is a returned literal, try the type of the function.
    if (llvh::isa<ESTree::ReturnStatementNode>(parent)) {
      if (auto *ftype = llvh::dyn_cast<TypedFunctionType>(
              outer_.curFunctionContext_->functionType->info)) {
        if (auto *arrTy = llvh::dyn_cast<ArrayType>(ftype)) {
          tryArrayElementType(arrTy->getElement());
          return;
        }
        if (auto *tupleTy =
                llvh::dyn_cast<TupleType>(ftype->getReturnType()->info)) {
          tryTupleType(ftype->getReturnType(), tupleTy);
          return;
        }
      }
    }

    // In principle, we could use the type of an enclosing array literal where
    // this literal is being spread or nested. However, we leave that
    // unsupported for now, as the enclosing literal would not have its type set
    // at this point.

    // We could not determine the type from the context, infer it from the
    // elements.
    Type *elTy;
    if (node->_elements.empty()) {
      // If there are no elements, we can't infer the type, so use 'any'.
      elTy = outer_.flowContext_.getAny();
    } else {
      // Construct a union of all the element types.
      llvh::SmallSetVector<Type *, 4> elTypes;
      for (const auto &arg : node->_elements)
        elTypes.insert(getElementType(&arg));

      elTy = outer_.flowContext_.createType(
          outer_.flowContext_.maybeCreateUnion(elTypes.getArrayRef()), node);
    }
    auto *arrTy = outer_.flowContext_.createArray(elTy);
    outer_.setNodeType(node, outer_.flowContext_.createType(arrTy));
  }

  void visit(ESTree::SpreadElementNode *node) {
    // Do nothing for the spread element itself, handled by the parent.
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::NullLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getNull());
  }
  void visit(ESTree::BooleanLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getBoolean());
  }
  void visit(ESTree::StringLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getString());
  }
  void visit(ESTree::TemplateLiteralNode *node) {
    for (ESTree::Node &quasi : node->_quasis)
      outer_.setNodeType(&quasi, outer_.flowContext_.getString());
    visitESTreeNodeList(*this, node->_expressions, node);
    outer_.setNodeType(node, outer_.flowContext_.getString());
  }
  void visit(ESTree::NumericLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getNumber());
  }
  void visit(ESTree::RegExpLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getAny());
  }
  void visit(ESTree::BigIntLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getBigInt());
  }
  void visit(ESTree::SHBuiltinNode *node) {
    // SHBuiltin handled at the call expression level.
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::UpdateExpressionNode *node) {
    visitESTreeNode(*this, node->_argument, node);
    Type *type = outer_.getNodeTypeOrAny(node->_argument);
    if (llvh::isa<NumberType>(type->info) ||
        llvh::isa<BigIntType>(type->info)) {
      // number and bigint don't change.
      outer_.setNodeType(node, type);
      return;
    }
    if (auto *unionType = llvh::dyn_cast<UnionType>(type->info)) {
      if (llvh::all_of(unionType->getTypes(), [](Type *t) -> bool {
            return llvh::isa<NumberType>(t->info) ||
                llvh::isa<BigIntType>(t->info);
          })) {
        // Unions of number/bigint don't change.
        outer_.setNodeType(node, type);
        return;
      }
    }
    if (llvh::isa<AnyType>(type->info)) {
      // any remains any.
      outer_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }
    outer_.sm_.error(
        node->getSourceRange(),
        "ft: update expression must be number or bigint");
  }

  enum class BinopKind : uint8_t {
    // clang-format off
    eq, ne, strictEq, strictNe, lt, le, gt, ge, shl, sshr, ushr,
    plus, minus, mul, div, rem, binOr, binXor, binAnd, exp, in, instanceOf
    // clang-format on
  };

  static BinopKind binopKind(llvh::StringRef str) {
    return llvh::StringSwitch<BinopKind>(str)
        .Case("==", BinopKind::eq)
        .Case("!=", BinopKind::ne)
        .Case("===", BinopKind::strictEq)
        .Case("!==", BinopKind::strictNe)
        .Case("<", BinopKind::lt)
        .Case("<=", BinopKind::le)
        .Case(">", BinopKind::gt)
        .Case(">=", BinopKind::ge)
        .Case("<<", BinopKind::shl)
        .Case(">>", BinopKind::sshr)
        .Case(">>>", BinopKind::ushr)
        .Case("+", BinopKind::plus)
        .Case("-", BinopKind::minus)
        .Case("*", BinopKind::mul)
        .Case("/", BinopKind::div)
        .Case("%", BinopKind::rem)
        .Case("|", BinopKind::binOr)
        .Case("^", BinopKind::binXor)
        .Case("&", BinopKind::binAnd)
        .Case("**", BinopKind::exp)
        .Case("in", BinopKind::in)
        .Case("instanceof", BinopKind::instanceOf);
  }

  enum class UnopKind : uint8_t {
    // clang-format off
    del, voidOp, typeof, plus, minus, tilde, bang, inc, dec
    // clang-format on
  };

  static UnopKind unopKind(llvh::StringRef str) {
    return llvh::StringSwitch<UnopKind>(str)
        .Case("delete", UnopKind::del)
        .Case("void", UnopKind::voidOp)
        .Case("typeof", UnopKind::typeof)
        .Case("+", UnopKind::plus)
        .Case("-", UnopKind::minus)
        .Case("~", UnopKind::tilde)
        .Case("!", UnopKind::bang)
        .Case("++", UnopKind::inc)
        .Case("--", UnopKind::dec);
  }

  static BinopKind assignKind(llvh::StringRef str) {
    return llvh::StringSwitch<BinopKind>(str)
        .Case("<<=", BinopKind::shl)
        .Case(">>=", BinopKind::sshr)
        .Case(">>>=", BinopKind::ushr)
        .Case("+=", BinopKind::plus)
        .Case("-=", BinopKind::minus)
        .Case("*=", BinopKind::mul)
        .Case("/=", BinopKind::div)
        .Case("%=", BinopKind::rem)
        .Case("|=", BinopKind::binOr)
        .Case("^=", BinopKind::binXor)
        .Case("&=", BinopKind::binAnd)
        .Case("**=", BinopKind::exp);
  }

  /// \return nullptr if the operation is not supported.
  Type *determineBinopType(BinopKind op, TypeKind lk, TypeKind rk) {
    struct BinTypes {
      BinopKind op;
      TypeKind res;
      // None indicates a wildcard, Any indicates the actual 'any' type.
      OptValue<TypeKind> left;
      OptValue<TypeKind> right;
    };

    static const BinTypes s_types[] = {
        // clang-format off
        {BinopKind::eq, TypeKind::Boolean, llvh::None, llvh::None},
        {BinopKind::ne, TypeKind::Boolean, llvh::None, llvh::None},
        {BinopKind::strictEq, TypeKind::Boolean, llvh::None, llvh::None},
        {BinopKind::strictNe, TypeKind::Boolean, llvh::None, llvh::None},

        {BinopKind::lt, TypeKind::Boolean, TypeKind::Number, TypeKind::Number},
        {BinopKind::lt, TypeKind::Boolean, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::lt, TypeKind::Boolean, TypeKind::String, TypeKind::String},
        {BinopKind::lt, TypeKind::Boolean, TypeKind::Any, llvh::None},
        {BinopKind::lt, TypeKind::Boolean, llvh::None, TypeKind::Any},

        {BinopKind::le, TypeKind::Boolean, TypeKind::Number, TypeKind::Number},
        {BinopKind::le, TypeKind::Boolean, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::le, TypeKind::Boolean, TypeKind::String, TypeKind::String},
        {BinopKind::le, TypeKind::Boolean, TypeKind::Any, llvh::None},
        {BinopKind::le, TypeKind::Boolean, llvh::None, TypeKind::Any},

        {BinopKind::gt, TypeKind::Boolean, TypeKind::Number, TypeKind::Number},
        {BinopKind::gt, TypeKind::Boolean, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::gt, TypeKind::Boolean, TypeKind::String, TypeKind::String},
        {BinopKind::gt, TypeKind::Boolean, TypeKind::Any, llvh::None},
        {BinopKind::gt, TypeKind::Boolean, llvh::None, TypeKind::Any},

        {BinopKind::ge, TypeKind::Boolean, TypeKind::Number, TypeKind::Number},
        {BinopKind::ge, TypeKind::Boolean, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::ge, TypeKind::Boolean, TypeKind::String, TypeKind::String},
        {BinopKind::ge, TypeKind::Boolean, TypeKind::Any, llvh::None},
        {BinopKind::ge, TypeKind::Boolean, llvh::None, TypeKind::Any},

        {BinopKind::shl, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::shl, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::shl, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::shl, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::sshr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::sshr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::sshr, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::sshr, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::ushr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::ushr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::ushr, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::ushr, TypeKind::Any, llvh::None, TypeKind::Any},

        {BinopKind::plus, TypeKind::String, TypeKind::String, TypeKind::String},
        {BinopKind::plus, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::plus, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::plus, TypeKind::String, TypeKind::Any, TypeKind::String},
        {BinopKind::plus, TypeKind::String, TypeKind::String, TypeKind::Any},
        {BinopKind::plus, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::plus, TypeKind::Any, llvh::None, TypeKind::Any},

        {BinopKind::minus, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::minus, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::minus, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::minus, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::mul, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::mul, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::mul, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::mul, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::div, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::div, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::div, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::div, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::rem, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::rem, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::rem, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::rem, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::binOr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binOr, TypeKind::Number, TypeKind::Any, TypeKind::Number},
        {BinopKind::binOr, TypeKind::Number, TypeKind::Number, TypeKind::Any},
        {BinopKind::binOr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binOr, TypeKind::BigInt, TypeKind::Any, TypeKind::BigInt},
        {BinopKind::binOr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::Any},
        {BinopKind::binOr, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::binOr, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::binXor, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binXor, TypeKind::Number, TypeKind::Any, TypeKind::Number},
        {BinopKind::binXor, TypeKind::Number, TypeKind::Number, TypeKind::Any},
        {BinopKind::binXor, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binXor, TypeKind::BigInt, TypeKind::Any, TypeKind::BigInt},
        {BinopKind::binXor, TypeKind::BigInt, TypeKind::BigInt, TypeKind::Any},
        {BinopKind::binXor, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::binXor, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::binAnd, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binAnd, TypeKind::Number, TypeKind::Any, TypeKind::Number},
        {BinopKind::binAnd, TypeKind::Number, TypeKind::Number, TypeKind::Any},
        {BinopKind::binAnd, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binAnd, TypeKind::BigInt, TypeKind::Any, TypeKind::BigInt},
        {BinopKind::binAnd, TypeKind::BigInt, TypeKind::BigInt, TypeKind::Any},
        {BinopKind::binAnd, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::binAnd, TypeKind::Any, TypeKind::Any, llvh::None},
        {BinopKind::exp, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::exp, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::exp, TypeKind::Any, llvh::None, TypeKind::Any},
        {BinopKind::exp, TypeKind::Any, TypeKind::Any, llvh::None},

        {BinopKind::in, TypeKind::Boolean, llvh::None, TypeKind::Any},
        {BinopKind::instanceOf, TypeKind::Boolean, llvh::None, TypeKind::ClassConstructor},
        {BinopKind::instanceOf, TypeKind::Boolean, llvh::None, TypeKind::UntypedFunction},
        {BinopKind::instanceOf, TypeKind::Boolean, llvh::None, TypeKind::Any},
        // clang-format on
    };
    static const BinTypes *const s_types_end =
        s_types + sizeof(s_types) / sizeof(s_types[0]);

    // Find the start of the section for this operator.
    auto it = std::lower_bound(
        s_types, s_types_end, op, [](const BinTypes &bt, BinopKind op) {
          return bt.op < op;
        });

    // Search for a match.
    for (; it != s_types_end && it->op == op; ++it) {
      if ((!it->left || *it->left == lk) && (!it->right || *it->right == rk)) {
        return outer_.flowContext_.getSingletonType(it->res);
      }
    }

    return nullptr;
  }

  void visit(ESTree::BinaryExpressionNode *node) {
    visitESTreeNode(*this, node->_left, node);
    visitESTreeNode(*this, node->_right, node);
    Type *lt = outer_.getNodeTypeOrAny(node->_left);
    Type *rt = outer_.getNodeTypeOrAny(node->_right);

    Type *res;
    if (Type *t = determineBinopType(
            binopKind(node->_operator->str()),
            lt->info->getKind(),
            rt->info->getKind())) {
      res = t;
    } else {
      outer_.sm_.error(
          node->getSourceRange(),
          llvh::Twine("ft: incompatible binary operation: ") +
              node->_operator->str() + " cannot be applied to " +
              lt->info->getKindName() + " and " + rt->info->getKindName());
      res = outer_.flowContext_.getAny();
    }

    outer_.setNodeType(node, res);
  }

  Type *determineUnopType(
      ESTree::UnaryExpressionNode *node,
      UnopKind op,
      TypeKind argKind) {
    struct UnTypes {
      UnopKind op;
      TypeKind res;
      // None indicates a wildcard, Any indicates the actual 'any' type.
      OptValue<TypeKind> arg;
    };

    static const UnTypes s_types[] = {
        // clang-format off
        {UnopKind::del, TypeKind::Boolean, llvh::None},
        {UnopKind::voidOp, TypeKind::Void, llvh::None},
        {UnopKind::typeof, TypeKind::String, llvh::None},
        {UnopKind::plus, TypeKind::Number, TypeKind::Number},
        {UnopKind::plus, TypeKind::Number, TypeKind::Any},
        {UnopKind::minus, TypeKind::BigInt, TypeKind::BigInt},
        {UnopKind::minus, TypeKind::Number, TypeKind::Number},
        {UnopKind::minus, TypeKind::Any, TypeKind::Any},
        {UnopKind::tilde, TypeKind::Number, TypeKind::Number},
        {UnopKind::tilde, TypeKind::BigInt, TypeKind::BigInt},
        {UnopKind::tilde, TypeKind::Any, TypeKind::Any},
        {UnopKind::bang, TypeKind::Boolean, llvh::None},
        {UnopKind::inc, TypeKind::Number, TypeKind::Number},
        {UnopKind::inc, TypeKind::BigInt, TypeKind::BigInt},
        {UnopKind::inc, TypeKind::Any, TypeKind::Any},
        {UnopKind::dec, TypeKind::Number, TypeKind::Number},
        {UnopKind::dec, TypeKind::BigInt, TypeKind::BigInt},
        {UnopKind::dec, TypeKind::Any, TypeKind::Any},
        // clang-format on
    };
    static const UnTypes *const s_types_end =
        s_types + sizeof(s_types) / sizeof(s_types[0]);

    // Find the start of the section for this operator.
    auto it = std::lower_bound(
        s_types, s_types_end, op, [](const UnTypes &bt, UnopKind op) {
          return bt.op < op;
        });

    // Search for a match.
    for (; it != s_types_end && it->op == op; ++it) {
      if (!it->arg || *it->arg == argKind) {
        return outer_.flowContext_.getSingletonType(it->res);
      }
    }

    return nullptr;
  }

  void visit(ESTree::UnaryExpressionNode *node) {
    visitESTreeNode(*this, node->_argument, node);
    Type *argType = outer_.getNodeTypeOrAny(node->_argument);

    Type *res;
    if (Type *t = determineUnopType(
            node, unopKind(node->_operator->str()), argType->info->getKind())) {
      res = t;
    } else {
      outer_.sm_.error(
          node->getSourceRange(),
          llvh::Twine("ft: incompatible unary operation: ") +
              node->_operator->str() + " cannot be applied to " +
              argType->info->getKindName());
      res = outer_.flowContext_.getAny();
    }

    outer_.setNodeType(node, res);
  }

  void visit(ESTree::LogicalExpressionNode *node) {
    visitESTreeNode(*this, node->_left, node);
    visitESTreeNode(*this, node->_right, node);
    Type *left = outer_.flowContext_.getNodeTypeOrAny(node->_left);
    Type *right = outer_.flowContext_.getNodeTypeOrAny(node->_right);

    auto hasNull = [](TypeInfo *t) -> bool {
      if (llvh::isa<NullType>(t))
        return true;
      if (auto *u = llvh::dyn_cast<UnionType>(t))
        return u->hasNull();
      return false;
    };

    auto hasVoid = [](TypeInfo *t) -> bool {
      if (llvh::isa<VoidType>(t))
        return true;
      if (auto *u = llvh::dyn_cast<UnionType>(t))
        return u->hasVoid();
      return false;
    };

    // The result of a logical expression is the union of both sides,
    // however if the operator is ?? or ||, then we can discard any null/void
    // that only appears on the left, because those can never be returned via
    // the left.
    FlowContext::UnionExcludes excludes{};
    if (node->_operator == outer_.kw_.identLogicalOr ||
        node->_operator == outer_.kw_.identNullishCoalesce) {
      if (hasVoid(left->info) && !hasVoid(right->info))
        excludes.excludeVoid = true;
      if (hasNull(left->info) && !hasNull(right->info))
        excludes.excludeNull = true;
    }

    Type *types[2]{
        outer_.flowContext_.getNodeTypeOrAny(node->_left),
        outer_.flowContext_.getNodeTypeOrAny(node->_right)};

    Type *unionType = outer_.flowContext_.createType(
        outer_.flowContext_.maybeCreateUnion(types, excludes));

    outer_.setNodeType(node, unionType);
  }

  enum class LogicalAssignmentOp : uint8_t {
    ShortCircuitOrKind, // ||=
    ShortCircuitAndKind, // &&=
    NullishCoalesceKind, // ??=
  };

  void visit(ESTree::AssignmentExpressionNode *node) {
    visitESTreeNode(*this, node->_left, node);
    visitESTreeNode(*this, node->_right, node);

    auto logicalAssign =
        llvh::StringSwitch<OptValue<LogicalAssignmentOp>>(
            node->_operator->str())
            .Case("||=", LogicalAssignmentOp::ShortCircuitOrKind)
            .Case("&&=", LogicalAssignmentOp::ShortCircuitAndKind)
            .Case("\?\?=", LogicalAssignmentOp::NullishCoalesceKind)
            .Default(llvh::None);
    if (logicalAssign) {
      outer_.sm_.error(node->getSourceRange(), "ft: unsupported");
      return;
    }

    Type *lt = outer_.getNodeTypeOrAny(node->_left);
    Type *rt = outer_.getNodeTypeOrAny(node->_right);
    Type *res;

    if (node->_operator->str() == "=") {
      auto [rtNarrow, cf] =
          tryNarrowType(outer_.getNodeTypeOrAny(node->_right), lt);
      if (!cf.canFlow) {
        outer_.sm_.error(
            node->getSourceRange(), "ft: incompatible assignment types");
        res = lt;
      } else {
        node->_right = outer_.implicitCheckedCast(node->_right, rtNarrow, cf);

        // If we don't need a checked cast, rt is possibly narrower than lt, but
        // never wider, so we want to use it as result.
        // This helps with cases like:
        //  let a: number|string, n: number; n = a = 5;
        res = cf.needCheckedCast ? lt : rt;
      }
    } else {
      Type *opResType = determineBinopType(
          assignKind(node->_operator->str()),
          lt->info->getKind(),
          rt->info->getKind());

      if (!opResType) {
        outer_.sm_.error(
            node->getSourceRange(),
            llvh::Twine("ft: incompatible binary operation: ") +
                node->_operator->str() + " cannot be applied to " +
                lt->info->getKindName() + " and " + rt->info->getKindName());
        opResType = outer_.flowContext_.getAny();
      }

      if (llvh::isa<AnyType>(lt->info)) {
        // If the target we are assigning to is untyped, there are no checks
        // needed.
        res = opResType;
      } else {
        // We are modifying a typed target. The type has to be compatible.
        CanFlowResult cf = canAFlowIntoB(opResType, lt);
        if (!cf.canFlow) {
          outer_.sm_.error(
              node->getSourceRange(), "ft: incompatible assignment types");
          res = lt;
        } else if (cf.needCheckedCast) {
          // Insert an ImplicitCheckedCast around the LHS in the
          // AssignmentExpressionNode, because there's no other place in the AST
          // that indicates that we want to cast the result of the binary
          // expression.
          // IRGen is aware of this and handles it specially.
          node->_left = outer_.implicitCheckedCast(node->_left, lt, cf);
          res = lt;
        } else {
          // If we don't need a checked cast, opResType is possibly narrower
          // than lt, but never wider, so we want to use it as result.
          res = opResType;
        }
      }
    }
    outer_.setNodeType(node, res);
  }

  void visit(ESTree::ArrayPatternNode *node) {
    // For now, this just marks the array pattern (used on the LHS of assignment
    // expressions) as a tuple, so that it can be used with destructuring.
    // This isn't called from variable declaration nodes here, because
    // AnnotateScopeDecls handles variable declarations directly.
    // The tuple type is then read by, e.g., visit(AssignmentExpressionNode *),
    // which will use the tuple type to typecheck the assignment itself.
    // TODO: Determine how to destructure from arrays.

    // Annotate the children of the array pattern.
    visitESTreeChildren(*this, node);

    llvh::SmallVector<Type *, 4> types;
    for (ESTree::Node &elem : node->_elements) {
      types.push_back(outer_.getNodeTypeOrAny(&elem));
    }
    outer_.setNodeType(
        node,
        outer_.flowContext_.createType(
            outer_.flowContext_.createTuple(types), node));
  }

  void visit(ESTree::ConditionalExpressionNode *node) {
    visitESTreeChildren(*this, node);

    Type *types[2]{
        outer_.getNodeTypeOrAny(node->_consequent),
        outer_.getNodeTypeOrAny(node->_alternate)};

    // The result of a conditional is the union of the two types.
    outer_.setNodeType(
        node,
        outer_.flowContext_.createType(
            outer_.flowContext_.maybeCreateUnion(types)));
  }

  void visit(ESTree::TypeParameterInstantiationNode *node) {
    // Do nothing.
    // These are handled in the parent node.
  }

  void visit(ESTree::CallExpressionNode *node) {
    visitESTreeChildren(*this, node);

    // Check for $SHBuiltin.
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
      if (llvh::isa<ESTree::SHBuiltinNode>(methodCallee->_object)) {
        checkSHBuiltin(
            node, llvh::cast<ESTree::IdentifierNode>(methodCallee->_property));
        return;
      }
    }

    if (auto *identCallee =
            llvh::dyn_cast<ESTree::IdentifierNode>(node->_callee)) {
      sema::Decl *decl = outer_.getDecl(identCallee);
      if (decl->generic) {
        if (!node->_typeArguments) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: type arguments required");
          return;
        }

        outer_.resolveCallToGenericFunctionSpecialization(
            node, identCallee, decl);
      }
    } else if (node->_typeArguments) {
      // Generics handled above.
      outer_.sm_.error(
          node->_callee->getSourceRange(),
          "ft: generic call only works on identifiers");
      return;
    }

    Type *calleeType = outer_.getNodeTypeOrAny(node->_callee);
    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info))
      return;

    if (!llvh::isa<BaseFunctionType>(calleeType->info)) {
      outer_.sm_.error(
          node->_callee->getSourceRange(), "ft: callee is not a function");
      return;
    }

    // If the callee is an untyped function, we have nothing to check.
    if (llvh::isa<UntypedFunctionType>(calleeType->info)) {
      outer_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }

    Type *returnType;
    llvh::ArrayRef<TypedFunctionType::Param> params{};

    if (auto *ftype = llvh::dyn_cast<TypedFunctionType>(calleeType->info)) {
      returnType = ftype->getReturnType();
      params = ftype->getParams();

      Type *expectedThisType = ftype->getThisParam()
          ? ftype->getThisParam()
          : outer_.flowContext_.getAny();

      // Check the type of "this".
      if (auto *methodCallee =
              llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
        Type *thisArgType = nullptr;
        if (auto *superNode =
                llvh::dyn_cast<ESTree::SuperNode>(methodCallee->_object)) {
          // 'super' calls implicitly pass the current class as 'this'.
          if (!outer_.curClassContext_->classType) {
            outer_.sm_.error(
                node->_callee->getSourceRange(),
                "ft: 'super' call outside class");
            return;
          }
          thisArgType = outer_.curClassContext_->classType;
        } else {
          thisArgType = outer_.getNodeTypeOrAny(methodCallee->_object);
        }

        if (!canAFlowIntoB(thisArgType->info, expectedThisType->info).canFlow) {
          outer_.sm_.error(
              methodCallee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      } else if (
          auto *superNode = llvh::dyn_cast<ESTree::SuperNode>(node->_callee)) {
        // 'super' calls implicitly pass the current class as 'this'.
        if (!outer_.curClassContext_->classType) {
          outer_.sm_.error(
              node->_callee->getSourceRange(),
              "ft: 'super' call outside class");
          return;
        }
        if (!canAFlowIntoB(
                 outer_.curClassContext_->classType->info,
                 expectedThisType->info)
                 .canFlow) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      } else {
        if (!canAFlowIntoB(
                 outer_.flowContext_.getVoid()->info, expectedThisType->info)
                 .canFlow) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      }
    } else {
      auto *nftype = llvh::cast<NativeFunctionType>(calleeType->info);
      returnType = nftype->getReturnType();
      params = nftype->getParams();
    }

    outer_.setNodeType(node, returnType);
    checkArgumentTypes(params, node, node->_arguments, "function");
  }

  void checkSHBuiltin(
      ESTree::CallExpressionNode *call,
      ESTree::IdentifierNode *builtin) {
    if (builtin->_name == outer_.kw_.identCall) {
      checkSHBuiltinCall(call);
      return;
    }
    if (builtin->_name == outer_.kw_.identCNull) {
      checkSHBuiltinCNull(call);
      return;
    }
    if (builtin->_name == outer_.kw_.identCNativeRuntime) {
      checkSHBuiltinCNativeRuntime(call);
      return;
    }
    if (builtin->_name == outer_.kw_.identExternC) {
      checkSHBuiltinExternC(call);
      return;
    }

    outer_.sm_.error(call->getSourceRange(), "unknown SH builtin call");
  }

  /// $SHBuiltin.call(fn, this, arg1, ...)
  /// must typecheck as an actual function call.
  void checkSHBuiltinCall(ESTree::CallExpressionNode *call) {
    auto it = call->_arguments.begin();
    if (it == call->_arguments.end()) {
      outer_.sm_.error(
          call->getSourceRange(), "ft: call requires at least two arguments");
      return;
    }
    ESTree::Node *callee = &*it;
    Type *calleeType = outer_.getNodeTypeOrAny(callee);
    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info))
      return;
    if (!llvh::isa<BaseFunctionType>(calleeType->info)) {
      outer_.sm_.error(
          callee->getSourceRange(), "ft: callee is not a function");
      return;
    }
    if (llvh::isa<NativeFunctionType>(calleeType->info)) {
      outer_.sm_.error(
          callee->getSourceRange(),
          "ft: callee is a native function, cannot use $SHBuiltin.call");
      return;
    }
    auto *ftype = llvh::dyn_cast<TypedFunctionType>(calleeType->info);

    // If the callee is an untyped function, we have nothing to check.
    if (!ftype) {
      outer_.setNodeType(call, outer_.flowContext_.getAny());
      return;
    }

    outer_.setNodeType(call, ftype->getReturnType());

    ++it;
    if (it == call->_arguments.end()) {
      outer_.sm_.error(
          call->getSourceRange(), "ft: call requires at least two arguments");
      return;
    }
    Type *expectedThisType = ftype->getThisParam()
        ? ftype->getThisParam()
        : outer_.flowContext_.getAny();
    ESTree::Node *thisArg = &*it;
    Type *thisArgType = outer_.getNodeTypeOrAny(thisArg);
    if (!canAFlowIntoB(thisArgType->info, expectedThisType->info).canFlow) {
      outer_.sm_.error(thisArg->getSourceRange(), "ft: 'this' type mismatch");
      return;
    }

    checkArgumentTypes(
        ftype->getParams(), call, call->_arguments, "function", 2);
    return;
  }

  /// SHBuiltin.c_null().
  void checkSHBuiltinCNull(ESTree::CallExpressionNode *call) {
    // Check the number and types of arguments.
    if (call->_arguments.size() != 0) {
      outer_.sm_.error(call->getSourceRange(), "ft: c_null takes no arguments");
      return;
    }
    outer_.setNodeType(call, outer_.flowContext_.getCPtr());
  }

  /// SHBuiltin.c_native_runtime().
  void checkSHBuiltinCNativeRuntime(ESTree::CallExpressionNode *call) {
    // Check the number and types of arguments.
    if (call->_arguments.size() != 0) {
      outer_.sm_.error(
          call->getSourceRange(), "ft: c_native_runtime takes no arguments");
      return;
    }
    outer_.setNodeType(call, outer_.flowContext_.getCPtr());
  }

  /// $SHBuiltin.extern_c({options}, function name():result {...})
  void checkSHBuiltinExternC(ESTree::CallExpressionNode *call) {
    // Check the number and types of arguments.
    if (call->_arguments.size() != 2) {
      outer_.sm_.error(
          call->getSourceRange(),
          "ft: extern_c requires exactly two arguments");
      return;
    }

    // Check arg 1.
    auto arg = call->_arguments.begin();
    auto *options = llvh::dyn_cast<ESTree::ObjectExpressionNode>(&*arg);
    if (!options) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c requires an object literal as the first argument");
      return;
    }
    // Parse the options.
    bool declaredOption = false;
    UniqueString *includeOption = nullptr;
    bool allowHVOption = false;
    if (!parseExternCOptions(
            options, &declaredOption, &includeOption, &allowHVOption))
      return;

    // Check arg 2.
    UniqueString *name;
    Type *funcType;
    TypedFunctionType *funcInfo;

    ++arg;
    auto *func = llvh::dyn_cast<ESTree::FunctionExpressionNode>(&*arg);
    if (!func) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c requires a function as the second argument");
      return;
    }
    if (!func->_id) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c requires a named function as the second argument");
      return;
    }
    name = llvh::cast<ESTree::IdentifierNode>(func->_id)->_name;
    funcType = outer_.flowContext_.findNodeType(func);
    assert(funcType && "function expression type must be set");
    if (!llvh::isa<TypedFunctionType>(funcType->info)) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c requires a typed function as the second argument");
      return;
    }
    funcInfo = llvh::cast<TypedFunctionType>(funcType->info);
    if (funcInfo->isAsync() || funcInfo->isGenerator()) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c does not support async or generator functions");
      return;
    }
    if (funcInfo->getThisParam()) {
      outer_.sm_.error(
          arg->getSourceRange(),
          "ft: extern_c does not support 'this' parameters");
      return;
    }

    // Extract the function signature.
    NativeSignature *signature;

    if (!func->_returnType) {
      outer_.sm_.error(
          func->getSourceRange(),
          "ft: extern_c requires a return type annotation");
      return;
    }
    llvh::SmallVector<NativeCType, 4> natParamTypes{};
    NativeCType natReturnType;

    // The return type, where we allow void.
    if (llvh::isa<VoidType>(funcInfo->getReturnType()->info))
      natReturnType = NativeCType::c_void;
    else if (!parseNativeAnnotation(
                 llvh::cast<ESTree::TypeAnnotationNode>(func->_returnType),
                 allowHVOption,
                 &natReturnType))
      return;

    for (auto &node : func->_params) {
      auto *param = llvh::cast<ESTree::IdentifierNode>(&node);
      if (!param->_typeAnnotation) {
        outer_.sm_.error(
            param->getSourceRange(),
            "ft: extern_c requires type annotations for all parameters");
        return;
      }
      NativeCType natParamType;
      if (!parseNativeAnnotation(
              llvh::cast<ESTree::TypeAnnotationNode>(param->_typeAnnotation),
              false,
              &natParamType))
        return;
      natParamTypes.push_back(natParamType);
    }

    signature = outer_.astContext_.getNativeContext().getSignature(
        natReturnType, natParamTypes);

    // Now that we have the signature, declare the extern and check for invalid
    // redeclaration.
    NativeExtern *ne = outer_.astContext_.getNativeContext().getExtern(
        name, signature, call->getStartLoc(), declaredOption, includeOption);
    if (ne->signature() != signature) {
      outer_.sm_.error(
          call->getSourceRange(),
          "ft: invalid redeclaration of native extern '" + name->str() + "'");
      if (ne->loc().isValid()) {
        outer_.sm_.note(ne->loc(), "ft: original declaration here");
      }
      return;
    }

    TypeInfo *nativeFuncInfo = outer_.flowContext_.createNativeFunction(
        funcInfo->getReturnType(), funcInfo->getParams(), signature);

    outer_.setNodeType(call, outer_.flowContext_.createType(nativeFuncInfo));
  }

  /// Extract the options from the options object literal. On error print an
  /// an error message and return false.
  bool parseExternCOptions(
      ESTree::ObjectExpressionNode *options,
      bool *declaredOption,
      UniqueString **includeOption,
      bool *allowHVOption) {
    *declaredOption = false;
    *includeOption = nullptr;

    auto parseObjRes = parseExternCObjectLiteral(options);
    if (!parseObjRes)
      return false;
    auto &map = *parseObjRes;
    bool success = true;

    // NOTE: Whenever we find a supported option, we erase it.

    // Parse an option of a specified literal type. On error print an error
    // message and clear the success flag.
    // \param lit The literal type to parse. The passed value is ignored, only
    //    the type matters.
    // \param typeName The name of the type to print in the error message.
    // \param optionName The name of the option to parse.
    // \param res Output parameter for the parsed value.
    auto parseOption = [&map, &success, this](
                           auto *lit,
                           llvh::StringLiteral typeName,
                           llvh::StringLiteral optionName,
                           auto *res) {
      using LitType = std::remove_pointer_t<decltype(lit)>;
      auto it = map.find(
          outer_.astContext_.getIdentifier(optionName).getUnderlyingPointer());
      if (it != map.end()) {
        lit = llvh::dyn_cast<LitType>(it->second->_value);
        if (lit) {
          *res = lit->_value;
        } else {
          outer_.sm_.error(
              it->second->getSourceRange(),
              "ft: extern_c option '" + optionName + "' must be a " + typeName +
                  " literal");
          success = false;
        }
        map.erase(it);
      }
    };

    auto parseString = [&parseOption](
                           llvh::StringLiteral optionName, UniqueString **res) {
      parseOption(
          (ESTree::StringLiteralNode *)nullptr, "string", optionName, res);
    };
    auto parseBool = [&parseOption](llvh::StringLiteral optionName, bool *res) {
      parseOption(
          (ESTree::BooleanLiteralNode *)nullptr, "boolean", optionName, res);
    };

    parseBool("declared", declaredOption);
    parseBool("hv", allowHVOption);
    parseString("include", includeOption);

    // Check for unsupported properties.
    for (auto &prop : map) {
      outer_.sm_.error(
          prop.second->getSourceRange(),
          "ft: extern_c does not support option '" + prop.first->str() + "'");
      success = false;
    }

    return success;
  }

  /// Helper to parse the options object literal used in extern_c. Ensures that
  /// only "normal" properties are present, and that the values are literals or
  /// object literals. On error prints an error message and returns None.
  /// On success returns a map of property names to PropertyNodes. The caller
  /// can quickly scan the names. It can also use the same function to scan
  /// nested object literals.
  ///
  /// \return None on error, otherwise a map of property names to
  ///     EStree::PropertyNode.
  llvh::Optional<
      llvh::SmallMapVector<UniqueString *, ESTree::PropertyNode *, 4>>
  parseExternCObjectLiteral(ESTree::ObjectExpressionNode *objLitNode) {
    llvh::SmallMapVector<UniqueString *, ESTree::PropertyNode *, 4> res{};

    for (ESTree::Node &n : objLitNode->_properties) {
      // The dyn_cast could perhaps be a cast, but just to be safe.
      auto *prop = llvh::dyn_cast<ESTree::PropertyNode>(&n);
      if (!prop || prop->_kind != outer_.kw_.identInit || prop->_computed ||
          prop->_method || prop->_shorthand) {
        outer_.sm_.error(
            n.getSourceRange(), "ft: extern_c: unsupported property format");
        return llvh::None;
      }

      // Check that the value is a literal, another object, or an array.
      auto *value = prop->_value;
      if (!(llvh::isa<ESTree::NullLiteralNode>(value) ||
            llvh::isa<ESTree::BooleanLiteralNode>(value) ||
            llvh::isa<ESTree::StringLiteralNode>(value) ||
            llvh::isa<ESTree::NumericLiteralNode>(value) ||
            llvh::isa<ESTree::BigIntLiteralNode>(value) ||
            llvh::isa<ESTree::ObjectExpressionNode>(value))) {
        outer_.sm_.error(
            value->getSourceRange(), "ft: extern_c: unsupported property type");
        return llvh::None;
      }

      // Note that we don't care about duplicates, we just want to use the last
      // one.
      res[llvh::cast<ESTree::IdentifierNode>(prop->_key)->_name] = prop;
    }

    // Note that we have to use std::move() since we are returning an optional.
    return std::move(res);
  }

  /// Parse a native type annotation. Return true on success and store the
  /// value in \p res. On error print an error message and return false.
  ///
  /// \param node The node to parse.
  /// \param allowRegular If true, allow type annotations that are not native
  ///     types.
  /// \param res Output parameter for the parsed type.
  bool parseNativeAnnotation(
      ESTree::TypeAnnotationNode *node,
      bool allowRegular,
      NativeCType *res) {
    *res = NativeCType::c_hermes_value;
    auto *ann = llvh::dyn_cast<ESTree::GenericTypeAnnotationNode>(
        node->_typeAnnotation);
    if (!ann || ann->_typeParameters) {
      if (allowRegular)
        return true;
      outer_.sm_.error(
          node->getSourceRange(), "ft: unsupported native type annotation");
      return false;
    }
    auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(ann->_id);
    if (!id) {
      if (allowRegular)
        return true;
      outer_.sm_.error(
          node->getSourceRange(), "ft: unsupported native type annotation");
      return false;
    }
    UniqueString *name = id->_name;
    auto it = outer_.nativeTypes_.find(name);
    if (it == outer_.nativeTypes_.end()) {
      if (allowRegular)
        return true;
      outer_.sm_.error(
          ann->_id->getSourceRange(),
          "ft: '" + name->str() + "' is not a native type");
      return false;
    }
    *res = it->second;
    return true;
  };

  void visit(ESTree::OptionalCallExpressionNode *node) {
    outer_.sm_.error(
        node->getSourceRange(), "ft: optional call expression not supported");
  }

  void visit(ESTree::NewExpressionNode *node) {
    visitESTreeChildren(*this, node);

    // Resolve generics using type arguments if necessary.
    if (auto *identCallee =
            llvh::dyn_cast<ESTree::IdentifierNode>(node->_callee)) {
      sema::Decl *decl = outer_.getDecl(identCallee);
      if (decl->generic) {
        if (!node->_typeArguments) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: type arguments required");
          return;
        }

        outer_.resolveGenericClassSpecialization(
            identCallee,
            llvh::cast<ESTree::TypeParameterInstantiationNode>(
                node->_typeArguments),
            decl);
      }
    } else if (node->_typeArguments) {
      // Generics handled above.
      outer_.sm_.error(
          node->_callee->getSourceRange(),
          "ft: generic call only works on identifiers");
      return;
    }

    Type *calleeType = outer_.getNodeTypeOrAny(node->_callee);
    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info))
      return;

    if (!llvh::isa<ClassConstructorType>(calleeType->info)) {
      outer_.sm_.error(
          node->_callee->getSourceRange(),
          "ft: callee is not a class constructor");
      return;
    }
    auto *classConsType = llvh::cast<ClassConstructorType>(calleeType->info);
    Type *classType = classConsType->getClassType();
    ClassType *classTypeInfo = classConsType->getClassTypeInfo();

    outer_.setNodeType(node, classType);

    // Does the class have an explicit constructor?
    if (Type *consFType = classTypeInfo->getConstructorType()) {
      checkArgumentTypes(
          llvh::cast<TypedFunctionType>(consFType->info)->getParams(),
          node,
          node->_arguments,
          "class " + classTypeInfo->getClassNameOrDefault() + " constructor");
    } else {
      if (!node->_arguments.empty()) {
        outer_.sm_.error(
            node->getSourceRange(),
            "ft: class " + classTypeInfo->getClassNameOrDefault() +
                " does not have an explicit constructor");
        return;
      }
    }
  }

  void visit(ESTree::SuperNode *node, ESTree::Node *parent) {
    if (!outer_.curClassContext_) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: super only supported in class");
      return;
    }

    // Check that the super call is valid.
    ClassType *curClassType = outer_.curClassContext_->getClassTypeInfo();
    ClassType *superClassType = curClassType->getSuperClassInfo();
    if (!superClassType) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: super requires a base class");
      return;
    }

    if (llvh::isa<ESTree::CallExpressionNode>(parent)) {
      // super() call calls the constructor of the super class.
      outer_.setNodeType(node, superClassType->getConstructorType());
    } else if (llvh::isa<ESTree::MemberExpressionNode>(parent)) {
      // super.property lookup is on the super class.
      outer_.setNodeType(node, curClassType->getSuperClass());
    } else {
      outer_.sm_.error(node->getSourceRange(), "ft: invalid usage of super");
      outer_.flowContext_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }
  }

  /// Check the types of the supplies arguments, adding checked casts if needed.
  /// \param offset the number of arguments to ignore at the front of \p
  ///   arguments. Used for $SHBuiltin.call, which has extra args at the front.
  bool checkArgumentTypes(
      llvh::ArrayRef<TypedFunctionType::Param> params,
      ESTree::Node *callNode,
      ESTree::NodeList &arguments,
      const llvh::Twine &calleeName,
      uint32_t offset = 0) {
    size_t numArgs = arguments.size() - offset;
    // FIXME: default arguments.
    if (params.size() != numArgs) {
      outer_.sm_.error(
          callNode->getSourceRange(),
          "ft: " + calleeName + " expects " + llvh::Twine(params.size()) +
              " arguments, but " + llvh::Twine(numArgs) + " supplied");
      return false;
    }

    auto begin = arguments.begin();
    std::advance(begin, offset);

    // Check the type of each argument.
    size_t argIndex = 0;
    for (auto it = begin, e = arguments.end(); it != e; ++argIndex, ++it) {
      ESTree::Node *arg = &*it;

      if (llvh::isa<ESTree::SpreadElementNode>(arg)) {
        outer_.sm_.error(
            arg->getSourceRange(), "ft: argument spread is not supported");
        return false;
      }

      const TypedFunctionType::Param &param = params[argIndex];
      Type *expectedType = param.second;
      Type *argType = outer_.getNodeTypeOrAny(arg);
      auto [argTypeNarrow, cf] = tryNarrowType(argType, expectedType);

      if (!cf.canFlow) {
        outer_.sm_.error(
            arg->getSourceRange(),
            "ft: " + calleeName + " parameter '" + param.first.str() +
                "' type mismatch");
        return false;
      }
      // If a cast is needed, replace the argument with the cast.
      if (cf.needCheckedCast && outer_.compile_) {
        // Insert the new node before the current node and erase the current
        // one.
        auto newIt = arguments.insert(
            it, *outer_.implicitCheckedCast(arg, argTypeNarrow, cf));
        arguments.erase(it);
        it = newIt;
      }
    }

    return true;
  }
};

void FlowChecker::visitExpression(ESTree::Node *node, ESTree::Node *parent) {
  ExprVisitor v(*this);
  visitESTreeNode(v, node, parent);
}

void FlowChecker::visit(ESTree::ExpressionStatementNode *node) {
  visitExpression(node->_expression, node);
}

void FlowChecker::visit(ESTree::IfStatementNode *node) {
  visitExpression(node->_test, node);
  visitESTreeNode(*this, node->_consequent, node);
  visitESTreeNode(*this, node->_alternate, node);
}
void FlowChecker::visit(ESTree::SwitchStatementNode *node) {
  visitExpression(node->_discriminant, node);
  visitESTreeNodeList(*this, node->_cases, node);
}
void FlowChecker::visit(ESTree::SwitchCaseNode *node) {
  visitExpression(node->_test, node);
  visitESTreeNodeList(*this, node->_consequent, node);
}
void FlowChecker::visit(ESTree::WhileStatementNode *node) {
  visitExpression(node->_test, node);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::DoWhileStatementNode *node) {
  visitESTreeNode(*this, node->_body, node);
  visitExpression(node->_test, node);
}
void FlowChecker::visit(ESTree::ForOfStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (llvh::isa<ESTree::VariableDeclarationNode>(node->_left))
    visitESTreeNode(*this, node->_left, node);
  else
    visitExpression(node->_left, node);
  visitExpression(node->_right, node);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::ForInStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (llvh::isa<ESTree::VariableDeclarationNode>(node->_left))
    visitESTreeNode(*this, node->_left, node);
  else
    visitExpression(node->_left, node);
  visitExpression(node->_right, node);
  visitESTreeNode(*this, node->_body, node);
}
void FlowChecker::visit(ESTree::ForStatementNode *node) {
  if (!resolveScopeTypesAndAnnotate(node, node->getScope()))
    return;
  if (node->_init) {
    if (llvh::isa<ESTree::VariableDeclarationNode>(node->_init))
      visitESTreeNode(*this, node->_init, node);
    else
      visitExpression(node->_init, node);
  }
  visitExpression(node->_test, node);
  visitExpression(node->_update, node);
  visitESTreeNode(*this, node->_body, node);
}

void FlowChecker::visit(ESTree::ReturnStatementNode *node) {
  // TODO: type check the return value.
  visitExpression(node->_argument, node);

  auto *ftype = llvh::dyn_cast<TypedFunctionType>(
      curFunctionContext_->functionType->info);

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
    if (it != visitedInits_.end())
      visitedInits_.erase(it);
    else
      visitExpression(declarator->_init, declarator);
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

/// Forward declaration information for generic type instantiations in
/// aliases.
class FlowChecker::GenericTypeInstantiation {
 public:
  /// The generic TypeDecl.
  TypeDecl *typeDecl;
  /// The generic type annotation node.
  ESTree::GenericTypeAnnotationNode *annotation;
  /// The type arguments provided to the generic type annotation, may contain
  /// incomplete unions or other forward-declared generics.
  llvh::SmallVector<Type *, 2> typeArgTypes{};
  GenericTypeInstantiation(
      TypeDecl *typeDecl,
      ESTree::GenericTypeAnnotationNode *annotation)
      : typeDecl(typeDecl), annotation(annotation) {}
};

class FlowChecker::FindLoopingTypes {
  /// Output set to store the looping types in.
  llvh::SmallDenseSet<Type *> &loopingTypes;

  /// Map from generic type annotations to their instantiations.
  const llvh::MapVector<Type *, GenericTypeInstantiation>
      &forwardGenericInstantiations;

  /// Visited set for the types so far.
  /// Use a SetVector so it can be quickly iterated to copy the types into
  /// loopingTypes set when a looping type is found.
  llvh::SmallSetVector<Type *, 4> visited{};

 public:
  /// Finds looping types reachable from \p type.
  /// \param loopingTypes[in/out] set to populate with looping types.
  FindLoopingTypes(
      llvh::SmallDenseSet<Type *> &loopingTypes,
      const llvh::MapVector<Type *, GenericTypeInstantiation>
          &forwardGenericInstantiations,
      Type *type)
      : loopingTypes(loopingTypes),
        forwardGenericInstantiations(forwardGenericInstantiations) {
    isTypeLooping(type);
  }

 private:
  bool isTypeLooping(Type *type) {
    bool inserted = visited.insert(type);
    if (!inserted) {
      // Found a cycle.
      // Copy all visited types to loopingTypes.
      loopingTypes.insert(visited.begin(), visited.end());
      return true;
    }

    auto popOnExit = llvh::make_scope_exit([this]() { visited.pop_back(); });

    if (loopingTypes.count(type)) {
      // Found a type that's already known to be looping.
      // Copy all visited types to loopingTypes.
      // This is necessary because we might have taken another path to end at
      // the known looping type, so we have to insert everything along that
      // second path.
      loopingTypes.insert(visited.begin(), visited.end());
      return true;
    }

    switch (type->info->getKind()) {
#define _HERMES_SEMA_FLOW_DEFKIND(name)                         \
  case TypeKind::name:                                          \
    return isLooping(type, llvh::cast<name##Type>(type->info)); \
    break;
      _HERMES_SEMA_FLOW_SINGLETONS _HERMES_SEMA_FLOW_COMPLEX_TYPES
#undef _HERMES_SEMA_FLOW_DEFKIND
    }
  }

  bool isLooping(Type *, SingletonType *) {
    // Nothing to do for singleton types.
    return false;
  }

  /// GenericType needs special handling because it's not a singleton,
  /// but has a list of type arguments that could be looping.
  bool isLooping(Type *type, GenericType *) {
    auto it = forwardGenericInstantiations.find(type);
    assert(it != forwardGenericInstantiations.end() && "can't find generic");

    bool result = false;
    for (Type *t : it->second.typeArgTypes) {
      if (isTypeLooping(t)) {
        result = true;
      }
    }
    return result;
  }

  bool isLooping(Type *, TypeWithId *type) {
    // Nominal type. Stop checking for recursion.
    return false;
  }

  bool isLooping(Type *, UnionType *type) {
    bool result = false;
    for (Type *t : type->getTypes()) {
      if (isTypeLooping(t)) {
        // Don't return here, have to run on all the union arms
        // so that if there's multiple looping arms they get registered.
        result = true;
      }
    }
    return result;
  }

  bool isLooping(Type *, ArrayType *type) {
    return isTypeLooping(type->getElement());
  }

  bool isLooping(Type *, TupleType *type) {
    bool result = false;
    for (Type *t : type->getTypes()) {
      if (isTypeLooping(t)) {
        // Don't return here, have to run on all types so that if there's
        // multiple looping union arms they get registered.
        result = true;
      }
    }
    return result;
  }

  bool isLooping(Type *, TypedFunctionType *type) {
    bool result = false;
    result |= isTypeLooping(type->getReturnType());
    if (type->getThisParam()) {
      result |= isTypeLooping(type->getThisParam());
    }
    for (const auto &[name, paramType] : type->getParams()) {
      result |= isTypeLooping(paramType);
    }
    return result;
  }

  bool isLooping(Type *, NativeFunctionType *type) {
    bool result = false;
    result |= isTypeLooping(type->getReturnType());
    for (const auto &[name, paramType] : type->getParams()) {
      result |= isTypeLooping(paramType);
    }
    return result;
  }

  bool isLooping(Type *, UntypedFunctionType *type) {
    return false;
  }
};

/// Type aliases combined with unions create a **dramatic complication**, since
/// they can be mutually self recursive. We need to declare types in stages,
/// first the "direct" ones, then resolve the aliases, unions, and generics.
///
/// 1. Iterate all scope types. Forward-declare all declared types as Type *.
/// 2. Resolve the aliases, checking for self-references.
/// 3. Complete all forward declared types. They can now refer to the newly
/// declared local types.
class FlowChecker::DeclareScopeTypes {
  /// Surrounding class.
  FlowChecker &outer;
  /// Declarations collected by the semantic validator.
  const sema::ScopeDecls &decls;
  /// The current lexical scope.
  sema::LexicalScope *const scope;
  /// The current lexical scope.
  ESTree::Node *const scopeNode;
  /// Type aliases declared in this scope.
  llvh::SmallVector<Type *, 4> localTypeAliases{};
  /// Mapping from the LHS to the RHS type of a Type alias.
  /// e.g. type A = B;
  /// maps from the Type representing 'A' to the type representing 'B'.
  /// Used for populating the LHS after resolving the RHS with
  /// resolveTypeAnnotation.
  llvh::SmallDenseMap<Type *, Type *> typeAliasResolutions{};
  /// Keep track of the generic instantiations created during type alias
  /// resolution, because they need to know the TypeInfo for the type aliases
  /// to decide whether to make a new specialization.
  llvh::MapVector<Type *, GenericTypeInstantiation>
      forwardGenericInstantiations{};
  /// Keep track of all forward declarations of classes, so they can be
  /// completed.
  llvh::SmallVector<Type *, 4> forwardClassDecls{};
  /// Keep track of all union types, so they can be canonicalized.
  llvh::SmallSetVector<Type *, 4> forwardUnions{};
  /// All the recursive union arms, which need to be canonicalized and uniqued
  /// independently of the non-recursive arms.
  llvh::SmallDenseSet<Type *> loopingUnionArms{};

  /// The generic specializations to be parsed after the class body is parsed.
  /// These have to be deferred here instead of in DeclareScopeTypes because
  /// we have to not defer parsing the superClass, e.g.
  std::vector<DeferredGenericClass> deferredParseGenerics{};

 public:
  DeclareScopeTypes(
      FlowChecker &outer,
      const sema::ScopeDecls &decls,
      sema::LexicalScope *scope,
      ESTree::Node *scopeNode)
      : outer(outer), decls(decls), scope(scope), scopeNode(scopeNode) {
    llvh::SaveAndRestore savedDeferredGenerics{
        outer.deferredParseGenerics_, &deferredParseGenerics};

    createForwardDeclarations();
    resolveAllAliases();
    completeForwardDeclarations();
    parseDeferredGenericClasses();
  }

 private:
  // Check if a type declaration with the specified name exists in the current
  // scope. If it exists, generate an error and return true.
  // \return true if this is a redeclaration (i.e. it is an error).
  bool isRedeclaration(ESTree::IdentifierNode *id) const {
    UniqueString *name = id->_name;
    TypeDecl *typeDecl = outer.bindingTable_.findInCurrentScope(name);
    if (!typeDecl)
      return false;

    outer.sm_.error(
        id->getStartLoc(),
        "ft: type " + name->str() + " already declared in this scope");
    if (typeDecl->astNode && typeDecl->astNode->getSourceRange().isValid()) {
      outer.sm_.note(
          typeDecl->astNode->getSourceRange(),
          "ft: previous declaration of " + name->str());
    }
    return true;
  };

  /// \return the parent of the generic (based on the scopeNode),
  ///   in which to insert new generic specializations.
  ESTree::Node *getGenericParentNode() const {
    if (auto *funcNode = llvh::dyn_cast<ESTree::FunctionLikeNode>(scopeNode)) {
      return ESTree::getBlockStatement(funcNode);
    }
    return scopeNode;
  }

  /// Forward declare all classes and record all aliases for later processing.
  void createForwardDeclarations() {
    for (ESTree::Node *declNode : decls) {
      if (llvh::isa<ESTree::VariableDeclarationNode>(declNode) ||
          llvh::isa<ESTree::ImportDeclarationNode>(declNode) ||
          llvh::isa<ESTree::CatchClauseNode>(declNode) ||
          llvh::isa<ESTree::FunctionDeclarationNode>(declNode)) {
        continue;
      }
      if (auto *classNode =
              llvh::dyn_cast<ESTree::ClassDeclarationNode>(declNode)) {
        // Class declaration.
        //
        auto *id = llvh::cast<ESTree::IdentifierNode>(classNode->_id);
        if (isRedeclaration(id))
          continue;

        sema::Decl *decl = outer.getDecl(id);
        decl->generic = classNode->_typeParameters != nullptr;

        if (decl->generic) {
          // Avoid visiting the body of the class until we have an instance.
          outer.bindingTable_.try_emplace(
              id->_name, TypeDecl(nullptr, scope, declNode, decl));

          outer.registerGeneric(
              decl,
              classNode,
              getGenericParentNode(),
              outer.bindingTable_.getCurrentScope());
          continue;
        }

        Type *newType = outer.flowContext_.createType(
            outer.flowContext_.createClass(
                Identifier::getFromPointer(id->_name)),
            classNode);
        forwardClassDecls.push_back(newType);

        outer.bindingTable_.try_emplace(
            id->_name, TypeDecl(newType, scope, declNode));

        bool success = outer.recordDecl(
            decl,
            outer.flowContext_.createType(
                outer.flowContext_.createClassConstructor(newType), classNode),
            id,
            classNode);
        assert(success && "class constructor unexpectedly re-declared");
        (void)success;
      } else if (
          auto *aliasNode = llvh::dyn_cast<ESTree::TypeAliasNode>(declNode)) {
        // Type alias.
        //
        auto *id = llvh::cast<ESTree::IdentifierNode>(aliasNode->_id);
        if (isRedeclaration(id))
          continue;
        Type *newType = outer.flowContext_.createType(declNode);
        localTypeAliases.push_back(newType);
        outer.bindingTable_.try_emplace(
            id->_name, TypeDecl(newType, scope, declNode));
      } else {
        outer.sm_.error(
            declNode->getSourceRange(),
            "ft: unsupported type declaration " + declNode->getNodeName());
      }
    }
  }

  /// Resolve all recorded aliases. At the end of this all local types should
  /// resolve to something: a primary type, a type in a surrounding scope, a
  /// local forward declared class, or a union of any of these.
  void resolveAllAliases() {
    for (Type *localType : localTypeAliases) {
      // Skip already resolved types.
      if (localType->info)
        continue;

      // If it's not resolved already it must be an alias.
      auto *aliasNode = llvh::cast<ESTree::TypeAliasNode>(localType->node);

      // Recursion can occur through generic annotations and name aliases.
      // Keep track of visited aliases to detect it.
      llvh::SmallDenseSet<ESTree::TypeAliasNode *> visited{};
      visited.insert(aliasNode);

      // Copy the resolved TypeInfo to the alias.
      // The alias Type is different than the Type on the right side.
      Type *resolvedType = resolveTypeAnnotation(aliasNode->_right, visited, 0);
      typeAliasResolutions[localType] = resolvedType;
    }

    // Transfer TypeInfo from the resolved Type to the alias Type.
    // This has to be done in a second pass because resolved Types
    // might still have nullptr TypeInfos until the first pass completes.
    for (Type *localType : localTypeAliases) {
      if (!localType->info) {
        populateTypeAlias(localType);
        assert(localType->info && "populateTypeAlias should populate the info");
      }

      // If it's a union, we'll have to resolve it as well.
      // Unions can turn into single types if they only contain 1 unique type.
      if (llvh::isa<UnionType>(localType->info)) {
        forwardUnions.insert(localType);
      }

      // If it's a generic, we'll have to resolve it as well.
      // Forward declared generics will have their true types instantiated
      // later.
      if (llvh::isa<GenericType>(localType->info)) {
        auto it =
            forwardGenericInstantiations.find(typeAliasResolutions[localType]);
        assert(it != forwardGenericInstantiations.end());
        GenericTypeInstantiation copy = it->second;
        forwardGenericInstantiations.insert({localType, copy});
      }
    }
  }

  /// Resolve a type annotation in the current scope. This assumes that all
  /// classes have been declared and all directly resolvable aliases have been
  /// resolved. It deals with the remaining cases:
  /// - a "generic" annotation which refers to another type by name. This can
  ///     lead to self-recursion. If the alias is local, it is resolved.
  /// - a primary type
  /// - a constructor type like array, which is forward declared and resolved
  /// - a union of any of the above.
  ///
  /// \param annotation the type annotation to resolve
  /// \param visited the set of visited nodes when resolving "generic"
  ///     annotations. Used to check self-recursion.
  /// \param depth track depth to avoid stack overflow.
  ///
  /// \return the resolved type.
  Type *resolveTypeAnnotation(
      ESTree::Node *annotation,
      llvh::SmallDenseSet<ESTree::TypeAliasNode *> &visited,
      unsigned depth) {
    /// Avoid stack overflow.
    if (++depth >= 32) {
      outer.sm_.error(
          annotation->getSourceRange(), "ft: too deeply nested aliases/unions");
      return outer.flowContext_.getAny();
    }

    /// Generic annotation. This annotation refers to another type by name.
    /// That type may be a constructor type (class, interface) or it could
    /// recursively alias to another generic annotation or a union.
    if (auto *gta =
            llvh::dyn_cast<ESTree::GenericTypeAnnotationNode>(annotation)) {
      auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(gta->_id);

      if (!id) {
        outer.sm_.error(
            gta->getSourceRange(), "ft: unsupported type annotation");
        return outer.flowContext_.getAny();
      }

      // Is it declared anywhere?
      // If so, find its innermost declaration.
      TypeDecl *typeDecl = outer.bindingTable_.find(id->_name);
      if (!typeDecl) {
        // Not declared anywhere!
        outer.sm_.error(
            id->getStartLoc(), "ft: undefined type " + id->_name->str());
        return outer.flowContext_.getAny();
      }

      if (typeDecl->genericClassDecl) {
        // Resolve to the specialized class.
        if (!gta->_typeParameters) {
          outer.sm_.error(
              gta->getSourceRange(),
              llvh::Twine("ft: missing generic arguments for '") +
                  id->_name->str() + "'");
          return outer.flowContext_.getAny();
        }

        // Make a placeholder type to represent the generic type for now.
        // The TypeInfo will be replaced with the concrete ClassType when we are
        // able to resolve TypeInfo for all type arguments.
        Type *type = outer.flowContext_.createType(
            outer.flowContext_.getGenericInfo(), annotation);
        GenericTypeInstantiation instantiation{typeDecl, gta};

        // Populate the type arg Types.
        ESTree::NodeList &args =
            llvh::cast<ESTree::TypeParameterInstantiationNode>(
                gta->_typeParameters)
                ->_params;
        for (ESTree::Node &node : args)
          instantiation.typeArgTypes.push_back(
              resolveTypeAnnotation(&node, visited, depth));

        // Add the instantiation to the list of forward declarations,
        // so that we can resolve them later.
        forwardGenericInstantiations.insert({type, std::move(instantiation)});
        return type;
      }

      // No need to recurse here because any references to this name will
      // correctly resolve to the forward-declared Type.
      assert(typeDecl->type && "all types are populated at fwd declaration");
      return typeDecl->type;
    }

    /// Union types require resolving every union "arm".
    if (auto *uta =
            llvh::dyn_cast<ESTree::UnionTypeAnnotationNode>(annotation)) {
      llvh::SmallVector<Type *, 4> types{};
      for (ESTree::Node &node : uta->_types)
        types.push_back(resolveTypeAnnotation(&node, visited, depth));
      // Make a non-canonicalized UnionType to be resolved with the rest of the
      // forwardUnions later.
      Type *result = outer.flowContext_.createType(
          outer.flowContext_.createNonCanonicalizedUnion(std::move(types)),
          annotation);
      forwardUnions.insert(result);
      return result;
    }

    /// A nullable annotation is a simple case of a union.
    if (auto *nta =
            llvh::dyn_cast<ESTree::NullableTypeAnnotationNode>(annotation)) {
      Type *result = outer.flowContext_.createType(
          outer.flowContext_.createNonCanonicalizedUnion({
              outer.flowContext_.getVoid(),
              outer.flowContext_.getNull(),
              resolveTypeAnnotation(nta->_typeAnnotation, visited, depth),
          }),
          annotation);
      forwardUnions.insert(result);
      return result;
    }

    // Array types require resolving the array element.
    if (auto *arr =
            llvh::dyn_cast<ESTree::ArrayTypeAnnotationNode>(annotation)) {
      return outer.flowContext_.createType(
          outer.flowContext_.createArray(
              resolveTypeAnnotation(arr->_elementType, visited, depth)),
          annotation);
    }

    if (auto *func =
            llvh::dyn_cast<ESTree::FunctionTypeAnnotationNode>(annotation)) {
      return outer.processFunctionTypeAnnotation(
          func, [this, &visited, depth](ESTree::Node *annotation) {
            return resolveTypeAnnotation(annotation, visited, depth);
          });
    }

    // The specified AST node represents a nominal type, so return the type.
    return outer.parseTypeAnnotation(annotation);
  }

  /// Populate the TypeInfo of \p aliasType with its corresponding resolvedType,
  /// based on the information in the typeAliases map.
  /// Detects reference cycles using \p visited, and reports an error and sets
  /// 'any' when a cycle is detected.
  /// Populates TypeInfo for all aliases along an alias chain to avoid
  /// repeating all the lookups.
  /// \post aliasType->info is non-null.
  void populateTypeAlias(Type *aliasType) {
    if (aliasType->info)
      return;

    // Recursion can occur through generic annotations and name aliases.
    // Keep track of visited aliases to detect it.
    llvh::SmallSetVector<Type *, 4> visited{};

    // Resolved TypeInfo to assign to all aliases in the chain.
    TypeInfo *resolvedInfo = nullptr;

    Type *curType = aliasType;
    visited.insert(curType);
    while (!resolvedInfo) {
      // Find the resolved type via map lookup.
      auto it = typeAliasResolutions.find(curType);
      assert(
          it != typeAliasResolutions.end() &&
          "all type aliases have a resolved type");
      Type *nextType = it->second;

      bool inserted = visited.insert(nextType);
      if (!inserted) {
        // Found a cycle.
        outer.sm_.error(
            nextType->node->getStartLoc(),
            "ft: type contains a circular reference to itself");
        // Set the info to 'any' to make it non-null and allow the checker to
        // continue.
        resolvedInfo = outer.flowContext_.getAnyInfo();
        break;
      }

      // Continue down the alias chain.
      resolvedInfo = nextType->info;
      curType = nextType;
    }

    // Set the info for all the Types in the chain.
    for (Type *t : visited) {
      t->info = resolvedInfo;
    }
  }

  /// All types declared in the scope have been resolved at the first level.
  /// Resolve the remaining forward declared types and canonicalize forward
  /// unions as much as possible.
  void completeForwardDeclarations() {
    // Complete all forward-declared unions.
    // First find all the recursive union arms.
    for (Type *type : forwardUnions) {
      FindLoopingTypes(loopingUnionArms, forwardGenericInstantiations, type);
    }

    // Now simplify and canonicalize as many union arms as possible.
    for (Type *type : forwardUnions) {
      llvh::SetVector<Type *> visited{};
      completeForwardType(type, visited);
    }
    // Now simplify and canonicalize the foward generics.
    for (auto &[type, generic] : forwardGenericInstantiations) {
      llvh::SetVector<Type *> visited{};
      completeForwardGeneric(type, generic, visited);
    }

    // Parse all forward-declared class types.
    for (Type *type : forwardClassDecls) {
      // This is necessary because we need to defer parsing the class to allow
      // using types defined after the class inside the class:
      //     class C {
      //       x: D
      //     };
      //     type D = number;
      auto *classNode = llvh::cast<ESTree::ClassDeclarationNode>(type->node);
      outer.visitExpression(classNode->_superClass, classNode);
      ParseClassType(
          outer,
          classNode->_superClass,
          classNode->_superTypeParameters,
          classNode->_body,
          type);
    }
  }

  /// Complete the forward declaration of the given \p type,
  /// replacing its \c info field with the resolved type.
  void completeForwardType(Type *type, llvh::SetVector<Type *> &visited) {
    if (llvh::isa<GenericType>(type->info)) {
      auto it = forwardGenericInstantiations.find(type);
      assert(it != forwardGenericInstantiations.end());
      completeForwardGeneric(type, it->second, visited);
      return;
    }

    if (auto *unionType = llvh::dyn_cast<UnionType>(type->info)) {
      completeForwardUnion(type, unionType, visited);
      return;
    }

    // Nothing to do.
    return;
  }

  /// DFS through unions starting at \p type so they get canonicalized in the
  /// right order when possible.
  /// After this function completes, the \c info field of \p type will contain
  /// either a UnionType with canonicalized arms or a single type (if everything
  /// has been deduplicated).
  /// Mutually recursive with completeForwardType and completeForwardGeneric.
  void completeForwardUnion(
      Type *type,
      UnionType *unionType,
      llvh::SetVector<Type *> &visited) {
    assert(unionType && "Expected a union");
    if (unionType->getNumNonLoopingTypes() >= 0) {
      // Already been completed.
      return;
    }

    if (!visited.insert(type)) {
      // Already attempting to complete this type,
      // but hit a cycle on this branch.
      outer.sm_.error(
          type->node->getSourceRange(),
          "ft: type contains a circular reference to itself");
      type->info = outer.flowContext_.getAnyInfo();
      return;
    }

    auto popOnExit =
        llvh::make_scope_exit([&visited]() { visited.pop_back(); });

    llvh::SmallVector<Type *, 4> nonLoopingTypes{};
    llvh::SmallVector<Type *, 4> loopingTypes{};

    for (Type *unionArm : unionType->getTypes()) {
      // Union contains a union, complete it so it can be flattened.
      // Or it's a forward-declared generic.
      if (forwardUnions.count(unionArm) ||
          llvh::isa<GenericType>(unionArm->info)) {
        completeForwardType(unionArm, visited);
      }

      // Looping arms are separated for slow uniquing.
      if (loopingUnionArms.count(unionArm)) {
        loopingTypes.push_back(unionArm);
        continue;
      }

      // We know now that this arm is non-looping.
      if (auto *unionArmInfo = llvh::dyn_cast<UnionType>(unionArm->info)) {
        // Flatten nested unions.
        for (Type *nestedElem : unionArmInfo->getNonLoopingTypes()) {
          nonLoopingTypes.push_back(nestedElem);
        }
        for (Type *nestedElem : unionArmInfo->getLoopingTypes()) {
          loopingTypes.push_back(nestedElem);
        }
      } else {
        nonLoopingTypes.push_back(unionArm);
      }
    }

    // Non-Looping types can be sorted the fast way.
    UnionType::sortAndUniqueNonLoopingTypes(nonLoopingTypes);
    // Looping types are uniqued in a slow path.
    UnionType::uniqueLoopingTypesSlow(loopingTypes);

    // Only one type? Just use it, otherwise we still need a union.
    if (nonLoopingTypes.size() == 1 && loopingTypes.empty()) {
      type->info = nonLoopingTypes.front()->info;
    } else if (nonLoopingTypes.empty() && loopingTypes.size() == 1) {
      type->info = loopingTypes.front()->info;
    } else {
      unionType->setCanonicalTypes(
          std::move(nonLoopingTypes), std::move(loopingTypes));
    }
  }

  /// DFS step for forward-declared generic.
  /// Specializes forward-declared generics when necessary, and when this step
  /// completes, \p type will have a TypeInfo that is a real ClassType (no
  /// longer Generic).
  /// Mutually recursive with completeForwardType and completeForwardUnion.
  void completeForwardGeneric(
      Type *type,
      const GenericTypeInstantiation &generic,
      llvh::SetVector<Type *> &visited) {
    if (!llvh::isa<GenericType>(type->info))
      return;

    if (!visited.insert(type)) {
      // Already attempting to complete this type,
      // but hit a cycle on this branch.
      outer.sm_.error(
          type->node->getSourceRange(),
          "ft: type contains a circular reference to itself");
      type->info = outer.flowContext_.getAnyInfo();
      return;
    }

    auto popOnExit =
        llvh::make_scope_exit([&visited]() { visited.pop_back(); });

    TypeDecl *typeDecl = generic.typeDecl;

    for (Type *arg : generic.typeArgTypes) {
      if (forwardUnions.count(arg) || llvh::isa<GenericType>(arg->info)) {
        completeForwardType(arg, visited);
      }
    }

    assert(typeDecl->genericClassDecl && "Expected a generic class");
    sema::Decl *newDecl = outer.specializeGenericWithParsedTypes(
        typeDecl->genericClassDecl,
        llvh::cast<ESTree::TypeParameterInstantiationNode>(
            generic.annotation->_typeParameters),
        generic.typeArgTypes,
        typeDecl->genericClassDecl->scope);

    if (!newDecl)
      type->info = outer.flowContext_.getAnyInfo();

    Type *classConsType = outer.getDeclType(newDecl);
    type->info = llvh::cast<ClassConstructorType>(classConsType->info)
                     ->getClassType()
                     ->info;
  }

  /// Parse every element of deferredGenericSpecializations,
  /// and enqueue them to be typechecked when the typecheckQueue is drained.
  void parseDeferredGenericClasses() {
    auto savedScope = outer.bindingTable_.getCurrentScope();

    // It's possible we don't want to increment i every iteration,
    // so don't do it in the loop update clause.
    for (size_t i = 0; i < deferredParseGenerics.size(); ++i) {
      // Move out because the vector may grow while we iterate over it.
      DeferredGenericClass deferred{std::move(deferredParseGenerics[i])};
      auto *specialization = deferred.specialization;

      LLVM_DEBUG(
          llvh::dbgs() << "Parsing deferred generic class: "
                       << llvh::cast<ESTree::IdentifierNode>(
                              specialization->_id)
                              ->_name->str()
                       << "\n");

      outer.bindingTable_.activateScope(deferred.scope);

      // Actually parse and move on to the next one.
      // The superClass node has already been visited,
      // and must have been parsed before the current one.
      ParseClassType(
          outer,
          specialization->_superClass,
          specialization->_superTypeParameters,
          specialization->_body,
          deferred.classType);

      // Don't typecheck right now, because we need to parse everything in
      // current scope before descending into child functions.
      outer.typecheckQueue_.emplace_back(std::move(deferred));
    }

    outer.bindingTable_.activateScope(savedScope);
    deferredParseGenerics.clear();
  }
};

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
              outer.visitExpression(declarator->_init, declarator);
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
          assert(llvh::isa<ESTree::BlockStatementNode>(parent));
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
      outer.visitExpression(declarator->_init, declarator);
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
      } else if (auto *obj = llvh::dyn_cast<ESTree::ObjectPatternNode>(node)) {
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

  DeclareScopeTypes(*this, *decls, scope, scopeNode);
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
  llvh::SmallVector<Type *, 4> types{};
  for (auto &n : node->_types) {
    if (llvh::isa<ESTree::TupleTypeSpreadElementNode>(&n)) {
      sm_.error(n.getSourceRange(), "ft: tuple spread unsupported");
      continue;
    }
    if (llvh::isa<ESTree::TupleTypeLabeledElementNode>(&n)) {
      sm_.error(n.getSourceRange(), "ft: tuple labels unsupported");
      continue;
    }
    types.push_back(parseTypeAnnotation(&n));
  }
  return flowContext_.createType(flowContext_.createTuple(types), node);
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

  if (td->genericClassDecl) {
    // Resolve to the specialized class.
    if (!node->_typeParameters) {
      sm_.error(
          node->getSourceRange(),
          llvh::Twine("ft: missing generic arguments for '") +
              id->_name->str() + "'");
      return flowContext_.getAny();
    }
    return resolveGenericClassSpecializationForType(node, td->genericClassDecl);
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

template <typename AnnotationCB>
Type *FlowChecker::processFunctionTypeAnnotation(
    ESTree::FunctionTypeAnnotationNode *node,
    AnnotationCB cb) {
  if (node->_rest || node->_typeParameters) {
    sm_.error(node->getSourceRange(), "unsupported function type params");
  }

  Type *thisType = node->_this
      ? cb(llvh::cast<ESTree::FunctionTypeParamNode>(node->_this)
               ->_typeAnnotation)
      : nullptr;
  Type *returnType = cb(node->_returnType);

  llvh::SmallVector<TypedFunctionType::Param, 4> paramsList{};
  for (ESTree::Node &n : node->_params) {
    if (auto *param = llvh::dyn_cast<ESTree::FunctionTypeParamNode>(&n)) {
      auto *id = llvh::cast_or_null<ESTree::IdentifierNode>(param->_name);
      if (param->_optional) {
        sm_.error(param->getSourceRange(), "unsupported optional parameter");
      }
      paramsList.emplace_back(
          Identifier::getFromPointer(id ? id->_name : nullptr),
          param->_typeAnnotation ? cb(param->_typeAnnotation) : nullptr);
    } else {
      sm_.warning(
          n.getSourceRange(),
          "ft: typing of pattern parameters not implemented, :any assumed");
      size_t idx = paramsList.size();
      paramsList.emplace_back(
          astContext_.getIdentifier(
              (llvh::Twine("?param_") + llvh::Twine(idx)).str()),
          flowContext_.getAny());
    }
  }

  return flowContext_.createType(
      flowContext_.createFunction(
          returnType,
          thisType,
          paramsList,
          /* isAsync */ false,
          /* isGenerator */ false),
      node);
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
  if (auto *func =
          llvh::dyn_cast<ESTree::FunctionDeclarationNode>(specialization)) {
    newDecl = getDecl(llvh::cast<ESTree::IdentifierNode>(
        llvh::cast<ESTree::FunctionDeclarationNode>(specialization)->_id));
  } else if (
      auto *classDecl =
          llvh::dyn_cast<ESTree::ClassDeclarationNode>(specialization)) {
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
  visitExpression(specialization->_superClass, specialization);

  if (deferredParseGenerics_) {
    LLVM_DEBUG(
        llvh::dbgs() << "Deferring parsing of generic class specialization: "
                     << newDecl->name.str() << "\n");
    deferredParseGenerics_->emplace_back(
        specialization, bindingTable_.getCurrentScope(), classType);
  } else {
    ParseClassType(
        *this,
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

} // namespace flow
} // namespace hermes
