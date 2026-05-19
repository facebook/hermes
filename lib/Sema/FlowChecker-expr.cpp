/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Typechecking visitors for expressions.
///
/// Expressions can have types inferred by using a 'constraint' type provided to
/// the visitExpression function. The constraint type is propagated through the
/// AST by the ExprVisitor, allowing us to recursively match parts of the
/// constraint and choose which types to emit for nested expressions.
/// The visitor may emit errors for expressions which aren't able to match the
/// constraint when it is provided, and may add implicitCheckedCasts in the AST
/// when needed to match the constraint.
/// However, the caller that provided the constraint cannot rely on the actual
/// type of the checked node being equal to the type of the constraint that was
/// passed in, so the caller still needs to check the type and report errors or
/// insert its own casts itself.
//===----------------------------------------------------------------------===//

#include "FlowChecker.h"

#if HERMES_PARSE_FLOW
#define DEBUG_TYPE "FlowChecker"

namespace hermes {
namespace flow {

/// Find an accessor field for \p property in \p classInfo, or return nullptr.
static const ClassType::Field *findAccessorField(
    Context &astContext,
    ESTree::Node *property,
    ClassType *classInfo) {
  OptValue<ClassType::FieldLookupEntry> opt;
  if (auto *pn = llvh::dyn_cast<ESTree::PrivateNameNode>(property)) {
    Identifier name = astContext.getPrivateNameIdentifier(
        llvh::cast<ESTree::IdentifierNode>(pn->_id)->_name);
    opt = classInfo->findPrivateField(name);
  } else {
    Identifier name = Identifier::getFromPointer(
        llvh::cast<ESTree::IdentifierNode>(property)->_name);
    opt = classInfo->findPublicField(name);
  }
  if (opt && opt->getField()->isAccessor())
    return opt->getField();
  return nullptr;
}

void FlowChecker::matchConstraintToType(
    Type *startConstraint,
    const Type *startType) {
  // Use worklist to avoid recursion, with SetVector to avoid revisiting
  // looping types.
  llvh::SmallSetVector<std::pair<Type *, const Type *>, 32> worklist;
  worklist.insert({startConstraint, startType});

  for (size_t i = 0; i < worklist.size(); ++i) {
    auto [constraint, type] = worklist[i];
    if (!constraint || !type)
      continue;

    if (llvh::isa<InferencePlaceholderType>(constraint->info)) {
      // Found a placeholder, replace it with the type that matches it.
      constraint->info = type->info;
      continue;
    }

    // It's possible at this point that the type used to be a placeholder, but
    // has been replaced with a non-placeholder. If that happened, then we
    // won't be able to infer a potentially conflicting type for the
    // placeholder.
    // For example, if we try to match (T is a placeholder):
    //   constraint=[T,T]
    //   type=[number,string]
    // it'll set T to number, and we won't be able to infer string for T.
    // Meaning that we'll always need to check the argument types
    // at the CallExpression visitor.

    if (constraint->info->getKind() != type->info->getKind()) {
      // Handle InferencePlaceholderArray constraint matched against Array
      // class type. This occurs during type inference when T[] is parsed as
      // InferencePlaceholderArrayType (because the placeholder can't be
      // specialized), but the actual argument is an Array<T> class
      // specialization.
      if (auto *constraintArr =
              llvh::dyn_cast<InferencePlaceholderArrayType>(constraint->info)) {
        if (flowContext_.isArrayClassType(type)) {
          Type *constraintElem = constraintArr->getElement();
          // Resolve the InferencePlaceholderArrayType to the actual array
          // class type.
          constraint->info = type->info;
          worklist.insert(
              {constraintElem, flowContext_.getArrayElementType(type)});
          continue;
        }
      }
      // When the constraint is a Union containing placeholders and the
      // actual type is not a Union, try to resolve placeholder arms by
      // matching them against the actual type.
      if (auto *constraintUnion = llvh::dyn_cast<UnionType>(constraint->info)) {
        for (Type *arm : constraintUnion->getTypes()) {
          worklist.insert({arm, type});
        }
        continue;
      }
      // Mismatch, nothing more to do.
      continue;
    }

    switch (type->info->getKind()) {
      // Singleton types have nothing to visit.
#define _HERMES_SEMA_FLOW_DEFKIND(name) \
  case TypeKind::name:                  \
    continue;
      _HERMES_SEMA_FLOW_SINGLETONS

      // Classes are nominally typed.
      case TypeKind::Class: {
        // If both sides are Array<T> specializations, recurse into the
        // element types so inference placeholders can be matched.
        if (flowContext_.isArrayClassType(constraint) &&
            flowContext_.isArrayClassType(type)) {
          worklist.insert(
              {flowContext_.getArrayElementType(constraint),
               flowContext_.getArrayElementType(type)});
        }
        continue;
      }
      case TypeKind::ClassConstructor:
      // Functions that we don't infer the types of here.
      case TypeKind::NativeFunction:
      case TypeKind::UntypedFunction:
      // InferencePlaceholderArray is only used as constraints, never as
      // actual types, so both sides having this kind shouldn't happen.
      case TypeKind::InferencePlaceholderArray:
        continue;

      case TypeKind::Union:
        // TODO: Determine how to match unions here, because any arm could
        // match any other arm, recursively. There's also potential for
        // looping union arms.
        continue;

      case TypeKind::Array: {
        // Visit the array element.
        auto *constraintArr = llvh::cast<ArrayType>(constraint->info);
        auto *typeArr = llvh::cast<ArrayType>(type->info);
        worklist.insert({constraintArr->getElement(), typeArr->getElement()});
        continue;
      }

      case TypeKind::Tuple: {
        // Visit the tuple elements.
        auto *constraintTuple = llvh::cast<TupleType>(constraint->info);
        auto *typeTuple = llvh::cast<TupleType>(type->info);
        if (constraintTuple->getTypes().size() !=
            typeTuple->getTypes().size()) {
          continue;
        }

        for (size_t i = 0, e = constraintTuple->getTypes().size(); i < e; ++i) {
          worklist.insert(
              {constraintTuple->getTypes()[i], typeTuple->getTypes()[i]});
        }
        continue;
      }

      case TypeKind::TypedFunction: {
        auto *constraintFn = llvh::cast<TypedFunctionType>(constraint->info);
        auto *typeFn = llvh::cast<TypedFunctionType>(type->info);
        // Match the common params and return type to resolve placeholders.
        // Param count mismatches are allowed here because inference only
        // needs to extract placeholder bindings; flowing compatibility is
        // checked separately.
        size_t minParams = std::min(
            constraintFn->getParams().size(), typeFn->getParams().size());

        worklist.insert({constraintFn->getThisParam(), typeFn->getThisParam()});
        for (size_t i = 0; i < minParams; ++i) {
          worklist.insert(
              {constraintFn->getParams()[i].type, typeFn->getParams()[i].type});
        }
        worklist.insert(
            {constraintFn->getReturnType(), typeFn->getReturnType()});
        continue;
      }

      case TypeKind::ExactObject: {
        // Visit the object properties.
        auto *constraintObj = llvh::cast<ExactObjectType>(constraint->info);
        auto *typeObj = llvh::cast<ExactObjectType>(type->info);
        if (constraintObj->getFields().size() != typeObj->getFields().size()) {
          continue;
        }

        for (size_t i = 0, e = constraintObj->getFields().size(); i < e; ++i) {
          worklist.insert(
              {constraintObj->getFields()[i].type,
               typeObj->getFields()[i].type});
        }
        continue;
      }

    } // switch
  }
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
  void visit(ESTree::Node *node, ESTree::Node *parent, Type *constraint) {
    if (0) {
      LLVM_DEBUG(
          llvh::dbgs() << "Unsupported node " << node->getNodeName()
                       << " in expr context\n");
      llvm_unreachable("invalid node in expression context");
    } else {
      visitESTreeChildren(*this, node, nullptr);
    }
  }

  void afterVisit(ESTree::Node *node, ESTree::Node *parent, Type *constraint) {
    // Now that we've found the type of the node, attempt to populate any
    // InferencePlaceholders based on the typechecked actual type of the
    // expression.
    // This must be done after every expression is visited, since the type
    // of the constraint needs to have placeholders filled in whenever possible
    // if we want to use the information to inform the type of the arguments.
    outer_.matchConstraintToType(constraint, outer_.getNodeTypeOrAny(node));
  }

  void visit(
      ESTree::FunctionExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    return outer_.visit(node);
  }
  void visit(
      ESTree::ArrowFunctionExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    if (auto *constraintFnType = llvh::dyn_cast_or_null<TypedFunctionType>(
            constraint ? constraint->info : nullptr)) {
      // If a constraint was provided, attempt to infer the return type of the
      // function.
      if (node->_typeParameters) {
        outer_.sm_.error(
            node->_typeParameters->getStartLoc(),
            "ft: type parameters not supported on function expressions");
        return;
      }

      // Populate the param types.
      size_t i = 0;
      llvh::SmallVector<TypedFunctionType::Param, 4> params{};
      for (const auto &param : node->_params) {
        // Default is 'any', but try to get a narrower type if possible.
        TypedFunctionType::Param typedFnParam{
            Identifier{}, outer_.flowContext_.getAny(), false};

        if (auto *id = llvh::dyn_cast<ESTree::IdentifierNode>(&param)) {
          typedFnParam.name = Identifier::getFromPointer(id->_name);
          if (id->_typeAnnotation) {
            // Use explicit type annotation that was provided.
            typedFnParam.type =
                outer_.parseOptionalTypeAnnotation(id->_typeAnnotation);
            if (i < constraintFnType->getParams().size()) {
              // Attempt to populate the constraint type with the explicit type.
              // This allows us to pass an arrow function that looks like
              //   (x: number) => x + 1
              // to a placeholder function type like `A => B` and infer that `A`
              // is number.
              outer_.matchConstraintToType(
                  constraintFnType->getParams()[i].type, typedFnParam.type);
            }
          } else if (i < constraintFnType->getParams().size()) {
            // No type annotation, but there's a constraint type, so use that.
            // Add the name of the parameter for better errors.
            typedFnParam.type = constraintFnType->getParams()[i].type;
          }
        } else {
          outer_.sm_.warning(
              param.getSourceRange(),
              "ft: arrow function destructuring not supported, assuming 'any'");
        }

        params.push_back(typedFnParam);
        ++i;
      }

      Type *returnType;
      if (node->_returnType) {
        // Use explicit return type annotation if possible.
        returnType = outer_.parseOptionalTypeAnnotation(node->_returnType);
        outer_.matchConstraintToType(
            constraintFnType->getReturnType(), returnType);
      } else {
        // Otherwise, use the constraint type.
        returnType = constraintFnType->getReturnType();
      }

      // The type of the arrow function that we've inferred.
      // Note that the return type is potentially an InferencePlaceholder,
      // which may be filled in when the ReturnStatement argument is visited by
      // ExprVisitor (with the return type as a constraint).
      Type *arrowInferenceType =
          outer_.flowContext_.createType(outer_.flowContext_.createFunction(
              returnType, nullptr, params, node->_async, false));
      outer_.setNodeType(node, arrowInferenceType);

      {
        FunctionContext functionContext{
            outer_,
            node,
            arrowInferenceType,
            outer_.curFunctionContext_->thisParamType,
            outer_.curFunctionContext_->newTargetType};
        outer_.visitFunctionLike(node, node->_body, node->_params);
      }
    } else {
      outer_.visit(node);
    }
  }
  void visit(
      ESTree::ClassExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    return outer_.visit(node);
  }

  void visit(
      ESTree::MetaPropertyNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    auto *meta = llvh::cast<ESTree::IdentifierNode>(node->_meta);
    auto *property = llvh::cast<ESTree::IdentifierNode>(node->_property);

    // Check for new.target.
    if (meta->_name == outer_.kw_.identNew &&
        property->_name == outer_.kw_.identTarget) {
      if (!outer_.curFunctionContext_) {
        outer_.sm_.error(
            node->getSourceRange(), "ft: invalid use of new.target");
        return;
      }

      outer_.setNodeType(node, outer_.curFunctionContext_->newTargetType);
      return;
    }

    // All other meta properties are not supported.
    outer_.sm_.error(node->getSourceRange(), "ft: unsupported meta property");
  }

  void
  visit(ESTree::IdentifierNode *node, ESTree::Node *parent, Type *constraint) {
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

    // In typed functions, 'arguments' is only allowed in 'arguments.length'.
    if (decl->special == sema::Decl::Special::Arguments) {
      // Get the function that 'arguments' belongs to to determine its type.
      sema::FunctionInfo *argumentsOwner = decl->scope->parentFunction;
      FunctionContext *argumentsContext = outer_.curFunctionContext_;
      // Walk up the stack to find the function that 'arguments' belongs to.
      // We won't hit the null context because the FunctionContext must exist.
      while (argumentsContext->semInfo != argumentsOwner)
        argumentsContext = argumentsContext->getPreviousContext();
      Type *funcType = argumentsContext->functionType;
      // For typed functions, ensure that this is arguments.length.
      if (funcType && llvh::isa<TypedFunctionType>(funcType->info)) {
        bool isArgumentsLength = false;
        if (auto *memberParent =
                llvh::dyn_cast<ESTree::MemberExpressionNode>(parent);
            memberParent && memberParent->_object == node &&
            !memberParent->_computed) {
          if (auto *propId = llvh::dyn_cast<ESTree::IdentifierNode>(
                  memberParent->_property);
              propId && propId->_name == outer_.kw_.identLength) {
            isArgumentsLength = true;
          }
        }
        if (!isArgumentsLength)
          outer_.sm_.error(
              node->getSourceRange(),
              "ft: 'arguments' is only allowed in 'arguments.length'"
              " in typed functions");
      }
    }

    // Generic decls don't have types set because they aren't real values.
    // 'arguments' is implicitly typed as 'any' since it's a runtime object.
    if (!type && !sema::Decl::isKindGlobal(decl->kind) && !decl->generic &&
        decl->special != sema::Decl::Special::Arguments) {
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

  void visit(
      ESTree::ThisExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(
        node,
        outer_.curFunctionContext_->thisParamType
            ? outer_.curFunctionContext_->thisParamType
            : outer_.flowContext_.getAny());
  }

  /// Member access on a ClassType. Handles Array<T> length/push/index as
  /// special cases of class member access, then dispatches to the public or
  /// private name lookup.
  Type *visitMemberClass(
      ESTree::MemberExpressionNode *node,
      ESTree::Node *parent,
      Type *objType,
      ClassType *classType) {
    // Array<T>: special-case .length / .push / numeric index. Private name
    // access on an array falls through to the generic class path below.
    // TODO: This is a HACK, fix it by making Array<T> more expressive.
    if (outer_.flowContext_.isArrayClassType(objType) &&
        (node->_computed ||
         !llvh::isa<ESTree::PrivateNameNode>(node->_property))) {
      if (node->_computed) {
        Type *indexType = outer_.getNodeTypeOrAny(node->_property);
        if (!llvh::isa<NumberType>(indexType->info) &&
            !llvh::isa<AnyType>(indexType->info)) {
          outer_.sm_.error(
              node->_property->getSourceRange(),
              "ft: array index must be a number");
        }
        return outer_.flowContext_.getArrayElementType(objType);
      }
      auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
      if (id->_name == outer_.kw_.identPush) {
        // TODO: Represent .push as a real function.
        return outer_.flowContext_.getAny();
      }
    }

    if (node->_computed) {
      outer_.sm_.error(
          node->_property->getSourceRange(),
          "ft: computed access to class instances not supported");
      return outer_.flowContext_.getAny();
    }

    // Read the name of the property.
    Identifier name;
    if (auto *privateName =
            llvh::dyn_cast<ESTree::PrivateNameNode>(node->_property)) {
      name = outer_.astContext_.getPrivateNameIdentifier(
          llvh::cast<ESTree::IdentifierNode>(privateName->_id)->_name);
      if (!outer_.classTypeIsEnclosing(classType)) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: private field " + name.str() + " not visible outside class " +
                classType->getClassNameOrDefault());
      }
    } else {
      auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
      name = Identifier::getFromPointer(id->_name);
    }
    // lookupPropertyOnClass returns null `type` (with non-null `field`) for
    // overloaded methods; resolution happens at the call site.
    auto [type, field] =
        outer_.lookupPropertyOnClass(classType, name, node->_property);
    if (!field) {
      // TODO: class declaration location.
      outer_.sm_.error(
          node->_property->getSourceRange(),
          "ft: property " + name.str() + " not defined in class " +
              classType->getClassNameOrDefault());
      return outer_.flowContext_.getAny();
    }

    // Setter-only access is only legal on the LHS of `=`.
    if (field->isAccessor() && !field->hasGetter()) {
      auto *assignParent =
          llvh::dyn_cast<ESTree::AssignmentExpressionNode>(parent);
      if (!assignParent || node != assignParent->_left ||
          assignParent->_operator != outer_.kw_.identEqual) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: cannot read setter-only property");
      }
    }
    // Overloaded methods may only be referenced as a direct call target.
    if (field->isOverloaded()) {
      auto *callParent = llvh::dyn_cast<ESTree::CallExpressionNode>(parent);
      if (!callParent || callParent->_callee != node) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: overloaded method " + name.str() +
                " cannot be referenced outside a call expression");
      }
    }
    return type;
  }

  /// Static member access via ClassName.property or ClassName.#property.
  Type *visitMemberClassConstructor(
      ESTree::MemberExpressionNode *node,
      ESTree::Node *parent,
      ClassConstructorType *consType) {
    if (node->_computed) {
      outer_.sm_.error(
          node->_property->getSourceRange(),
          "ft: computed access to class statics not supported");
      return outer_.flowContext_.getAny();
    }
    auto *classTypeInfo = llvh::cast<ClassType>(consType->getClassType()->info);
    bool isPrivate = false;
    Identifier name;
    ESTree::IdentifierNode *propId;
    if (auto *privateName =
            llvh::dyn_cast<ESTree::PrivateNameNode>(node->_property)) {
      isPrivate = true;
      propId = llvh::cast<ESTree::IdentifierNode>(privateName->_id);
      name = outer_.astContext_.getPrivateNameIdentifier(propId->_name);
      if (!outer_.classTypeIsEnclosing(classTypeInfo)) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: private static " + name.str() + " not visible outside class " +
                classTypeInfo->getClassNameOrDefault());
      }
    } else {
      propId = llvh::cast<ESTree::IdentifierNode>(node->_property);
      name = Identifier::getFromPointer(propId->_name);
    }
    auto *staticInfo = classTypeInfo->getStaticObjectTypeInfo();
    OptValue<ClassType::FieldLookupEntry> optStaticField;
    if (staticInfo) {
      optStaticField = isPrivate ? staticInfo->findPrivateField(name)
                                 : staticInfo->findPublicField(name);
    }
    if (!optStaticField) {
      outer_.sm_.error(
          node->_property->getSourceRange(),
          llvh::Twine("ft: static property ") + name.str() +
              " not defined in class " +
              classTypeInfo->getClassNameOrDefault());
      return outer_.flowContext_.getAny();
    }
    const auto *field = optStaticField->getField();
    if (field->isAccessor()) {
      // Static accessor: result type is the field type.
      // Do NOT propagate Decl — IRGen handles the call.
      if (!field->hasGetter()) {
        auto *assign = llvh::dyn_cast<ESTree::AssignmentExpressionNode>(parent);
        bool isSimpleAssignTarget = assign && assign->_left == node &&
            assign->_operator == outer_.kw_.identEqual;
        if (!isSimpleAssignTarget) {
          outer_.sm_.error(
              node->_property->getSourceRange(),
              "ft: cannot read setter-only property");
        }
      }
      return field->type;
    }

    // Overloaded static methods may only be referenced as a direct call
    // target.
    if (field->isOverloaded()) {
      auto *callParent = llvh::dyn_cast<ESTree::CallExpressionNode>(parent);
      if (!callParent || callParent->_callee != node) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: overloaded method " + name.str() +
                " cannot be referenced outside a call expression");
      }
    }

    // Propagate the Decl from the definition key to the call-site property
    // so IRGen can look it up.
    if (ESTree::IdentifierNode *keyNode = field->staticKeyNode) {
      if (auto *decl = outer_.semContext_.getExpressionDecl(keyNode))
        outer_.semContext_.setExpressionDecl(propId, decl);
    }
    return field->type;
  }

  /// Named/computed access on an exact object type.
  Type *visitMemberExactObject(
      ESTree::MemberExpressionNode *node,
      ExactObjectType *exactObjType) {
    if (node->_computed) {
      // TODO: determine what this should do for real.
      // Flow allows this and just returns 'any' (deliberately unsound).
      outer_.sm_.error(
          node->_property->getSourceRange(),
          "ft: computed access to exact object types not supported");
      return outer_.flowContext_.getAny();
    }
    auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
    auto optFieldIdx =
        exactObjType->findField(Identifier::getFromPointer(id->_name));
    if (!optFieldIdx) {
      outer_.sm_.error(
          node->_property->getSourceRange(),
          "ft: property " + id->_name->str() + " not defined in object");
      return outer_.flowContext_.getAny();
    }
    return exactObjType->getFields()[*optFieldIdx].type;
  }

  /// Numeric-literal indexed access or .length on a tuple type.
  Type *visitMemberTuple(
      ESTree::MemberExpressionNode *node,
      TupleType *tupleType) {
    if (node->_computed) {
      auto *idx = llvh::dyn_cast<ESTree::NumericLiteralNode>(node->_property);
      if (!idx) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: tuple property access requires an number literal index");
        return outer_.flowContext_.getAny();
      }
      double d = idx->_value;
      if (d < 0 || d >= tupleType->getTypes().size()) {
        outer_.sm_.error(
            node->_property->getSourceRange(), "ft: tuple index out of bounds");
        return outer_.flowContext_.getAny();
      }
      // d is in bounds of the valid integer indices so the cast is safe.
      if ((uint32_t)d != d) {
        // ulen can only compare equal to d when d is a valid uint32 integer.
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: tuple index must be a non-negative integer");
        return outer_.flowContext_.getAny();
      }
      return tupleType->getTypes()[(uint32_t)d];
    }
    // Named property access to tuple.
    auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
    // TODO: We may want to allow calling into array functions with tuples
    // (providing the union of all elements as the generic type of the
    // array), but that's not supported yet.
    if (id->_name == outer_.kw_.identLength) {
      // Tuple .length is a number.
      return outer_.flowContext_.getNumber();
    }
    outer_.sm_.error(
        node->_property->getSourceRange(), "ft: unknown tuple property");
    return outer_.flowContext_.getAny();
  }

  /// Indexed access, .length, or builtin method access on a string.
  Type *visitMemberString(ESTree::MemberExpressionNode *node) {
    if (node->_computed) {
      Type *indexType = outer_.getNodeTypeOrAny(node->_property);
      if (!llvh::isa<NumberType>(indexType->info) &&
          !llvh::isa<AnyType>(indexType->info)) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            "ft: string index must be a number");
      }
      return outer_.flowContext_.getString();
    }
    auto *id = llvh::cast<ESTree::IdentifierNode>(node->_property);
    if (id->_name == outer_.kw_.identLength)
      return outer_.flowContext_.getNumber();
    if (auto *builtinDecl = outer_.flowContext_.findBuiltinMethod(
            {TypeKind::String, id->_name, /* isStatic */ false})) {
      // Found a builtin method - store for CallExpression to use.
      // The actual type will be determined by CallExpression.
      // For now, use any.
      outer_.setBuiltinMethodDecl(node, builtinDecl);
      return outer_.flowContext_.getAny();
    }
    outer_.sm_.error(
        node->_property->getSourceRange(), "ft: unknown string property");
    return outer_.flowContext_.getAny();
  }

  void visit(
      ESTree::MemberExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // TODO: types
    visitESTreeNode(*this, node->_object, node, nullptr);
    if (node->_computed)
      visitESTreeNode(*this, node->_property, node, nullptr);
    resolveMemberExpressionType(node, parent);
  }

  /// Compute and set the type of \p node, assuming its \c _object (and
  /// \c _property when computed) have already been visited.
  void resolveMemberExpressionType(
      ESTree::MemberExpressionNode *node,
      ESTree::Node *parent) {
    Type *objType = outer_.getNodeTypeOrAny(node->_object);
    Type *resType = outer_.flowContext_.getAny();

    // 'arguments.length' is always typed as number.
    if (auto *objId = llvh::dyn_cast<ESTree::IdentifierNode>(node->_object);
        objId && !node->_computed) {
      auto *objDecl = outer_.getDecl(objId);
      if (objDecl && objDecl->special == sema::Decl::Special::Arguments) {
        if (auto *propId =
                llvh::dyn_cast<ESTree::IdentifierNode>(node->_property);
            propId && propId->_name == outer_.kw_.identLength) {
          outer_.setNodeType(node, outer_.flowContext_.getNumber());
          return;
        }
      }
    }

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
      resType = visitMemberClass(node, parent, objType, classType);
    } else if (
        auto *consType = llvh::dyn_cast<ClassConstructorType>(objType->info)) {
      resType = visitMemberClassConstructor(node, parent, consType);
    } else if (
        auto *exactObjType = llvh::dyn_cast<ExactObjectType>(objType->info)) {
      resType = visitMemberExactObject(node, exactObjType);
    } else if (auto *tupleType = llvh::dyn_cast<TupleType>(objType->info)) {
      resType = visitMemberTuple(node, tupleType);
    } else if (llvh::isa<StringType>(objType->info)) {
      resType = visitMemberString(node);
    } else if (!llvh::isa<AnyType>(objType->info)) {
      if (node->_computed) {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            llvh::Twine(
                "ft: indexed access only allowed on array/tuple/string, found ") +
                objType->info->getKindName());
      } else {
        outer_.sm_.error(
            node->_property->getSourceRange(),
            llvh::Twine(
                "ft: named property access only allowed on objects, found ") +
                objType->messageString());
      }
    }

    outer_.setNodeType(node, resType);
  }
  void visit(
      ESTree::OptionalMemberExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // TODO: types
    outer_.sm_.warning(
        node->getSourceRange(),
        "ft: optional member expression not implemented");
    visitESTreeNode(*this, node->_object, node, nullptr);
    if (node->_computed)
      visitESTreeNode(*this, node->_property, node, nullptr);
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
    visitESTreeNode(*this, expression, node, resTy);

    auto *expTy = outer_.getNodeTypeOrAny(expression);
    auto cf = outer_.canAFlowIntoB(expTy->info, resTy->info);
    if (!cf.canFlow) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: cast from incompatible type");
    }

    outer_.setNodeType(node, resTy);
  }

  void visit(
      ESTree::TypeCastExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitExplicitCast(
        node,
        node->_expression,
        llvh::cast<ESTree::TypeAnnotationNode>(node->_typeAnnotation)
            ->_typeAnnotation);
  }

  void visit(
      ESTree::AsExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitExplicitCast(node, node->_expression, node->_typeAnnotation);
  }

  /// Visits an array expression as a tuple given the \p constraint and its
  /// TypeInfo \p tupleTy.
  void visitArrayExpressionAsTuple(
      ESTree::ArrayExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint,
      TupleType *tupleTy) {
    assert(
        constraint && tupleTy && constraint->info == tupleTy &&
        "must have a tuple type");
    auto &elements = node->_elements;
    size_t i = 0;
    bool tooFewTupleElements = false;
    for (auto it = elements.begin(); it != elements.end(); ++it, ++i) {
      if (i >= tupleTy->getTypes().size()) {
        tooFewTupleElements = true;
        break;
      }
      ESTree::Node *elem = &*it;
      Type *constraintElemTy = tupleTy->getTypes()[i];
      visitESTreeNodeNoReplace(*this, elem, node, constraintElemTy);
      if (llvh::isa<ESTree::SpreadElementNode>(elem)) {
        outer_.sm_.error(
            elem->getSourceRange(), "ft: tuple spread element not supported");
        continue;
      }
      // Check that the actual type is compatible with the constraint.
      Type *actualElemTy = outer_.getNodeTypeOrAny(elem);
      CanFlowResult cf = outer_.canAFlowIntoB(actualElemTy, constraintElemTy);
      if (!cf.canFlow) {
        outer_.sm_.error(
            elem->getSourceRange(),
            llvh::Twine("ft: incompatible tuple element type at index: ") +
                llvh::Twine(i));
        continue;
      }
      // Add a checked cast if needed.
      if (cf.canFlow && cf.needCheckedCast) {
        auto newIt = elements.insert(
            it, *outer_.implicitCheckedCast(elem, constraintElemTy, cf));
        elements.erase(it);
        it = newIt;
      }
    }

    if (tooFewTupleElements || i != tupleTy->getTypes().size()) {
      outer_.sm_.error(
          node->getSourceRange(),
          llvh::Twine("ft: incompatible tuple type, expected ") +
              llvh::Twine(tupleTy->getTypes().size()) + " elements, found " +
              llvh::Twine(elements.size()));
      outer_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }

    outer_.setNodeType(node, constraint);
  }

  /// Visits an array expression as an array.
  /// \param constraint the constraint on the array type, if it is an Array
  ///   ClassType.
  void visitArrayExpressionAsArray(
      ESTree::ArrayExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    bool hasArrayConstraint = false;
    Type *constraintElemTy = nullptr;
    if (constraint) {
      if (outer_.flowContext_.isArrayClassType(constraint)) {
        hasArrayConstraint = true;
        constraintElemTy = outer_.flowContext_.getArrayElementType(constraint);
      } else if (
          auto *inferArray =
              llvh::dyn_cast<InferencePlaceholderArrayType>(constraint->info)) {
        hasArrayConstraint = true;
        constraintElemTy = inferArray->getElement();
      }
    }

    // Construct a union of all the element types if there's no constraint.
    llvh::SmallSetVector<Type *, 4> elTypes{};
    auto &elements = node->_elements;
    size_t i = 0;
    for (auto it = elements.begin(); it != elements.end(); ++it, ++i) {
      ESTree::Node *elem = &*it;

      Type *actualElemTy = nullptr;
      if (auto *spread = llvh::dyn_cast<ESTree::SpreadElementNode>(elem)) {
        Type *constraintSpreadElemTy =
            hasArrayConstraint ? constraint : nullptr;
        visitESTreeNodeNoReplace(
            *this, spread->_argument, node, constraintSpreadElemTy);
        auto *spreadTy = outer_.getNodeTypeOrAny(spread->_argument);
        if (!outer_.flowContext_.isArrayClassType(spreadTy)) {
          // TODO: Handle spread of non-arrays.
          outer_.sm_.error(
              spread->_argument->getSourceRange(),
              "ft: spread argument must be an array");
          continue;
        }
        actualElemTy = outer_.flowContext_.getArrayElementType(spreadTy);
      } else {
        visitESTreeNodeNoReplace(*this, elem, node, constraintElemTy);
        actualElemTy = outer_.getNodeTypeOrAny(elem);
      }

      elTypes.insert(actualElemTy);
      if (constraintElemTy) {
        // If there's a constraint on the element type, check that each
        // element conforms and insert implicit casts when necessary.
        CanFlowResult cf = outer_.canAFlowIntoB(actualElemTy, constraintElemTy);
        if (!cf.canFlow) {
          outer_.sm_.error(
              elem->getSourceRange(),
              llvh::Twine("ft: incompatible array element type at index: ") +
                  llvh::Twine(i));
          continue;
        }
        // Add a checked cast if needed. Skip spread elements, since we need
        // to cast each element they produce, rather than the spread itself.
        if (cf.needCheckedCast) {
          if (llvh::isa<ESTree::SpreadElementNode>(elem)) {
            // We don't support spread elements with checked casts yet,
            // because we have to cast each element individually,
            outer_.sm_.error(
                elem->getSourceRange(),
                "ft: spread element with checked cast not supported");
          } else {
            auto newIt = elements.insert(
                it, *outer_.implicitCheckedCast(elem, constraintElemTy, cf));
            elements.erase(it);
            it = newIt;
          }
        }
      }
    }

    if (constraint &&
        !llvh::isa<InferencePlaceholderArrayType>(constraint->info)) {
      outer_.setNodeType(node, constraint);
    } else if (elTypes.empty()) {
      // If there's no elements in the union, then just use 'any'.
      outer_.sm_.warning(
          node->getSourceRange(),
          "ft: empty array with no context, assuming 'any' array");
      outer_.setNodeType(node, outer_.flowContext_.getAny());
    } else {
      // Otherwise, construct a union of all the element types.
      Type *elemUnion = outer_.flowContext_.createType(
          outer_.flowContext_.maybeCreateUnion(elTypes.getArrayRef()));
      Type *arrType = outer_.getSpecializedArrayClassType(
          elemUnion, node->getSourceRange());
      if (arrType) {
        outer_.setNodeType(node, arrType);
      } else {
        outer_.sm_.error(
            node->getSourceRange(), "ft: Array type requires TypedLib");
        outer_.setNodeType(node, outer_.flowContext_.getAny());
      }
    }
  }

  void visit(
      ESTree::ArrayExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    if (auto *tupleTy = llvh::dyn_cast_or_null<TupleType>(
            constraint ? constraint->info : nullptr)) {
      // The type of ArrayExpression is only Tuple when that constraint was
      // passed in from above.
      visitArrayExpressionAsTuple(node, parent, constraint, tupleTy);
    } else {
      visitArrayExpressionAsArray(node, parent, constraint);
    }
  }

  void visit(
      ESTree::ObjectExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // Failed to make an object type that matches the properties,
    // assume 'any' and continue.
    bool assumeAny = false;

    // Fields of the object.
    llvh::SmallVector<ExactObjectType::Field, 4> fields;
    // Name of the key, mapping to index in the fields vector.
    llvh::SmallDenseMap<UniqueString *, size_t> names;

    auto *constraintObjectType = llvh::dyn_cast_or_null<ExactObjectType>(
        constraint ? constraint->info : nullptr);

    for (ESTree::Node &node : node->_properties) {
      // Spread element: merge fields from an exact-object source.
      if (auto *spread = llvh::dyn_cast<ESTree::SpreadElementNode>(&node)) {
        visitESTreeNodeNoReplace(*this, spread->_argument, spread, nullptr);
        Type *spreadTy = outer_.getNodeTypeOrAny(spread->_argument);
        if (llvh::isa<AnyType>(spreadTy->info)) {
          outer_.sm_.warning(
              node.getSourceRange(),
              "ft: unsupported property for typed object, assuming 'any'");
          assumeAny = true;
        } else if (
            auto *spreadObjTy =
                llvh::dyn_cast<ExactObjectType>(spreadTy->info)) {
          for (const auto &srcField : spreadObjTy->getFields()) {
            auto [it, inserted] = names.try_emplace(
                srcField.name.getUnderlyingPointer(), fields.size());
            if (inserted) {
              fields.emplace_back(srcField.name, srcField.type);
            } else {
              fields[it->second].type = srcField.type;
            }
          }
        } else {
          outer_.sm_.error(
              spread->getSourceRange(),
              "ft: spread argument must be an exact object type");
        }
        continue;
      }

      auto *prop = llvh::dyn_cast<ESTree::PropertyNode>(&node);
      // prop->_kind being "init" makes sure this isn't a getter/setter.
      if (!prop || prop->_computed || prop->_kind != outer_.kw_.identInit ||
          prop->_method) {
        // Exact object type doesn't support this, so bail.
        outer_.sm_.warning(
            node.getSourceRange(),
            "ft: unsupported property for typed object, assuming 'any'");
        assumeAny = true;
        break;
      }

      visitESTreeNodeNoReplace(*this, prop->_key, prop, nullptr);

      UniqueString *name = outer_.propertyKeyAsIdentifier(prop->_key);
      if (!name || name == outer_.kw_.identUnderscoreProto) {
        outer_.sm_.warning(
            prop->_key->getSourceRange(),
            "ft: unsupported key for typed object, assuming 'any'");
        assumeAny = true;
        break;
      }

      Type *constraintValueType = nullptr;
      if (constraintObjectType) {
        if (auto optField = constraintObjectType->findField(
                Identifier::getFromPointer(name))) {
          constraintValueType =
              constraintObjectType->getFields()[*optField].type;
        }
      }

      // Use the value constraint to visit the value node.
      visitESTreeNodeNoReplace(*this, prop->_value, prop, constraintValueType);

      Type *valueType = outer_.getNodeTypeOrAny(prop->_value);

      if (constraintValueType) {
        auto cf = outer_.canAFlowIntoB(valueType, constraintValueType);
        if (!cf.canFlow) {
          outer_.sm_.error(
              prop->_value->getSourceRange(),
              llvh::Twine("ft: incompatible property type for '") +
                  name->str() + "'");
        }
        if (cf.needCheckedCast) {
          prop->_value =
              outer_.implicitCheckedCast(prop->_value, constraintValueType, cf);
        }
        valueType = constraintValueType;
      }

      // Handle duplicate keys.
      auto [it, inserted] = names.try_emplace(name, fields.size());
      if (inserted) {
        // New field.
        fields.emplace_back(Identifier::getFromPointer(name), valueType);
      } else {
        // Existing field, update the type to the later value's type.
        fields[it->second].type = valueType;
      }
    }

    if (assumeAny) {
      // Failed to make an object type that matches the properties.
      outer_.flowContext_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }

    outer_.setNodeType(
        node,
        outer_.flowContext_.createType(
            outer_.flowContext_.createExactObject(fields), node));
  }

  void visit(
      ESTree::SpreadElementNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // Do nothing for the spread element itself, handled by the parent.
    visitESTreeChildren(*this, node, nullptr);
  }

  void
  visit(ESTree::NullLiteralNode *node, ESTree::Node *parent, Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getNull());
  }
  void visit(
      ESTree::BooleanLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getBoolean());
  }
  void visit(
      ESTree::StringLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getString());
  }
  void visit(
      ESTree::TemplateLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    for (ESTree::Node &quasi : node->_quasis)
      outer_.setNodeType(&quasi, outer_.flowContext_.getString());
    visitESTreeNodeList(*this, node->_expressions, node, nullptr);
    outer_.setNodeType(node, outer_.flowContext_.getString());
  }
  void visit(
      ESTree::NumericLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getNumber());
  }
  void visit(
      ESTree::RegExpLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getAny());
  }
  void visit(
      ESTree::BigIntLiteralNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.setNodeType(node, outer_.flowContext_.getBigInt());
  }
  void
  visit(ESTree::SHBuiltinNode *node, ESTree::Node *parent, Type *constraint) {
    // SHBuiltin handled at the call expression level.
    visitESTreeChildren(*this, node, nullptr);
  }

  void visit(
      ESTree::UpdateExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeNode(*this, node->_argument, node, nullptr);
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
    del, voidOp, typeOf, plus, minus, tilde, bang, inc, dec
    // clang-format on
  };

  static UnopKind unopKind(llvh::StringRef str) {
    return llvh::StringSwitch<UnopKind>(str)
        .Case("delete", UnopKind::del)
        .Case("void", UnopKind::voidOp)
        .Case("typeof", UnopKind::typeOf)
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

  void visit(
      ESTree::BinaryExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeNode(*this, node->_left, node, nullptr);
    visitESTreeNode(*this, node->_right, node, nullptr);
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
        {UnopKind::typeOf, TypeKind::String, llvh::None},
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

  void visit(
      ESTree::UnaryExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeNode(*this, node->_argument, node, nullptr);
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

  void visit(
      ESTree::LogicalExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeNode(*this, node->_left, node, constraint);
    visitESTreeNode(*this, node->_right, node, constraint);
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

  void visit(
      ESTree::AssignmentExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
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

    // For a plain '=' assignment whose LHS is an ArrayPattern, visit the
    // RHS first so its type can be used as the constraint when typing the
    // pattern. We don't pass a constraint back to the RHS — destructuring
    // from an array literal isn't useful in practice, so we don't bother
    // preserving the legacy LHS-first walk for that case.
    bool arrayPatternLHS = node->_operator == outer_.kw_.identEqual &&
        llvh::isa<ESTree::ArrayPatternNode>(node->_left);

    Type *lhsConstraint = nullptr;
    if (arrayPatternLHS) {
      visitESTreeNode(*this, node->_right, node, nullptr);
      lhsConstraint = outer_.getNodeTypeOrAny(node->_right);
    }
    visitESTreeNode(*this, node->_left, node, lhsConstraint);

    // Check if the LHS is an accessor property.
    if (auto *mem = llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_left)) {
      if (!mem->_computed) {
        auto *objType = outer_.getNodeTypeOrAny(mem->_object);
        // Handle the assignment side of the fact that we specially handle
        // 'length' for FastArrays.
        if (outer_.flowContext_.isArrayClassType(objType)) {
          auto *id = llvh::cast<ESTree::IdentifierNode>(mem->_property);
          outer_.sm_.error(
              node->getSourceRange(),
              "ft: cannot assign to property '" + id->_name->str() +
                  "' of typed Array");
          return;
        }
        const ClassType::Field *accessorField = nullptr;
        if (auto *classType = llvh::dyn_cast<ClassType>(objType->info)) {
          accessorField = findAccessorField(
              outer_.astContext_,
              mem->_property,
              classType->getHomeObjectTypeInfo());
        } else if (
            auto *consType =
                llvh::dyn_cast<ClassConstructorType>(objType->info)) {
          auto *classTypeInfo =
              llvh::cast<ClassType>(consType->getClassType()->info);
          if (auto *staticInfo = classTypeInfo->getStaticObjectTypeInfo())
            accessorField = findAccessorField(
                outer_.astContext_, mem->_property, staticInfo);
        }
        if (accessorField) {
          if (!accessorField->hasSetter()) {
            outer_.sm_.error(
                node->getSourceRange(),
                "ft: cannot assign to getter-only property");
            return;
          }
          // Override the LHS type to the setter parameter type so the
          // RHS is checked against it.
          auto *setterFn =
              llvh::cast<TypedFunctionType>(accessorField->setterType->info);
          outer_.setNodeType(node->_left, setterFn->getParams()[0].type);
        }
      }
    }

    Type *lt = outer_.getNodeTypeOrAny(node->_left);

    Type *res;
    if (node->_operator->str() == "=") {
      if (!arrayPatternLHS) {
        // Use the type of the LHS as the constraint for the RHS for '='.
        visitESTreeNode(*this, node->_right, node, lt);
      }
      Type *rt = outer_.getNodeTypeOrAny(node->_right);

      auto [rtNarrow, cf] = outer_.tryNarrowType(rt, lt);
      if (!cf.canFlow) {
        outer_.sm_.error(
            node->getSourceRange(),
            "ft: incompatible assignment type: cannot implicitly cast from " +
                rt->messageString() + " to " + lt->messageString());
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
      visitESTreeNode(*this, node->_right, node, nullptr);
      Type *rt = outer_.getNodeTypeOrAny(node->_right);

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
        CanFlowResult cf = outer_.canAFlowIntoB(opResType, lt);
        if (!cf.canFlow) {
          outer_.sm_.error(
              node->getSourceRange(),
              "ft: incompatible assignment type: cannot implicitly cast from " +
                  rt->messageString() + " to " + lt->messageString());
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

  void visit(
      ESTree::ArrayPatternNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // This is only invoked for the LHS of an AssignmentExpression — variable
    // declaration patterns are handled directly by AnnotateScopeDecls.
    assert(
        !llvh::isa<ESTree::VariableDeclaratorNode>(parent) &&
        "use AnnotateScopeDecls for declarations");

    /// Verify that an already-visited non-pattern child's type is compatible
    /// with the expected element type of the destructuring.
    auto checkChild = [this](ESTree::Node *target, Type *expected) {
      Type *ct = outer_.getNodeTypeOrAny(target);
      CanFlowResult cf = outer_.canAFlowIntoB(expected, ct);
      if (!cf.canFlow || cf.needCheckedCast) {
        outer_.sm_.error(
            target->getSourceRange(),
            "ft: incompatible element type in array destructuring");
      }
    };

    // When the parent assignment knows the RHS type, it forwards it as
    // \p constraint. We use it to type the pattern correctly in a single
    // pass: an Array<T> constraint flows T into each element (and Array<T>
    // into a trailing rest binding), so IRGen dispatches to
    // emitDestructuringTypedArray. Other constraints (or no constraint) fall
    // through to the legacy tuple-of-children-types behavior, which lets
    // the assignment's tryNarrowType check element compatibility.
    if (outer_.flowContext_.isArrayClassType(constraint)) {
      // Default values aren't supported in typed array destructuring yet.
      // Emit a clear Sema error here so users don't get IRGen's generic
      // "unsupported destructuring target" message.
      for (ESTree::Node &child : node->_elements) {
        if (llvh::isa<ESTree::AssignmentPatternNode>(&child)) {
          outer_.sm_.error(
              child.getSourceRange(),
              "ft: default values are not yet supported "
              "in typed array destructuring");
          outer_.setNodeType(node, outer_.flowContext_.getAny());
          return;
        }
      }

      outer_.setNodeType(node, constraint);
      Type *elemType = outer_.flowContext_.getArrayElementType(constraint);

      for (ESTree::Node &child : node->_elements) {
        if (llvh::isa<ESTree::EmptyNode>(&child))
          continue;
        if (auto *rest = llvh::dyn_cast<ESTree::RestElementNode>(&child)) {
          // Rest binding receives the array type itself.
          visitESTreeNodeNoReplace(*this, rest->_argument, &child, constraint);
          if (!llvh::isa<ESTree::ArrayPatternNode>(rest->_argument)) {
            checkChild(rest->_argument, constraint);
          }
          break;
        }
        visitESTreeNodeNoReplace(*this, &child, node, elemType);
        if (!llvh::isa<ESTree::ArrayPatternNode>(&child)) {
          checkChild(&child, elemType);
        }
      }
      return;
    }

    // For tuple constraints, rest elements are not permitted. The parser
    // guarantees rest, if present, is the last element. Setting the node
    // type to any short-circuits the assignment-level narrow check so the
    // user only sees the actionable rest-rejection error.
    if (constraint && llvh::isa<TupleType>(constraint->info) &&
        !node->_elements.empty()) {
      if (auto *rest = llvh::dyn_cast<ESTree::RestElementNode>(
              &node->_elements.back())) {
        outer_.sm_.error(
            rest->getSourceRange(),
            "ft: rest element not allowed when destructuring a tuple");
        outer_.setNodeType(node, outer_.flowContext_.getAny());
        return;
      }
    }

    // Annotate the children of the array pattern.
    visitESTreeChildren(*this, node, nullptr);

    llvh::SmallVector<Type *, 4> types;
    for (ESTree::Node &elem : node->_elements)
      types.push_back(outer_.getNodeTypeOrAny(&elem));

    outer_.setNodeType(
        node,
        outer_.flowContext_.createType(
            outer_.flowContext_.createTuple(types), node));
  }

  void visit(
      ESTree::ConditionalExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeNode(*this, node->_test, node, nullptr);
    visitESTreeNode(*this, node->_consequent, node, constraint);
    visitESTreeNode(*this, node->_alternate, node, constraint);

    Type *types[2]{
        outer_.getNodeTypeOrAny(node->_consequent),
        outer_.getNodeTypeOrAny(node->_alternate)};

    // The result of a conditional is the union of the two types.
    outer_.setNodeType(
        node,
        outer_.flowContext_.createType(
            outer_.flowContext_.maybeCreateUnion(types)));
  }

  void visit(
      ESTree::TypeParameterInstantiationNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // Do nothing.
    // These are handled in the parent node.
  }

  void visit(
      ESTree::CallExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    // Check for $SHBuiltin.
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
      if (llvh::isa<ESTree::SHBuiltinNode>(methodCallee->_object)) {
        checkSHBuiltin(
            node, llvh::cast<ESTree::IdentifierNode>(methodCallee->_property));
        return;
      }
    }

    // Whether we have to visit the arguments or not depends on whether we
    // visited them during generic type argument inference.
    bool shouldVisitArguments = true;
    // Whether we need to visit the callee. Set to false when the callee has
    // already been visited above.
    bool shouldVisitCallee = true;
    // Whether overload resolution was already performed (e.g. via explicit
    // type arguments). Skip the later overloaded method check if true.
    bool overloadResolved = false;

    // Check for fn.call(thisArg, args...) on a function-typed receiver.
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee);
        methodCallee && !methodCallee->_computed &&
        !llvh::isa<ESTree::SuperNode>(methodCallee->_object)) {
      if (auto *propId =
              llvh::dyn_cast<ESTree::IdentifierNode>(methodCallee->_property);
          propId && propId->_name == outer_.kw_.identCall) {
        // Visit just the object so we can inspect its type.
        visitESTreeNode(*this, methodCallee->_object, methodCallee, nullptr);
        Type *objType = outer_.getNodeTypeOrAny(methodCallee->_object);
        if (llvh::isa<BaseFunctionType>(objType->info)) {
          if (node->_typeArguments) {
            outer_.sm_.error(
                node->_typeArguments->getSourceRange(),
                "ft: type arguments not allowed on function.call");
            return;
          }
          checkFunctionPrototypeCall(node, methodCallee->_object, objType);
          return;
        }
        // Receiver isn't a function: finish resolving the member expression
        // (without re-visiting the object) and let the regular path emit the
        // appropriate diagnostic.
        resolveMemberExpressionType(methodCallee, node);
        shouldVisitCallee = false;
      }
    }

    // Handle generic calls and method calls.

    if (auto *identCallee =
            llvh::dyn_cast<ESTree::IdentifierNode>(node->_callee)) {
      sema::Decl *decl = outer_.getDecl(identCallee);
      if (decl->generic) {
        if (node->_typeArguments) {
          outer_.resolveCallToGenericFunctionSpecialization(
              node, identCallee, decl);
        } else {
          // Attempt to infer the type arguments.
          auto [visited, typeArgs] =
              outer_.inferTypeArgumentsForGenericFunctionCall(
                  node, identCallee, decl);
          if (visited)
            shouldVisitArguments = false;
          if (typeArgs.empty()) {
            outer_.sm_.error(
                node->getStartLoc(),
                "could not infer type arguments for generic function");
            return;
          }
          outer_.resolveCallToGenericFunctionSpecializationWithParsedTypes(
              node, identCallee, typeArgs, decl);
        }
      }
    } else if (auto *memCallee =
                   llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee);
               memCallee && node->_typeArguments && !memCallee->_computed) {
      // Handle explicit type arguments on generic method calls.
      // Visit the object early to determine the class type, unless the
      // callee has already been visited above (in which case its object
      // has been too).
      if (shouldVisitCallee)
        visitESTreeNode(*this, memCallee->_object, memCallee, nullptr);
      shouldVisitCallee = false;
      Type *objType = outer_.getNodeTypeOrAny(memCallee->_object);

      // Determine property name and whether it's private.
      bool isPrivate;
      Identifier name;
      if (auto *pn =
              llvh::dyn_cast<ESTree::PrivateNameNode>(memCallee->_property)) {
        isPrivate = true;
        name = outer_.astContext_.getPrivateNameIdentifier(
            llvh::cast<ESTree::IdentifierNode>(pn->_id)->_name);
      } else {
        auto *id = llvh::cast<ESTree::IdentifierNode>(memCallee->_property);
        isPrivate = false;
        name = Identifier::getFromPointer(id->_name);
      }

      // Look up the method in the appropriate type.
      const ClassType::Field *field = nullptr;
      if (auto *classType = llvh::dyn_cast<ClassType>(objType->info)) {
        field =
            outer_.lookupPropertyOnClass(classType, name, memCallee->_property)
                .second;
      } else if (
          auto *consType =
              llvh::dyn_cast<ClassConstructorType>(objType->info)) {
        auto *classTypeInfo =
            llvh::cast<ClassType>(consType->getClassType()->info);
        if (auto *staticInfo = classTypeInfo->getStaticObjectTypeInfo()) {
          auto optMethod = isPrivate ? staticInfo->findPrivateField(name)
                                     : staticInfo->findPublicField(name);
          if (optMethod)
            field = optMethod->getField();
        }
      }

      if (field) {
        if (field->isOverloaded()) {
          auto *typeArgsNode =
              llvh::cast<ESTree::TypeParameterInstantiationNode>(
                  node->_typeArguments);
          if (!resolveOverloadedMethodCall(
                  node, memCallee, field, typeArgsNode))
            return;
          shouldVisitArguments = false;
          overloadResolved = true;
        } else if (llvh::isa<GenericType>(field->type->info)) {
          outer_.resolveCallToGenericMethodSpecialization(
              node, memCallee, field->method);
        } else {
          outer_.sm_.error(
              node->_typeArguments->getSourceRange(),
              "ft: type arguments provided for non-generic method");
          return;
        }
      }
    } else if (node->_typeArguments) {
      // Generics handled above.
      outer_.sm_.error(
          node->_callee->getSourceRange(), "ft: invalid generic function call");
      return;
    }

    // Don't visit the arguments yet, since we may be able to constrain their
    // types using the type of the function.
    if (shouldVisitCallee) {
      visitESTreeNode(*this, node->_callee, node, nullptr);
    }
    Type *calleeType = outer_.getNodeTypeOrAny(node->_callee);

    // Handle overloaded method calls if they haven't been resolved based on
    // explicit type arguments.
    // Check before generic inference since some overloads may be generic.
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee);
        !overloadResolved && methodCallee && !methodCallee->_computed) {
      Type *objType = outer_.getNodeTypeOrAny(methodCallee->_object);
      const flow::ClassType::Field *field = nullptr;

      Identifier name;
      bool isPrivate = false;
      if (auto *pn = llvh::dyn_cast<ESTree::PrivateNameNode>(
              methodCallee->_property)) {
        name = outer_.astContext_.getPrivateNameIdentifier(
            llvh::cast<ESTree::IdentifierNode>(pn->_id)->_name);
        isPrivate = true;
      } else {
        auto *id = llvh::cast<ESTree::IdentifierNode>(methodCallee->_property);
        name = Identifier::getFromPointer(id->_name);
      }

      if (auto *classType = llvh::dyn_cast<flow::ClassType>(objType->info)) {
        field =
            outer_
                .lookupPropertyOnClass(classType, name, methodCallee->_property)
                .second;
      } else if (
          auto *consType =
              llvh::dyn_cast<flow::ClassConstructorType>(objType->info)) {
        auto *classTypeInfo =
            llvh::cast<flow::ClassType>(consType->getClassType()->info);
        if (auto *staticInfo = classTypeInfo->getStaticObjectTypeInfo()) {
          auto optMethod = isPrivate ? staticInfo->findPrivateField(name)
                                     : staticInfo->findPublicField(name);
          if (optMethod)
            field = optMethod->getField();
        }
      }
      if (field && field->isOverloaded()) {
        if (!resolveOverloadedMethodCall(node, methodCallee, field))
          return;
        shouldVisitArguments = false;
        calleeType = outer_.getNodeTypeOrAny(node->_callee);
      }
    }

    // Handle generic method inference (no explicit type arguments).
    // After visiting the callee, if the type is GenericType, attempt to
    // infer the type arguments from the call arguments.
    if (llvh::isa<GenericType>(calleeType->info)) {
      if (auto *memCallee =
              llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee);
          memCallee && !memCallee->_computed) {
        Type *objType = outer_.getNodeTypeOrAny(memCallee->_object);

        // Determine property name and whether it's private.
        bool isPrivate;
        Identifier name;
        if (auto *pn =
                llvh::dyn_cast<ESTree::PrivateNameNode>(memCallee->_property)) {
          isPrivate = true;
          name = outer_.astContext_.getPrivateNameIdentifier(
              llvh::cast<ESTree::IdentifierNode>(pn->_id)->_name);
        } else {
          auto *id = llvh::cast<ESTree::IdentifierNode>(memCallee->_property);
          isPrivate = false;
          name = Identifier::getFromPointer(id->_name);
        }

        OptValue<ClassType::FieldLookupEntry> optMethod;
        if (auto *classType = llvh::dyn_cast<ClassType>(objType->info)) {
          auto *homeObj = classType->getHomeObjectTypeInfo();
          optMethod = isPrivate ? homeObj->findPrivateField(name)
                                : homeObj->findPublicField(name);
        } else if (
            auto *consType =
                llvh::dyn_cast<ClassConstructorType>(objType->info)) {
          auto *classTypeInfo =
              llvh::cast<ClassType>(consType->getClassType()->info);
          if (auto *staticInfo = classTypeInfo->getStaticObjectTypeInfo()) {
            optMethod = isPrivate ? staticInfo->findPrivateField(name)
                                  : staticInfo->findPublicField(name);
          }
        }

        if (optMethod && optMethod->getField()->method) {
          auto [didVisitArgs, typeArgs] =
              outer_.inferTypeArgumentsForGenericMethodCall(
                  node, memCallee, optMethod->getField()->method);
          if (didVisitArgs)
            shouldVisitArguments = false;
          if (typeArgs.empty()) {
            outer_.sm_.error(
                node->getStartLoc(),
                "ft: could not infer type arguments for generic method");
            return;
          }
          outer_.resolveCallToGenericMethodSpecializationWithParsedTypes(
              node, memCallee, typeArgs, optMethod->getField()->method);
          calleeType = outer_.getNodeTypeOrAny(node->_callee);
        }
      }
    }

    // Handle builtin method calls.
    // Must be after visiting the callee since setBuiltinMethodDecl is called
    // during MemberExpression visit.
    if (auto *methodCallee =
            llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
      if (auto *builtinDecl = outer_.getBuiltinMethodDecl(methodCallee)) {
        if (outer_.resolveBuiltinMethodCall(node, methodCallee, builtinDecl))
          shouldVisitArguments = false;
        calleeType = outer_.getNodeTypeOrAny(node->_callee);
      }
    }

    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info)) {
      visitESTreeNodeList(*this, node->_arguments, node, nullptr);
      return;
    }

    if (!llvh::isa<BaseFunctionType>(calleeType->info)) {
      outer_.sm_.error(
          node->_callee->getSourceRange(), "ft: callee is not a function");
      return;
    }

    // If the callee is an untyped function, we have nothing to check.
    if (llvh::isa<UntypedFunctionType>(calleeType->info)) {
      visitESTreeNodeList(*this, node->_arguments, node, nullptr);
      outer_.setNodeType(node, outer_.flowContext_.getAny());
      return;
    }

    Type *returnType;
    llvh::ArrayRef<TypedFunctionType::Param> params{};

    if (auto *ftype = llvh::dyn_cast<TypedFunctionType>(calleeType->info)) {
      returnType = ftype->getReturnType();
      params = ftype->getParams();
    } else {
      auto *nftype = llvh::cast<NativeFunctionType>(calleeType->info);
      returnType = nftype->getReturnType();
      params = nftype->getParams();
    }

    if (shouldVisitArguments) {
      size_t i = 0;
      for (ESTree::Node &argNode : node->_arguments) {
        // Constrain types of arguments before visiting when possible.
        Type *argConstraint = i < params.size() ? params[i].type : nullptr;
        // Don't bother with error reporting here, we'll report them later
        // when we actually try to typecheck the arguments.
        visitESTreeNodeNoReplace(*this, &argNode, node, argConstraint);
        ++i;
      }
    }

    if (auto *ftype = llvh::dyn_cast<TypedFunctionType>(calleeType->info)) {
      Type *expectedThisType = ftype->getThisParam()
          ? ftype->getThisParam()
          : outer_.flowContext_.getAny();

      // Check the type of "this".
      if (auto *methodCallee =
              llvh::dyn_cast<ESTree::MemberExpressionNode>(node->_callee)) {
        Type *thisArgType = nullptr;
        if (llvh::isa<ESTree::SuperNode>(methodCallee->_object)) {
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

        if (!outer_.canAFlowIntoB(thisArgType->info, expectedThisType->info)
                 .canFlow) {
          outer_.sm_.error(
              methodCallee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      } else if (llvh::isa<ESTree::SuperNode>(node->_callee)) {
        // 'super' calls implicitly pass the current class as 'this'.
        if (!outer_.curClassContext_->classType) {
          outer_.sm_.error(
              node->_callee->getSourceRange(),
              "ft: 'super' call outside class");
          return;
        }
        if (!outer_
                 .canAFlowIntoB(
                     outer_.curClassContext_->classType->info,
                     expectedThisType->info)
                 .canFlow) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      } else {
        if (!outer_
                 .canAFlowIntoB(
                     outer_.flowContext_.getVoid()->info,
                     expectedThisType->info)
                 .canFlow) {
          outer_.sm_.error(
              node->_callee->getSourceRange(), "ft: 'this' type mismatch");
          return;
        }
      }
    }

    outer_.setNodeType(node, returnType);
    checkCallArgumentTypes(params, node, node->_arguments, "function");
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
    if (builtin->_name == outer_.kw_.identModuleFactory) {
      checkSHBuiltinModuleFactory(call);
      return;
    }
    if (builtin->_name == outer_.kw_.identExport) {
      checkSHBuiltinExport(call);
      return;
    }
    if (builtin->_name == outer_.kw_.identImport) {
      checkSHBuiltinImport(call);
      return;
    }

    if (builtin->_name == outer_.kw_.identFastArrayPop) {
      checkSHBuiltinFastArrayPop(call);
      return;
    }

    if (builtin->_name == outer_.kw_.identFastArrayLength) {
      checkSHBuiltinFastArrayLength(call);
      return;
    }

    outer_.sm_.error(call->getSourceRange(), "unknown SH builtin call");
  }

  /// $SHBuiltin.fastArrayLength(arr: Array<T>): number.
  void checkSHBuiltinFastArrayLength(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    if (call->_arguments.size() != 1) {
      outer_.sm_.error(
          call->getSourceRange(),
          "ft: fastArrayLength requires exactly one argument");
      return;
    }
    ESTree::Node *arrArg = &call->_arguments.front();
    Type *argType = outer_.getNodeTypeOrAny(arrArg);
    if (!outer_.flowContext_.isArrayClassType(argType)) {
      outer_.sm_.error(
          arrArg->getSourceRange(),
          "ft: fastArrayLength argument must be an array");
      return;
    }
    outer_.setNodeType(call, outer_.flowContext_.getNumber());
  }

  /// $SHBuiltin.fastArrayPop(arr: Array<T>, n: number): T | void.
  void checkSHBuiltinFastArrayPop(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    if (call->_arguments.size() != 2) {
      outer_.sm_.error(
          call->getSourceRange(),
          "ft: fastArrayPop requires exactly two arguments");
      return;
    }
    auto it = call->_arguments.begin();
    ESTree::Node *arrArg = &*it++;
    ESTree::Node *countArg = &*it;
    Type *argType = outer_.getNodeTypeOrAny(arrArg);
    if (!outer_.flowContext_.isArrayClassType(argType)) {
      outer_.sm_.error(
          arrArg->getSourceRange(),
          "ft: fastArrayPop argument must be an array");
      return;
    }
    Type *countType = outer_.getNodeTypeOrAny(countArg);
    if (!llvh::isa<NumberType>(countType->info) &&
        !llvh::isa<AnyType>(countType->info)) {
      outer_.sm_.error(
          countArg->getSourceRange(),
          "ft: fastArrayPop count argument must be a number");
      return;
    }
    Type *elemType = outer_.flowContext_.getArrayElementType(argType);
    Type *voidType = outer_.flowContext_.getVoid();
    Type *resType = outer_.flowContext_.createType(
        outer_.flowContext_.maybeCreateUnion({elemType, voidType}));
    outer_.setNodeType(call, resType);
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
    visitESTreeNode(*this, callee, call, nullptr);

    /// Visit the rest of the arguments without any constraints,
    /// starting at \c it.
    auto visitRemainingArgumentsWithoutConstraint =
        [this, &it, call]() -> void {
      for (auto e = call->_arguments.end(); it != e; ++it) {
        ESTree::Node *arg = &*it;
        visitESTreeNode(*this, arg, call, nullptr);
      }
    };

    Type *calleeType = outer_.getNodeTypeOrAny(callee);
    // If the callee has no type, we have nothing to do/check.
    if (llvh::isa<AnyType>(calleeType->info)) {
      ++it;
      visitRemainingArgumentsWithoutConstraint();
      return;
    }
    if (!llvh::isa<BaseFunctionType>(calleeType->info)) {
      ++it;
      visitRemainingArgumentsWithoutConstraint();
      outer_.sm_.error(
          callee->getSourceRange(), "ft: callee is not a function");
      return;
    }
    if (llvh::isa<NativeFunctionType>(calleeType->info)) {
      ++it;
      visitRemainingArgumentsWithoutConstraint();
      outer_.sm_.error(
          callee->getSourceRange(),
          "ft: callee is a native function, cannot use $SHBuiltin.call");
      return;
    }
    auto *ftype = llvh::dyn_cast<TypedFunctionType>(calleeType->info);

    // If the callee is an untyped function, we have nothing to check.
    if (!ftype) {
      ++it;
      visitRemainingArgumentsWithoutConstraint();
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

    size_t i = 0;
    for (auto e = call->_arguments.end(); it != e; ++it) {
      Type *constraint =
          i < ftype->getParams().size() ? ftype->getParams()[i].type : nullptr;
      visitESTreeNodeNoReplace(*this, &*it, call, constraint);
      ++i;
    }

    Type *expectedThisType = ftype->getThisParam()
        ? ftype->getThisParam()
        : outer_.flowContext_.getAny();
    ESTree::Node *thisArg = &*it;
    Type *thisArgType = outer_.getNodeTypeOrAny(thisArg);
    if (!outer_.canAFlowIntoB(thisArgType->info, expectedThisType->info)
             .canFlow) {
      outer_.sm_.error(thisArg->getSourceRange(), "ft: 'this' type mismatch");
      return;
    }

    checkCallArgumentTypes(
        ftype->getParams(), call, call->_arguments, "function", 2);
    return;
  }

  /// Typecheck \c fn.call(thisArg, args...) on a function-typed receiver.
  /// Precondition: \p fn has been visited and \p fnType is its resolved
  /// BaseFunctionType.
  void checkFunctionPrototypeCall(
      ESTree::CallExpressionNode *call,
      ESTree::Node *fn,
      Type *fnType) {
    auto it = call->_arguments.begin();

    /// Visit the rest of the arguments without any constraints,
    /// starting at \c it.
    auto visitRemainingArgumentsWithoutConstraint =
        [this, &it, call]() -> void {
      for (auto e = call->_arguments.end(); it != e; ++it) {
        ESTree::Node *arg = &*it;
        visitESTreeNode(*this, arg, call, nullptr);
      }
    };

    assert(
        llvh::isa<BaseFunctionType>(fnType->info) &&
        "checkFunctionPrototypeCall requires a BaseFunctionType receiver");
    if (llvh::isa<NativeFunctionType>(fnType->info)) {
      visitRemainingArgumentsWithoutConstraint();
      outer_.sm_.error(
          fn->getSourceRange(),
          "ft: callee is a native function, cannot use function.call");
      return;
    }
    auto *ftype = llvh::dyn_cast<TypedFunctionType>(fnType->info);

    // If the receiver is an untyped function, we have nothing to check.
    if (!ftype) {
      visitRemainingArgumentsWithoutConstraint();
      outer_.setNodeType(call, outer_.flowContext_.getAny());
      return;
    }

    outer_.setNodeType(call, ftype->getReturnType());

    if (it == call->_arguments.end()) {
      outer_.sm_.error(
          call->getSourceRange(),
          "ft: function.call requires a 'this' argument");
      return;
    }

    // Visit thisArg with no parameter constraint (it is bound to 'this',
    // not to a regular parameter).
    ESTree::Node *thisArg = &*it;
    visitESTreeNodeNoReplace(*this, thisArg, call, nullptr);
    ++it;

    // Visit remaining args with constraints from the function's parameters.
    size_t i = 0;
    for (auto e = call->_arguments.end(); it != e; ++it) {
      Type *constraint =
          i < ftype->getParams().size() ? ftype->getParams()[i].type : nullptr;
      visitESTreeNodeNoReplace(*this, &*it, call, constraint);
      ++i;
    }

    Type *expectedThisType = ftype->getThisParam()
        ? ftype->getThisParam()
        : outer_.flowContext_.getAny();
    Type *thisArgType = outer_.getNodeTypeOrAny(thisArg);
    if (!outer_.canAFlowIntoB(thisArgType->info, expectedThisType->info)
             .canFlow) {
      outer_.sm_.error(thisArg->getSourceRange(), "ft: 'this' type mismatch");
      return;
    }

    checkCallArgumentTypes(
        ftype->getParams(), call, call->_arguments, "function.call", 1);
    return;
  }

  /// SHBuiltin.c_null().
  void checkSHBuiltinCNull(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    // Check the number and types of arguments.
    if (call->_arguments.size() != 0) {
      outer_.sm_.error(call->getSourceRange(), "ft: c_null takes no arguments");
      return;
    }
    outer_.setNodeType(call, outer_.flowContext_.getCPtr());
  }

  /// SHBuiltin.c_native_runtime().
  void checkSHBuiltinCNativeRuntime(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
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
    visitESTreeChildren(*this, call, nullptr);
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

  void checkSHBuiltinModuleFactory(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    assert(
        call->_arguments.size() == 2 &&
        "Ensured by checks in SemanticResolver.");
    auto argsIter = call->_arguments.begin();
    // Skip to the second argument.
    argsIter++;

    auto *modFactoryFunctionArg = &(*argsIter);
    // The type of the call is the type of the second (function) argument.
    outer_.setNodeType(call, outer_.getNodeTypeOrAny(modFactoryFunctionArg));
  }

  void checkSHBuiltinExport(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    // The type of the call is void.
    outer_.setNodeType(call, outer_.flowContext_.getVoid());
  }

  void checkSHBuiltinImport(ESTree::CallExpressionNode *call) {
    visitESTreeChildren(*this, call, nullptr);
    assert(
        call->_arguments.size() == 3 &&
        "Ensured by checks in SemanticResolver.");
    auto argsIter = call->_arguments.begin();
    argsIter++;
    argsIter++;

    auto *importExp = &(*argsIter);
    // The type of the call is the type of the third argument.
    outer_.setNodeType(call, outer_.getNodeTypeOrAny(importExp));
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

  void visit(
      ESTree::OptionalCallExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    outer_.sm_.error(
        node->getSourceRange(), "ft: optional call expression not supported");
  }

  void visit(
      ESTree::NewExpressionNode *node,
      ESTree::Node *parent,
      Type *constraint) {
    visitESTreeChildren(*this, node, nullptr);

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

    // Find the effective constructor type by walking up the class chain.
    // An implicit constructor forwards to the nearest ancestor's constructor.
    Type *consFType = nullptr;
    for (ClassType *cur = classTypeInfo; cur; cur = cur->getSuperClassInfo()) {
      if ((consFType = cur->getConstructorType()))
        break;
    }
    if (consFType) {
      checkCallArgumentTypes(
          llvh::cast<TypedFunctionType>(consFType->info)->getParams(),
          node,
          node->_arguments,
          "class " + classTypeInfo->getClassNameOrDefault() + " constructor");
    } else if (!node->_arguments.empty()) {
      outer_.sm_.error(
          node->getSourceRange(),
          "ft: class " + classTypeInfo->getClassNameOrDefault() +
              " does not have an explicit constructor");
      return;
    }
  }

  void visit(ESTree::SuperNode *node, ESTree::Node *parent, Type *constraint) {
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

  /// Check the types of the supplied arguments, adding checked casts if
  /// needed. If \p reportErrors is false, silently check without emitting
  /// errors or inserting casts.
  /// \param offset the number of arguments to ignore at the front of \p
  ///   arguments. Used for $SHBuiltin.call, which has extra args at the front.
  bool checkCallArgumentTypes(
      llvh::ArrayRef<TypedFunctionType::Param> params,
      ESTree::Node *callNode,
      ESTree::NodeList &arguments,
      const llvh::Twine &calleeName,
      uint32_t offset = 0,
      bool reportErrors = true) {
    size_t numArgs = arguments.size() - offset;
    bool hasRest = !params.empty() && params.back().rest;
    size_t numNonRestParams = hasRest ? params.size() - 1 : params.size();

    // \return whether the param may be omitted at the call site if it is
    // explicitly optional or if `void` flows into its type.
    auto paramOmittable = [this](const TypedFunctionType::Param &p) -> bool {
      return p.optional ||
          outer_.canAFlowIntoB(outer_.flowContext_.getVoid(), p.type).canFlow;
    };

    // Extract element type for rest param if present.
    Type *restElementType = nullptr;
    if (hasRest) {
      Type *restParamType = params.back().type;
      if (outer_.flowContext_.isArrayClassType(restParamType)) {
        restElementType =
            outer_.flowContext_.getArrayElementType(restParamType);
      }
    }

    if (hasRest) {
      // With rest param, need at least the required non-rest params.
      size_t numRequired = numNonRestParams;
      while (numRequired > 0 && paramOmittable(params[numRequired - 1]))
        --numRequired;
      if (numArgs < numRequired) {
        if (reportErrors) {
          outer_.sm_.error(
              callNode->getSourceRange(),
              "ft: " + calleeName + " expects at least " +
                  llvh::Twine(numRequired) + " arguments, but " +
                  llvh::Twine(numArgs) + " supplied");
        }
        return false;
      }
    } else if (
        numArgs > params.size() ||
        !llvh::all_of(params.drop_front(numArgs), paramOmittable)) {
      // Reject extra args, or fewer args when some trailing missing param
      // is not omittable.
      if (reportErrors) {
        // Count the number of required parameters.
        size_t numRequired = params.size();
        while (numRequired > 0 && paramOmittable(params[numRequired - 1]))
          --numRequired;
        outer_.sm_.error(
            callNode->getSourceRange(),
            "ft: " + calleeName + " expects " +
                (numArgs > params.size()
                     ? "at most " + llvh::Twine(params.size())
                     : (numRequired != params.size()
                            ? "at least " + llvh::Twine(numRequired)
                            : llvh::Twine(params.size()))) +
                " arguments, but " + llvh::Twine(numArgs) + " supplied");
      }
      return false;
    }

    auto begin = arguments.begin();
    std::advance(begin, offset);

    // Check the type of each argument.
    size_t argIndex = 0;
    for (auto it = begin, e = arguments.end(); it != e; ++argIndex, ++it) {
      ESTree::Node *arg = &*it;

      if (llvh::isa<ESTree::SpreadElementNode>(arg)) {
        if (reportErrors) {
          outer_.sm_.error(
              arg->getSourceRange(), "ft: argument spread is not supported");
        }
        return false;
      }

      Type *expectedType;
      if (argIndex < numNonRestParams) {
        expectedType = params[argIndex].type;
      } else if (restElementType) {
        expectedType = restElementType;
      } else {
        // Rest param without Array<T> type, skip checking.
        continue;
      }

      Type *argType = outer_.getNodeTypeOrAny(arg);
      auto [argTypeNarrow, cf] = outer_.tryNarrowType(argType, expectedType);

      if (!cf.canFlow) {
        if (reportErrors) {
          std::string argTypeStr = argType->messageString();
          std::string expectedTypeStr = expectedType->messageString();
          if (argIndex < numNonRestParams && params[argIndex].name.isValid()) {
            outer_.sm_.error(
                arg->getSourceRange(),
                "ft: " + calleeName + " parameter '" +
                    params[argIndex].name.str() +
                    "' type mismatch: cannot assign " + argTypeStr + " to " +
                    expectedTypeStr);
          } else {
            outer_.sm_.error(
                arg->getSourceRange(),
                "ft: " + calleeName + " parameter #" +
                    llvh::Twine(argIndex + 1) +
                    " type mismatch: cannot assign " + argTypeStr + " to " +
                    expectedTypeStr);
          }
        }
        return false;
      }
      // If a cast is needed, replace the argument with the cast.
      if (reportErrors && cf.needCheckedCast && outer_.compile_) {
        // Insert the new node before the current node and erase the
        // current one.
        auto newIt = arguments.insert(
            it, *outer_.implicitCheckedCast(arg, argTypeNarrow, cf));
        arguments.erase(it);
        it = newIt;
      }
    }

    return true;
  }

  /// Resolve a call to an overloaded method by selecting the matching
  /// overload and setting the callee type. Does not set the return type
  /// on the call node — the caller should fall through to the normal
  /// call checking path for argument type checking and return type.
  /// On success, the call's arguments have already been visited.
  /// \param explicitTypeArgs optional explicit type arguments node from the
  ///   call expression. If non-null, only generic overloads with matching
  ///   type-parameter count are considered, and the explicit types are used
  ///   directly instead of inference; non-generic overloads are skipped.
  /// \return false on error (already reported), true on success.
  LLVM_NODISCARD bool resolveOverloadedMethodCall(
      ESTree::CallExpressionNode *node,
      ESTree::MemberExpressionNode *callee,
      const ClassType::Field *field,
      ESTree::TypeParameterInstantiationNode *explicitTypeArgs = nullptr) {
    assert(field->isOverloaded() && "field must be overloaded");

    Type *objType = outer_.getNodeTypeOrAny(callee->_object);

    // Pre-parse explicit type arguments if provided.
    llvh::SmallVector<Type *, 2> explicitTypeArgTypes;
    if (explicitTypeArgs) {
      for (ESTree::Node &arg : explicitTypeArgs->_params) {
        explicitTypeArgTypes.push_back(outer_.parseTypeAnnotation(&arg));
      }
    }

    // Visit arguments without constraints to determine their types.
    for (ESTree::Node &arg : node->_arguments) {
      outer_.visitExpression(&arg, node, nullptr);
    }

    // Check each overload for type compatibility with the
    // arguments, selecting the unique match.
    // If there's two matches, it's an ambiguous callsite.
    // If there's no matches it doesn't typecheck.
    Type *matchedType = nullptr;
    sema::Decl *matchedDecl = nullptr;

    for (const auto &[overloadMethod, originalOverloadType] :
         field->overloads) {
      Type *overloadType = originalOverloadType;
      sema::Decl *overloadDecl = nullptr;

      if (llvh::isa<GenericType>(originalOverloadType->info)) {
        llvh::SmallVector<Type *, 2> typeArgs;
        if (explicitTypeArgs) {
          // Only consider generic overloads whose type-parameter count
          // matches the number of explicit type arguments.
          auto *fe = llvh::cast<ESTree::FunctionExpressionNode>(
              overloadMethod->_value);
          auto *typeParams = llvh::cast<ESTree::TypeParameterDeclarationNode>(
              fe->_typeParameters);
          if (typeParams->_params.size() != explicitTypeArgTypes.size())
            continue;
          typeArgs.assign(
              explicitTypeArgTypes.begin(), explicitTypeArgTypes.end());
        } else {
          // Infer type arguments and specialize.
          auto [didVisitArgs, inferred] =
              outer_.inferTypeArgumentsForGenericMethodCall(
                  node, callee, overloadMethod);
          if (inferred.empty())
            continue;
          typeArgs = std::move(inferred);
        }
        auto result = outer_.specializeGenericMethodWithParsedTypes(
            overloadMethod, node->getSourceRange(), typeArgs, objType);
        if (!result.type)
          continue;
        overloadType = result.type;
        overloadDecl = result.decl;
      } else {
        // Non-generic overload: explicit type arguments don't apply.
        if (explicitTypeArgs)
          continue;
        overloadDecl = outer_.semContext_.getExpressionDecl(
            ESTree::getPropertyIdentifier(overloadMethod->_key));
      }

      auto *fnType = llvh::dyn_cast<TypedFunctionType>(overloadType->info);
      if (!fnType)
        continue;

      // Check arity and argument types (silent — no error reporting).
      if (!checkCallArgumentTypes(
              fnType->getParams(),
              node,
              node->_arguments,
              /* calleeName */ "",
              /* offset */ 0,
              /* reportErrors */ false))
        continue;

      if (matchedType) {
        outer_.sm_.error(
            node->getSourceRange(),
            "ft: ambiguous call: multiple overloads match");
        return false;
      }
      matchedType = overloadType;
      matchedDecl = overloadDecl;
    }

    if (!matchedType) {
      outer_.sm_.error(
          node->getSourceRange(), "ft: no matching overload for call");
      return false;
    }

    assert(matchedDecl && "overloaded methods have a Decl (they are final)");
    outer_.semContext_.setExpressionDecl(
        ESTree::getPropertyIdentifier(callee->_property), matchedDecl);

    outer_.setNodeType(callee, matchedType);
    return true;
  }
};

void FlowChecker::visitExpression(
    ESTree::Node *node,
    ESTree::Node *parent,
    Type *constraint) {
  ExprVisitor v(*this);
  visitESTreeNode(v, node, parent, constraint);
}

} // namespace flow
} // namespace hermes

#endif // HERMES_PARSE_FLOW
