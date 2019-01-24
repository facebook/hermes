/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/AST/SemanticValidator.h"
#include "hermes/Support/PerfSection.h"
#include "hermes/Support/RegExpSerialization.h"

#include "RecursiveVisitor.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/SaveAndRestore.h"

using llvm::cast;
using llvm::dyn_cast;
using llvm::dyn_cast_or_null;
using llvm::isa;
using llvm::SaveAndRestore;

using namespace hermes::ESTree;

namespace hermes {
namespace sem {

namespace {

class SemanticValidator {
  Context &astContext_;
  /// A copy of Context::getSM() for easier access.
  SourceErrorManager &sm_;

  /// All semantic tables are persisted here.
  SemContext &semCtx_;

  /// Save the initial error count so we know whether we generated any errors.
  const unsigned initialErrorCount_;

  struct Label {
    enum Kind { BLOCK_LABEL, LOOP_LABEL };

    /// Is this a block or a loop label.
    Kind kind;
    /// Where it was declared.
    IdentifierNode *node;
  };

  class FunctionContext; // Forward declaration.
  FunctionContext *funcCtx_{};

  /// Identifier for "arguments".
  const UniqueString *const identArguments_;
  /// Identifier for "eval".
  const UniqueString *const identEval_;
  /// Identifier for "delete".
  const UniqueString *const identDelete_;
  /// Identifier for "use strict".
  const UniqueString *const identUseStrict_;

#ifndef NDEBUG
  /// Our parser detects strictness and initializes the flag in every node,
  /// but if we are reading an external AST, we must look for "use strict" and
  /// initialize the flag ourselves here.
  /// For consistency we always perform the detection, but in debug mode we also
  /// want to ensure that our results match what the parser generated. This
  /// flag indicates whether strictness is preset or not.
  bool strictnessIsPreset_{false};
#endif

  /// Holds all per-function state, specifically label tables. Should always be
  /// constructed on the stack.
  class FunctionContext {
    SemanticValidator *validator_;
    FunctionContext *oldContextValue_;

   public:
    /// The associated seminfo object
    sem::FunctionInfo *const semInfo;

    /// Are we inside a loop.
    bool inLoop = false;
    /// Are we inside a switch.
    bool inSwitch = false;
    /// Is this function in strict mode.
    bool strictMode = false;

    /// The currently active labels in the function.
    llvm::DenseMap<NodeLabel, Label> labelMap;

    explicit FunctionContext(SemanticValidator *validator)
        : validator_(validator),
          oldContextValue_(validator->funcCtx_),
          semInfo(validator->semCtx_.createFunction()) {
      if (validator->funcCtx_) {
        strictMode = validator->funcCtx_->strictMode;
      }
      validator->funcCtx_ = this;
    }
    ~FunctionContext() {
      validator_->funcCtx_ = oldContextValue_;
    }

    /// \return true if this is the "global scope" function context, in other
    /// words not a real function.
    bool isGlobalScope() const {
      return !oldContextValue_;
    }
  };

 public:
  explicit SemanticValidator(Context &astContext, sem::SemContext &semCtx)
      : astContext_(astContext),
        sm_(astContext.getSourceErrorManager()),
        semCtx_(semCtx),
        initialErrorCount_(sm_.getErrorCount()),
        identArguments_(
            astContext.getIdentifier("arguments").getUnderlyingPointer()),
        identEval_(astContext.getIdentifier("eval").getUnderlyingPointer()),
        identDelete_(astContext.getIdentifier("delete").getUnderlyingPointer()),
        identUseStrict_(
            astContext.getIdentifier("use strict").getUnderlyingPointer()) {}

  // Perform the validation on whole AST.
  bool doIt(Node *rootNode) {
    visitESTreeNode(*this, rootNode);
    return sm_.getErrorCount() == initialErrorCount_;
  }

  /// Perform the validation on an individual function.
  bool doFunction(Node *function, bool strict) {
    FunctionContext functionContext(this);
    functionContext.strictMode = strict;
    visitESTreeNode(*this, function);
    return sm_.getErrorCount() == initialErrorCount_;
  }

  /// If we're only checking a subtree, initialize with a context.
  void initializeWithContext(Node *function) {}

  /// Handle the default case for all nodes which we ignore, but we still want
  /// to visit their children.
  void visit(Node *node) {
    visitESTreeChildren(*this, node);
  }

  void visit(ProgramNode *node) {
#ifndef NDEBUG
    strictnessIsPreset_ = node->strictness != Strictness::NotSet;
#endif
    FunctionContext newFuncCtx{this};
    newFuncCtx.strictMode = astContext_.isStrictMode();
    scanDirectivePrologue(node->_body);
    setNodeStrictness(&node->strictness, newFuncCtx.strictMode);
    visitESTreeChildren(*this, node);

    node->setSemInfo(newFuncCtx.semInfo);
  }

  void visit(VariableDeclaratorNode *varDecl) {
    validateDeclarationName(varDecl->_id);
    curFunction()->semInfo->decls.push_back(varDecl);
    visitESTreeChildren(*this, varDecl);
  }

  void visit(IdentifierNode *identifier) {
    if (identifier->_name == identEval_ && !astContext_.getEnableEval())
      sm_.error(identifier->getSourceRange(), "'eval' is disabled");
  }

  /// Process a function declaration by creating a new FunctionContext.
  void visit(FunctionDeclarationNode *funcDecl) {
    if (funcCtx_)
      curFunction()->semInfo->closures.push_back(funcDecl);
    visitFunction(
        funcDecl,
        funcDecl->_id,
        funcDecl->_params,
        funcDecl->_body,
        &funcDecl->strictness);
  }

  void visit(ObjectMethodNode *oe) {
    // Object methods require a new function context.
    visitFunction(oe, nullptr, oe->_params, oe->_body, &oe->strictness);
  }

  /// Process a function expression by creating a new FunctionContext.
  void visit(FunctionExpressionNode *funcExpr) {
    visitFunction(
        funcExpr,
        funcExpr->_id,
        funcExpr->_params,
        funcExpr->_body,
        &funcExpr->strictness);
  }

  /// Ensure that the left side of for-in is an l-value.
  void visit(ForInStatementNode *forIn) {
    SaveAndRestore<bool> saveInLoop(curFunction()->inLoop, true);

    if (!isa<VariableDeclarationNode>(forIn->_left))
      if (!isLValue(forIn->_left))
        sm_.error(
            forIn->_left->getSourceRange(),
            "invalid left-hand side in for-in-loop");
    visitESTreeChildren(*this, forIn);
  }

  /// Ensure that the left side of assgnments is an l-value.
  void visit(AssignmentExpressionNode *assignment) {
    // Check if the left-hand side is valid.
    if (!isLValue(assignment->_left))
      sm_.error(
          assignment->_left->getSourceRange(),
          "invalid assignment left-hand side");
    visitESTreeChildren(*this, assignment);
  }

  /// Ensure that the operand of ++/-- is an l-value.
  void visit(UpdateExpressionNode *update) {
    // Check if the left-hand side is valid.
    if (!isLValue(update->_argument))
      sm_.error(
          update->_argument->getSourceRange(),
          "invalid operand in update operation");
    visitESTreeChildren(*this, update);
  }

  /// Declare named labels, checking for duplicates, etc.
  void visit(LabeledStatementNode *labelStmt) {
    auto id = dyn_cast<IdentifierNode>(labelStmt->_label);
    assert(id && "Malformed AST: label is not an Identifier");

    // Determine if this is a loop label by checking if it directly encloses
    // a loop or another label enclosing a loop.
    Label::Kind labelKind = Label::BLOCK_LABEL;
    {
      LabeledStatementNode *curStmt = labelStmt;
      do {
        if (isa<LoopStatementNode>(curStmt->_body)) {
          labelKind = Label::LOOP_LABEL;
          break;
        }
      } while ((curStmt = dyn_cast<LabeledStatementNode>(curStmt->_body)));
    }

    // Define the new label, checking for a previous definition.
    auto insertRes =
        curFunction()->labelMap.insert({id->_name, {labelKind, id}});
    if (!insertRes.second) {
      sm_.error(
          id->getSourceRange(),
          llvm::Twine("label '") + id->_name->str() + "' is already defined");
      sm_.note(
          insertRes.first->second.node->getSourceRange(),
          "previous definition");
    }
    // Auto-erase the label on exit, if we inserted it.
    const auto &deleter = llvm::make_scope_exit([=]() {
      if (insertRes.second)
        curFunction()->labelMap.erase(id->_name);
    });
    (void)deleter;

    visitESTreeChildren(*this, labelStmt);
  }

  /// Check RegExp syntax.
  void visit(RegExpLiteralNode *regexp) {
    llvm::StringRef regexpError;
    if (!CompiledRegExp::tryCompile(
            regexp->_pattern->str(), regexp->_flags->str(), &regexpError)) {
      sm_.error(
          regexp->getSourceRange(),
          "Invalid regular expression: " + Twine(regexpError));
    }
    visitESTreeChildren(*this, regexp);
  }

  void visit(DoWhileStatementNode *loop) {
    SaveAndRestore<bool> saveInLoop(curFunction()->inLoop, true);
    visitESTreeChildren(*this, loop);
  }
  void visit(ForStatementNode *loop) {
    SaveAndRestore<bool> saveInLoop(curFunction()->inLoop, true);
    visitESTreeChildren(*this, loop);
  }
  void visit(WhileStatementNode *loop) {
    SaveAndRestore<bool> saveInLoop(curFunction()->inLoop, true);
    visitESTreeChildren(*this, loop);
  }
  void visit(SwitchStatementNode *switchStmt) {
    SaveAndRestore<bool> saveInSwitch(curFunction()->inSwitch, true);
    visitESTreeChildren(*this, switchStmt);
  }

  void visit(BreakStatementNode *breakStmt) {
    if (auto id = dyn_cast_or_null<IdentifierNode>(breakStmt->_label)) {
      // A labeled break.
      if (!curFunction()->labelMap.count(id->_name)) {
        sm_.error(
            id->getSourceRange(),
            Twine("label '") + id->_name->str() + "' is not defined");
      }
    } else {
      // Anonymous break.
      if (!curFunction()->inLoop && !curFunction()->inSwitch) {
        sm_.error(
            breakStmt->getSourceRange(),
            "'break' not within a loop or a switch");
      }
    }

    visitESTreeChildren(*this, breakStmt);
  }

  void visit(ContinueStatementNode *continueStmt) {
    if (auto id = dyn_cast_or_null<IdentifierNode>(continueStmt->_label)) {
      // A labeled continue.
      auto labelIt = curFunction()->labelMap.find(id->_name);
      if (labelIt == curFunction()->labelMap.end()) {
        sm_.error(
            id->getSourceRange(),
            Twine("label '") + id->_name->str() + "' is not defined");
      } else if (labelIt->second.kind != Label::LOOP_LABEL) {
        sm_.error(
            id->getSourceRange(),
            llvm::Twine("continue label '") + id->_name->str() +
                "' is not a loop label");
        sm_.note(labelIt->second.node->getSourceRange(), "label defined here");
      }
    } else {
      // Anonymous continue.
      if (!curFunction()->inLoop) {
        sm_.error(
            continueStmt->getSourceRange(), "'continue' not within a loop");
      }
    }
    visitESTreeChildren(*this, continueStmt);
  }

  void visit(ReturnStatementNode *returnStmt) {
    if (curFunction()->isGlobalScope())
      sm_.error(returnStmt->getSourceRange(), "'return' not in a function");
    visitESTreeChildren(*this, returnStmt);
  }

  void visit(UnaryExpressionNode *unaryExpr) {
    // Check for unqualified delete in strict mode.
    if (unaryExpr->_operator == identDelete_) {
      if (curFunction()->strictMode &&
          isa<IdentifierNode>(unaryExpr->_argument)) {
        sm_.error(
            unaryExpr->getSourceRange(),
            "'delete' of a variable is not allowed in strict mode");
      }
    }
    visitESTreeChildren(*this, unaryExpr);
  }

 private:
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
  /// \param blockStatement the body
  /// \param[out] strictness set *strictness to the strictness.
  void visitFunction(
      FunctionLikeNode *node,
      const Node *id,
      NodeList &params,
      Node *blockStatement,
      Strictness *strictness) {
    FunctionContext newFuncCtx{this};
    node->setSemInfo(newFuncCtx.semInfo);

    if (id)
      validateDeclarationName(id);

    bool isBlock = isa<ESTree::BlockStatementNode>(blockStatement);
    assert(
        (isBlock || isa<ESTree::EmptyNode>(blockStatement)) &&
        "Function body is neither block nor empty lazy stub");

    if (isBlock) {
      scanDirectivePrologue(
          cast<ESTree::BlockStatementNode>(blockStatement)->_body);
    }
    setNodeStrictness(strictness, newFuncCtx.strictMode);

    for (auto &param : params)
      validateDeclarationName(&param);

    // let's check if we have seen this parameter name before
    if (newFuncCtx.strictMode) {
      llvm::SmallSet<NodeLabel, 8> paramNameSet;
      for (auto &param : params) {
        auto &curIdNode = cast<IdentifierNode>(param);
        auto insert_result = paramNameSet.insert(curIdNode._name);
        if (insert_result.second == false) {
          sm_.error(
              curIdNode.getSourceRange(),
              "cannot declare two parameters with the same name '" +
                  curIdNode._name->str() + "'");
        }
      }
    }

    visitESTreeChildren(*this, node);
  }

  /// Scan a list of directives in the beginning of a program of function
  /// (see ES5.1 4.1 - a directive is a statement consisting of a single
  /// string literal).
  /// Update the flags in the function context to reflect the directives. (We
  /// currently only recognize "use strict".)
  void scanDirectivePrologue(NodeList &body) {
    for (auto &nodeRef : body) {
      auto directive = ESTree::matchDirective(&nodeRef);
      if (!directive)
        break;

      if (*directive == identUseStrict_)
        curFunction()->strictMode = true;
    }
  }

  /// Determine if the argument is something that can be assigned to: a
  /// variable or a property. 'arguments' cannot be assigned to in strict mode,
  /// but we don't support code generation for assigning to it in any mode.
  bool isLValue(const Node *node) const {
    if (isa<MemberExpressionNode>(node))
      return true;
    if (!isa<IdentifierNode>(node))
      return false;

    auto *idNode = cast<IdentifierNode>(node);

    /// 'arguments' cannot be modified in strict mode, but we also don't
    /// support modifying it in non-strict mode yet.
    if (idNode->_name == identArguments_)
      return false;

    // 'eval' cannot be used as a variable in strict mode. If it is disabled we
    // we don't report an error because it will be reported separately.
    if (idNode->_name == identEval_ && curFunction()->strictMode &&
        astContext_.getEnableEval())
      return false;

    return true;
  }

  /// In strict mode 'arguments' and 'eval' cannot be used in declarations.
  bool isValidDeclarationName(const Node *node) const {
    // The identifier is sometimes optional, in which case it is valid.
    if (!node)
      return true;

    auto *idNode = cast<IdentifierNode>(node);

    // 'arguments' cannot be redeclared in strict mode.
    if (idNode->_name == identArguments_ && curFunction()->strictMode)
      return false;

    // 'eval' cannot be redeclared in strict mode. If it is disabled we
    // we don't report an error because it will be reported separately.
    if (idNode->_name == identEval_ && curFunction()->strictMode &&
        astContext_.getEnableEval())
      return false;

    return true;
  }

  /// If the supplied Identifier node is not a valid name to be used in a
  /// declaration, report an error.
  void validateDeclarationName(const Node *node) {
    if (!isValidDeclarationName(node)) {
      sm_.error(
          node->getSourceRange(),
          "cannot declare '" + cast<IdentifierNode>(node)->_name->str() + "'");
    }
  }

  void setNodeStrictness(ESTree::Strictness *nodeStrictness, bool strictMode) {
    auto strictness = ESTree::makeStrictness(strictMode);
    assert(
        (!strictnessIsPreset_ || *nodeStrictness == strictness) &&
        "Preset strictness is different from detected strictness");
    *nodeStrictness = strictness;
  }
};

} // anonymous namespace

bool validateAST(Context &astContext, SemContext &semCtx, Node *root) {
  PerfSection validation("Validating JavaScript function AST");
  // Validate the entire AST.
  SemanticValidator validator{astContext, semCtx};
  return validator.doIt(root);
}

bool validateFunctionAST(
    Context &astContext,
    SemContext &semCtx,
    Node *function,
    bool strict) {
  PerfSection validation("Validating JavaScript function AST: Deep");
  SemanticValidator validator{astContext, semCtx};
  return validator.doFunction(function, strict);
}

} // namespace sem
} // namespace hermes
