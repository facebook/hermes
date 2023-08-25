/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SemanticResolver.h"

#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/ScopeExit.h"
#include "llvh/Support/SaveAndRestore.h"

using namespace hermes::ESTree;

namespace hermes {
namespace sema {

namespace {

/// Encodes the result of checking for termination in CheckImplicitReturn.
/// targetLabels encodes where execution may continue to.
class TerminationResult {
 public:
  /// Indicates execution that continues to the next statement when stored in
  /// targetLabels.
  /// Offset the invalid key by 2 to avoid interfering with DenseMapInfo.
  static constexpr unsigned kNextStatementLabel = ~0u - 2;

  /// Set of target label indices.
  /// Having multiple labels in this set indicates that each of them may be
  /// reached in some execution of the function.
  ///
  /// There is a special label:
  /// kNextStatementLabel indicates that the next statement may execute.
  ///
  /// Other labels indicate that the corresponding LabelDecorationBase may
  /// complete and execute the statement following it.
  /// e.g. If a SwitchStatement's associated label index is in this set,
  /// then the switch may complete and the statement after the switch may run.
  ///
  /// Only continuation points within the same function are tracked.
  /// if there's no elements in this set, then the queried statement definitely
  /// terminates (ends execution of the function).
  llvh::SmallDenseSet<unsigned, 2> targetLabels{};

  /// \return true when the target label set contains kNextStatementLabel,
  /// indicating that the execution can continue to the following statement.
  bool mayExecuteNextStatement() const {
    return targetLabels.count(kNextStatementLabel);
  }
  /// \return true when the target label set ONLY contains kNextStatementLabel,
  /// indicating that the execution can continue to the following statement.
  bool mustExecuteNextStatement() const {
    return targetLabels.size() == 1 &&
        *targetLabels.begin() == kNextStatementLabel;
  }
  /// \return true when the target label set is empty, indicating that we can't
  /// execute any more statements anywhere inside the current function.
  bool mustTerminate() const {
    return targetLabels.empty();
  }

  /// \return a TerminationResult with one label: \p labelIndex.
  static TerminationResult makeSingleLabel(unsigned labelIndex) {
    TerminationResult result{};
    result.targetLabels.insert(labelIndex);
    return result;
  }
  /// \return a TerminationResult with one label: continue to next statement.
  static TerminationResult makeNextStatement() {
    return makeSingleLabel(kNextStatementLabel);
  }
  /// \return a TerminationResult with no labels, indicating no control flow
  /// to any other statements in this function.
  static TerminationResult makeMustTerminate() {
    TerminationResult result{};
    return result;
  }
};

/// Runs a conservative check to determine whether there are any possible paths
/// through the function which end in an implicit 'undefined' return.
class CheckImplicitReturn {
 public:
  explicit CheckImplicitReturn() {}

  // \return the termination result of any \p node.
  TerminationResult checkTermination(ESTree::Node *node) {
    switch (node->getKind()) {
      case ESTree::NodeKind::BlockStatement: {
        auto *block = llvh::cast<ESTree::BlockStatementNode>(node);
        return checkTerminationStatementList(block->_body);
      }

      case ESTree::NodeKind::IfStatement: {
        auto *ifStatement = llvh::cast<ESTree::IfStatementNode>(node);
        auto consequentRes = checkTermination(ifStatement->_consequent);
        if (ifStatement->_alternate) {
          // All targets from both branches are possible.
          auto alternateRes = checkTermination(ifStatement->_alternate);
          consequentRes.targetLabels.insert(
              alternateRes.targetLabels.begin(),
              alternateRes.targetLabels.end());
        } else {
          // No alternate, so this statement can also continue.
          consequentRes.targetLabels.insert(
              TerminationResult::kNextStatementLabel);
        }
        // Added the additional labels to the consequentRes, return it.
        return consequentRes;
      }

      case ESTree::NodeKind::ForStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<ForStatementNode>(node),
            llvh::cast<ForStatementNode>(node)->_body,
            false);
      case ESTree::NodeKind::ForInStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<ForInStatementNode>(node),
            llvh::cast<ForInStatementNode>(node)->_body,
            false);
      case ESTree::NodeKind::ForOfStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<ForOfStatementNode>(node),
            llvh::cast<ForOfStatementNode>(node)->_body,
            false);
      case ESTree::NodeKind::WhileStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<WhileStatementNode>(node),
            llvh::cast<WhileStatementNode>(node)->_body,
            false);
      case ESTree::NodeKind::DoWhileStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<DoWhileStatementNode>(node),
            llvh::cast<DoWhileStatementNode>(node)->_body,
            true);
      case ESTree::NodeKind::LabeledStatement:
        return checkTerminationLoopOrLabeledStatement(
            llvh::cast<LabeledStatementNode>(node),
            llvh::cast<LabeledStatementNode>(node)->_body,
            true);

      case ESTree::NodeKind::SwitchStatement:
        return checkTerminationSwitchStatement(
            llvh::cast<ESTree::SwitchStatementNode>(node));
      case ESTree::NodeKind::TryStatement:
        return checkTerminationTryStatement(
            llvh::cast<ESTree::TryStatementNode>(node));

      case ESTree::NodeKind::ReturnStatement:
        // Explicit return will always prevent implicit return.
        return TerminationResult::makeMustTerminate();
      case ESTree::NodeKind::ThrowStatement:
        // Throw will prevent the next statement in the current list from
        // executing. It's possible it will result in execution of a catch or
        // finally in this function, but that is handled at the TryStatement
        // level.
        return TerminationResult::makeMustTerminate();

      case ESTree::NodeKind::ContinueStatement:
        /// For 'continue', conservatively assume that the condition of the loop
        /// could be false and we will run the statement after the loop.
        return TerminationResult::makeSingleLabel(
            llvh::cast<ESTree::ContinueStatementNode>(node)->getLabelIndex());
      case ESTree::NodeKind::BreakStatement:
        return TerminationResult::makeSingleLabel(
            llvh::cast<ESTree::BreakStatementNode>(node)->getLabelIndex());

      case ESTree::NodeKind::WithStatement:
        return checkTermination(
            llvh::cast<ESTree::WithStatementNode>(node)->_body);

      case ESTree::NodeKind::DebuggerStatement:
      case ESTree::NodeKind::EmptyStatement:
      case ESTree::NodeKind::ExpressionStatement:
        return TerminationResult::makeNextStatement();

      default:
        assert(
            !llvh::isa<ESTree::StatementNode>(node) &&
            "unhandled statement in statement list");
        // This is not a JS statement, so it's not going to do any control flow.
        // e.g. this could be a TypeAliasNode, Import, Export, or some other
        // non-executing statement.
        return TerminationResult::makeNextStatement();
    }
  }

 private:
  // \return the termination result of the provided list of statements.
  TerminationResult checkTerminationStatementList(ESTree::NodeList &stmts) {
    TerminationResult result{};
    for (ESTree::Node &stmt : stmts) {
      // Check for continuation from previous statement and erase the continue,
      // because this is the continuation.
      result.targetLabels.erase(TerminationResult::kNextStatementLabel);

      // Add all the possible target labels to the final result.
      auto stmtRes = checkTermination(&stmt);
      result.targetLabels.insert(
          stmtRes.targetLabels.begin(), stmtRes.targetLabels.end());
      if (!stmtRes.mayExecuteNextStatement()) {
        // Statement list doesn't continue, so we're done scanning it.
        return result;
      }
    }
    // Made it through the whole statement list.
    result.targetLabels.insert(TerminationResult::kNextStatementLabel);
    return result;
  }

  // \return the termination result of a loop or a labeled statement.
  // \param mustExecute whether the body must run at least once.
  TerminationResult checkTerminationLoopOrLabeledStatement(
      ESTree::LabelDecorationBase *node,
      ESTree::Node *body,
      bool mustExecute) {
    // Whether the function may continue execution on the next statement after
    // \p node.
    // * Loops with preconditions aren't guaranteed to execute so they
    // may continue.
    // * Do-while must run the loop body at least once.
    // * break/continue with the label associated with \p node may continue to
    // the next statement (checked below).
    bool mayExecuteNextStatement = !mustExecute;

    auto bodyRes = checkTermination(body);
    auto it = bodyRes.targetLabels.find(node->getLabelIndex());
    if (it != bodyRes.targetLabels.end()) {
      // Breaks within this labeled statement are continues after it.
      bodyRes.targetLabels.erase(it);
      mayExecuteNextStatement = true;
    }

    if (mayExecuteNextStatement) {
      bodyRes.targetLabels.insert(TerminationResult::kNextStatementLabel);
    }
    return bodyRes;
  }

  // \return the termination result of a try statement.
  TerminationResult checkTerminationTryStatement(
      ESTree::TryStatementNode *node) {
    auto tryRes = checkTermination(node->_block);

    assert(
        !(node->_handler && node->_finalizer) &&
        "try-catch-finally should have been transformed by SemanticResolver");
    if (node->_handler) {
      // Both the try and catch must be terminating if there's no finalizer.
      auto catchRes = checkTermination(
          llvh::cast<ESTree::CatchClauseNode>(node->_handler)->_body);
      tryRes.targetLabels.insert(
          catchRes.targetLabels.begin(), catchRes.targetLabels.end());
      return tryRes;
    } else {
      auto finallyRes = checkTermination(node->_finalizer);
      if (finallyRes.mustTerminate()) {
        // If the finally block terminates, the try-finally will terminate after
        // executing the finally.
        return finallyRes;
      }
      if (tryRes.mustTerminate() && finallyRes.mustExecuteNextStatement()) {
        // If the try definitely terminates and the finally can't break
        // to another handler in this function, the try-finally will definitely
        // terminate.
        // However, we also check that the finally has no control flow
        // that would prevent the try from being able to terminate, e.g.
        //     label:
        //       try { return 1; }
        //       finally { break label; }
        // needs to avoid this branch, which mustExecuteNextStatement checks.
        return tryRes;
      }
      // Otherwise, we just combine the possible next points of the try-finally.
      tryRes.targetLabels.insert(
          finallyRes.targetLabels.begin(), finallyRes.targetLabels.end());
      return tryRes;
    }
  }

  // \return the termination result of a switch statement, accounting for breaks
  // and fallthrough.
  TerminationResult checkTerminationSwitchStatement(
      ESTree::SwitchStatementNode *node) {
    TerminationResult result{};
    bool foundDefault = false;
    for (ESTree::Node &child : node->_cases) {
      // Check for fallthrough from previous case and erase the continue,
      // because this is the continuation.
      result.targetLabels.erase(TerminationResult::kNextStatementLabel);

      auto *switchCase = llvh::cast<ESTree::SwitchCaseNode>(&child);
      if (!switchCase->_test)
        foundDefault = true;

      auto caseRes = checkTerminationStatementList(switchCase->_consequent);
      result.targetLabels.insert(
          caseRes.targetLabels.begin(), caseRes.targetLabels.end());
    }

    // Check for explicit break from this switch statement.
    // If there's an explicit break, remove it from the result.
    bool foundExplicitBreak = result.targetLabels.erase(node->getLabelIndex());

    // If we found explicit breaks or if there's no default case, we can make it
    // past this switch statement.
    if (foundExplicitBreak || !foundDefault) {
      result.targetLabels.insert(TerminationResult::kNextStatementLabel);
    }

    return result;
  }
};

} // namespace

bool mayReachImplicitReturn(ESTree::FunctionLikeNode *root) {
  CheckImplicitReturn visitor{};
  BlockStatementNode *block = getBlockStatement(root);
  assert(block && "arrows have been normalized to contain block bodies");
  auto result = visitor.checkTermination(block);
  assert(
      (result.targetLabels.empty() || result.mustExecuteNextStatement()) &&
      "all user-declared labels must be removed by the end of the function");
  return result.mayExecuteNextStatement();
}

} // namespace sema
} // namespace hermes
