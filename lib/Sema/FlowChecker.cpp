/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "FlowChecker.h"

#include "llvh/ADT/SetVector.h"

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
  resolveScopeTypesAndAnnotate(rootNode, rootNode->getScope());
  if (sm_.getErrorCount()) {
    // Avoid running the visitor to check types if the resolution failed,
    // because the visitor relies on data populated during
    // resolveScopeTypesAndAnnotate.
    return false;
  }
  visitESTreeNode(*this, rootNode);
  return sm_.getErrorCount() == 0;
}

void FlowChecker::visit(ESTree::ProgramNode *node) {
  visitESTreeChildren(*this, node);
}

void FlowChecker::visit(ESTree::FunctionDeclarationNode *node) {
  sema::Decl *decl = getDecl(llvh::cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "function declaration must have been resolved");
  Type *declType = getDeclType(decl);

  // If this declaration is of a global property, then it doesn't have a
  // function type, because that would be unsound. So, we have to parse the
  // function type here, to make it available inside the function.
  auto *ftype = llvh::dyn_cast<FunctionType>(declType->info);
  if (!ftype) {
    declType = parseFunctionType(
        node->_params, node->_returnType, node->_async, node->_generator);
    ftype = llvh::cast<FunctionType>(declType->info);
  }

  FunctionContext functionContext(*this, node, declType, ftype->getThisParam());
  visitFunctionLike(node, node->_body, node->_params);
}

void FlowChecker::visit(ESTree::FunctionExpressionNode *node) {
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

  FunctionContext functionContext(
      *this,
      node,
      ftype,
      llvh::cast<FunctionType>(ftype->info)->getThisParam());
  visitFunctionLike(node, node->_body, node->_params);
}

void FlowChecker::visit(ESTree::ArrowFunctionExpressionNode *node) {
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
      ESTree::Node *body,
      Type *classType)
      : outer_(outer), superClassType(resolveSuperClass(superClass)) {
    ClassType *superClassTypeInfo = nullptr;
    if (superClassType) {
      superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
      // Offset based on the superclass if necessary, to avoid overwriting
      // existing fields.
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
        Type *methodType = parseMethodDefinition(method);
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
  Type *resolveSuperClass(ESTree::Node *superClass) {
    if (!superClass)
      return nullptr;
    if (!llvh::isa<ESTree::IdentifierNode>(superClass)) {
      outer_.sm_.error(
          superClass->getStartLoc(),
          "ft: only identifiers may be extended as superclasses");
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

  Type *parseMethodDefinition(ESTree::MethodDefinitionNode *method) {
    auto *fe = llvh::cast<ESTree::FunctionExpressionNode>(method->_value);

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
          fe->_params, nullptr, false, false, outer_.flowContext_.getVoid());
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
          fe->_params, fe->_returnType, fe->_async, fe->_generator);

      // Check if the method is inherited, and reuse the index.
      const ClassType::Field *superMethod = nullptr;
      if (superClassType) {
        auto *superClassTypeInfo = llvh::cast<ClassType>(superClassType->info);
        auto superIt = superClassTypeInfo->getHomeObjectTypeInfo()->findField(
            Identifier::getFromPointer(id->_name));
        if (superIt) {
          // Field is inherited.
          superMethod = superIt->getField();
          // Overriding method's function type must flow into the overridden
          // method's function type.
          CanFlowResult flowRes =
              canAFlowIntoB(methodType->info, superMethod->type->info);
          if (!flowRes.canFlow) {
            outer_.sm_.error(
                method->getStartLoc(),
                "ft: incompatible method type for override");
          }
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

void FlowChecker::visit(ESTree::ClassExpressionNode *node) {
  visitExpression(node->_superClass, node);

  auto *id = llvh::cast_or_null<ESTree::IdentifierNode>(node->_id);
  Type *classType = flowContext_.createType();
  ParseClassType(*this, node->_superClass, node->_body, classType);

  Type *consType =
      flowContext_.createType(flowContext_.createClassConstructor(classType));

  setNodeType(node, consType);

  // A new scope for the class expression name.
  ScopeRAII scope(*this);
  // If there was a class id T, in the new scope declare class type T and
  // declaration for class constructor T.
  if (id) {
    bindingTable_.insert(
        id->_name, TypeDecl(classType, node->getScope(), node));

    sema::Decl *decl = getDecl(id);
    assert(decl && "class expression id must be resolved");
    assert(
        declTypes_.count(decl) == 0 &&
        "class expression id type already resolved");
    recordDecl(decl, consType, id, node);
  }

  ClassContext classContext(*this, classType);
  visitESTreeNode(*this, node->_body, node);
}

void FlowChecker::visit(ESTree::ClassDeclarationNode *node) {
  sema::Decl *decl = getDecl(llvh::cast<ESTree::IdentifierNode>(node->_id));
  assert(decl && "class declaration must have been resolved");
  auto *classType =
      llvh::cast<ClassConstructorType>(getDeclType(decl)->info)->getClassType();

  ClassContext classContext(*this, classType);
  visitESTreeNode(*this, node->_body, node);
}

void FlowChecker::visit(ESTree::MethodDefinitionNode *node) {
  auto *fe = llvh::cast<ESTree::FunctionExpressionNode>(node->_value);

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

    // The type is either the type of the identifier or "any".
    Type *type = outer_.flowContext_.findDeclType(decl);
    if (!type && !sema::Decl::isKindGlobal(decl->kind)) {
      outer_.sm_.error(
          node->getSourceRange(), "local variable used prior to declaration");
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
                llvh::isa<FunctionType>(resType->info) &&
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
    } else if (!llvh::isa<AnyType>(objType->info)) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: properties not defined for type");
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

  void visit(ESTree::TypeCastExpressionNode *node) {
    auto *resTy = outer_.parseTypeAnnotation(
        llvh::cast<ESTree::TypeAnnotationNode>(node->_typeAnnotation)
            ->_typeAnnotation);
    // Populate the type of this node before visiting the expression, since it
    // is already known. This also allows the result type to be used as context
    // while we are visiting the expression being cast. For instance, if we are
    // casting an empty array literal, the resulting type of the cast can be
    // used to set the element type of the array.
    outer_.setNodeType(node, resTy);
    visitESTreeNode(*this, node->_expression, node);

    auto *expTy = outer_.getNodeTypeOrAny(node->_expression);
    auto cf = canAFlowIntoB(expTy->info, resTy->info);
    if (!cf.canFlow) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: cast from incompatible type");
    }
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
    auto tryElementType = [node, this, getElementType](Type *elTy) {
      auto &elements = node->_elements;
      for (auto it = elements.begin(); it != elements.end(); ++it) {
        ESTree::Node *arg = &*it;
        Type *argTy = getElementType(arg);
        auto cf = canAFlowIntoB(argTy->info, elTy->info);
        if (!cf.canFlow) {
          outer_.sm_.error(
              arg->getSourceRange(), "ft: incompatible element type");
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
      }
      auto *arrTy = outer_.flowContext_.createArray();
      arrTy->init(elTy);
      outer_.setNodeType(node, outer_.flowContext_.createType(arrTy));
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
            tryElementType(arrTy->getElement());
            return;
          }
        }
      }
    }

    // If this array expression is immediately cast to something else, try using
    // the type we are casting to.
    if (auto *cast = llvh::dyn_cast<ESTree::TypeCastExpressionNode>(parent)) {
      auto *resTy = outer_.getNodeTypeOrAny(cast);
      if (auto *arrTy = llvh::dyn_cast<ArrayType>(resTy->info)) {
        tryElementType(arrTy->getElement());
        return;
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

      elTy = outer_.flowContext_.maybeCreateUnion(elTypes.getArrayRef());
    }
    auto *arrTy = outer_.flowContext_.createArray();
    arrTy->init(elTy);
    outer_.setNodeType(node, outer_.flowContext_.createType(arrTy));
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
  void visit(ESTree::NumericLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getNumber());
  }
  void visit(ESTree::RegExpLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getAny());
  }
  void visit(ESTree::BigIntLiteralNode *node) {
    outer_.setNodeType(node, outer_.flowContext_.getBigInt());
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
      // any becomes (number|bigint).
      llvh::SmallVector<Type *, 2> types{
          outer_.flowContext_.getNumber(), outer_.flowContext_.getBigInt()};
      outer_.setNodeType(node, outer_.flowContext_.maybeCreateUnion(types));
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

  Type *determineBinopType(BinopKind op, TypeKind lk, TypeKind rk) {
    struct BinTypes {
      BinopKind op;
      TypeKind res, left, right;
    };

    static const BinTypes s_types[] = {
        // clang-format off
        {BinopKind::eq, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::ne, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::strictEq, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::strictNe, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::lt, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::le, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::gt, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::ge, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},

        {BinopKind::shl, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::shl, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::sshr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::sshr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::ushr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::ushr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},

        {BinopKind::plus, TypeKind::String, TypeKind::String, TypeKind::Any},
        {BinopKind::plus, TypeKind::String, TypeKind::Any, TypeKind::String},
        {BinopKind::plus, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::plus, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},

        {BinopKind::minus, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::minus, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::mul, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::mul, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::div, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::div, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::rem, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::rem, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binOr, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binOr, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binXor, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binXor, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::binAnd, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::binAnd, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},
        {BinopKind::exp, TypeKind::Number, TypeKind::Number, TypeKind::Number},
        {BinopKind::exp, TypeKind::BigInt, TypeKind::BigInt, TypeKind::BigInt},

        {BinopKind::in, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
        {BinopKind::instanceOf, TypeKind::Boolean, TypeKind::Any, TypeKind::Any},
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
      if ((it->left == TypeKind::Any || it->left == lk) &&
          (it->right == TypeKind::Any || it->right == rk)) {
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
      res = outer_.flowContext_.getAny();
    }

    outer_.setNodeType(node, res);
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
      CanFlowResult cf = canAFlowIntoB(rt->info, lt->info);
      if (!cf.canFlow) {
        outer_.sm_.error(
            node->getSourceRange(), "ft: incompatible assignment types");
        res = lt;
      } else {
        node->_right = outer_.implicitCheckedCast(node->_right, rt, cf);
        res = rt;
      }
    } else {
      res = determineBinopType(
          assignKind(node->_operator->str()),
          lt->info->getKind(),
          rt->info->getKind());

      if (llvh::isa<AnyType>(lt->info)) {
        // If the target we are assigning to is untyped, there are no checks
        // needed.
        if (!res)
          res = outer_.flowContext_.getAny();
      } else {
        // We are modifying a typed target. The type has to be compatible.
        // FIXME: we have to be able to deal with implicit checked casts in
        // cases like this. For now just ensure that the types are the same.
        if (res != lt) {
          outer_.sm_.error(
              node->getSourceRange(), "ft: incompatible assignment types");
          res = lt;
        }
      }
    }
    outer_.setNodeType(node, res);
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

    Type *calleeType = outer_.getNodeTypeOrAny(node->_callee);
    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info))
      return;

    if (!llvh::isa<FunctionType>(calleeType->info)) {
      outer_.sm_.error(
          node->_callee->getSourceRange(), "ft: callee is not a function");
      return;
    }
    auto *ftype = llvh::cast<FunctionType>(calleeType->info);

    outer_.setNodeType(node, ftype->getReturnType());

    Type *expectedThisType = ftype->getThisParam()
        ? ftype->getThisParam()
        : outer_.flowContext_.getAny();

    // Check the type of "this".
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
      Type *thisArgType = outer_.getNodeTypeOrAny(methodCallee->_object);
      if (!canAFlowIntoB(thisArgType->info, expectedThisType->info).canFlow) {
        outer_.sm_.error(
            methodCallee->getSourceRange(), "ft: 'this' type mismatch");
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

    checkArgumentTypes(ftype, node, node->_arguments, "function");
  }

  void checkSHBuiltin(
      ESTree::CallExpressionNode *call,
      ESTree::IdentifierNode *builtin) {
    if (builtin->_name == outer_.kw_.identCall) {
      checkSHBuiltinCall(call);
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
    if (!llvh::isa<FunctionType>(calleeType->info)) {
      outer_.sm_.error(
          callee->getSourceRange(), "ft: callee is not a function");
      return;
    }
    auto *ftype = llvh::cast<FunctionType>(calleeType->info);
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

    checkArgumentTypes(ftype, call, call->_arguments, "function", 2);
    return;
  }

  void visit(ESTree::OptionalCallExpressionNode *node) {
    outer_.sm_.error(
        node->getSourceRange(), "ft: optional call expression not supported");
  }

  void visit(ESTree::NewExpressionNode *node) {
    visitESTreeChildren(*this, node);

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
          llvh::cast<FunctionType>(consFType->info),
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
    if (!llvh::isa<ESTree::CallExpressionNode>(parent) ||
        !outer_.curClassContext_) {
      outer_.sm_.error(
          node->getSourceRange(),
          "ft: super only supported in constructor call");
      return;
    }

    ClassType *superClassType =
        outer_.curClassContext_->getClassTypeInfo()->getSuperClassInfo();
    if (!superClassType) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: super requires a base class");
      return;
    }

    outer_.setNodeType(node, superClassType->getConstructorType());
  }

  /// Check the types of the supplies arguments, adding checked casts if needed.
  /// \param offset the number of arguments to ignore at the front of \p
  ///   arguments. Used for $SHBuiltin.call, which has extra args at the front.
  bool checkArgumentTypes(
      FunctionType *ftype,
      ESTree::Node *callNode,
      ESTree::NodeList &arguments,
      const llvh::Twine &calleeName,
      uint32_t offset = 0) {
    size_t numArgs = arguments.size() - offset;
    // FIXME: default arguments.
    if (ftype->getParams().size() != numArgs) {
      outer_.sm_.error(
          callNode->getSourceRange(),
          "ft: " + calleeName + " expects " +
              llvh::Twine(ftype->getParams().size()) + " arguments, but " +
              llvh::Twine(numArgs) + " supplied");
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

      const FunctionType::Param &param = ftype->getParams()[argIndex];
      Type *expectedType = param.second;
      CanFlowResult cf =
          canAFlowIntoB(outer_.getNodeTypeOrAny(arg), expectedType);
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
            it, *outer_.implicitCheckedCast(arg, expectedType, cf));
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
void FlowChecker::visit(ESTree::ForStatementNode *node) {
  resolveScopeTypesAndAnnotate(node, node->getScope());
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

  FunctionType *ftype =
      llvh::cast<FunctionType>(curFunctionContext_->functionType->info);
  assert(ftype && "return in global context");

  // Return without an argument and "void" return type is OK.
  if (!node->_argument && llvh::isa<VoidType>(ftype->getReturnType()->info))
    return;

  Type *argType = node->_argument ? getNodeTypeOrAny(node->_argument)
                                  : flowContext_.getVoid();

  CanFlowResult cf = canAFlowIntoB(argType, ftype->getReturnType());
  if (!cf.canFlow) {
    // TODO: pretty print types.
    sm_.error(
        node->getSourceRange(),
        "ft: return value incompatible with return type");
  }
  node->_argument =
      implicitCheckedCast(node->_argument, ftype->getReturnType(), cf);
}

void FlowChecker::visit(ESTree::BlockStatementNode *node) {
  ScopeRAII scope(*this);
  resolveScopeTypesAndAnnotate(node, node->getScope());
  visitESTreeChildren(*this, node);
}

void FlowChecker::visit(ESTree::VariableDeclarationNode *node) {
  for (ESTree::Node &n : node->_declarations) {
    auto *declarator = llvh::cast<ESTree::VariableDeclaratorNode>(&n);
    if (!flowContext_.findNodeType(declarator->_init)) {
      visitExpression(declarator->_init, declarator);
    }
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
      if (!declarator->_init)
        continue;

      sema::Decl *decl = getDecl(id);
      Type *lt = getDeclType(decl);
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
  resolveScopeTypesAndAnnotate(node, node->getScope());
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

  resolveScopeTypesAndAnnotate(node, node->getSemInfo()->getFunctionScope());
  visitESTreeNode(*this, body, node);
}

/// Type aliases combined with unions create a **dramatic complication**, since
/// they can be mutually self recursive. We need to declare types in stages,
/// first the "direct" ones, then resolve the aliases and unions.
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
  /// Type aliases declared in this scope.
  llvh::SmallVector<Type *, 4> localTypeAliases{};
  /// Mapping from the LHS to the RHS type of a Type alias.
  /// e.g. type A = B;
  /// maps from the Type representing 'A' to the type representing 'B'.
  /// Used for populating the LHS after resolving the RHS with
  /// resolveTypeAnnotation.
  llvh::SmallDenseMap<Type *, Type *> typeAliasResolutions{};
  /// Keep track of all forward declarations of classes, so they can be
  /// completed.
  llvh::SmallVector<Type *, 4> forwardClassDecls{};

 public:
  DeclareScopeTypes(
      FlowChecker &outer,
      const sema::ScopeDecls &decls,
      sema::LexicalScope *scope)
      : outer(outer), decls(decls), scope(scope) {
    createForwardDeclarations();
    resolveAllAliases();
    completeForwardDeclarations();
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
    outer.sm_.note(
        typeDecl->astNode->getSourceRange(),
        "ft: previous declaration of " + name->str());
    return true;
  };

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
        Type *newType = outer.flowContext_.createType(
            outer.flowContext_.createClass(
                Identifier::getFromPointer(id->_name)),
            classNode);
        forwardClassDecls.push_back(newType);

        outer.bindingTable_.insert(
            id->_name, TypeDecl(newType, scope, declNode));

        bool success = outer.recordDecl(
            outer.getDecl(id),
            outer.flowContext_.createType(
                outer.flowContext_.createClassConstructor(newType), classNode),
            id,
            classNode);
        assert(success && "class constructor unexpectedly re-declared"),
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
        outer.bindingTable_.insert(
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
      auto *id = llvh::cast<ESTree::IdentifierNode>(gta->_id);

      // Is it declared anywhere?
      // If so, find its innermost declaration.
      TypeDecl *typeDecl = outer.bindingTable_.find(id->_name);
      if (!typeDecl) {
        // Not declared anywhere!
        outer.sm_.error(
            id->getStartLoc(), "ft: undefined type " + id->_name->str());
        return outer.flowContext_.getAny();
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
      return outer.flowContext_.maybeCreateUnion(types);
    }

    /// A nullable annotation is a simple case of a union.
    if (auto *nta =
            llvh::dyn_cast<ESTree::NullableTypeAnnotationNode>(annotation)) {
      return outer.flowContext_.createType(
          outer.flowContext_.createPopulatedNullable(
              resolveTypeAnnotation(nta->_typeAnnotation, visited, depth)),
          nta);
    }

    // The specified AST node represents a constructor type or a primary type,
    // so forward declare (if constructor) and return the type.
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
  /// Resolve the remaining forward declared types.
  void completeForwardDeclarations() {
    // Complete all forward-declared types.
    for (Type *type : forwardClassDecls) {
      // This is necessary because we need to defer parsing the class to allow
      // using types defined after the class inside the class:
      //     class C {
      //       x: D
      //     };
      //     type D = number;
      auto *classNode = llvh::cast<ESTree::ClassDeclarationNode>(type->node);
      outer.visitExpression(classNode->_superClass, classNode);
      ParseClassType(outer, classNode->_superClass, classNode->_body, type);
    }
  }
};

class FlowChecker::AnnotateScopeDecls {
  FlowChecker &outer;

 public:
  AnnotateScopeDecls(FlowChecker &outer, const sema::ScopeDecls &decls)
      : outer(outer) {
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
        annotateFunctionDeclaration(funcDecl);
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

 private:
  void annotateVariableDeclaration(
      ESTree::VariableDeclarationNode *declaration) {
    for (ESTree::Node &n : declaration->_declarations) {
      auto *declarator = llvh::cast<ESTree::VariableDeclaratorNode>(&n);
      if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(declarator->_id)) {
        sema::Decl *decl = outer.getDecl(id);
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
          // Attempt to infer the RHS of the declarator by calling the typecheck
          // visitor on it to see if it's able to associate a type with the init
          // node.
          outer.visitExpression(declarator->_init, declarator);
          if (Type *inferred =
                  outer.flowContext_.findNodeType(declarator->_init)) {
            type = inferred;
          }
        }

        outer.recordDecl(decl, type, id, declarator);
      } else {
        outer.sm_.warning(
            declarator->_id->getSourceRange(),
            "ft: typing of pattern declarators not implemented, :any assumed");
      }
    }
  }

  void annotateFunctionDeclaration(ESTree::FunctionDeclarationNode *funcDecl) {
    auto *id = llvh::cast<ESTree::IdentifierNode>(funcDecl->_id);
    sema::Decl *decl = outer.getDecl(id);
    Type *type = outer.parseFunctionType(
        funcDecl->_params,
        funcDecl->_returnType,
        funcDecl->_async,
        funcDecl->_generator);

    // Global properties don't have sound types, since they can be
    // overwritten without our knowledge and control.
    if (decl->kind == sema::Decl::Kind::GlobalProperty) {
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

void FlowChecker::resolveScopeTypesAndAnnotate(
    ESTree::Node *scopeNode,
    sema::LexicalScope *scope) {
  const sema::ScopeDecls *decls =
      curFunctionContext_->declCollector->getScopeDeclsForNode(scopeNode);
  if (!decls || decls->empty())
    return;

  assert(scope && "declarations found but no lexical scope");

  DeclareScopeTypes(*this, *decls, scope);
  AnnotateScopeDecls(*this, *decls);
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
    Type *defaultReturnType) {
  llvh::SmallVector<FunctionType::Param, 4> paramsList{};

  for (ESTree::Node &n : params) {
    if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(&n)) {
      paramsList.emplace_back(
          Identifier::getFromPointer(id->_name),
          parseOptionalTypeAnnotation(id->_typeAnnotation));
    } else {
      sm_.warning(
          n.getSourceRange(),
          "ft: typing of pattern parameters not implemented, :any assumed");
      paramsList.emplace_back(Identifier(), flowContext_.getAny());
    }
  }

  Type *returnType =
      parseOptionalTypeAnnotation(optReturnTypeAnnotation, defaultReturnType);
  Type *thisParamType = nullptr;
  llvh::ArrayRef<FunctionType::Param> paramsRef(paramsList);

  // Check if the first parameter is "this", since it is treated specially.
  if (!paramsRef.empty() &&
      paramsRef.front().first.getUnderlyingPointer() == kw_.identThis) {
    thisParamType = paramsRef.front().second;
    paramsRef = paramsRef.drop_front();
  }

  FunctionType *res = flowContext_.createFunction();
  res->init(returnType, thisParamType, paramsRef, isAsync, isGenerator);
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
  if (!node)
    return flowContext_.getAny();

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
    case ESTree::NodeKind::GenericTypeAnnotation:
      return parseGenericTypeAnnotation(
          llvh::cast<ESTree::GenericTypeAnnotationNode>(node));

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
  return flowContext_.maybeCreateUnion(types);
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
  Type *arr = flowContext_.createType(flowContext_.createArray(), node);
  llvh::cast<ArrayType>(arr->info)->init(
      parseTypeAnnotation(node->_elementType));
  return arr;
}

Type *FlowChecker::parseGenericTypeAnnotation(
    ESTree::GenericTypeAnnotationNode *node) {
  auto *id = llvh::cast<ESTree::IdentifierNode>(node->_id);
  TypeDecl *td = bindingTable_.find(id->_name);

  if (!td) {
    sm_.error(id->getSourceRange(), "ft: undefined type " + id->_name->str());
    return flowContext_.getAny();
  }

  return td->type;
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
    if (arrayA->getElement()->info->compare(arrayB->getElement()->info) == 0)
      return {.canFlow = true};
    return {};
  }

  if (ClassType *classA = llvh::dyn_cast<ClassType>(a)) {
    ClassType *classB = llvh::dyn_cast<ClassType>(b);
    if (!classB)
      return {};
    return canAFlowIntoB(classA, classB);
  }

  if (FunctionType *funcA = llvh::dyn_cast<FunctionType>(a)) {
    FunctionType *funcB = llvh::dyn_cast<FunctionType>(b);
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
    FunctionType *a,
    FunctionType *b) {
  // Function a can flow into b when:
  // * they're the same kind of function (async, generator, etc)
  // * all parameters of b can flow into parameters of a
  // * return type of a can flow into return type of b
  // * no checked casts are needed (can't flow into an 'any' parameter)

  if (a->isAsync() != b->isAsync())
    return {};
  if (a->isGenerator() != b->isGenerator())
    return {};

  if (!a->getThisParam() != !b->getThisParam()) {
    // Only one of the functions is missing `this`, can't flow.
    return {};
  }
  if (a->getThisParam() && b->getThisParam()) {
    // Both functions have `this`, it must be checked.
    CanFlowResult flowRes = canAFlowIntoB(b->getThisParam(), a->getThisParam());
    if (!flowRes.canFlow || flowRes.needCheckedCast)
      return {};
  }

  {
    // TODO: Handle default arguments, which will allow for a changing number of
    // parameters. Example:
    //   let funcB : (a: number) => number;
    //   function funcA(a: number, b: number = 10) : number {
    //     return 0;
    //   }
    //   funcB = funcA;
    // funcA flows into funcA here because the default argument allows the
    // caller to change the number of parameters.
    if (a->getParams().size() != b->getParams().size())
      return {};

    for (size_t i = 0, e = a->getParams().size(); i < e; ++i) {
      Type *paramA = a->getParams()[i].second;
      Type *paramB = b->getParams()[i].second;
      CanFlowResult flowRes = canAFlowIntoB(paramB, paramA);
      if (!flowRes.canFlow || flowRes.needCheckedCast)
        return {};
    }
  }

  {
    CanFlowResult flowRes =
        canAFlowIntoB(a->getReturnType(), b->getReturnType());
    if (!flowRes.canFlow || flowRes.needCheckedCast)
      return {};
  }

  return {.canFlow = true};
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

} // namespace flow
} // namespace hermes
