/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//===----------------------------------------------------------------------===//
/// \file
/// Implementation for DeclareScopeTypes, along with FlowChecker functions that
/// allow calling into DeclareScopeTypes:
/// declareScopeTypes and resolveGenericTypeAlias.
/// Includes DeclareScopeTypes dependencies:
/// FindLoopingTypes and GenericTypeInstantiation.
///
/// Type aliases combined with unions create a **dramatic complication**, since
/// they can be mutually self recursive. We need to declare types in stages,
/// first the "direct" ones, then resolve the aliases, unions, and generics.
///
/// 1. Iterate all scope types. Forward-declare all declared types as Type *.
/// 2. Resolve the aliases, checking for self-references.
/// 3. Complete all forward declared types. They can now refer to the newly
/// declared local types.
///
/// Generic type aliases are registered along with the other generics,
/// but they don't require an AST node to be cloned during specialization
/// (IRGen isn't going to look at it) so their specializations are just the
/// resolved \c Type *.
//===----------------------------------------------------------------------===//

#include "FlowChecker.h"

#include "llvh/ADT/MapVector.h"
#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/SaveAndRestore.h"

#define DEBUG_TYPE "FlowChecker"

namespace hermes {
namespace flow {

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

class FlowChecker::DeclareScopeTypes {
  /// Surrounding class.
  FlowChecker &outer;
  /// The current lexical scope.
  sema::LexicalScope *const scope;
  /// The current lexical scope.
  /// Only used when registering newly discovered generics,
  /// nullptr when DeclareScopeTypes is called to resolve a generic type alias.
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
  llvh::MapVector<Type *, FlowChecker::GenericTypeInstantiation>
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
  std::vector<FlowChecker::DeferredGenericClass> deferredParseGenerics{};

 public:
  DeclareScopeTypes(
      FlowChecker &outer,
      const sema::ScopeDecls &decls,
      sema::LexicalScope *scope,
      ESTree::Node *scopeNode)
      : outer(outer), scope(scope), scopeNode(scopeNode) {
    llvh::SaveAndRestore savedDeferredGenerics{
        outer.deferredParseGenerics_, &deferredParseGenerics};

    createForwardDeclarations(decls);
    resolveAllAliases();
    completeForwardDeclarations();
    parseDeferredGenericClasses();
  }

  /// Run DeclareScopeTypes starting from a single generic type annotation.
  /// \return the resolved type for the generic type alias specialization.
  static Type *resolveGenericTypeAlias(
      FlowChecker &outer,
      ESTree::GenericTypeAnnotationNode *gta,
      TypeDecl *typeDecl) {
    DeclareScopeTypes declareScopeTypes(outer, typeDecl->scope);
    llvh::SmallDenseSet<ESTree::TypeAliasNode *> visited{};
    // Don't call createForwardDeclarations, because we're directly calling
    // resolveTypeAnnotation and we don't have a list of decls.
    Type *type = declareScopeTypes.resolveTypeAnnotation(gta, visited, 0);
    declareScopeTypes.completeForwardDeclarations();
    declareScopeTypes.parseDeferredGenericClasses();
    return type;
  }

 private:
  // Create an empty DeclareScopeTypes, used for resolveGenericTypeAlias.
  DeclareScopeTypes(FlowChecker &outer, sema::LexicalScope *scope)
      : outer(outer), scope(scope), scopeNode(nullptr) {}

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
    assert(scopeNode && "no scope node found");
    if (auto *funcNode = llvh::dyn_cast<ESTree::FunctionLikeNode>(scopeNode)) {
      return ESTree::getBlockStatement(funcNode);
    }
    return scopeNode;
  }

  /// Forward declare all classes and record all aliases for later processing.
  void createForwardDeclarations(const sema::ScopeDecls &decls) {
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

        // Generic type alias, register it but just move on.
        if (aliasNode->_typeParameters) {
          outer.bindingTable_.try_emplace(
              id->_name, TypeDecl(nullptr, scope, aliasNode));
          outer.registerGenericAlias(
              aliasNode,
              getGenericParentNode(),
              outer.bindingTable_.getCurrentScope());
          continue;
        }

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

      if (!typeDecl->type) {
        // Found a generic TypeDecl.
        // Resolve to the generic specialization.
        if (!gta->_typeParameters) {
          outer.sm_.error(
              gta->getSourceRange(),
              llvh::Twine("ft: missing generic arguments for '") +
                  id->_name->str() + "'");
          return outer.flowContext_.getAny();
        }

        // Make a placeholder type to represent the generic type for now.
        // The TypeInfo will be replaced with the concrete Type when we are
        // able to resolve TypeInfo for all type arguments.
        // We don't yet care whether this is a generic class or type alias.
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
    if (aliasType->info && !llvh::isa<GenericType>(aliasType->info))
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
    // Instantiating generic type aliases may cause new forward declarations
    // to be created. To account for this, keep iterating until no more are
    // introduced.
    size_t unionIdx = 0;
    size_t genericIdx = 0;
    while (unionIdx < forwardUnions.size() ||
           genericIdx < forwardGenericInstantiations.size()) {
      // Complete all forward-declared unions.
      // First find all the recursive union arms.
      for (size_t i = unionIdx, e = forwardUnions.size(); i < e; ++i) {
        Type *type = forwardUnions[i];
        FindLoopingTypes(loopingUnionArms, forwardGenericInstantiations, type);
      }
      // Now simplify and canonicalize as many union arms as possible.
      for (size_t e = forwardUnions.size(); unionIdx < e; ++unionIdx) {
        Type *type = forwardUnions[unionIdx];
        llvh::SetVector<Type *> visited{};
        completeForwardType(type, visited);
      }
      // Now simplify and canonicalize the foward generics.
      for (size_t e = forwardGenericInstantiations.size(); genericIdx < e;
           ++genericIdx) {
        // forwardGenericInstantiations may expand so we can't store the
        // iterator.
        auto &[type, generic] =
            *(forwardGenericInstantiations.begin() + genericIdx);
        llvh::SetVector<Type *> visited{};
        completeForwardGeneric(type, generic, visited);
      }
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
      outer.parseClassType(
          classNode->_superClass,
          classNode->_superTypeParameters,
          classNode->_body,
          type);
    }
  }

  /// Complete the forward declaration of the given \p type,
  /// replacing its \c info field with the resolved type.
  void completeForwardType(Type *type, llvh::SetVector<Type *> &visited) {
    LLVM_DEBUG(
        llvh::dbgs() << "Completing forward type: " << type << " "
                     << type->info->getKindName() << '\n');

    if (llvh::isa<GenericType>(type->info)) {
      auto it = forwardGenericInstantiations.find(type);
      assert(it != forwardGenericInstantiations.end());
      completeForwardGeneric(type, it->second, visited);
      LLVM_DEBUG(
          llvh::dbgs() << "Completed forward type: " << type << " "
                       << type->info->getKindName() << '\n');
    }

    // A forward generic may resolve to a union, which needs to be completed.

    if (auto *unionType = llvh::dyn_cast<UnionType>(type->info)) {
      completeForwardUnion(type, unionType, visited);
      LLVM_DEBUG(
          llvh::dbgs() << "Completed forward type: " << type << " "
                       << type->info->getKindName() << '\n');
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
    assert(!typeDecl->type && "typeDecl must be generic");

    for (Type *arg : generic.typeArgTypes) {
      if (forwardUnions.count(arg) || llvh::isa<GenericType>(arg->info)) {
        completeForwardType(arg, visited);
      }
    }

    if (!typeDecl->genericClassDecl) {
      // No genericClassDecl, this is a generic type alias.
      auto *aliasNode = llvh::cast<ESTree::TypeAliasNode>(typeDecl->astNode);
      GenericInfo<Type> &genericInfo =
          outer.getGenericAliasInfoMustExist(aliasNode);

      GenericInfo<Type>::TypeArgsVector typeArgs;
      for (Type *arg : generic.typeArgTypes)
        typeArgs.push_back(arg->info);
      GenericInfo<Type>::TypeArgsRef typeArgsRef = typeArgs;

      if (Type *resolved = genericInfo.getSpecialization(typeArgs)) {
        LLVM_DEBUG(
            llvh::errs() << "Found specialization for " << type << " "
                         << resolved << "\n");
        typeAliasResolutions[type] = resolved;
        populateTypeAlias(type);
        return;
      }

      // This is not a generic class, it's a generic type alias.
      // Specialize it with the provided arguments by resolving the type.
      ScopeRAII paramScope{outer};
      bool populated = outer.validateAndBindTypeParameters(
          llvh::cast<ESTree::TypeParameterDeclarationNode>(
              aliasNode->_typeParameters),
          llvh::cast<ESTree::TypeParameterInstantiationNode>(
              generic.annotation->_typeParameters),
          generic.typeArgTypes,
          scope);
      if (!populated) {
        LLVM_DEBUG(llvh::dbgs() << "Failed to bind type parameters\n");
        type->info = outer.flowContext_.getAnyInfo();
        return;
      }

      // Resolve the generic type alias to its specialization.
      // This may add to forwardGenericInstantiations and forwardUnions.
      llvh::SmallDenseSet<ESTree::TypeAliasNode *> visitedTypes{};
      Type *resolved =
          resolveTypeAnnotation(aliasNode->_right, visitedTypes, 0);

      typeArgsRef =
          genericInfo.addSpecialization(outer, std::move(typeArgs), resolved);

      // The resolved type might not be complete, so we have to complete it
      // before populating the type alias.
      // We have to call FindLoopingTypes first, because we this type
      // didn't exist when FindLoopingTypes was initially called.
      if (forwardUnions.count(resolved) ||
          llvh::isa<GenericType>(resolved->info)) {
        FindLoopingTypes(
            loopingUnionArms, forwardGenericInstantiations, resolved);
        completeForwardType(resolved, visited);
      }

      type->info = resolved->info;
      return;
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
      outer.parseClassType(
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

/// Function allows FlowChecker.cpp to call into DeclareScopeTypes.
void FlowChecker::declareScopeTypes(
    const sema::ScopeDecls &decls,
    sema::LexicalScope *scope,
    ESTree::Node *scopeNode) {
  DeclareScopeTypes(*this, decls, scope, scopeNode);
}

/// Function allows FlowChecker.cpp to call into DeclareScopeTypes.
Type *FlowChecker::resolveGenericTypeAlias(
    ESTree::GenericTypeAnnotationNode *gta,
    TypeDecl *typeDecl) {
  return DeclareScopeTypes::resolveGenericTypeAlias(*this, gta, typeDecl);
}

} // namespace flow
} // namespace hermes
