/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/AST2JS/AST2JS.h"

#include "hermes/AST/RecursiveVisitor.h"
#include "hermes/Support/Conversions.h"
#include "hermes/Support/UTF8.h"

#include "llvh/ADT/StringSwitch.h"
#include "llvh/Support/Format.h"

namespace hermes {
using namespace ESTree;

namespace {

enum Precedence {
  kAlwaysParenPrec = 0,
  kSeqPrec,
  kArrowPrec,
  kYieldPrec,
  kAssignPrec,
  kCondPrec,
  kBinPrecStart,
  kBinPrecEnd = kBinPrecStart + 20,
  kUnaryPrec,
  kPostUpdatePrec,
  kTaggedTemplatePrec,
  kNewNoArgsPrec,
  kMemberPrec,
  kPrimaryPrec,
  kTopPrec,
};

Precedence getBinaryPrecedence(llvh::StringRef op) {
  return llvh::StringSwitch<Precedence>(op)
#define BINOP(name, str, prec) .Case(str, (Precedence)(prec + kBinPrecStart))
#include "hermes/Parser/TokenKinds.def"
      .Case("in", (Precedence)(8 + kBinPrecStart))
      .Case("instanceof", (Precedence)(8 + kBinPrecStart))
      .Default(kAlwaysParenPrec);
}

/// Precedence and associativity of an operator.
struct PrecRTL {
  Precedence prec; // Higher value means higher precedence.
  bool rtl; // right to left.

  /* implicit */ PrecRTL(Precedence prec, bool rtl = false)
      : prec(prec), rtl(rtl) {}
};

/// Child position for the purpose of determining whether the child needs
/// parenthesis.
enum class ChildPos {
  left,
  anywhere,
  right,
};

/// Whether parenthesis are needed around something.
enum class NeedParens {
  // Parens are not needed.
  no,
  // Parens are needed.
  yes,
  // Space is sufficient.
  space
};

NeedParens np(bool x) {
  return x ? NeedParens::yes : NeedParens::no;
}

class GenJS {
  /// Output the generated JS here.
  llvh::raw_ostream &OS_;
  /// Pretty print or not.
  bool const pretty_;
  /// Indentation step. In the future it may be configurable.
  static constexpr int indentStep_ = 2;

  /// Current indentation level. Only used in "pretty" mode.
  int indent_ = 0;

 public:
  GenJS(llvh::raw_ostream &OS, bool const pretty) : OS_(OS), pretty_(pretty) {}

  void doIt(Node *root) {
    visitESTreeNode(*this, root);
    OS_ << '\n';
  }

  /// Needed by RecursiveVisitorDispatch. Optionally can protect against too
  /// deep nesting.
  bool incRecursionDepth(Node *) {
    return true;
  }
  void decRecursionDepth() {}

  // TODO: remove this catch all function to find out which visitors need to be
  // TODO:   defined at compile time.
  void visit(Node *n) {
    llvh::errs() << "Unsupported AST node kind: " << n->getNodeName() << " ";
    abort();
  }

  void visit(IdentifierNode *node) {
    OS_ << node->_name->str();
  }
  void visit(NullLiteralNode *) {
    OS_ << "null";
  }
  void visit(BooleanLiteralNode *node) {
    OS_ << (node->_value ? "true" : "false");
  }
  void visit(StringLiteralNode *node) {
    OS_ << '"';
    printEscapedStringContents(node->_value->str(), '"');
    OS_ << '"';
  }
  void visit(RegExpLiteralNode *node) {
    // FIXME: escaping, etc?
    OS_ << '/';
    OS_ << node->_pattern->str();
    OS_ << '/';
    OS_ << node->_flags->str();
  }
  void visit(TemplateLiteralNode *node) {
    constexpr llvh::StringLiteral esc2("${");

    OS_ << '`';
    auto ecur = node->_expressions.begin();
    auto eend = node->_expressions.end();
    for (auto &_elem : node->_quasis) {
      auto *elem = llvh::cast<TemplateElementNode>(&_elem);
      OS_ << elem->_raw->str();
      if (ecur != eend) {
        OS_ << esc2;
        visitESTreeNode(*this, &*ecur++, node);
        OS_ << '}';
      }
    }
    OS_ << '`';
  }
  void visit(NumericLiteralNode *node) {
    char buf8[hermes::NUMBER_TO_STRING_BUF_SIZE];
    size_t len = hermes::numberToString(node->_value, buf8, sizeof(buf8));
    OS_.write(buf8, len);
  }

  void visit(ThisExpressionNode *) {
    OS_ << "this";
  }
  void visit(SuperNode *) {
    OS_ << "super";
  }

  void visit(TaggedTemplateExpressionNode *node) {
    printChild(node->_tag, node, ChildPos::left);
    printChild(node->_quasi, node, ChildPos::right);
  }

  void visit(SequenceExpressionNode *node) {
    int i = 0;
    for (auto &expr : node->_expressions) {
      if (i++)
        comma();
      printChild(&expr, node, i == 1 ? ChildPos::left : ChildPos::right);
    }
  }

  void visit(ObjectExpressionNode *node) {
    visitProps(node->_properties, node);
  }
  void visit(ObjectPatternNode *node) {
    visitProps(node->_properties, node);
  }
  void visitProps(NodeList &props, Node *parent) {
    OS_ << '{';
    int i = 0;
    for (auto &prop : props) {
      if (i++)
        comma();
      visitESTreeNode(*this, &prop, parent);
    }
    OS_ << '}';
  }

  void visit(PropertyNode *node) {
    bool needSep = false;

    if (node->_kind->str() != "init") {
      // get/set
      OS_ << node->_kind->str();
      needSep = true;
    } else if (node->_method) {
      auto *fe = llvh::cast<FunctionExpressionNode>(node->_value);
      if (fe->_async) {
        OS_ << "async";
        needSep = true;
      }
      if (fe->_generator) {
        OS_ << '*';
        needSep = false;
        space();
      }
    }

    if (node->_computed) {
      if (needSep)
        space();
      needSep = false;
      OS_ << '[';
    }
    if (needSep)
      OS_ << ' ';
    needSep = false;
    visitESTreeNode(*this, node->_key);
    if (node->_computed)
      OS_ << ']';
    if (node->_shorthand)
      return;

    if (node->_kind->str() != "init" || node->_method) {
      // getter/setter or method.
      auto *fe = llvh::cast<FunctionExpressionNode>(node->_value);
      visitFuncParamsAndBody(fe->_params, fe->_body, fe);
    } else {
      // Regular property.
      OS_ << ':';
      space();
      printCommaExpression(node->_value, node);
    }
  }

  void visit(YieldExpressionNode *node) {
    if (node->_delegate) {
      OS_ << "yield* ";
    } else {
      OS_ << "yield ";
    }
    printChild(node->_argument, node, ChildPos::right);
  }

  void visit(ArrayPatternNode *node) {
    OS_ << '[';
    int i = 0;
    for (auto &elem : node->_elements) {
      if (i++ != 0)
        comma();
      printCommaExpression(&elem, node);
    }
    OS_ << ']';
  }
  void visit(ArrayExpressionNode *node) {
    OS_ << '[';
    int i = 0;
    for (auto &elem : node->_elements) {
      if (i++ != 0)
        comma();
      printCommaExpression(&elem, node);
    }
    if (node->_trailingComma)
      comma();
    OS_ << ']';
  }
  void visit(RestElementNode *node) {
    OS_ << "...";
    space();
    visitESTreeNode(*this, node->_argument);
  }

  void visit(EmptyNode *node) {}

  void visit(CallExpressionNode *node) {
    printChild(node->_callee, node, ChildPos::left);
    OS_.write('(');
    int i = 0;
    for (auto &arg : node->_arguments) {
      if (i++ != 0)
        comma();
      printCommaExpression(&arg, node);
    }
    OS_.write(')');
  }

  void visit(NewExpressionNode *node) {
    OS_ << "new ";
    printChild(node->_callee, node, ChildPos::anywhere);
    OS_.write('(');
    int i = 0;
    for (auto &arg : node->_arguments) {
      if (i++ != 0)
        comma();
      printCommaExpression(&arg, node);
    }
    OS_.write(')');
  }

  void visit(AssignmentPatternNode *node) {
    printChild(node->_left, node, ChildPos::left);
    space();
    OS_ << "=";
    space();
    printChild(node->_right, node, ChildPos::right);
  }

  void visit(AssignmentExpressionNode *node) {
    printChild(node->_left, node, ChildPos::left);
    space();
    OS_ << node->_operator->str();
    space();
    printChild(node->_right, node, ChildPos::right);
  }

  void visit(UnaryExpressionNode *node) {
    bool const ident = isalpha(node->_operator->str().front());
    OS_ << node->_operator->str();
    if (ident)
      OS_ << ' ';
    printChild(node->_argument, node, ChildPos::right);
  }

  void visit(UpdateExpressionNode *node) {
    if (node->_prefix) {
      OS_ << node->_operator->str();
      printChild(node->_argument, node, ChildPos::right);
    } else {
      printChild(node->_argument, node, ChildPos::left);
      OS_ << node->_operator->str();
    }
  }

  void visit(MemberExpressionNode *node) {
    visitESTreeNode(*this, node->_object, node);
    if (node->_computed)
      OS_ << '[';
    else
      OS_ << '.';
    visitESTreeNode(*this, node->_property, node);
    if (node->_computed)
      OS_ << ']';
  }

  void visit(LogicalExpressionNode *node) {
    printChild(node->_left, node, ChildPos::left);
    space();
    OS_ << node->_operator->str();
    space();
    printChild(node->_right, node, ChildPos::right);
  }
  void visit(BinaryExpressionNode *node) {
    bool const ident = isalpha(node->_operator->str().front());
    printChild(node->_left, node, ChildPos::left);
    space(ident);
    OS_ << node->_operator->str();
    space(ident);
    printChild(node->_right, node, ChildPos::right);
  }
  void visit(ConditionalExpressionNode *node) {
    printChild(node->_test, node, ChildPos::left);
    space();
    OS_ << '?';
    space();
    printChild(node->_consequent, node, ChildPos::anywhere);
    space();
    OS_ << ':';
    space();
    printChild(node->_alternate, node, ChildPos::right);
  }

  void visit(VariableDeclarationNode *node) {
    OS_ << node->_kind->str() << ' ';
    int i = 0;
    for (auto &decl : node->_declarations) {
      if (i++ != 0)
        comma();
      visitESTreeNode(*this, &decl, node);
    }
  }

  void visit(VariableDeclaratorNode *node) {
    visitESTreeNode(*this, node->_id, node);
    if (node->_init) {
      OS_ << (pretty_ ? " = " : "=");
      visitESTreeNode(*this, node->_init, node);
    }
  }

  void visit(ThrowStatementNode *node) {
    OS_ << "throw ";
    visitESTreeNode(*this, node->_argument, node);
  }
  void visit(ReturnStatementNode *node) {
    OS_ << "return";
    if (node->_argument) {
      OS_ << ' ';
      visitESTreeNode(*this, node->_argument, node);
    }
  }

  void visit(WhileStatementNode *node) {
    OS_ << "while";
    space();
    OS_ << '(';
    visitESTreeNode(*this, node->_test, node);
    OS_ << ')';
    visitStmtOrBlock(node->_body, false, node);
  }

  void visit(DoWhileStatementNode *node) {
    OS_ << "do";
    bool block = visitStmtOrBlock(node->_body, false, node);
    if (block) {
      space();
    } else {
      OS_ << ";";
      newline();
    }
    OS_ << "while";
    space();
    OS_ << '(';
    visitESTreeNode(*this, node->_test, node);
    OS_ << ')';
  }

  void visit(IfStatementNode *node) {
    OS_ << "if";
    space();
    OS_ << '(';
    visitESTreeNode(*this, node->_test, node);
    OS_ << ')';
    bool forceBlock = node->_alternate && isIfWithoutElse(node->_consequent);
    bool block = visitStmtOrBlock(node->_consequent, forceBlock, node);
    if (!node->_alternate)
      return;
    if (!block) {
      OS_ << ";";
      newline();
    } else {
      space();
    }
    OS_ << "else";
    visitStmtOrBlock(node->_alternate, false, node);
  }

  /// Visit a statement node which is the body of a loop or a clause in an if.
  /// It could be a block statement.
  /// \return true if block
  bool visitStmtOrBlock(Node *node, bool forceBlock, Node *parent) {
    auto *block = llvh::dyn_cast<BlockStatementNode>(node);
    if ((block && block->_body.empty()) ||
        llvh::isa<EmptyStatementNode>(node)) {
      space();
      OS_ << "{}";
      return true;
    }
    if (block || forceBlock) {
      space();
      OS_ << '{';
      incIndent();
      newline();
      if (block)
        visitStmtList(block->_body, block);
      else
        visitStmtInABlock(node, parent);
      decIndent();
      newline();
      OS_ << "}";
      return true;
    } else {
      incIndent();
      newline();
      visitESTreeNode(*this, node, parent);
      decIndent();
      return false;
    }
  }

  void visit(EmptyStatementNode *) {}

  void visit(ForStatementNode *node) {
    OS_ << "for(";
    printChild(node->_init, node, ChildPos::left);
    OS_ << ';';
    if (node->_test) {
      space();
      visitESTreeNode(*this, node->_test, node);
    }
    OS_ << ';';
    if (node->_update) {
      space();
      visitESTreeNode(*this, node->_update, node);
    }
    OS_ << ')';
    visitStmtOrBlock(node->_body, false, node);
  }

  void visit(ForInStatementNode *node) {
    visitForInOf("in", node->_left, node->_right, node->_body, node);
  }
  void visit(ForOfStatementNode *node) {
    visitForInOf("of", node->_left, node->_right, node->_body, node);
  }
  void visitForInOf(
      llvh::StringRef op,
      Node *left,
      Node *right,
      Node *body,
      Node *node) {
    OS_ << "for(";
    visitESTreeNode(*this, left, node);
    OS_ << ' ' << op << ' ';
    visitESTreeNode(*this, right, node);
    OS_ << ')';
    visitStmtOrBlock(body, false, node);
  }

  void visit(TryStatementNode *node) {
    OS_ << "try";
    visitStmtOrBlock(node->_block, true, node);
    visitESTreeNode(*this, node->_handler, node);
    if (node->_finalizer)
      visitStmtOrBlock(node->_finalizer, true, node);
  }
  void visit(CatchClauseNode *node) {
    space();
    OS_ << "catch";
    if (node->_param) {
      space();
      OS_ << '(';
      visitESTreeNode(*this, node->_param, node);
      OS_ << ')';
    }
    visitStmtOrBlock(node->_body, true, node);
  }

  void visit(SwitchStatementNode *node) {
    OS_ << "switch";
    space();
    OS_ << '(';
    visitESTreeNode(*this, node->_discriminant, node);
    OS_ << ')';
    space();
    OS_ << '{';
    newline();
    for (auto &c : node->_cases) {
      visitESTreeNode(*this, &c, node);
      newline();
    }
    OS_ << '}';
  }
  void visit(SwitchCaseNode *node) {
    if (node->_test) {
      OS_ << "case ";
      visitESTreeNode(*this, node->_test, node);
    } else {
      OS_ << "default";
    }
    OS_ << ':';
    if (!node->_consequent.empty()) {
      incIndent();
      newline();
      visitStmtList(node->_consequent, node);
      decIndent();
    }
  }

  void visit(BreakStatementNode *node) {
    OS_ << "break";
    if (node->_label) {
      OS_ << ' ';
      visitESTreeNode(*this, node->_label, node);
    }
  }
  void visit(ContinueStatementNode *node) {
    OS_ << "continue";
    if (node->_label) {
      OS_ << ' ';
      visitESTreeNode(*this, node->_label, node);
    }
  }
  void visit(LabeledStatementNode *node) {
    visitESTreeNode(*this, node->_label, node);
    OS_ << ':';
    newline();
    visitESTreeNode(*this, node->_body, node);
  }

  void visit(BlockStatementNode *node) {
    if (node->_body.empty()) {
      OS_ << "{}";
      return;
    }
    OS_ << '{';
    incIndent();
    newline();

    visitStmtList(node->_body, node);

    decIndent();
    newline();
    OS_ << '}';
  }

  void visit(ProgramNode *node) {
    visitStmtList(node->_body, node);
  }

  void visitStmtInABlock(Node *stmt, Node *parent) {
    visitESTreeNode(*this, stmt, parent);
    if (!endsWithBlock(stmt))
      OS_ << ";";
  }
  void visitStmtList(NodeList &list, Node *parent) {
    int i = 0;
    for (auto &stmt : list) {
      if (i++)
        newline();
      visitStmtInABlock(&stmt, parent);
    }
  }

  void visit(ClassExpressionNode *node) {
    visitClass(node->_id, node->_superClass, node->_body, node);
  }
  void visit(ClassDeclarationNode *node) {
    visitClass(node->_id, node->_superClass, node->_body, node);
  }
  void visitClass(Node *id, Node *superClass, Node *body, Node *parent) {
    OS_ << "class";
    if (id) {
      OS_ << ' ';
      visitESTreeNode(*this, id, parent);
    }
    if (superClass) {
      OS_ << ' ';
      visitESTreeNode(*this, superClass, parent);
    }
    space();
    visitESTreeNode(*this, body, parent);
  }
  void visit(ClassBodyNode *node) {
    OS_ << '{';
    if (node->_body.empty()) {
      OS_ << '}';
      return;
    }
    incIndent();
    newline();

    for (auto &p : node->_body) {
      visitESTreeNode(*this, &p, node);
      newline();
    }

    OS_ << '}';
    decIndent();
  }

  void visit(FunctionExpressionNode *node) {
    visitFuncLike(
        node->_async,
        node->_generator,
        node->_id,
        node->_params,
        node->_body,
        node);
  }

  void visit(FunctionDeclarationNode *node) {
    visitFuncLike(
        node->_async,
        node->_generator,
        node->_id,
        node->_params,
        node->_body,
        node);
  }

  void visit(ArrowFunctionExpressionNode *node) {
    bool needSep = false;
    if (node->_async) {
      OS_ << "async";
      needSep = true;
    }
    // Single parameter doesn't need parens. But only in expression mode,
    // otherwise it is ugly.
    if ((node->_expression || !pretty_) && node->_params.size() == 1) {
      if (needSep)
        OS_ << ' ';
      visitESTreeNode(*this, &node->_params.front(), node);
    } else {
      OS_ << '(';
      int i = 0;
      for (auto &param : node->_params) {
        if (i++)
          comma();
        visitESTreeNode(*this, &param, node);
      }
      OS_ << ')';
    }
    space();
    OS_ << "=>";
    space();
    if (llvh::isa<BlockStatementNode>(node->_body))
      visitESTreeNode(*this, node->_body, node);
    else
      printChild(node->_body, node, ChildPos::right);
  }

  void visitFuncLike(
      bool async,
      bool generator,
      Node *id,
      NodeList &params,
      Node *body,
      Node *node) {
    if (async)
      OS_ << "async ";
    OS_ << "function";
    if (generator) {
      OS_ << '*';
      if (id)
        space();
    } else {
      if (id)
        OS_ << ' ';
    }
    if (id)
      visitESTreeNode(*this, id, node);
    visitFuncParamsAndBody(params, body, node);
  }

  void visitFuncParamsAndBody(NodeList &params, Node *body, Node *node) {
    OS_ << '(';
    int i = 0;
    for (auto &param : params) {
      if (i++ != 0)
        comma();
      visitESTreeNode(*this, &param, node);
    }
    OS_ << ')';
    visitESTreeNode(*this, body, node);
  }

  void visit(ExpressionStatementNode *node) {
    printChild(node->_expression, node, ChildPos::anywhere);
  }

 private:
  void incIndent() {
    indent_ += indentStep_;
    assert(indent_ > 0 && "Impressive! Indent overflow.");
  }
  void decIndent() {
    indent_ -= indentStep_;
    assert(indent_ >= 0 && "indent underflow");
  }
  void comma() {
    OS_ << (pretty_ ? ", " : ",");
  }
  void space(bool force = false) {
    if (force || pretty_)
      OS_.write(' ');
  }
  void newline() {
    if (pretty_)
      OS_.write('\n').indent(indent_);
  }

  void printChild(Node *child, Node *parent, ChildPos childPosition) {
    if (!child)
      return;
    printParens(child, parent, needParens(parent, child, childPosition));
  }

  /// Print one expression in a sequence separated by comma. It needs parens
  /// if its precedence is <= comma.
  void printCommaExpression(Node *child, Node *parent) {
    printParens(
        child,
        parent,
        getPrecedence(child).prec <= kSeqPrec ? NeedParens::yes
                                              : NeedParens::no);
  }

  void printParens(Node *child, Node *parent, NeedParens needParens) {
    if (needParens == NeedParens::yes)
      OS_ << '(';
    else if (needParens == NeedParens::space)
      OS_ << ' ';
    visitESTreeNode(*this, child, parent);
    if (needParens == NeedParens::yes)
      OS_ << ')';
  }

  /// Print the escaped contents of the specified string.
  /// \param esc1 Additional character to escape. This is the string separator.
  /// \param esc2 An optional string to escape. It this string is non-empty and
  ///     it is encountered, its first character will be escaped.
  void printEscapedStringContents(
      llvh::StringRef str,
      char esc1,
      llvh::StringRef esc2 = {});

  /// \return the precendence and associativity of an operator.
  PrecRTL getPrecedence(Node *node);

  /// Check whether we need parens for the child, when rendered in the parent
  /// @param childPosition where the child is printed relative to the parent.
  NeedParens needParens(Node *parent, Node *child, ChildPos childPosition);

  /// \return true if the node (which could be null) is an IfStatement without
  /// an else.
  static bool isIfWithoutElse(Node *node) {
    auto *ifN = llvh::dyn_cast_or_null<IfStatementNode>(node);
    return ifN && !ifN->_alternate;
  }

  /// \return true if the AST expression starts with a node satisfying the
  ///     predicate \pred when rendered. This involves recursively checking the
  ///     left children.
  /// \param parent only set when the function is calling itself recursively
  ///     with the parent of the \c expr node.
  bool _exprStartsWith(Node *expr, Node *parent, bool (*pred)(Node *));

  /// \return true if the AST expression starts with a node satisfying the
  ///     predicate \pred when rendered. This involves recursively checking the
  ///     left children.
  bool exprStartsWith(Node *expr, bool (*pred)(Node *)) {
    return _exprStartsWith(expr, nullptr, pred);
  }

  /// \return true if the node ends with a block (so we don't need to generate
  /// a semi-colon).
  static bool endsWithBlock(Node *node);
};

void GenJS::printEscapedStringContents(
    llvh::StringRef str,
    char esc1,
    llvh::StringRef esc2) {
  for (const char *cur = str.data(), *e = str.data() + str.size(); cur < e;) {
    uint32_t cp = decodeUTF8<true>(cur, [](const llvh::Twine &) {});
    // Check for classic escapes first.
    switch (cp) {
      case '\\':
        OS_ << "\\\\";
        continue;
      case '\b':
        OS_ << "\\b";
        continue;
      case '\f':
        OS_ << "\\f";
        continue;
      case '\n':
        OS_ << "\\n";
        continue;
      case '\r':
        OS_ << "\\r";
        continue;
      case '\t':
        OS_ << "\\t";
        continue;
      case '\v':
        OS_ << "\\v";
        continue;
    }
    // Check for escaping.
    if (cp == (unsigned char)esc1) {
      OS_ << '\\' << esc1;
      continue;
      ;
    }
    if (!esc2.empty() && cp == (unsigned char)esc2.front()) {
      if (str.substr(cur - str.data() - 1, esc2.size()) == esc2) {
        OS_ << '\\' << esc2;
        cur += esc2.size() - 1;
        continue;
      }
    }
    // Printable.
    if (cp >= 0x20 && cp <= 0x7f) {
      OS_.write((char)cp);
      continue;
    }
    // Non-printable single byte
    if (cp < 0x100) {
      OS_ << "\\x" << llvh::format_hex_no_prefix(cp, 2);
      continue;
    }
    // Non-printable two bytes.
    if (cp < 0x10000) {
      OS_ << "\\u" << llvh::format_hex_no_prefix(cp, 4);
      continue;
    }
    // utf-32 encoded. Decode into a surrogate pair.
    uint16_t u16buf[2], *p = u16buf;
    encodeUTF16(p, cp);
    OS_ << "\\u" << llvh::format_hex_no_prefix(u16buf[0], 4) << "\\u"
        << llvh::format_hex_no_prefix(u16buf[1], 4);
  }
}

PrecRTL GenJS::getPrecedence(Node *node) {
  // Precedence order taken from
  // https://github.com/facebook/flow/blob/master/src/parser_utils/output/js_layout_generator.ml

  if (llvh::isa<IdentifierNode>(node) || llvh::isa<NullLiteralNode>(node) ||
      llvh::isa<BooleanLiteralNode>(node) ||
      llvh::isa<StringLiteralNode>(node) ||
      llvh::isa<NumericLiteralNode>(node) ||
      llvh::isa<RegExpLiteralNode>(node) ||
      llvh::isa<ThisExpressionNode>(node) || llvh::isa<SuperNode>(node) ||
      llvh::isa<ArrayExpressionNode>(node) ||
      llvh::isa<ObjectExpressionNode>(node) ||
      llvh::isa<ObjectPatternNode>(node) ||
      llvh::isa<FunctionExpressionNode>(node) ||
      llvh::isa<ClassExpressionNode>(node) ||
      llvh::isa<TemplateLiteralNode>(node) ||
      llvh::isa<RestElementNode>(node)) {
    return kPrimaryPrec;
  }

  if (llvh::isa<MemberExpressionNode>(node) ||
      llvh::isa<MetaPropertyNode>(node) ||
      llvh::isa<CallExpressionNode>(node) ||
      // `new foo()` has higher precedence than `new foo`. In pretty mode we
      // always append the `()`, but otherwise we must check the number of args.
      (llvh::isa<NewExpressionNode>(node) &&
       (pretty_ || !llvh::cast<NewExpressionNode>(node)->_arguments.empty()))) {
    return kMemberPrec;
  }

  if (llvh::isa<NewExpressionNode>(node)) {
    return kNewNoArgsPrec;
  }

  if (llvh::isa<TaggedTemplateExpressionNode>(node) ||
      llvh::isa<ImportExpressionNode>(node)) {
    return kTaggedTemplatePrec;
  }

  if (llvh::isa<UpdateExpressionNode>(node) &&
      !llvh::cast<UpdateExpressionNode>(node)->_prefix) {
    return kPostUpdatePrec;
  }

  if (llvh::isa<UpdateExpressionNode>(node) ||
      llvh::isa<UnaryExpressionNode>(node)) {
    return {kUnaryPrec, true};
  }

  if (auto *bin = llvh::dyn_cast<BinaryExpressionNode>(node))
    return getBinaryPrecedence(bin->_operator->str());
  if (auto *bin = llvh::dyn_cast<LogicalExpressionNode>(node))
    return getBinaryPrecedence(bin->_operator->str());

  if (llvh::isa<ConditionalExpressionNode>(node))
    return {kCondPrec, true};

  if (llvh::isa<AssignmentExpressionNode>(node) ||
      llvh::isa<AssignmentPatternNode>(node))
    return {kAssignPrec, true};

  if (llvh::isa<YieldExpressionNode>(node))
    return kYieldPrec;

  if (llvh::isa<ArrowFunctionExpressionNode>(node))
    return kYieldPrec;

  if (llvh::isa<SequenceExpressionNode>(node))
    return {kSeqPrec, true};

  return kAlwaysParenPrec;
}

static bool isUnary(Node *node, llvh::StringRef op) {
  return llvh::isa<UnaryExpressionNode>(node) &&
      llvh::cast<UnaryExpressionNode>(node)->_operator->str() == op;
}
static bool isUpdatePrefix(Node *node, llvh::StringRef op) {
  return llvh::isa<UpdateExpressionNode>(node) &&
      llvh::cast<UpdateExpressionNode>(node)->_prefix &&
      llvh::cast<UpdateExpressionNode>(node)->_operator->str() == op;
}
static bool isNegativeNumber(Node *node) {
  return llvh::isa<NumericLiteralNode>(node) &&
      llvh::cast<NumericLiteralNode>(node)->_value < 0;
}
static bool isBinary(Node *node, llvh::StringRef op) {
  return llvh::isa<BinaryExpressionNode>(node) &&
      llvh::cast<BinaryExpressionNode>(node)->_operator->str() == op;
}

static bool checkPlus(Node *node) {
  return isUnary(node, "+") || isUpdatePrefix(node, "++");
}
static bool checkMinus(Node *node) {
  return isUnary(node, "-") || isUpdatePrefix(node, "--") ||
      isNegativeNumber(node);
}

NeedParens
GenJS::needParens(Node *parent, Node *child, ChildPos childPosition) {
  // The explicit checks taken from
  // https://github.com/facebook/flow/blob/master/src/parser_utils/output/js_layout_generator.ml

  // (x) => ({x: 10}) needs parens to avoid confusing it with a block and a
  // labelled statement.
  if (llvh::isa<ArrowFunctionExpressionNode>(parent)) {
    if (childPosition == ChildPos::right &&
        llvh::isa<ObjectExpressionNode>(child)) {
      return NeedParens::yes;
    }
  }
  // for((a in b);..;..) needs parens to avoid confusing it with for(a in b).
  else if (llvh::isa<ForStatementNode>(parent)) {
    return np(
        llvh::isa<BinaryExpressionNode>(child) &&
        llvh::cast<BinaryExpressionNode>(child)->_operator->str() == "in");
  }
  // Expression statement like (function () {} + 1) needs parens.
  else if (llvh::isa<ExpressionStatementNode>(parent)) {
    return np(exprStartsWith(child, [](Node *expr) {
      return llvh::isa<FunctionExpressionNode>(expr) ||
          llvh::isa<ClassExpressionNode>(expr) ||
          llvh::isa<ObjectExpressionNode>(expr) ||
          llvh::isa<ObjectPatternNode>(expr);
    }));
  }
  // -(-x) or -(--x) or -(-5)
  else if (isUnary(parent, "-") && exprStartsWith(child, checkMinus)) {
    return pretty_ ? NeedParens::yes : NeedParens::space;
  }
  // +(+x) or +(++x)
  else if (isUnary(parent, "+") && exprStartsWith(child, checkPlus)) {
    return pretty_ ? NeedParens::yes : NeedParens::space;
  }
  // a-(-x) or a-(--x) or a-(-5)
  else if (
      childPosition == ChildPos::right && isBinary(parent, "-") &&
      exprStartsWith(child, checkMinus)) {
    return pretty_ ? NeedParens::yes : NeedParens::space;
  }
  // a+(+x) or a+(++x)
  else if (
      childPosition == ChildPos::right && isBinary(parent, "+") &&
      exprStartsWith(child, checkPlus)) {
    return pretty_ ? NeedParens::yes : NeedParens::space;
  }

  auto childPr = getPrecedence(child);
  if (childPr.prec == kAlwaysParenPrec)
    return NeedParens::yes;

  auto parentPr = getPrecedence(parent);

  if (childPr.prec < parentPr.prec) {
    // Child is definitely a danger.
    return NeedParens::yes;
  }
  if (childPr.prec > parentPr.prec) {
    // Definitely cool.
    return NeedParens::no;
  }
  // Equal precedence, so associativity (rtl/ltr) is what matters.
  if (childPosition == ChildPos::anywhere) {
    // Child could be anywhere, so always paren.
    return NeedParens::yes;
  }
  if (childPr.prec == kTopPrec) {
    // Both precedences are safe.
    return NeedParens::no;
  }
  // Check if child is on the dangerous side.
  return parentPr.rtl ? np(childPosition == ChildPos::left)
                      : np(childPosition == ChildPos::right);
}

bool GenJS::_exprStartsWith(Node *expr, Node *parent, bool (*pred)(Node *)) {
  // NOTE: I hope this function is converted into a loop.
  if (parent && needParens(parent, expr, ChildPos::left) == NeedParens::yes)
    return false;

  if (pred(expr))
    return true;

  if (auto *call = llvh::dyn_cast<CallExpressionNode>(expr))
    return _exprStartsWith(call->_callee, expr, pred);
  if (auto *call = llvh::dyn_cast<OptionalCallExpressionNode>(expr))
    return _exprStartsWith(call->_callee, expr, pred);
  if (auto *bin = llvh::dyn_cast<BinaryExpressionNode>(expr))
    return _exprStartsWith(bin->_left, expr, pred);
  if (auto *bin = llvh::dyn_cast<LogicalExpressionNode>(expr))
    return _exprStartsWith(bin->_left, expr, pred);
  if (auto *cond = llvh::dyn_cast<ConditionalExpressionNode>(expr))
    return _exprStartsWith(cond->_test, expr, pred);
  if (auto *ass = llvh::dyn_cast<AssignmentExpressionNode>(expr))
    return _exprStartsWith(ass->_left, expr, pred);
  if (auto *upd = llvh::dyn_cast<UpdateExpressionNode>(expr))
    return !upd->_prefix && _exprStartsWith(upd->_argument, expr, pred);
  if (auto *un = llvh::dyn_cast<UnaryExpressionNode>(expr))
    return !un->_prefix && _exprStartsWith(un->_argument, expr, pred);
  if (auto *m = llvh::dyn_cast<MemberExpressionNode>(expr))
    return _exprStartsWith(m->_object, expr, pred);
  if (auto *t = llvh::dyn_cast<TaggedTemplateExpressionNode>(expr))
    return _exprStartsWith(t->_tag, expr, pred);

  return false;
}

bool GenJS::endsWithBlock(Node *node) {
  // NOTE: I hope this function is converted into a loop.
  if (!node)
    return false;
  if (llvh::isa<BlockStatementNode>(node))
    return true;
  if (llvh::isa<FunctionDeclarationNode>(node))
    return true;
  if (auto *s = llvh::dyn_cast<WhileStatementNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<ForInStatementNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<ForOfStatementNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<ForStatementNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<WithStatementNode>(node))
    return endsWithBlock(s->_body);
  if (llvh::isa<SwitchStatementNode>(node))
    return true;
  if (auto *s = llvh::dyn_cast<LabeledStatementNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<TryStatementNode>(node))
    return endsWithBlock(s->_finalizer ? s->_finalizer : s->_handler);
  if (auto *s = llvh::dyn_cast<CatchClauseNode>(node))
    return endsWithBlock(s->_body);
  if (auto *s = llvh::dyn_cast<IfStatementNode>(node))
    return endsWithBlock(s->_alternate ? s->_alternate : s->_consequent);
  if (llvh::isa<ClassDeclarationNode>(node))
    return true;
  return false;
}

} // Anonymous namespace

void generateJS(llvh::raw_ostream &OS, Node *root, bool pretty) {
  GenJS genJS{OS, pretty};
  genJS.doIt(root);
}

} // namespace hermes
