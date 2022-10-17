/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_AST_SEMANTICVALIDATOR_H
#define HERMES_AST_SEMANTICVALIDATOR_H

#include "hermes/AST/SemValidate.h"

#include "hermes/AST/RecursiveVisitor.h"

namespace hermes {
namespace sem {

using namespace hermes::ESTree;

// Forward declarations
class FunctionContext;
class SemanticValidator;

//===----------------------------------------------------------------------===//
// Keywords

class Keywords {
 public:
  /// Identifier for "arguments".
  const UniqueString *const identArguments;
  /// Identifier for "eval".
  const UniqueString *const identEval;
  /// Identifier for "delete".
  const UniqueString *const identDelete;
  /// Identifier for "this".
  const UniqueString *const identThis;
  /// Identifier for "use strict".
  const UniqueString *const identUseStrict;
  /// Identifier for "show source ".
  const UniqueString *const identShowSource;
  /// Identifier for "hide source ".
  const UniqueString *const identHideSource;
  /// Identifier for "sensitive".
  const UniqueString *const identSensitive;
  /// Identifier for "var".
  const UniqueString *const identVar;
  /// Identifier for "let".
  const UniqueString *const identLet;
  /// Identifier for "const".
  const UniqueString *const identConst;
  /// "+".
  const UniqueString *const identPlus;
  /// "-".
  const UniqueString *const identMinus;
  /// "=".
  const UniqueString *const identAssign;

  Keywords(Context &astContext);
};

//===----------------------------------------------------------------------===//
// SemanticValidator

/// Class the performs all semantic validation
class SemanticValidator {
  friend class FunctionContext;

  Context &astContext_;
  /// A copy of Context::getSM() for easier access.
  SourceErrorManager &sm_;

  /// Buffer all generated messages and print them sorted in the end.
  SourceErrorManager::SaveAndBufferMessages bufferMessages_;

  /// All semantic tables are persisted here.
  SemContext &semCtx_;

  /// Save the initial error count so we know whether we generated any errors.
  const unsigned initialErrorCount_;

  /// Keywords we will be checking for.
  Keywords kw_;

  /// The current function context.
  FunctionContext *funcCtx_{};

  /// True if we are validating a formal parameter list.
  bool isFormalParams_{false};

  /// True if we are preparing the AST to be compiled by Hermes, including
  /// erroring on features which we parse but don't compile and transforming
  /// the AST. False if we just want to validate the AST.
  bool compile_;

  /// The maximum AST nesting level. Once we reach it, we report an error and
  /// stop.
  static constexpr unsigned MAX_RECURSION_DEPTH =
#if defined(HERMES_LIMIT_STACK_DEPTH) && !defined(_MSC_VER)
      512
#elif defined(_MSC_VER) && defined(HERMES_SLOW_DEBUG)
      256
#elif defined(_MSC_VER)
      512
#else
      1024
#endif
      ;
  /// MAX_RECURSION_DEPTH minus the current AST nesting level. Once it reaches
  /// 0, we report an error and stop modifying it.
  unsigned recursionDepth_ = MAX_RECURSION_DEPTH;

 public:
  explicit SemanticValidator(
      Context &astContext,
      sem::SemContext &semCtx,
      bool compile);

  // Perform the validation on whole AST.
  bool doIt(Node *rootNode);

  /// Perform the validation on an individual function.
  bool doFunction(Node *function, bool strict);

  /// This method implements the first part of the protocol defined by
  /// RecursiveVisitor. It is supposed to return true if everything is normal,
  /// and false if we should not visit the current node.
  /// It maintains the current AST nesting level, and generates an error the
  /// first time it exceeds the maximum nesting level. Once that happens, it
  /// always returns false.
  bool incRecursionDepth(Node *n) {
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
    assert(
        recursionDepth_ < MAX_RECURSION_DEPTH &&
        "recursionDepth_ cannot go negative");
    if (LLVM_LIKELY(recursionDepth_ != 0))
      ++recursionDepth_;
  }

  /// Handle the default case for all nodes which we ignore, but we still want
  /// to visit their children.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ProgramNode *node);
  void visit(FunctionDeclarationNode *funcDecl);
  void visit(FunctionExpressionNode *funcExpr);
  void visit(ArrowFunctionExpressionNode *arrowFunc);

  void visit(VariableDeclaratorNode *varDecl, Node *parent);

  void visit(MetaPropertyNode *metaProp);
  void visit(IdentifierNode *identifier);

  void visit(ForInStatementNode *forIn);
  void visit(ForOfStatementNode *forOf);
  void visitForInOf(LoopStatementNode *loopNode, Node *left);

  void visit(BinaryExpressionNode *bin);
  void visit(AssignmentExpressionNode *assignment);
  void visit(UpdateExpressionNode *update);

  void visit(LabeledStatementNode *labelStmt);

  void visit(RegExpLiteralNode *regexp);

  void visit(TryStatementNode *tryStatement);

  void visit(DoWhileStatementNode *loop);
  void visit(ForStatementNode *loop);
  void visit(WhileStatementNode *loop);
  void visit(SwitchStatementNode *switchStmt);

  void visit(BreakStatementNode *breakStmt);
  void visit(ContinueStatementNode *continueStmt);

  void visit(ReturnStatementNode *returnStmt);
  void visit(YieldExpressionNode *yieldExpr);
  void visit(AwaitExpressionNode *awaitExpr);

  void visit(UnaryExpressionNode *unaryExpr);

  void visit(ArrayPatternNode *arrayPat);

  void visit(SpreadElementNode *S, Node *parent);

  void visit(ClassExpressionNode *node);
  void visit(ClassDeclarationNode *node);
  void visit(PrivateNameNode *node);
  void visit(ClassPrivatePropertyNode *node);

  void visit(ImportDeclarationNode *importDecl);
  void visit(ImportDefaultSpecifierNode *importDecl);
  void visit(ImportNamespaceSpecifierNode *importDecl);
  void visit(ImportSpecifierNode *importDecl);

  void visit(ExportNamedDeclarationNode *exportDecl);
  void visit(ExportDefaultDeclarationNode *exportDecl);
  void visit(ExportAllDeclarationNode *exportDecl);

  void visit(CoverEmptyArgsNode *CEA);
  void visit(CoverTrailingCommaNode *CTC);
  void visit(CoverInitializerNode *CI);
  void visit(CoverRestElementNode *R);
#if HERMES_PARSE_FLOW
  void visit(CoverTypedIdentifierNode *R);
#endif

 private:
  inline bool haveActiveContext() const {
    return funcCtx_ != nullptr;
  }

  inline FunctionContext *curFunction() {
    assert(funcCtx_ && "No active function context");
    return funcCtx_;
  }
  inline const FunctionContext *curFunction() const {
    assert(funcCtx_ && "No active function context");
    return funcCtx_;
  }

  /// Process a function declaration by creating a new FunctionContext. Update
  /// the context with the strictness of the function.
  /// \param node the current node
  /// \param id if not null, the associated name (for validation)
  /// \param params the parameter list
  /// \param body the body. It may be a BlockStatementNode, an EmptyNode (for
  ///     lazy functions), or an expression (for simple arrow functions).
  void
  visitFunction(FunctionLikeNode *node, Node *id, NodeList &params, Node *body);

  /// Scan a list of directives in the beginning of a program or function
  /// (see ES5.1 4.1 - a directive is a statement consisting of a single
  /// string literal).
  /// Update the flags in the function context to reflect the directives.
  /// \return the node containing "use strict" or nullptr.
  Node *scanDirectivePrologue(NodeList &body);

  /// Determine if the argument is something that can be assigned to: a
  /// variable or a property. 'arguments' cannot be assigned to in strict mode,
  /// but we don't support code generation for assigning to it in any mode.
  bool isLValue(const Node *node) const;

  /// In strict mode 'arguments' and 'eval' cannot be used in declarations.
  bool isValidDeclarationName(const IdentifierNode *idNode) const;

  /// Ensure that the declared identifier(s) is valid to be used in a
  /// declaration and append them to the specified list.
  /// \param node is one of nullptr, EmptyNode, IdentifierNode, PatternNode.
  /// \param idents if not-null, all identifiers are appended there.
  void validateDeclarationNames(
      FunctionInfo::VarDecl::Kind declKind,
      Node *node,
      llvh::SmallVectorImpl<FunctionInfo::VarDecl> *idents);

  /// Ensure that the specified node is a valid target for an assignment, in
  /// other words it is an l-value, a Pattern (checked recursively) or an Empty
  /// (used by elision).
  void validateAssignmentTarget(const Node *node);

  /// Set directives derived information (e.g. strictness, source visibility)
  /// to a function-like node.
  /// Data is retrieved from \c curFunction().
  void setDirectiveDerivedInfo(FunctionLikeNode *node);

  /// Called when the any of the source visibility directives are seen.
  /// Only a stronger source visibility from inner function scope can override
  /// the current source visibility set by outer function scope.
  void tryOverrideSourceVisibility(SourceVisibility newSourceVisibility);

  /// Get the LabelDecorationBase depending on the node type.
  static LabelDecorationBase *getLabelDecorationBase(StatementNode *node);

  /// Visit the parameters and body of \p node, setting isFormalParams_
  /// correctly.
  void visitParamsAndBody(FunctionLikeNode *node);

  /// We call this when we exceed the maximum recursion depth.
  void recursionDepthExceeded(Node *n);
};

//===----------------------------------------------------------------------===//
// FunctionContext

/// Holds all per-function state, specifically label tables. Should always be
/// constructed on the stack.
class FunctionContext {
  SemanticValidator *validator_;
  FunctionContext *oldContextValue_;

 public:
  struct Label {
    /// Where it was declared.
    IdentifierNode *declarationNode;

    /// Statement targeted by the label. It is either a LoopStatement or a
    /// LabeledStatement.
    StatementNode *targetStatement;
  };

  /// The AST node for the function.
  FunctionLikeNode *node;

  /// The associated seminfo object
  sem::FunctionInfo *const semInfo;

  /// The most nested active loop statement.
  LoopStatementNode *activeLoop = nullptr;
  /// The most nested active loop or switch statement.
  StatementNode *activeSwitchOrLoop = nullptr;
  /// Is this function in strict mode.
  bool strictMode = false;
  /// Source visibility of this function.
  SourceVisibility sourceVisibility{SourceVisibility::Default};

  /// The currently active labels in the function.
  llvh::DenseMap<NodeLabel, Label> labelMap;

  explicit FunctionContext(
      SemanticValidator *validator,
      bool strictMode,
      FunctionLikeNode *node,
      SourceVisibility sourceVisibility = SourceVisibility::Default);

  ~FunctionContext();

  /// \return true if this is the "global scope" function context, in other
  /// words not a real function.
  bool isGlobalScope() const {
    return !oldContextValue_;
  }

  /// Allocate a new label in the current context.
  unsigned allocateLabel() {
    return semInfo->allocateLabel();
  }
};

} // namespace sem
} // namespace hermes

#endif // HERMES_AST_SEMANTICVALIDATOR_H
