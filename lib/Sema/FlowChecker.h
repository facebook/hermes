/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_FLOWCHECKER_H
#define HERMES_SEMA_FLOWCHECKER_H

#include "DeclCollector.h"

#include "hermes/ADT/PersistentScopedMap.h"
#include "hermes/AST/Context.h"
#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Sema/FlowContext.h"
#include "hermes/Sema/Keywords.h"
#include "hermes/Sema/SemContext.h"

namespace hermes {
namespace flow {

/// Specialization to allow hashing of types in accordance with the
/// TypeInfo::hash and TypeInfo::equals methods.
struct TypeArrayDenseMapInfo
    : public llvh::DenseMapInfo<llvh::ArrayRef<TypeInfo *>> {
  // static inline T getEmptyKey();
  // static inline T getTombstoneKey();
  static unsigned getHashValue(llvh::ArrayRef<TypeInfo *> val) {
    llvh::hash_code hash{0};
    for (const TypeInfo *type : val)
      hash = llvh::hash_combine(hash, type->hash());
    return hash;
  }
  static bool isEqual(
      llvh::ArrayRef<TypeInfo *> LHS,
      llvh::ArrayRef<TypeInfo *> RHS) {
    if (LHS == RHS)
      return true;
    auto const EMPTY = getEmptyKey();
    auto const TOMB = getTombstoneKey();
    if (LHS == EMPTY || LHS == TOMB || RHS == EMPTY || RHS == TOMB)
      return false;
    if (LHS.size() != RHS.size())
      return false;
    for (size_t i = 0, e = LHS.size(); i < e; ++i)
      if (!LHS[i]->equals(RHS[i]))
        return false;
    return true;
  }
};

/// Class the performs all resolution.
/// Reports errors if validation fails.
class FlowChecker : public ESTree::RecursionDepthTracker<FlowChecker> {
  /// AST context.
  Context &astContext_;

  /// A copy of Context::getSM() for easier access.
  SourceErrorManager &sm_;

  /// Buffer all generated messages and print them sorted in the end.
  SourceErrorManager::SaveAndBufferMessages bufferMessages_;

  sema::SemContext &semContext_;

  /// Flow types are stored here.
  FlowContext &flowContext_;

  /// Map of DeclCollector instances associated with every function.
  sema::DeclCollectorMapTy &declCollectorMap_;

  /// Keywords we will be checking for.
  sema::Keywords &kw_;

  struct TypeDecl {
    /// nullptr is used to indicate a generic declaration.
    Type *type;
    /// The lexical scope in which the type was declared.
    /// TODO: this is not used, should we remove it?
    sema::LexicalScope *scope;
    /// Optional AST node for reporting re-declarations.
    ESTree::Node *astNode;
    /// When this is a generic declaration, the Decl of the class.
    sema::Decl *genericClassDecl;
    explicit TypeDecl(
        Type *type,
        sema::LexicalScope *scope,
        ESTree::Node *astNode,
        sema::Decl *genericClassDecl = nullptr)
        : type(type),
          scope(scope),
          astNode(astNode),
          genericClassDecl(genericClassDecl) {
      assert(
          (!type ^ !genericClassDecl) && "either we have a type or a generic");
      assert(scope && "scope must be non-null");
    }
  };

  /// The scoped binding table mapping from string to binding.
  using TypeBindingTableTy =
      hermes::PersistentScopedMap<UniqueString *, TypeDecl>;
  using TypeBindingTableScopeTy =
      hermes::PersistentScopedMapScope<UniqueString *, TypeDecl>;
  using TypeBindingTableScopePtrTy =
      hermes::PersistentScopedMapScopePtr<UniqueString *, TypeDecl>;

  /// The currently lexically visible names of types.
  TypeBindingTableTy bindingTable_{};

  /// Information about the current function.
  class FunctionContext;

  /// The current function context.
  FunctionContext *curFunctionContext_ = nullptr;

  /// Information about the current class.
  class ClassContext;

  /// The current class context.
  ClassContext *curClassContext_ = nullptr;

  /// Holds information needed on how to instantiate a generic.
  class GenericInfo {
   public:
    /// Storage for type arguments.
    /// The Type pointers point to the storage in FlowContext, so they're
    /// stable.
    /// SmallVector is usable here because once these vectors are added to
    /// the typeArgStorage_ deque, they are never modified and won't move.
    using TypeArgsVector = llvh::SmallVector<TypeInfo *, 2>;

    /// Reference to a TypeArgs, used for storing in the DenseMap.
    using TypeArgsRef = llvh::ArrayRef<TypeInfo *>;

    /// Full AST for the generic.
    /// e.g. if it's a generic function, this is the FunctionLikeNode.
    ESTree::Node *originalNode;

    /// Parent of the generic declaration.
    /// Contains the NodeList to insert the AST into.
    ESTree::Node *parent;

    /// The binding table scope for the generic.
    /// Activated when the generic is instantiated.
    TypeBindingTableScopePtrTy bindingTableScope;

    /// Map from the list of type arguments to the specialized, typechecked
    /// resultant AST for the specialization.
    llvh::DenseMap<TypeArgsRef, ESTree::Node *, TypeArrayDenseMapInfo>
        specializations{};

   public:
    /// \param bindingTableScope takes shared ownership of the binding table
    ///   scope.
    GenericInfo(
        ESTree::Node *originalNode,
        ESTree::Node *parent,
        const TypeBindingTableScopePtrTy &bindingTableScope)
        : originalNode(originalNode),
          parent(parent),
          bindingTableScope(bindingTableScope) {
      // We need to insert into the parent.
      assert(
          (llvh::isa<ESTree::ProgramNode>(parent) ||
           llvh::isa<ESTree::BlockStatementNode>(parent)) &&
          "Invalid parent");
    }

    GenericInfo(const GenericInfo &other) = delete;
    GenericInfo operator=(const GenericInfo &other) = delete;

    /// \return the specialization if it exists, otherwise nullptr.
    ESTree::Node *getSpecialization(TypeArgsRef args);

    /// Add the specialization for \p args to the \c specializations map.
    /// \return a reference to the internal storage containing the type args.
    TypeArgsRef addSpecialization(
        FlowChecker &outer,
        TypeArgsVector &&args,
        ESTree::Node *node);
  };

  /// Types associated with declarations.
  llvh::DenseMap<const sema::Decl *, Type *> &declTypes_;
  /// The AST node for every declaration. Used for reporting re-declarations.
  llvh::DenseMap<sema::Decl *, ESTree::Node *> declNodes_{};

  /// Record the variable initializers visited when attempting to infer the
  /// types of declarations in order to avoid visiting them again.
  llvh::DenseSet<ESTree::Node *> visitedInits_{};

  /// True if we are preparing the AST to be compiled by Hermes, including
  /// erroring on features which we parse but don't compile and transforming
  /// the AST. False if we just want to validate the AST.
  bool const compile_;

  /// Mapping from native type names to their enum code. Populated by
  /// \c declareNativeTypes()
  llvh::DenseMap<UniqueString *, NativeCType> nativeTypes_;

  /// Storage for the TypeArgs vectors.
  /// Elements of typeArgStorage_ contain the storage for the keys in
  /// \c GenericInfo::specializations.
  /// Once stored here, the TypeArgsVectors are not modified, so TypeArgsRef
  /// pointers into the vectors can be stored.
  std::deque<GenericInfo::TypeArgsVector> typeArgStorage_{};

  /// Stable storage for the GenericInfo.
  std::deque<GenericInfo> generics_;

  /// Maps from the Decl to a reference to an element of \c generics_.
  /// Populated values are not nullable.
  /// Store a pointer to allow for empty/tombstone values in DenseMap.
  llvh::DenseMap<const sema::Decl *, GenericInfo *> genericsMap_;

  /// Description of the generic that we have deferred typechecking for.
  struct DeferredGenericClass {
    /// The declaration node.
    ESTree::ClassDeclarationNode *specialization;
    /// The scope where the generic was defined,
    /// with the parameter names bound to the corresponding type variables.
    TypeBindingTableScopePtrTy scope;
    /// The type for the deferred generic.
    Type *classType;

    DeferredGenericClass(
        ESTree::ClassDeclarationNode *specialization,
        TypeBindingTableScopePtrTy scope,
        Type *declType)
        : specialization(specialization), scope(scope), classType(declType) {}
  };

  /// List of generics that we haven't parsed yet because they might
  /// refer to other generics that haven't been parsed yet.
  std::vector<DeferredGenericClass> *deferredParseGenerics_ = nullptr;

  /// Queue of the generics that we haven't finished typechecking yet,
  /// which need their bodies typechecked.
  std::deque<DeferredGenericClass> typecheckQueue_;

 public:
  explicit FlowChecker(
      Context &astContext,
      sema::SemContext &semContext,
      FlowContext &flowContext,
      sema::DeclCollectorMapTy &declCollectorMap,
      bool compile);

  /// Run semantic resolution and store the result in \c semCtx_.
  /// \param rootNode the top-level program/JS module node to run resolution on.
  /// \return false on error.
  bool run(ESTree::ProgramNode *rootNode);

  /// Executed once at the top scope to define all native types. This is a hack.
  /// Eventually we should have a proper module system.
  void declareNativeTypes(sema::LexicalScope *rootScope);

  /// We call this when we exceed the maximum recursion depth.
  void recursionDepthExceeded(ESTree::Node *n);

  /// Default case for all ignored nodes, we still want to visit their children.
  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::ProgramNode *node);

  /// Run typechecking on a function that's known to not be generic.
  /// Called from visit(FunctionDeclarationNode *) when there are no type
  /// parameters, or after cloning a new specialization of a generic function.
  void visitNonGenericFunctionDeclaration(
      ESTree::FunctionDeclarationNode *node,
      sema::Decl *decl);

  void visit(ESTree::FunctionDeclarationNode *node);
  void visit(ESTree::FunctionExpressionNode *node);
  void visit(ESTree::ArrowFunctionExpressionNode *node);

  /// Run typechecking on the body of a class that we already have the type for.
  void visitClassNode(
      ESTree::Node *classNode,
      ESTree::ClassBodyNode *body,
      Type *classType);

  void visit(ESTree::ClassExpressionNode *node);
  void visit(ESTree::ClassDeclarationNode *node);
  void visit(ESTree::MethodDefinitionNode *node);

  void visit(ESTree::TypeAnnotationNode *node);
  void visit(ESTree::IdentifierNode *identifierNode);

  /// Used in an expression context.
  class ExprVisitor;

  /// Invoke the expression visitor.
  void visitExpression(ESTree::Node *node, ESTree::Node *parent);

  void visit(ESTree::ExpressionStatementNode *node);
  void visit(ESTree::IfStatementNode *node);
  void visit(ESTree::SwitchStatementNode *node);
  void visit(ESTree::SwitchCaseNode *node);
  void visit(ESTree::WhileStatementNode *node);
  void visit(ESTree::DoWhileStatementNode *node);
  void visit(ESTree::ForOfStatementNode *node);
  void visit(ESTree::ForInStatementNode *node);
  void visit(ESTree::ForStatementNode *node);
  void visit(ESTree::ReturnStatementNode *node);
  void visit(ESTree::BlockStatementNode *node);
  void visit(ESTree::VariableDeclarationNode *node);

  void visit(ESTree::CatchClauseNode *node);

 private:
  /// A RAII object automatic scopes.
  class ScopeRAII {
   public:
    /// The binding table scope.
    TypeBindingTableScopeTy bindingScope;

    /// Create a binding scope and push a semantic scope.
    /// \param scopeNode is the AST node with which to associate the scope.
    explicit ScopeRAII(FlowChecker &checker)
        : bindingScope(checker.bindingTable_) {}

    /// Pops the created scope if it was pushed.
    ~ScopeRAII() = default;
  };

  void visitFunctionLike(
      ESTree::FunctionLikeNode *node,
      ESTree::Node *body,
      ESTree::NodeList &params);

  /// Check that the implicit return at the end of the function is valid if it
  /// may be reached. Report an error if it isn't valid.
  void checkImplicitReturnType(ESTree::FunctionLikeNode *node);

  /// Forward declaration information for generic type instantiations in
  /// aliases.
  /// Needed by both DeclareScopeTypes and FindLoopingTypes.
  class GenericTypeInstantiation;

  /// Resolve and declare all types named in a scope.
  class DeclareScopeTypes;

  /// Check whether a type contains loops (even indirectly).
  /// Determines whether it is considered "unsortable" in union arms.
  /// Recurses through all structural types, stops on nominal types because they
  /// can be compared easily, and don't count as looping union arms.
  class FindLoopingTypes;

  /// Parse all sema declarations type annotations and associate them
  /// with the declarations.
  class AnnotateScopeDecls;

  /// Declare all named types in the specified scope and annotate the
  /// declarations with them.
  /// \return true on success. Do not typecheck the scope if this returns false.
  /// LLVM_NODISCARD to ensure that we always enforce the check.
  LLVM_NODISCARD bool resolveScopeTypesAndAnnotate(
      ESTree::Node *scopeNode,
      sema::LexicalScope *scope);

  /// Finish typechecking all the deferred nodes that have been queued up.
  void drainTypecheckQueue();

  /// Record the declaration's type and declaring AST node, while checking for
  /// and reporting re-declarations.
  /// \return true if there was no error.
  bool recordDecl(
      sema::Decl *decl,
      Type *type,
      ESTree::IdentifierNode *id,
      ESTree::Node *astDeclNode);

  /// Return the type associated with a sema::Decl. The association must
  /// exist.
  Type *getDeclType(sema::Decl *decl) const {
    return flowContext_.getDeclType(decl);
  }

  /// Associate a type with an expression node. Do nothing if the specified
  /// type is nullptr.
  void setNodeType(ESTree::Node *node, Type *type) {
    flowContext_.setNodeType(node, type);
  }

  /// Return the optional type associated with a node or "any" if no type.
  Type *getNodeTypeOrAny(ESTree::Node *node) const {
    return flowContext_.getNodeTypeOrAny(node);
  }

  /// \param optReturnTypeAnnotation is nullptr or ESTree::TypeAnnotationNode
  /// \param defaultReturnType optional return type if the return annotation
  ///     is missing. nullptr here is a shortcut for "any".
  /// \param defaultThisType optional this type if the annotation is missing.
  Type *parseFunctionType(
      ESTree::NodeList &params,
      ESTree::Node *optReturnTypeAnnotation,
      bool isAsync,
      bool isGenerator,
      Type *defaultReturnType = nullptr,
      Type *defaultThisType = nullptr);

  /// Parse an optional type annotation. If it is nullptr, return any, otherwise
  /// parse the inner annotation (which cannot be null).
  /// \param defaultType optional type to use if the annotation is missing.
  ///     nullptr here is a shortcut for "any".
  Type *parseOptionalTypeAnnotation(
      ESTree::Node *optAnnotation,
      Type *defaultType = nullptr);

  /// Parse a type annotation into a type.
  /// \param node the non-null type annotation AST node.
  Type *parseTypeAnnotation(ESTree::Node *node);

  Type *parseUnionTypeAnnotation(ESTree::UnionTypeAnnotationNode *node);
  Type *parseNullableTypeAnnotation(ESTree::NullableTypeAnnotationNode *node);
  Type *parseArrayTypeAnnotation(ESTree::ArrayTypeAnnotationNode *node);
  Type *parseTupleTypeAnnotation(ESTree::TupleTypeAnnotationNode *node);
  Type *parseGenericTypeAnnotation(ESTree::GenericTypeAnnotationNode *node);
  Type *parseFunctionTypeAnnotation(ESTree::FunctionTypeAnnotationNode *node);

  /// Parse a class type into an already created (but empty) class.
  class ParseClassType;

  /// Visit the \p node for either resolution or parsing and call \p cb on each
  /// of the type annotations in it.
  /// \return the constructed FunctionType.
  template <typename AnnotationCB>
  inline Type *processFunctionTypeAnnotation(
      ESTree::FunctionTypeAnnotationNode *node,
      AnnotationCB cb);

  /// Result indicating whether a type can flow into another type. If it can,
  /// additionally indicates whether a checked cast is needed.
  struct CanFlowResult {
    bool canFlow = false;
    /// When the type can flow, this field indicates whether a checked cast is
    /// needed.
    bool needCheckedCast = false;
  };

  /// Return true if type \p a can "flow" into type \p b.
  /// TODO: generate message explaining why not.
  static CanFlowResult canAFlowIntoB(Type *a, Type *b) {
    assert(a->info && b->info && "types haven't been populated yet");
    return canAFlowIntoB(a->info, b->info);
  }
  static CanFlowResult canAFlowIntoB(TypeInfo *a, TypeInfo *b);
  static CanFlowResult canAFlowIntoB(ClassType *a, ClassType *b);
  static CanFlowResult canAFlowIntoB(TupleType *a, TupleType *b);

  /// How to handle 'this' parameters when checking if function types can flow.
  enum class ThisFlowDirection {
    /// Supertype this parameters flow into subtype this parameters.
    Default,
    /// Subtype this parameters flow into supertype this parameters.
    MethodOverride,
  };

  /// \param thisFlow how to handle 'this' parameter.
  static CanFlowResult canAFlowIntoB(
      BaseFunctionType *a,
      BaseFunctionType *b,
      ThisFlowDirection thisFlow = ThisFlowDirection::Default);

  /// Different from regular function type flowing, because 'this' parameters
  /// must be handled specially in the method override scenario.
  /// In only this case, 'this' in \p a must be a subtype of \p b.
  /// In canAFlowIntoB, having a parameter in \p a that is a subtype of \p b
  /// would fail to typecheck.
  /// \return whether \p a can be a method override for \p b.
  static bool canAOverrideB(BaseFunctionType *a, BaseFunctionType *b) {
    return canAFlowIntoB(a, b, ThisFlowDirection::MethodOverride).canFlow;
  }

  /// Try to narrow a union with a single non-optional arm to the non-optional
  /// type so that it can be used in expressions.
  /// Non-optional means non-null and non-void.
  /// Used assuming that Flow has already determined that the type is
  /// supposed to have been narrowed by some condition/assignment at this point.
  /// \return narrowed type, or nullptr if the type is not a union with a single
  /// non-optional arm.
  static Type *getNonOptionalSingleType(Type *exprType);

  /// Try to narrow the type of the node into the target type, provided that the
  /// \p targetType is a non-optional arm of the optional \p exprType.
  /// Used for AssignmentExpression or CallExpression arguments, e.g.
  /// \param exprType the type of the expression.
  /// \param targetType the type we want to narrow to.
  /// \return (resType, cf) where resType is a type that \p exprType flows into,
  /// and cf is the CanFlowResult for exprType into resType. If
  /// exprType has been narrowed to resType, then cf.needCheckedCast is true,
  /// and the caller needs to insert the implicit checked cast.
  /// resType may be targetType if the checked cast should cast to targetType.
  static std::pair<Type *, CanFlowResult> tryNarrowType(
      Type *exprType,
      Type *targetType);

  /// If \c canFlow.needCheckedCast is set and \c compile_ is set, allocate an
  /// implicit checked cast node from the specified \p argument to
  /// the specified type \p toType and return it. Otherwise return the argument.
  ESTree::Node *implicitCheckedCast(
      ESTree::Node *argument,
      Type *toType,
      CanFlowResult canFlow);

  /// Return the non-null expression sema::Decl associated with the identifier.
  sema::Decl *getDecl(ESTree::IdentifierNode *id) {
    return semContext_.getExpressionDecl(id);
  }

  /// Add the \p typeArgTypes to the binding table based on the names provided
  /// in \p node.
  /// Ensure that there are the correct number of type arguments and that they
  /// are valid to pass.
  /// \param params the type parameter declaration.
  /// \param typeArgsNode the type arguments to pass.
  /// \param typeArgTypes the actual Types to instantiate the arguments with.
  /// \param scope the lexical scope to associate with each TypeDecl.
  /// \pre the binding table's scope is set to the new scope in which to place
  ///   the bindings (i.e. a direct child of the binding table scope the generic
  ///   was declared with).
  /// \return true on success, false on failure and report an error.
  LLVM_NODISCARD bool validateAndBindTypeParameters(
      ESTree::TypeParameterDeclarationNode *params,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      llvh::ArrayRef<Type *> typeArgTypes,
      sema::LexicalScope *scope);

  /// If necessary, specialize and typecheck the specialization of a generic
  /// function.
  /// Parses the type arguments in \p typeArgsNode.
  /// \param node the call expression passing the type arguments
  /// \param callee the name of the generic being called
  /// \param oldDecl the original Decl for the non-specialized generic function
  /// \return the new Decl for the specialization of the function,
  ///   nullptr on error.
  sema::Decl *specializeGeneric(
      sema::Decl *oldDecl,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      sema::LexicalScope *scope);

  /// If necessary, specialize and typecheck the specialization of a generic
  /// function.
  /// \param node the call expression passing the type arguments
  /// \param callee the name of the generic being called
  /// \param oldDecl the original Decl for the non-specialized generic function
  /// \return the new Decl for the specialization of the function,
  ///   nullptr on error.
  sema::Decl *specializeGenericWithParsedTypes(
      sema::Decl *oldDecl,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      llvh::ArrayRef<Type *> typeArgTypes,
      sema::LexicalScope *scope);

  /// If necessary, specialize and typecheck the specialization of a generic
  /// function.
  /// Set the type of the callee to the specialized version of the function.
  /// \param node the call expression passing the type arguments
  /// \param callee the name of the generic being called
  /// \param oldDecl the original Decl for the non-specialized generic function
  void resolveCallToGenericFunctionSpecialization(
      ESTree::CallExpressionNode *node,
      ESTree::IdentifierNode *callee,
      sema::Decl *oldDecl);

  /// Run the typechecker on a newly created specialization of a generic
  /// function.
  /// \param typeArgTypes the type arguments passed to the generic function.
  /// \param oldDecl the original Decl for the non-specialized generic function.
  /// \param newDecl the new Decl for the specialization of the function.
  void typecheckGenericFunctionSpecialization(
      ESTree::FunctionDeclarationNode *specialization,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      llvh::ArrayRef<Type *> typeArgTypes,
      sema::Decl *oldDecl,
      sema::Decl *newDecl);

  /// Create the ClassType for the specialization.
  /// Record the declaration in \p newDecl.
  /// If deferredGenericSpecializations_ is set,
  /// add the specialization to the list of deferred specializations.
  /// Otherwise, parse immediately and enqueue a typecheck.
  /// \param typeArgTypes the type arguments passed to the generic function.
  /// \param oldDecl the original Decl for the non-specialized generic function.
  /// \param newDecl the new Decl for the specialization of the function.
  void typecheckGenericClassSpecialization(
      ESTree::ClassDeclarationNode *specialization,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      llvh::ArrayRef<Type *> typeArgTypes,
      sema::Decl *oldDecl,
      sema::Decl *newDecl);

  /// Given a generic type for a class, resolve it into a class constructor.
  /// Annotate the \p nameNode with the resolved ClassConstructorType.
  /// \return the ClassType corresponding to the constructor.
  Type *resolveGenericClassSpecialization(
      ESTree::IdentifierNode *nameNode,
      ESTree::TypeParameterInstantiationNode *typeArgsNode,
      sema::Decl *oldDecl);

  /// Given a generic type for a class, resolve it into a class constructor.
  /// \return the ClassType corresponding to the constructor.
  Type *resolveGenericClassSpecializationForType(
      ESTree::GenericTypeAnnotationNode *genericTypeNode,
      sema::Decl *oldDecl);

  /// Register the \p decl generic with its original AST \p node.
  /// Increment refcount of \p bindingTableScope so it can be kept alive and
  /// activated on specialization.
  /// \param decl the generic declaration.
  /// \param node the original AST node.
  /// \param parent the parent of \p node.
  void registerGeneric(
      sema::Decl *decl,
      ESTree::Node *node,
      ESTree::Node *parent,
      const TypeBindingTableScopePtrTy &bindingTableScope);

  /// \pre \p decl is a generic declaration that has been registered.
  /// \return the generic info associated with \p decl.
  GenericInfo &getGenericInfoMustExist(sema::Decl *decl);
};

class FlowChecker::FunctionContext {
  FlowChecker &outer_;
  /// The previous context to be restored on destruction. nullptr if this is the
  /// global function.
  FunctionContext *const prevContext_;

 public:
  /// The DeclCollector associated with this function.
  const sema::DeclCollector *const declCollector;

  /// The external signature of the current function. If nullptr, this is the
  /// global function.
  Type *const functionType;

  /// The type of the "this" parameter. If nullptr, this is a global function
  /// with an implicit "this" paramater. Depending on the compilation mode,
  /// usages may be invalid.
  ///
  /// Note that the type of "this" is not necessarily the same as
  /// FunctionType::thisParamType, because of arrow functions.
  Type *const thisParamType;

  FunctionContext(const FunctionContext &) = delete;
  void operator=(const FunctionContext &) = delete;

  /// \param declCollectorNode the AST node with an associated DeclCollector.
  FunctionContext(
      FlowChecker &outer,
      ESTree::FunctionLikeNode *declCollectorNode,
      Type *functionType,
      Type *thisParamType)
      : outer_(outer),
        prevContext_(outer.curFunctionContext_),
        declCollector(
            outer.declCollectorMap_.find(declCollectorNode)->second.get()),
        functionType(functionType),
        thisParamType(thisParamType) {
    assert(
        outer.declCollectorMap_.count(declCollectorNode) &&
        "no declCollector for this node");
    outer.curFunctionContext_ = this;
  }

  ~FunctionContext() {
    outer_.curFunctionContext_ = prevContext_;
  }
};

// TODO: this is probably not needed. We can directly visit the class members,
//       passing in the required info.
class FlowChecker::ClassContext {
  FlowChecker &outer_;

  /// The previous context, restored on destruction.
  ClassContext *const prevContext_;

 public:
  Type *const classType;

  ClassContext(const ClassContext &) = delete;
  void operator=(const ClassContext &) = delete;

  ClassContext(FlowChecker &outer, Type *const classType)
      : outer_(outer),
        prevContext_(outer.curClassContext_),
        classType(classType) {
    outer.curClassContext_ = this;
  }

  ~ClassContext() {
    outer_.curClassContext_ = prevContext_;
  }

  ClassType *getClassTypeInfo() {
    return llvh::cast<ClassType>(classType->info);
  }
};

} // namespace flow
} // namespace hermes

#endif
