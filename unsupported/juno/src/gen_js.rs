/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::{Node, NodeKind, Visitor};
use std::{
    fmt,
    io::{self, BufWriter, Write},
};

/// Whether to pretty-print the generated JS.
/// Does not do full formatting of the source, but does add indentation and
/// some extra spaces to make source more readable.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Pretty {
    No,
    Yes,
}

/// Generate JS for `root` and print it to `out`.
pub fn generate<W: Write>(out: W, root: &Node, pretty: Pretty) -> io::Result<()> {
    GenJS::gen_root(out, root, pretty)
}

/// Associativity direction.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum Assoc {
    /// Left to right associativity.
    Ltr,

    /// Right to left associativity.
    Rtl,
}

mod precedence {
    pub type Precedence = u32;
    pub const ALWAYS_PAREN: Precedence = 0;
    pub const SEQ: Precedence = 1;
    pub const ARROW: Precedence = 2;
    pub const YIELD: Precedence = 3;
    pub const ASSIGN: Precedence = 4;
    pub const COND: Precedence = 5;
    pub const BIN_START: Precedence = 6;
    pub const UNARY: Precedence = 26;
    pub const POST_UPDATE: Precedence = 27;
    pub const TAGGED_TEMPLATE: Precedence = 28;
    pub const NEW_NO_ARGS: Precedence = 29;
    pub const MEMBER: Precedence = 30;
    pub const PRIMARY: Precedence = 31;
    pub const TOP: Precedence = 32;

    pub fn get_binary_precedence(op: &str) -> Precedence {
        match op {
            "**" => 12,
            "*" => 11,
            "%" => 11,
            "/" => 11,
            "+" => 10,
            "-" => 10,
            "<<" => 9,
            ">>" => 9,
            ">>>" => 9,
            "<" => 8,
            ">" => 8,
            "<=" => 8,
            ">=" => 8,
            "==" => 7,
            "!=" => 7,
            "===" => 7,
            "!==" => 7,
            "&" => 6,
            "^" => 5,
            "|" => 4,
            "&&" => 3,
            "||" => 2,
            "??" => 1,
            "in" => 8 + BIN_START,
            "instanceof" => 8 + BIN_START,
            _ => ALWAYS_PAREN,
        }
    }
}

/// Child position for the purpose of determining whether the child needs parens.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum ChildPos {
    Left,
    Anywhere,
    Right,
}

/// Whether parens are needed around something.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum NeedParens {
    /// No parentheses needed.
    No,
    /// Parentheses required.
    Yes,
    /// A space character is sufficient to distinguish.
    /// Used in unary operations, e.g.
    Space,
}

impl From<bool> for NeedParens {
    fn from(x: bool) -> NeedParens {
        if x { NeedParens::Yes } else { NeedParens::No }
    }
}

/// Whether to force a space when adding a space in JS generation.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum ForceSpace {
    No,
    Yes,
}

/// Generator for output JS. Walks the AST to output real JS.
struct GenJS<W: Write> {
    /// Where to write the generated JS.
    out: BufWriter<W>,

    /// Whether to pretty print the output JS.
    pretty: Pretty,

    /// Size of the indentation step.
    /// May be configurable in the future.
    indent_step: usize,

    /// Current indentation level, used in pretty mode.
    indent: usize,

    /// Some(err) if an error has occurred when writing, else None.
    error: Option<io::Error>,
}

/// Print to the output stream if no errors have been seen so far.
/// `$gen_js` is a mutable reference to the GenJS struct.
/// `$arg` arguments follow the format pattern used by `format!`.
macro_rules! out {
    ($gen_js:expr, $($arg:tt)*) => {{
        $gen_js.write_if_no_errors(format_args!($($arg)*));
    }}
}

impl<W: Write> GenJS<W> {
    /// Generate JS for `root` and flush the output.
    /// If at any point, JS generation resulted in an error, return `Err(err)`,
    /// otherwise return `Ok(())`.
    fn gen_root(writer: W, root: &Node, pretty: Pretty) -> io::Result<()> {
        let mut gen_js = GenJS {
            out: BufWriter::new(writer),
            pretty,
            indent_step: 2,
            indent: 0,
            error: None,
        };
        root.visit(&mut gen_js, None);
        out!(gen_js, "\n");
        match gen_js.error {
            None => gen_js.out.flush(),
            Some(err) => Err(err),
        }
    }

    /// Write to the `out` writer if we haven't seen any errors.
    /// If we have seen any errors, do nothing.
    /// Used via the `out!` macro.
    fn write_if_no_errors(&mut self, args: fmt::Arguments<'_>) {
        if self.error.is_none() {
            if let Err(e) = self.out.write_fmt(args) {
                self.error = Some(e);
            }
        }
    }

    /// Generate the JS for each node kind.
    fn gen_node(&mut self, node: &Node, _parent: Option<&Node>) {
        use NodeKind::*;
        match &node.kind {
            Identifier { name, .. } => {
                out!(self, "{}", &name.str);
            }
            BooleanLiteral { value, .. } => {
                out!(self, "{}", if *value { "true" } else { "false" });
            }
            NullLiteral => {
                out!(self, "null");
            }
            ThisExpression => {
                out!(self, "this");
            }
            Super => {
                out!(self, "super");
            }
            _ => {
                unimplemented!("Unsupported AST node kind: {}", node.kind.name());
            }
        };
    }

    /// Increase the indent level.
    fn inc_indent(&mut self) {
        self.indent += self.indent_step;
    }

    /// Decrease the indent level.
    fn dec_indent(&mut self) {
        self.indent -= self.indent_step;
    }

    /// Print a ',', with a trailing space in pretty mode.
    fn comma(&mut self) {
        out!(
            self,
            "{}",
            match self.pretty {
                Pretty::No => ",",
                Pretty::Yes => ", ",
            }
        )
    }

    /// Print a ' ' if forced by ForceSpace::Yes or pretty mode.
    fn space(&mut self, force: ForceSpace) {
        if self.pretty == Pretty::Yes || force == ForceSpace::Yes {
            out!(self, " ");
        }
    }

    /// Print a newline and indent if pretty.
    fn newline(&mut self) {
        if self.pretty == Pretty::Yes {
            out!(self, "\n{:indent$}", indent = self.indent as usize);
        }
    }

    /// Print the child of a `parent` node at the position `child_pos`.
    fn print_child(&mut self, child: Option<&Node>, parent: &Node, child_pos: ChildPos) {
        if let Some(child) = child {
            self.print_parens(child, parent, self.need_parens(parent, child, child_pos));
        }
    }

    fn print_parens(&mut self, child: &Node, parent: &Node, need_parens: NeedParens) {
        if need_parens == NeedParens::Yes {
            out!(self, "(");
        } else if need_parens == NeedParens::Space {
            out!(self, " ");
        }
        child.visit(self, Some(parent));
        if need_parens == NeedParens::Yes {
            out!(self, " ");
        }
    }

    /// Return the precedence and associativity of `node`.
    fn get_precedence(&self, node: &Node) -> (precedence::Precedence, Assoc) {
        // Precedence order taken from
        // https://github.com/facebook/flow/blob/master/src/parser_utils/output/js_layout_generator.ml
        use precedence::*;
        use NodeKind::*;
        match &node.kind {
            Identifier { .. }
            | NullLiteral { .. }
            | BooleanLiteral { .. }
            | StringLiteral { .. }
            | NumericLiteral { .. }
            | RegExpLiteral { .. }
            | ThisExpression { .. }
            | Super { .. }
            | ArrayExpression { .. }
            | ObjectExpression { .. }
            | ObjectPattern { .. }
            | FunctionExpression { .. }
            | ClassExpression { .. }
            | TemplateLiteral { .. } => (PRIMARY, Assoc::Ltr),
            MemberExpression { .. } | MetaProperty { .. } | CallExpression { .. } => {
                (MEMBER, Assoc::Ltr)
            }
            NewExpression { arguments, .. } => {
                // `new foo()` has higher precedence than `new foo`. In pretty mode we
                // always append the `()`, but otherwise we must check the number of args.
                if self.pretty == Pretty::Yes || !arguments.is_empty() {
                    (MEMBER, Assoc::Ltr)
                } else {
                    (NEW_NO_ARGS, Assoc::Ltr)
                }
            }
            TaggedTemplateExpression { .. } | ImportExpression { .. } => {
                (TAGGED_TEMPLATE, Assoc::Ltr)
            }
            UpdateExpression { prefix, .. } => {
                if *prefix {
                    (POST_UPDATE, Assoc::Ltr)
                } else {
                    (UNARY, Assoc::Rtl)
                }
            }
            UnaryExpression { .. } => (UNARY, Assoc::Rtl),
            BinaryExpression { operator, .. } => (get_binary_precedence(&operator.str), Assoc::Ltr),
            LogicalExpression { operator, .. } => {
                (get_binary_precedence(&operator.str), Assoc::Ltr)
            }
            ConditionalExpression { .. } => (COND, Assoc::Rtl),
            AssignmentExpression { .. } => (ASSIGN, Assoc::Rtl),
            YieldExpression { .. } | ArrowFunctionExpression { .. } => (YIELD, Assoc::Ltr),
            SequenceExpression { .. } => (SEQ, Assoc::Rtl),
            _ => (ALWAYS_PAREN, Assoc::Ltr),
        }
    }

    /// Return whether parentheses are needed around the `child` node,
    /// which is situated at `child_pos` position in relation to its `parent`.
    fn need_parens(&self, parent: &Node, child: &Node, child_pos: ChildPos) -> NeedParens {
        use NodeKind::*;
        if matches!(parent.kind, ArrowFunctionExpression { .. }) {
            // (x) => ({x: 10}) needs parens to avoid confusing it with a block and a
            // labelled statement.
            if child_pos == ChildPos::Right && matches!(child.kind, ObjectExpression { .. }) {
                return NeedParens::Yes;
            }
        } else if matches!(parent.kind, ForStatement { .. }) {
            // for((a in b);..;..) needs parens to avoid confusing it with for(a in b).
            return NeedParens::from(match &child.kind {
                BinaryExpression { operator, .. } => &operator.str == "in",
                _ => false,
            });
        } else if matches!(parent.kind, ExpressionStatement { .. }) {
            // Expression statement like (function () {} + 1) needs parens.
            return NeedParens::from(self.root_starts_with(child, |expr| -> bool {
                matches!(
                    expr.kind,
                    FunctionExpression { .. }
                        | ClassExpression { .. }
                        | ObjectExpression { .. }
                        | ObjectPattern { .. }
                )
            }));
        } else if (is_unary_op(parent, "-") && self.root_starts_with(child, check_minus))
            || (is_unary_op(parent, "+") && self.root_starts_with(child, check_plus))
            || (child_pos == ChildPos::Right
                && is_binary_op(parent, "-")
                && self.root_starts_with(child, check_minus))
            || (child_pos == ChildPos::Right
                && is_binary_op(parent, "+")
                && self.root_starts_with(child, check_plus))
        {
            // -(-x) or -(--x) or -(-5)
            // +(+x) or +(++x)
            // a-(-x) or a-(--x) or a-(-5)
            // a+(+x) or a+(++x)
            return if self.pretty == Pretty::Yes {
                NeedParens::Yes
            } else {
                NeedParens::Space
            };
        }

        let (child_prec, _child_assoc) = self.get_precedence(child);
        if child_prec == precedence::ALWAYS_PAREN {
            return NeedParens::Yes;
        }

        let (parent_prec, parent_assoc) = self.get_precedence(parent);

        if child_prec < parent_prec {
            // Child is definitely a danger.
            return NeedParens::Yes;
        }
        if child_prec > parent_prec {
            // Definitely cool.
            return NeedParens::No;
        }
        // Equal precedence, so associativity (rtl/ltr) is what matters.
        if child_pos == ChildPos::Anywhere {
            // Child could be anywhere, so always paren.
            return NeedParens::Yes;
        }
        if child_prec == precedence::TOP {
            // Both precedences are safe.
            return NeedParens::No;
        }
        // Check if child is on the dangerous side.
        NeedParens::from(if parent_assoc == Assoc::Rtl {
            child_pos == ChildPos::Left
        } else {
            child_pos == ChildPos::Right
        })
    }

    fn root_starts_with<F: Fn(&Node) -> bool>(&self, expr: &Node, pred: F) -> bool {
        self.expr_starts_with(expr, None, pred)
    }

    fn expr_starts_with<F: Fn(&Node) -> bool>(
        &self,
        expr: &Node,
        parent: Option<&Node>,
        pred: F,
    ) -> bool {
        use NodeKind::*;

        if let Some(parent) = parent {
            if self.need_parens(parent, expr, ChildPos::Left) == NeedParens::Yes {
                return false;
            }
        }

        if pred(expr) {
            return true;
        }

        // Ensure the recursive calls are the last things to run,
        // hopefully the compiler makes this into a loop.
        match &expr.kind {
            CallExpression { callee, .. } => self.expr_starts_with(callee, Some(expr), pred),
            OptionalCallExpression { callee, .. } => {
                self.expr_starts_with(callee, Some(expr), pred)
            }
            BinaryExpression { left, .. } => self.expr_starts_with(left, Some(expr), pred),
            LogicalExpression { left, .. } => self.expr_starts_with(left, Some(expr), pred),
            ConditionalExpression { test, .. } => self.expr_starts_with(test, Some(expr), pred),
            AssignmentExpression { left, .. } => self.expr_starts_with(left, Some(expr), pred),
            UpdateExpression {
                prefix, argument, ..
            } => !*prefix && self.expr_starts_with(argument, Some(expr), pred),
            UnaryExpression {
                prefix, argument, ..
            } => !*prefix && self.expr_starts_with(argument, Some(expr), pred),
            MemberExpression { object, .. } => self.expr_starts_with(object, Some(expr), pred),
            TaggedTemplateExpression { tag, .. } => self.expr_starts_with(tag, Some(expr), pred),
            _ => false,
        }
    }
}

impl<W: Write> Visitor for GenJS<W> {
    fn call(&mut self, node: &Node, parent: Option<&Node>) {
        self.gen_node(node, parent);
    }
}

fn is_unary_op(node: &Node, op: &str) -> bool {
    match &node.kind {
        NodeKind::UnaryExpression { operator, .. } => operator.str == op,
        _ => false,
    }
}

fn is_update_prefix(node: &Node, op: &str) -> bool {
    match &node.kind {
        NodeKind::UpdateExpression {
            prefix, operator, ..
        } => *prefix && operator.str == op,
        _ => false,
    }
}

fn is_negative_number(node: &Node) -> bool {
    match &node.kind {
        NodeKind::NumericLiteral { value, .. } => *value < 0f64,
        _ => false,
    }
}

fn is_binary_op(node: &Node, op: &str) -> bool {
    match &node.kind {
        NodeKind::BinaryExpression { operator, .. } => operator.str == op,
        _ => false,
    }
}

fn is_if_without_else(node: &Node) -> bool {
    match &node.kind {
        NodeKind::IfStatement { alternate, .. } => alternate.is_none(),
        _ => false,
    }
}

fn check_plus(node: &Node) -> bool {
    is_unary_op(node, "+") || is_update_prefix(node, "++")
}

fn check_minus(node: &Node) -> bool {
    is_unary_op(node, "-") || is_update_prefix(node, "--")
}
