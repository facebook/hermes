/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SEMA_SEMANTICRESOLVER_H
#define HERMES_SEMA_SEMANTICRESOLVER_H

#include "DeclCollector.h"
#include "hermes/ADT/ScopedHashTable.h"
#include "hermes/AST/Keywords.h"
#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {

using DeclarationFileListTy = std::vector<ESTree::ProgramNode *>;

namespace sema {

class FunctionContext;
class SemContext;

/// Class the performs all resolution.
/// Reports errors if validation fails.
class SemanticResolver {
  Context &astContext_;
  /// A copy of Context::getSM() for easier access.
  SourceErrorManager &sm_;

  /// Buffer all generated messages and print them sorted in the end.
  SourceErrorManager::SaveAndBufferMessages bufferMessages_;

  /// Global function context, nullptr until populated.
  FunctionContext *globalFunctionContext_{nullptr};

  /// All semantic tables are persisted here.
  SemContext &semCtx_;

  /// Keywords we will be checking for.
  hermes::sem::Keywords kw_;

  /// A list of parsed files containing global ambient declarations that should
  /// be inserted in the global scope.
  const DeclarationFileListTy &ambientDecls_;

  /// If not null, store all instances of DeclCollector here, for use by other
  /// passes.
  DeclCollectorMapTy *const saveDecls_;

  /// Current function context.
  FunctionContext *curFunctionContext_ = nullptr;

  /// Current lexical scope.
  LexicalScope *curScope_{nullptr};

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
  using BindingTableTy = hermes::ScopedHashTable<UniqueString *, Binding>;
  using BindingTableScopeTy =
      hermes::ScopedHashTableScope<UniqueString *, Binding>;

  /// The currently lexically visible names.
  BindingTableTy bindingTable_{};

  /// The global scope.
  BindingTableScopeTy *globalScope_ = nullptr;

  /// True if we are preparing the AST to be compiled by Hermes, including
  /// erroring on features which we parse but don't compile and transforming
  /// the AST. False if we just want to validate the AST.
  bool compile_;

  /// The maximum AST nesting level. Once we reach it, we report an error and
  /// stop.
  static constexpr unsigned MAX_RECURSION_DEPTH =
#if defined(HERMES_LIMIT_STACK_DEPTH) || defined(_MSC_VER)
      512
#else
      1024
#endif
      ;
  /// MAX_RECURSION_DEPTH minus the current AST nesting level. Once it reaches
  /// 0, we report an error and stop modifying it.
  unsigned recursionDepth_ = MAX_RECURSION_DEPTH;

 public:
  /// \param semCtx the result of resolution will be stored here.
  /// \param saveDecls if not null, the map will contain all DeclCollector
  ///     instances for reuse by later passes.
  /// \param compile whether this resolution is intended to compile or just
  ///   parsing.
  explicit SemanticResolver(
      Context &astContext,
      sema::SemContext &semCtx,
      const DeclarationFileListTy &ambientDecls,
      DeclCollectorMapTy *saveDecls,
      bool compile);

  explicit SemanticResolver(
      Context &astContext,
      sema::SemContext &semCtx,
      const DeclarationFileListTy &ambientDecls,
      bool compile)
      : SemanticResolver(astContext, semCtx, ambientDecls, nullptr, compile) {}

  /// Run semantic resolution and store the result in \c semCtx_.
  /// \param rootNode the top-level program/JS module node to run resolution on.
  /// \return false on error.
  bool run(ESTree::ProgramNode *rootNode);

  /// Validate and resolve a CommonJS function expression. It will use the
  /// existing global function and global scope, which must have been created
  /// by a previous invocation of \c run().
  bool runCommonJSModule(ESTree::FunctionExpressionNode *rootNode);

  /// \return a reference to the keywords struct.
  const sem::Keywords &keywords() const {
    return kw_;
  }

  /// \return the current function context.
  FunctionContext *functionContext() {
    return curFunctionContext_;
  }
  /// \return the current function context.
  const FunctionContext *functionContext() const {
    return curFunctionContext_;
  }

  /// Extract the declared identifiers from a declaration AST node's "id" field.
  /// Normally that is just a single identifier, but it can be more in case of
  /// destructuring.
  void extractDeclaredIdentsFromID(
      ESTree::Node *node,
      llvh::SmallVectorImpl<ESTree::IdentifierNode *> &idents);

  /// Default case for all ignored nodes, we still want to visit their children.
  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::ProgramNode *node);

  void visit(ESTree::FunctionDeclarationNode *funcDecl);
  void visit(ESTree::FunctionExpressionNode *funcExpr);
  void visit(ESTree::ArrowFunctionExpressionNode *arrowFunc);

  void visit(ESTree::IdentifierNode *identifier, ESTree::Node *parent);

  void visit(ESTree::AssignmentExpressionNode *assignment);
  void visit(ESTree::UpdateExpressionNode *node);
  void visit(ESTree::UnaryExpressionNode *node);

  void visit(ESTree::BlockStatementNode *node);

  void visit(ESTree::SwitchStatementNode *node);

  void visit(ESTree::ForInStatementNode *node) {
    visitForInOf(node, node, node->_left);
  }
  void visit(ESTree::ForOfStatementNode *node) {
    visitForInOf(node, node, node->_left);
  }
  void visitForInOf(
      ESTree::LoopStatementNode *node,
      ESTree::ScopeDecorationBase *scopeDeco,
      ESTree::Node *left);

  void visit(ESTree::ForStatementNode *node);

  void visit(ESTree::DoWhileStatementNode *node);
  void visit(ESTree::WhileStatementNode *node);

  void visit(ESTree::LabeledStatementNode *node);

  void visit(ESTree::BreakStatementNode *node);
  void visit(ESTree::ContinueStatementNode *node);

  void visit(ESTree::WithStatementNode *node);

  void visit(ESTree::TryStatementNode *tryStatement);
  void visit(ESTree::CatchClauseNode *node);

  void visit(ESTree::RegExpLiteralNode *regexp);

  void visit(ESTree::MetaPropertyNode *node);

  void visit(ESTree::ImportDeclarationNode *importDecl);

  void visit(ESTree::ClassDeclarationNode *node);
  void visit(ESTree::ClassExpressionNode *node);
  void visit(ESTree::PrivateNameNode *node);
  void visit(ESTree::ClassPrivatePropertyNode *node);
  void visit(ESTree::ClassPropertyNode *node);
  void visit(ESTree::MethodDefinitionNode *node);

  void visit(ESTree::CallExpressionNode *node);

  void visit(ESTree::SpreadElementNode *node, ESTree::Node *parent);

  void visit(ESTree::ReturnStatementNode *node);
  void visit(ESTree::YieldExpressionNode *node);
  void visit(ESTree::AwaitExpressionNode *awaitExpr);

  void visit(ESTree::ExportNamedDeclarationNode *node);
  void visit(ESTree::ExportDefaultDeclarationNode *node);
  void visit(ESTree::ExportAllDeclarationNode *node);

  void visit(ESTree::CoverEmptyArgsNode *node);
  void visit(ESTree::CoverTrailingCommaNode *node);
  void visit(ESTree::CoverInitializerNode *node);
  void visit(ESTree::CoverRestElementNode *node);
#if HERMES_PARSE_FLOW
  void visit(ESTree::CoverTypedIdentifierNode *node);
  void visit(ESTree::TypeAliasNode *node);
#endif

  /// This method implements the first part of the protocol defined by
  /// RecursiveVisitor. It is supposed to return true if everything is normal,
  /// and false if we should not visit the current node.
  /// It maintains the current AST nesting level, and generates an error the
  /// first time it exceeds the maximum nesting level. Once that happens, it
  /// always returns false.
  bool incRecursionDepth(ESTree::Node *n) {
    if (LLVM_UNLIKELY(recursionDepth_ == 0))
      return false;
    --recursionDepth_;
    if (LLVM_UNLIKELY(recursionDepth_ == 0)) {
      recursionDepthExceeded(n);
      return false;
    }
    return true;
  }

  /// This is the second part of the protocol defined by RecursiveVisitor.
  /// Once we have reached the maximum nesting level, it does nothing. Otherwise
  /// it decrements the nesting level.
  void decRecursionDepth() {
    if (LLVM_LIKELY(recursionDepth_ != 0))
      ++recursionDepth_;
  }

  friend class FunctionContext;

 private:
  /// A RAII object automatic scopes.
  /// On construction it creates a new binding table scope and a new
  /// semantic scope, setting curScope_.
  /// On destruction it destroys the binding scope and resets curScope_.
  /// \param scopeDecoration if not null, will associate the new scope with
  ///   the node provided.
  class ScopeRAII {
   public:
    /// Create a binding scope and push a semantic scope.
    /// \param scopeNode is the AST node with which to associate the scope.
    explicit ScopeRAII(
        SemanticResolver &resolver,
        ESTree::ScopeDecorationBase *scopeDecoration = nullptr);

    /// Pops the created scope if it was pushed.
    ~ScopeRAII();

    BindingTableScopeTy &getBindingScope() {
      return bindingScope_;
    }

   private:
    /// The semantic context. If non-null, pop a scope on destruction.
    SemanticResolver &resolver_;
    /// Old LexicalScope to restore on pop.
    LexicalScope *oldScope_;
    /// The binding table scope.
    BindingTableScopeTy bindingScope_;
  };

  inline FunctionInfo *curFunctionInfo();
  inline const FunctionInfo *curFunctionInfo() const;

  void visitFunctionLike(
      ESTree::FunctionLikeNode *node,
      ESTree::Node *body,
      ESTree::NodeList &params);
  void visitFunctionExpression(
      ESTree::FunctionExpressionNode *node,
      ESTree::Node *body,
      ESTree::NodeList &params);

  /// Resolve an identifier to a declaration and record the resolution.
  /// Emit a warning for undeclared identifiers in strict mode.
  /// Record an undeclared global property if no declaration is found.
  void resolveIdentifier(ESTree::IdentifierNode *identifier, bool inTypeof);

  /// Look up \p identifier to see if it already has been resolved or has a
  /// binding assigned.
  /// Assigns the associated declaration if it exists.
  /// \return the declaration, `nullptr` if unresolvable or failed to resolve.
  Decl *checkIdentifierResolved(ESTree::IdentifierNode *identifier);

  /// Declare all declarations optionally associated with \p scopeNode
  /// by the DeclCollector in the current scope.
  void processCollectedDeclarations(ESTree::Node *scopeNode);

  /// Declare all declarations in \p decls by calling
  /// `validateAndDeclareIdentifier`.
  void processDeclarations(const ScopeDecls &decls);

  /// Extract the list of declared identifiers in a declaration node and return
  /// the declaration kind of the node.
  /// Function declarations are returned as DeclKind::ScopedFunction,
  /// so they can be distinguished.
  Decl::Kind extractIdentsFromDecl(
      ESTree::Node *node,
      llvh::SmallVectorImpl<ESTree::IdentifierNode *> &idents);

  /// Try to create a declaration of the specified kind and name in the current
  /// scope. If the declaration is invalid, print an error message without
  /// creating it.
  /// \param declKind the semantic declaration kind
  /// \param idNode the AST node containing the name
  /// \param declNode the AST node of the declaration. Used for function
  ///     declarations.
  void validateAndDeclareIdentifier(
      Decl::Kind kind,
      ESTree::IdentifierNode *ident);

  /// Ensure that the specified identifier is valid to be used in a declaration.
  /// Return true if valid, otherwise generate an error and return false.
  bool validateDeclarationName(
      Decl::Kind declKind,
      const ESTree::IdentifierNode *idNode) const;

  /// Ensure that the specified node is a valid target for an assignment, in
  /// other words it is an l-value, a Pattern (checked recursively) or an Empty
  /// (used by elision).
  /// Report errors if any are found.
  void validateAssignmentTarget(const ESTree::Node *node);

  /// \return true if the `node` is an LValue: a member expression or an
  /// identifier which is a valid LValue.
  bool isLValue(const ESTree::Node *node);

  /// Information about directives encountered in the beginning of a program
  /// or function.
  struct FoundDirectives {
    /// If a "use strict" directive is found, this field points to the AST node
    /// (used for error reporting),otherwise it is nullptr.
    ESTree::Node *useStrictNode = nullptr;
    /// If a source visibility directive is found, it is stored here, otherwise
    /// the value is SourceVisibility::Default.
    SourceVisibility sourceVisibility = SourceVisibility::Default;
  };

  /// Scan the list of directives in the beginning of a program or function.
  /// (see ES5.1 4.1 - a directive is a statement consisting of a single
  /// string literal).
  /// \param body list of statements to scan.
  /// \return information about the encountered directives.
  FoundDirectives scanDirectives(ESTree::NodeList &body) const;

  /// Mark \p scope and every one of its ancestor scopes as users of local
  /// `eval()`.
  static void registerLocalEval(LexicalScope *scope);

  // \param decl must be non-null.
  // \return whether the specified declaration is in the current function.
  bool declInCurFunction(Decl *decl) {
    assert(decl && "declInCurFunction requires non-null arg");
    return decl->scope->parentFunction == curFunctionInfo();
  }

  /// We call this when we exceed the maximum recursion depth.
  void recursionDepthExceeded(ESTree::Node *n);

  /// Declare the list of ambient decls that was passed to the constructor.
  void processAmbientDecls();
};

/// RAII class which holds per-function state during resolution,
/// including a pointer to the SemContext's \c FunctionInfo.
/// Includes label tables and collected declarations.
/// Must always be constructed on the stack.
/// At construction: pushes itself onto the \c functionStack_.
/// At destruction: pops itself from the \c functionStack_.
class FunctionContext {
  SemanticResolver &resolver_;

  FunctionContext *const prevContext_;

 public:
  struct Label {
    /// Where it was declared.
    ESTree::IdentifierNode *declarationNode;

    /// Statement targeted by the label. It is either a LoopStatement or a
    /// LabeledStatement.
    ESTree::StatementNode *targetStatement;
  };

  /// The associated seminfo object
  sema::FunctionInfo *semInfo;

  /// The AST node of the function.
  ESTree::FunctionLikeNode *const node;

  /// The currently active labels in the function.
  llvh::DenseMap<ESTree::NodeLabel, Label> labelMap;

  /// Most nested active loop statement.
  ESTree::LoopStatementNode *currentLoop{nullptr};

  /// The most nested active loop or switch statement.
  ESTree::StatementNode *currentLoopOrSwitch{nullptr};

  /// True if we are validating a formal parameter list.
  bool isFormalParams{false};

  /// All declarations in the function.
  std::unique_ptr<DeclCollector> decls;

  /// Just a tag for readability when invoking the special constructor.
  struct ExistingGlobalScopeTag {};

  /// Create a function context for the existing global scope.
  explicit FunctionContext(
      SemanticResolver &resolver,
      FunctionInfo *globalSemInfo,
      ExistingGlobalScopeTag);

  explicit FunctionContext(
      SemanticResolver &resolver,
      ESTree::FunctionLikeNode *node,
      FunctionInfo *parentSemInfo,
      bool strict,
      SourceVisibility sourceVisibility);

  ~FunctionContext();

  /// \return true if this is the "global scope" function context, in other
  /// words not a real function.
  bool isGlobalScope() const {
    return this == resolver_.globalFunctionContext_;
  }

  /// \return the optional function name, or nullptr.
  UniqueString *getFunctionName() const;
};

inline FunctionInfo *SemanticResolver::curFunctionInfo() {
  return functionContext()->semInfo;
}
const FunctionInfo *SemanticResolver::curFunctionInfo() const {
  return functionContext()->semInfo;
}

/// Visitor pass for marking variables as Unresolvable based on local `eval()`
/// or `with`.
class Unresolver {
 public:
  /// Mark all declarations that are at a lower depth than \p depth as
  /// unresolvable, starting at \p root.
  static void run(uint32_t depth, ESTree::Node *root);

  void visit(ESTree::Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ESTree::IdentifierNode *node);

  /// Needed by RecursiveVisitorDispatch. Optionally can protect against too
  /// deep nesting.
  bool incRecursionDepth(ESTree::Node *) {
    return true;
  }
  void decRecursionDepth() {}

 private:
  explicit Unresolver(uint32_t depth) : depth_(depth) {}

 private:
  /// Depth of the scope which contains the construct which could shadow
  /// variables dynamically.
  /// e.g. the depth of the function containing a local `eval()`.
  uint32_t depth_;
};

} // namespace sema
} // namespace hermes

#endif
