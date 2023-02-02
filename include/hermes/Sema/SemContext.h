/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_SEMCONTEXT_H
#define HERMES_SEMA_SEMCONTEXT_H

#include "hermes/AST/ESTree.h"

#include <deque>

namespace hermes {
namespace sema {

class Decl;
class LexicalScope;
class FunctionInfo;
class SemContext;

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
    /// Function declaration visible only in its lexical scope.
    ScopedFunction,

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
    return kind >= Kind::ScopedFunction;
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
};

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
  /// True if the function is strict mode.
  bool strict;
  /// Source visibility of this function.
  SourceVisibility sourceVisibility = SourceVisibility::Default;
  /// True if this function is an arrow function.
  bool const arrow;
  /// False if the parameter list contains any patterns.
  bool simpleParameterList = true;

  /// Whether this function references "arguments" identifier.
  bool usesArguments = false;

  /// Whether this function contains arrow functions.
  bool containsArrowFunctions = false;

  /// This is a logical or of the \c usesArguments flags of all contained
  /// arrow functions. This will be used as a conservative estimate of
  /// whether a non-arrow function needs to eagerly create and capture its
  /// Arguments object.
  bool containsArrowFunctionsUsingArguments = false;

  /// How many labels have been allocated in this function so far.
  uint32_t numLabels{0};

  /// Allocate a new label and return its index.
  uint32_t allocateLabel() {
    return numLabels++;
  }

  FunctionInfo(
      ESTree::FunctionLikeNode *funcNode,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope,
      bool strict,
      SourceVisibility sourceVisibility)
      : parentFunction(parentFunction),
        parentScope(parentScope),
        strict(strict),
        sourceVisibility(sourceVisibility),
        arrow(llvh::isa<ESTree::ArrowFunctionExpressionNode>(funcNode)) {}

  /// \return the top-level lexical scope of the function.
  LexicalScope *getFunctionScope() const {
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
class SemContext {
  friend class SemContextDumper;

 public:
  explicit SemContext();
  ~SemContext();

  /// \param parentFunction may be null.
  /// \param parentScope may be null.
  /// \return a new function.
  FunctionInfo *newFunction(
      ESTree::FunctionLikeNode *funcNode,
      FunctionInfo *parentFunction,
      LexicalScope *parentScope,
      bool strict,
      SourceVisibility sourceVisibility);
  /// \param parentFunction the function in which to put the scope, nullable.
  /// \param parentScope the parent lexical scope, nullable.
  /// \return a new lexical scope.
  LexicalScope *newScope(
      FunctionInfo *parentFunction,
      LexicalScope *parentScope);
  /// \param scope not nullable.
  /// \return a new declaration in \p scope.
  Decl *newDeclInScope(
      UniqueString *name,
      Decl::Kind kind,
      LexicalScope *scope,
      Decl::Special special = Decl::Special::NotSpecial);

  /// \return a new global property.
  Decl *newGlobal(UniqueString *name, Decl::Kind kind);

  /// Assert that the global function and the global scope have been created.
  void assertGlobalFunctionAndScope() {
    assert(!functions_.empty() && "global function has not been created");
    assert(!scopes_.empty() && "global scope has not been created");
  }

  /// \return the global function.
  FunctionInfo *getGlobalFunction() {
    return &functions_.at(0);
  }
  /// \return the global lexical scope.
  LexicalScope *getGlobalScope() {
    return &scopes_.at(0);
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

 private:
  /// Storage for all functions.
  std::deque<FunctionInfo> functions_{};

  /// Storage for all lexical scopes.
  std::deque<LexicalScope> scopes_{};

  /// Storage for all variable declarations.
  std::deque<Decl> decls_{};
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

  void printDeclRef(llvh::raw_ostream &os, const Decl *d);

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
