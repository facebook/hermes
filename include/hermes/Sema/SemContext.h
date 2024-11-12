/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_SEMCONTEXT_H
#define HERMES_SEMA_SEMCONTEXT_H

#include "hermes/ADT/PersistentScopedMap.h"
#include "hermes/AST/ESTree.h"
#include "hermes/Sema/Keywords.h"

#include <deque>

namespace hermes {
namespace sema {

class Decl;
class LexicalScope;
class FunctionInfo;
class SemContext;

/// Binding between an identifier and its declaration in a scope.
struct Binding {
  Decl *decl = nullptr;
  /// The declaring node. Note that this is nullable.
  ESTree::IdentifierNode *ident = nullptr;

  Binding() = default;
  Binding(Decl *decl, ESTree::IdentifierNode *ident)
      : decl(decl), ident(ident) {}

  bool isValid() const {
    return decl != nullptr;
  }
  void invalidate() {
    decl = nullptr;
    ident = nullptr;
  }
};

/// The scoped binding table mapping from string to binding.
using BindingTableTy = hermes::PersistentScopedMap<UniqueString *, Binding>;
using BindingTableScopeTy =
    hermes::PersistentScopedMapScope<UniqueString *, Binding>;
using BindingTableScopePtrTy =
    hermes::PersistentScopedMapScopePtr<UniqueString *, Binding>;

/// Variable declaration.
class Decl {
 public:
  /// The kind of variable declaration.
  /// Determines scoping, among other things.
  enum class Kind : uint8_t {
    // ==== Let-like declarations ===
    Let,
    Const,
    Class,
    Import,
    /// A catch variable bound with let-like rules (non-ES5).
    /// ES14.0 B.3.4 handles ES5-style catch bindings differently.
    Catch,
    /// Function declaration visible only in its lexical scope.
    ScopedFunction,
    /// A single catch variable declared like this "catch (e)", see
    /// ES10 B.3.5 VariableStatements in Catch Blocks
    ES5Catch,

    // ==== other declarations ===
    /// Name of a function expression, which is visible within the function
    /// but not outside it.
    FunctionExprName,
    /// Name of a class expression, which is visible within the class
    /// but not outside it.
    ClassExprName,

    // ==== Var-like declarations ===

    /// "var" in function scope.
    Var,
    Parameter,
    /// "var" in global scope.
    GlobalProperty,
    /// Ambient global property,
    /// Used implicitly without corresponding "var" in global scope.
    UndeclaredGlobalProperty,
  };

  /// Certain identifiers must be treated differently by later parts
  /// of the program.
  /// Indicate if this identifier is specially treated "arguments" or "eval".
  enum class Special : uint8_t {
    NotSpecial,
    Arguments,
    Eval,
  };

  /// \return true if this declaration kind obeys the TDZ.
  static bool isKindTDZ(Kind kind) {
    return kind <= Kind::Class;
  }

  /// \return true if this kind of declaration is function scope (and can be
  /// re-declared).
  static bool isKindVarLike(Kind kind) {
    return kind >= Kind::Var;
  }
  static bool isKindVarLikeOrScopedFunction(Kind kind) {
    return isKindVarLike(kind) || kind == Kind::ScopedFunction;
  }
  /// \return true if this kind of declaration is lexically scoped (and cannot
  /// be re-declared).
  static bool isKindLetLike(Kind kind) {
    return kind <= Kind::ES5Catch;
  }
  /// \return true if this kind of declaration is a global property.
  static bool isKindGlobal(Kind kind) {
    return kind >= Kind::GlobalProperty;
  }

  /// Identifier that is declared.
  Identifier const name;
  /// What kind of declaration it is.
  Kind kind;
  /// Whether this is a generic declaration.
  /// The type checker can use this flag to keep track of which declarations are
  /// generic.
  /// SemanticResolver itself doesn't set this flag because it has no
  /// understanding of types.
  bool generic = false;
  /// If this is a special declaration, identify which one.
  Special const special;

  /// The lexical scope of the declaration. Could be nullptr for special
  /// declarations, since they are technically unscoped.
  LexicalScope *const scope;

  /// Field used by the consumer of the context.
  void *customData = nullptr;

  Decl(
      Identifier name,
      Kind kind,
      Special special,
      LexicalScope *scope = nullptr)
      : name(name), kind(kind), special(special), scope(scope) {}
  Decl(Identifier name, Kind kind, LexicalScope *scope)
      : Decl(name, kind, Special::NotSpecial, scope) {}
  /// Cloning constructor.
  /// Only invoked during ESTreeClone from cloneDeclIntoScope.
  Decl(Decl *decl, LexicalScope *scope)
      : Decl(decl->name, decl->kind, decl->special, scope) {
    assert(
        decl->customData == nullptr &&
        "custom data shouldn't be populated during ESTreeClone");
  }
};

/// Lexical scopes within a function.
class LexicalScope {
 public:
  /// The global depth of this scope, where 0 is the root scope (globally).
  uint32_t depth;
  /// The function owning this lexical scope.
  /// May never be null.
  FunctionInfo *const parentFunction{};
  /// The enclosing lexical scope (it could be in another function).
  /// Null if this is the root scope.
  LexicalScope *const parentScope{};

  /// All declarations made in this scope.
  llvh::SmallVector<Decl *, 2> decls{};

  /// A list of functions that need to be hoisted and materialized before we
  /// can generate the rest of the scope.
  llvh::SmallVector<ESTree::FunctionDeclarationNode *, 2> hoistedFunctions{};

  /// True if this scope or any descendent scopes have a local eval call.
  /// If any descendent uses local eval,
  /// it's impossible to know whether local variables are modified.
  bool localEval = false;

  /// \param parentFunction must not be null.
  /// \param parentScope may be null.
  LexicalScope(FunctionInfo *parentFunction, LexicalScope *parentScope)
      : depth(parentScope ? parentScope->depth + 1 : 0),
        parentFunction(parentFunction),
        parentScope(parentScope) {
    assert(parentFunction && "lexical scope must be in a function");
  }

  /// Partial cloning constructor.
  /// Does not copy hoistedFunctions, because those contain AST nodes.
  /// Only called during ESTreeClone from prepareClonedScope.
  /// \param scope the scope to clone.
  /// \param parentFunction the parent function for the new scope.
  /// \param parentScope the parent scope for the new scope.
  LexicalScope(
      SemContext &semCtx,
      LexicalScope *scope,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope);
};

// An enum indicating whether a function expression is an arrow function.
enum class FuncIsArrow { Yes, No };

/// Semantic information about functions.
class FunctionInfo {
 public:
  /// The function surrounding this function.
  /// Null if this is the root function.
  FunctionInfo *const parentFunction;
  /// The enclosing lexical scope.
  /// Null if this is the root function.
  LexicalScope *const parentScope;
  /// All lexical scopes in this function.
  /// The first one is the function scope.
  llvh::SmallVector<LexicalScope *, 4> scopes{};
  /// A list of imports that need to be hoisted and materialized before we
  /// can generate the rest of the function.
  /// Any line of the file may use the imported values.
  llvh::SmallVector<ESTree::ImportDeclarationNode *, 2> imports{};
  /// The implicitly declared "arguments" object.
  /// It is declared only if it is used.
  /// Should be populated by calling \c SemContext::funcArgumentsDecl.
  hermes::OptValue<Decl *> argumentsDecl{llvh::None};
  /// Index of the function scope in the scopes vector.
  /// The index isn't constant in the case of parameter expressions which
  /// introduce new scopes (e.g. for function expression names), so this has to
  /// be stored separately.
  /// Stored as an index to make cloning FunctionInfo easier (we don't have to
  /// make old->new scope associations in ESTreeClone or write any extra logic,
  /// just copy the index).
  /// UINT32_MAX is used until this field is set.
  uint32_t functionBodyScopeIdx = UINT32_MAX;
  /// True if the function is strict mode.
  bool strict;
  /// Custom directives found in this function.
  CustomDirectives customDirectives{};
  /// True if this function is an arrow function.
  bool const arrow;
  /// False if the parameter list contains any patterns.
  bool simpleParameterList = true;
  /// True if the parameter list contains any expressions.
  /// If there are expressions, then the first scope in the scopes_ list will be
  /// the parameter scope, and the second scope will be the function scope.
  bool hasParameterExpressions = false;

  /// Whether this function references "arguments" identifier.
  bool usesArguments = false;

  /// Whether this function contains arrow functions.
  bool containsArrowFunctions = false;

  /// This is a logical or of the \c usesArguments flags of all contained
  /// arrow functions. This will be used as a conservative estimate of
  /// whether a non-arrow function needs to eagerly create and capture its
  /// Arguments object.
  bool containsArrowFunctionsUsingArguments = false;

  /// Whether the function might execute the implicit 'undefined' return at the
  /// end.
  /// This is determined conservatively, so there may be some functions that in
  /// reality can't reach the implicit return, but this bool is set to 'true'
  /// anyway.
  bool mayReachImplicitReturn = true;

  /// Lazy compilation: the parent binding table scope of this function.
  /// Eager/eval compilation: the binding table scope of this function.
  /// In both cases, we're storing the parent of the code we want to eventually
  /// compile - in the case of lazy compilation we're trying to compile this
  /// function itself, while in eval we're going to compile (potentially many)
  /// lexical children, so it's convenient to use the same field.
  BindingTableScopePtrTy bindingTableScope;

  /// How many labels have been allocated in this function so far.
  uint32_t numLabels{0};

  /// Allocate a new label and return its index.
  uint32_t allocateLabel() {
    return numLabels++;
  }

  /// The \p isArrowFunctionExpression arg indicates whether the function is
  /// an arrow function.
  FunctionInfo(
      FuncIsArrow isArrowFunctionExpression,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope,
      bool strict,
      CustomDirectives customDirectives)
      : parentFunction(parentFunction),
        parentScope(parentScope),
        strict(strict),
        customDirectives(customDirectives),
        arrow(isArrowFunctionExpression == FuncIsArrow::Yes) {}

  /// Partial cloning constructor.
  /// Called only from ESTreeClone via prepareClonedFunction.
  /// Cannot copy the scopes because we don't know all the new scopes yet,
  /// so ESTreeClone will populate them later.
  /// \param parentFunction the parent function of the new FunctionInfo.
  /// \param parentScope the parent scope of the new FunctionInfo.
  FunctionInfo(
      FunctionInfo *function,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope);

  /// \pre the functionScopeIdx field has been set.
  /// \return the top-level lexical scope of the function body.
  LexicalScope *getFunctionBodyScope() const {
    assert(functionBodyScopeIdx < scopes.size() && "functionScopeIdx not set");
    return scopes[functionBodyScopeIdx];
  }

  /// \return the lexical scope which contains the parameter declarations,
  /// which may be the same as the function scope itself.
  /// The scope is guaranteed to contain all Parameter Decls, though it may
  /// contain other Decls as well.
  LexicalScope *getParameterScope() const {
    assert(!scopes.empty() && "no parameter scope added yet");
    return scopes[0];
  }
};

/// Semantic information regarding the program.
/// Storage for FunctionInfo, LexicalScope, and Decl objects.
/// New objects are allocated in a deque and a stable pointer is returned
/// which will point to a valid piece of memory until the SemContext is
/// destroyed.
/// These allocated objects point to each other within the SemContext,
/// so they can be freely passed around so long as the SemContext is alive.
///
/// SemContexts can be placed into a tree structure, where all the SemContext
/// share a single binding table, but store their actual data in their own
/// deques. This allows us to free data used for 'eval' once the corresponding
/// runtime data is freed, because the scopes in the binding table are also
/// refcounted.
class SemContext {
  friend class SemContextDumper;

 public:
  /// Convenient storage of "keyword" identifiers used by various part of the
  /// infrastructure.
  Keywords kw;

  /// Construct a SemContext with an optional parent.
  /// If a parent is provided, the binding table will be shared with the parent.
  explicit SemContext(
      Context &astContext,
      const std::shared_ptr<SemContext> &parent = nullptr);

  ~SemContext();

  /// \p node may be null, in which case the answer is No.
  static FuncIsArrow nodeIsArrow(ESTree::Node *node);

  /// \param parentFunction may be null.
  /// \param parentScope may be null.
  /// \return a new function.
  FunctionInfo *newFunction(
      FuncIsArrow isArrow,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope,
      bool strict,
      CustomDirectives customDirectives);
  /// Clone the function, without the AST nodes inside it.
  /// Used from ESTreeClone prior to cloning the body AST of the function.
  FunctionInfo *prepareClonedFunction(
      FunctionInfo *function,
      FunctionInfo *newParentFunction,
      LexicalScope *newParentScope);
  /// \param parentFunction the function in which to put the scope, nullable.
  /// \param parentScope the parent lexical scope, nullable.
  /// \return a new lexical scope.
  LexicalScope *newScope(
      FunctionInfo *parentFunction,
      LexicalScope *parentScope);
  /// Clone the scope, but the new scope won't have any hoisted functions.
  /// Used from ESTreeClone prior to cloning the body AST of the scope.
  /// Hoisted functions are handled separately.
  LexicalScope *prepareClonedScope(
      LexicalScope *scope,
      FunctionInfo *newParentFunction,
      LexicalScope *newParentScope);
  /// \param scope not nullable.
  /// \return a new declaration in \p scope.
  Decl *newDeclInScope(
      UniqueString *name,
      Decl::Kind kind,
      LexicalScope *scope,
      Decl::Special special = Decl::Special::NotSpecial);
  /// \param scope the new scope to clone into, not nullable.
  /// \return a clone of \p oldDecl which lives in \p scope.
  Decl *cloneDeclIntoScope(Decl *oldDecl, LexicalScope *scope);

  /// \return a new global property.
  Decl *newGlobal(UniqueString *name, Decl::Kind kind);

  /// Assert that the global function and the global scope have been created.
  void assertGlobalFunctionAndScope() {
    assert(!functions_.empty() && "global function has not been created");
    assert(!scopes_.empty() && "global scope has not been created");
  }

  /// \return the global function.
  FunctionInfo *getGlobalFunction() {
    return &root_->functions_.front();
  }
  /// \return the global lexical scope.
  LexicalScope *getGlobalScope() {
    return &root_->scopes_.front();
  }

  /// Set the binding table global scope.
  void setBindingTableGlobalScope(
      const BindingTableScopePtrTy &bindingTableScope) {
    assert(
        root_ == this &&
        "cannot set binding table global scope in child SemContext, "
        "must be set from root");
    bindingTableGlobalScope_ = bindingTableScope;
  }

  /// \return the binding table global scope.
  const BindingTableScopePtrTy &getBindingTableGlobalScope() const {
    return root_->bindingTableGlobalScope_;
  }

  /// Create or retrieve the arguments declaration in \p func.
  /// If `func` is an arrow function, find the closest ancestor that
  /// is not an arrow function and use that function's `arguments`. If we end
  /// up looking for `arguments` in global scope, an ambient declaration is
  /// created and returned.
  /// \p name the object doesn't have access to the AST node, so the
  ///   "arguments" string has to be passed in.
  /// \return the special arguments declaration in the specified function.
  Decl *funcArgumentsDecl(FunctionInfo *func, UniqueString *argumentsName);

  /// \return the declaration in which this identifier participates.
  /// `nullptr` if no resolution has been recorded.
  Decl *getDeclarationDecl(ESTree::IdentifierNode *node);

  /// \pre the identifier hasn't been marked "unresolvable".
  /// \return the declaration to which the identifier has been resolved,
  /// `nullptr` if no resolution has been recorded.
  Decl *getExpressionDecl(ESTree::IdentifierNode *node) {
    using ID = ESTree::IdentifierDecoration;
    assert(
        !node->isUnresolvable() &&
        "Attempt to read decl for unresolvable identifier");
    return (node->declState_ & ID::BitHaveExpr) ? (Decl *)node->decl_ : nullptr;
  }

  /// Set the "expression decl" of the specified identifier node.
  /// \pre the identifier hasn't been marked "unresolvable".
  void setExpressionDecl(ESTree::IdentifierNode *node, Decl *decl);

  /// Set the "declaration decl" of the specified identifier node.
  void setDeclarationDecl(ESTree::IdentifierNode *node, Decl *decl);

  /// Set the "declaration decl" and the "expression decl" of the identifier
  /// node to the same value.
  void setBothDecl(ESTree::IdentifierNode *node, Decl *decl) {
    setExpressionDecl(node, decl);
    setDeclarationDecl(node, decl);
  }

  /// \return the constructor of a class, if it has one, else nullptr.
  ESTree::MethodDefinitionNode *getConstructor(ESTree::ClassLikeNode *node);

  BindingTableTy &getBindingTable() {
    return root_->bindingTable_;
  }
  const BindingTableTy &getBindingTable() const {
    return root_->bindingTable_;
  }

 private:
  /// The parent SemContext of this SemContext.
  /// If null, this SemContext has no parent.
  /// Use shared_ptr because these will form a tree, and the parent can be freed
  /// when it has no users and no children.
  std::shared_ptr<SemContext> parent_;

  /// Non-owning pointer to the root SemContext,
  /// used for efficiently finding the global scope/global function.
  /// If this SemContext is the root, it points to itself.
  /// If this SemContext has a parent, it points to another SemContext.
  SemContext *root_;

  /// The currently lexically visible names.
  /// Only lives in the root SemContext.
  /// All other SemContexts will have empty binding tables,
  /// and will read the binding table from the root.
  BindingTableTy bindingTable_{};

  /// Global binding table scope.
  /// This is only set in a root SemContext.
  /// Any child SemContexts must read it from the root.
  BindingTableScopePtrTy bindingTableGlobalScope_{};

  /// Storage for all functions.
  std::deque<FunctionInfo> functions_{};

  /// Storage for all lexical scopes.
  std::deque<LexicalScope> scopes_{};

  /// Storage for all variable declarations.
  std::deque<Decl> decls_{};

  /// This side table is used to associate a "declaration decl" with an
  /// ESTree::IdentifierNode, when the "declaration decl" and the
  /// "expression decl" are both set and are not the same value.
  llvh::DenseMap<ESTree::IdentifierNode *, Decl *>
      sideIdentifierDeclarationDecl_{};
};

class SemContextDumper {
 public:
  using AnnotateDeclFunc =
      std::function<void(llvh::raw_ostream &, const Decl *)>;

 public:
  explicit SemContextDumper() {}
  template <typename F>
  explicit SemContextDumper(F f) : annotateDecl_(f) {}

  void printSemContext(llvh::raw_ostream &os, const SemContext &semCtx);

  void printFunction(
      llvh::raw_ostream &os,
      const FunctionInfo &f,
      unsigned level = 0);

  void
  printScope(llvh::raw_ostream &os, const LexicalScope *s, unsigned level = 0);

  void printScopeRef(llvh::raw_ostream &os, const LexicalScope *s);

  void printDecl(llvh::raw_ostream &os, const Decl *d);

  void
  printDeclRef(llvh::raw_ostream &os, const Decl *d, bool printName = true);

 private:
  /// Optional callback printing a Decl annotation.
  AnnotateDeclFunc annotateDecl_{};

  class PtrNumberingImpl {
    /// The number to be assigned next.
    size_t nextNumber_ = 1;
    /// Map from pointer to a number.
    llvh::DenseMap<const void *, size_t> numbers_{};

   protected:
    /// Find the existing number or allocate a new one for the specified
    /// pointer. \return the consecutive number associated with this
    /// declaration.
    size_t getNumberImpl(const void *ptr);
  };

  template <typename T>
  class Numbering : public PtrNumberingImpl {
   public:
    size_t getNumber(const T *ptr) {
      return getNumberImpl(ptr);
    }
  };

  Numbering<Decl> declNumbers_{};
  Numbering<LexicalScope> scopeNumbers_{};
};

} // namespace sema
} // namespace hermes

#endif
