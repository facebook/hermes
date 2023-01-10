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

  Decl(Identifier name, Kind kind, LexicalScope *scope)
      : name(name), kind(kind), special(Special::NotSpecial), scope(scope) {}
  Decl(Identifier name, Kind kind, Special special)
      : name(name), kind(kind), special(special), scope(nullptr) {}
  Decl(Identifier name, Kind kind, Special special, LexicalScope *scope)
      : name(name), kind(kind), special(special), scope(scope) {}

  void dump(unsigned level = 0) const;
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

  void dump(const SemContext *sem = nullptr, unsigned level = 0) const;
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
  bool arrow;
  /// False if the parameter list contains any patterns.
  bool simpleParameterList = true;

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

  void dump(const SemContext *sem = nullptr, unsigned level = 0) const;
};

/// Semantic information regarding the program.
/// Storage for FunctionInfo, LexicalScope, and Decl objects.
/// New objects are allocated in a deque and a stable pointer is returned
/// which will point to a valid piece of memory until the SemContext is
/// destroyed.
/// These allocated objects point to each other within the SemContext,
/// so they can be freely passed around so long as the SemContext is alive.
class SemContext {
 public:
  SemContext(Context &ctx);
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

  /// \return the global function.
  FunctionInfo *getGlobalFunction() {
    return &functions_.at(0);
  }
  /// \return the global lexical scope.
  LexicalScope *getGlobalScope() {
    return &scopes_.at(0);
  }
  /// \return the "eval" declaration, used for local "eval" calls.
  Decl *getEvalDecl() {
    assert(evalDecl_ && "evalDecl_ must be initialized in ctor");
    return evalDecl_;
  }

  /// Create or retrieve the arguments declaration in \p func.
  /// If `func` is an arrow function, find the closest ancestor that
  /// is not an arrow function and use that function's `arguments`.
  /// \p name the object doesn't have access to the AST node, so the
  ///   "arguments" string has to be passed in.
  /// \return the special arguments declaration in the specified function.
  Decl *funcArgumentsDecl(FunctionInfo *func, UniqueString *argumentsName);

  void dump() const;

 private:
  /// The special global "eval" declaration.
  /// Stored to use when we are performing "eval" anywhere in the function,
  /// so the "eval" IdentifierNode can be associated with this declaration,
  /// which is "undeclared global".
  Decl *evalDecl_;

  /// Storage for all functions.
  std::deque<FunctionInfo> functions_{};

  /// Storage for all lexical scopes.
  std::deque<LexicalScope> scopes_{};

  /// Storage for all variable declarations.
  std::deque<Decl> decls_{};
};

} // namespace sema
} // namespace hermes

#endif
