/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::{
    ast::{Node, NodeKind, NodePtr, StringLiteral, Visitor},
    convert,
};
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

    pub const UNION_TYPE: Precedence = 1;
    pub const INTERSECTION_TYPE: Precedence = 2;

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

/// Whether to force the statements to be emitted inside a new block `{ }`.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
enum ForceBlock {
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
    fn gen_node(&mut self, node: &Node, parent: Option<&Node>) {
        use NodeKind::*;
        match &node.kind {
            Empty => {}
            Metadata => {}

            Program { body } => {
                self.visit_stmt_list(body, node);
            }

            FunctionExpression {
                id,
                params,
                body,
                generator,
                is_async,
                ..
            }
            | FunctionDeclaration {
                id,
                params,
                body,
                generator,
                is_async,
                ..
            } => {
                if *is_async {
                    out!(self, "async ");
                }
                out!(self, "function");
                if *generator {
                    out!(self, "*");
                    if id.is_some() {
                        self.space(ForceSpace::No);
                    }
                } else {
                    if id.is_some() {
                        self.space(ForceSpace::No);
                    }
                }
                if let Some(id) = id {
                    id.visit(self, Some(node));
                }
                self.visit_func_params_body(params, body, node);
            }

            ArrowFunctionExpression {
                params,
                body,
                expression,
                is_async,
                ..
            } => {
                let mut need_sep = false;
                if *is_async {
                    out!(self, "async");
                    need_sep = true;
                }
                // Single parameter doesn't need parens. But only in expression mode,
                // otherwise it is ugly.
                if params.len() == 1 && (*expression || self.pretty == Pretty::No) {
                    if need_sep {
                        out!(self, " ");
                        params[0].visit(self, Some(node));
                    }
                } else {
                    out!(self, "(");
                    for (i, param) in params.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        param.visit(self, Some(node));
                    }
                    out!(self, ")");
                }
                self.space(ForceSpace::No);
                out!(self, "=>");
                self.space(ForceSpace::No);
                match &body.kind {
                    BlockStatement { .. } => {
                        body.visit(self, Some(node));
                    }
                    _ => {
                        self.print_child(Some(body), node, ChildPos::Right);
                    }
                }
            }

            WhileStatement { body, test } => {
                out!(self, "while");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(self, Some(node));
                out!(self, ")");
                self.visit_stmt_or_block(body, ForceBlock::No, node);
            }
            DoWhileStatement { body, test } => {
                out!(self, "do");
                let block = self.visit_stmt_or_block(body, ForceBlock::No, node);
                if block {
                    self.space(ForceSpace::No);
                } else {
                    out!(self, ";");
                    self.newline();
                }
                out!(self, "while");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(self, Some(node));
                out!(self, ")");
            }

            ForInStatement { left, right, body } => {
                out!(self, "for(");
                left.visit(self, Some(node));
                out!(self, " in ");
                right.visit(self, Some(node));
                out!(self, ")");
                self.visit_stmt_or_block(body, ForceBlock::No, node);
            }
            ForOfStatement {
                left,
                right,
                body,
                is_await,
            } => {
                out!(self, "for{}(", if *is_await { " await" } else { "" });
                left.visit(self, Some(node));
                out!(self, " of ");
                right.visit(self, Some(node));
                out!(self, ")");
                self.visit_stmt_or_block(body, ForceBlock::No, node);
            }
            ForStatement {
                init,
                test,
                update,
                body,
            } => {
                out!(self, "for(");
                self.print_child(init.as_deref(), node, ChildPos::Left);
                out!(self, ";");
                if let Some(test) = test {
                    self.space(ForceSpace::No);
                    test.visit(self, Some(node));
                }
                out!(self, ";");
                if let Some(update) = update {
                    self.space(ForceSpace::No);
                    update.visit(self, Some(node));
                }
                out!(self, ")");
                self.visit_stmt_or_block(body, ForceBlock::No, node);
            }

            DebuggerStatement => {
                out!(self, "debugger");
            }
            EmptyStatement => {}

            BlockStatement { body } => {
                if body.is_empty() {
                    out!(self, "{{}}");
                } else {
                    out!(self, "{{");
                    self.inc_indent();
                    self.newline();
                    self.visit_stmt_list(body, node);
                    self.dec_indent();
                    self.newline();
                    out!(self, "}}");
                }
            }

            BreakStatement { label } => {
                out!(self, "break");
                if let Some(label) = label {
                    self.space(ForceSpace::Yes);
                    label.visit(self, Some(node));
                }
            }
            ContinueStatement { label } => {
                out!(self, "continue");
                if let Some(label) = label {
                    self.space(ForceSpace::Yes);
                    label.visit(self, Some(node));
                }
            }

            ThrowStatement { argument } => {
                out!(self, "throw ");
                argument.visit(self, Some(node));
            }
            ReturnStatement { argument } => {
                out!(self, "return");
                if let Some(argument) = argument {
                    out!(self, " ");
                    argument.visit(self, Some(node));
                }
            }
            WithStatement { object, body } => {
                out!(self, "with");
                self.space(ForceSpace::No);
                out!(self, "(");
                object.visit(self, Some(node));
                out!(self, ")");
                self.visit_stmt_or_block(body, ForceBlock::No, node);
            }

            SwitchStatement {
                discriminant,
                cases,
            } => {
                out!(self, "switch");
                self.space(ForceSpace::No);
                out!(self, "(");
                discriminant.visit(self, Some(node));
                out!(self, ")");
                self.space(ForceSpace::No);
                out!(self, "{{");
                self.newline();
                for case in cases {
                    case.visit(self, Some(node));
                    self.newline();
                }
                out!(self, "}}");
            }
            SwitchCase { test, consequent } => {
                match test {
                    Some(test) => {
                        out!(self, "case ");
                        test.visit(self, Some(node));
                    }
                    None => {
                        out!(self, "default");
                    }
                };
                out!(self, ":");
                if !consequent.is_empty() {
                    self.inc_indent();
                    self.newline();
                    self.visit_stmt_list(consequent, node);
                    self.dec_indent();
                }
            }

            LabeledStatement { label, body } => {
                label.visit(self, Some(node));
                out!(self, ":");
                self.newline();
                body.visit(self, Some(node));
            }

            ExpressionStatement { expression, .. } => {
                self.print_child(Some(expression), node, ChildPos::Anywhere);
            }

            TryStatement {
                block,
                handler,
                finalizer,
            } => {
                out!(self, "try");
                self.visit_stmt_or_block(block, ForceBlock::Yes, node);
                if let Some(handler) = handler {
                    handler.visit(self, Some(node));
                }
                if let Some(finalizer) = finalizer {
                    self.visit_stmt_or_block(finalizer, ForceBlock::Yes, node);
                }
            }

            IfStatement {
                test,
                consequent,
                alternate,
            } => {
                out!(self, "if");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(self, Some(node));
                out!(self, ")");
                let force_block = if alternate.is_some() && is_if_without_else(consequent) {
                    ForceBlock::Yes
                } else {
                    ForceBlock::No
                };
                let block = self.visit_stmt_or_block(consequent, force_block, node);
                if let Some(alternate) = alternate {
                    if !block {
                        out!(self, ";");
                        self.newline();
                    } else {
                        self.space(ForceSpace::No);
                    }
                    out!(self, "else");
                    self.visit_stmt_or_block(alternate, ForceBlock::No, node);
                }
            }

            BooleanLiteral { value, .. } => {
                out!(self, "{}", if *value { "true" } else { "false" });
            }
            NullLiteral => {
                out!(self, "null");
            }
            StringLiteral { value, .. } => {
                out!(self, "\"");
                self.print_escaped_string_literal(value, '"');
                out!(self, "\"");
            }
            NumericLiteral { value } => {
                out!(self, "{}", convert::number_to_string(*value));
            }
            RegExpLiteral { pattern, flags } => {
                // FIXME: Needs to escape '/', might need more escaping as well.
                out!(self, "/{}/{}", pattern.str, flags.str);
            }
            ThisExpression => {
                out!(self, "this");
            }
            Super => {
                out!(self, "super");
            }

            SequenceExpression { expressions } => {
                for (i, expr) in expressions.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_child(
                        Some(expr),
                        node,
                        if i == 1 {
                            ChildPos::Left
                        } else {
                            ChildPos::Right
                        },
                    );
                }
            }

            ObjectExpression { properties, .. } => {
                self.visit_props(properties, node);
            }
            ArrayExpression {
                elements,
                trailing_comma,
            } => {
                out!(self, "[");
                for (i, elem) in elements.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_comma_expression(elem, node);
                }
                if *trailing_comma {
                    self.comma();
                }
                out!(self, "]");
            }

            SpreadElement { argument } => {
                out!(self, "...");
                argument.visit(self, Some(node));
            }

            NewExpression {
                callee, arguments, ..
            } => {
                out!(self, "new ");
                self.print_child(Some(callee), node, ChildPos::Left);
                out!(self, "(");
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_comma_expression(arg, node);
                }
                out!(self, ")");
            }
            YieldExpression { argument, delegate } => {
                out!(self, "yield");
                if *delegate {
                    out!(self, "*");
                    self.space(ForceSpace::No);
                } else if argument.is_some() {
                    out!(self, " ");
                }
                if let Some(argument) = argument {
                    argument.visit(self, Some(node));
                }
            }
            AwaitExpression { argument } => {
                out!(self, "await ");
                argument.visit(self, Some(node));
            }

            ImportExpression { source, attributes } => {
                out!(self, "import(");
                source.visit(self, Some(node));
                if let Some(attributes) = attributes {
                    out!(self, ",");
                    self.space(ForceSpace::No);
                    attributes.visit(self, Some(node));
                }
                out!(self, ")");
            }

            CallExpression {
                callee, arguments, ..
            } => {
                self.print_child(Some(callee), node, ChildPos::Left);
                out!(self, "(");
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_comma_expression(arg, node);
                }
                out!(self, ")");
            }
            OptionalCallExpression {
                callee,
                arguments,
                optional,
                ..
            } => {
                self.print_child(Some(callee), node, ChildPos::Left);
                out!(self, "{}(", if *optional { "?." } else { "" });
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_comma_expression(arg, node);
                }
                out!(self, ")");
            }

            AssignmentExpression {
                operator,
                left,
                right,
            } => {
                self.print_child(Some(left), node, ChildPos::Left);
                self.space(ForceSpace::No);
                out!(self, "{}", operator.str);
                self.space(ForceSpace::No);
                self.print_child(Some(right), node, ChildPos::Right);
            }
            UnaryExpression {
                operator,
                argument,
                prefix,
            } => {
                let ident = operator.str.chars().next().unwrap().is_alphabetic();
                if *prefix {
                    out!(self, "{}", operator.str);
                    if ident {
                        out!(self, " ");
                    }
                    self.print_child(Some(argument), node, ChildPos::Right);
                } else {
                    self.print_child(Some(argument), node, ChildPos::Left);
                    if ident {
                        out!(self, " ");
                    }
                    out!(self, "{}", operator.str);
                }
            }
            UpdateExpression {
                operator,
                argument,
                prefix,
            } => {
                if *prefix {
                    out!(self, "{}", operator.str);
                    self.print_child(Some(argument), node, ChildPos::Right);
                } else {
                    self.print_child(Some(argument), node, ChildPos::Left);
                    out!(self, "{}", operator.str);
                }
            }
            MemberExpression {
                object,
                property,
                computed,
            } => {
                object.visit(self, Some(node));
                if *computed {
                    out!(self, "[");
                } else {
                    out!(self, ".");
                }
                property.visit(self, Some(node));
                if *computed {
                    out!(self, "]");
                }
            }
            OptionalMemberExpression {
                object,
                property,
                computed,
                optional,
            } => {
                object.visit(self, Some(node));
                if *computed {
                    out!(self, "{}[", if *optional { "?." } else { "" });
                } else {
                    out!(self, "{}.", if *optional { "?" } else { "" });
                }
                property.visit(self, Some(node));
                if *computed {
                    out!(self, "]");
                }
            }

            BinaryExpression {
                left,
                right,
                operator,
            } => {
                let ident = operator.str.chars().next().unwrap().is_alphabetic();
                self.print_child(Some(left), node, ChildPos::Left);
                self.space(if ident {
                    ForceSpace::Yes
                } else {
                    ForceSpace::No
                });
                out!(self, "{}", operator.str);
                self.space(if ident {
                    ForceSpace::Yes
                } else {
                    ForceSpace::No
                });
                self.print_child(Some(right), node, ChildPos::Right);
            }

            Directive { value } => {
                value.visit(self, Some(node));
            }
            DirectiveLiteral { .. } => {
                unimplemented!("No escaping for directive literals");
            }

            ConditionalExpression {
                test,
                consequent,
                alternate,
            } => {
                self.print_child(Some(test), node, ChildPos::Left);
                self.space(ForceSpace::No);
                out!(self, "?");
                self.space(ForceSpace::No);
                self.print_child(Some(consequent), node, ChildPos::Anywhere);
                self.space(ForceSpace::No);
                out!(self, ":");
                self.space(ForceSpace::No);
                self.print_child(Some(alternate), node, ChildPos::Right);
            }

            Identifier { name, .. } => {
                out!(self, "{}", &name.str);
            }
            PrivateName { id } => {
                out!(self, "#");
                id.visit(self, Some(node));
            }
            MetaProperty { meta, property } => {
                meta.visit(self, Some(node));
                out!(self, ".");
                property.visit(self, Some(node));
            }

            CatchClause { param, body } => {
                self.space(ForceSpace::No);
                out!(self, "catch");
                if let Some(param) = param {
                    self.space(ForceSpace::No);
                    out!(self, "(");
                    param.visit(self, Some(node));
                    out!(self, ")");
                }
                self.visit_stmt_or_block(body, ForceBlock::Yes, node);
            }

            VariableDeclaration { kind, declarations } => {
                out!(self, "{}", kind.str);
                for (i, decl) in declarations.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    decl.visit(self, Some(node));
                }
            }
            VariableDeclarator { init, id } => {
                id.visit(self, Some(node));
                if let Some(init) = init {
                    out!(
                        self,
                        "{}",
                        match self.pretty {
                            Pretty::Yes => " = ",
                            Pretty::No => "=",
                        }
                    );
                    init.visit(self, Some(node));
                }
            }

            TemplateLiteral {
                quasis: _,
                expressions: _,
            } => {
                unimplemented!("TemplateLiteral");
                // out!(self, "`");
                // let mut it_expr = expressions.iter();
                // for quasi in quasis {
                //     if let TemplateElement { raw, .. } = &quasi.kind {
                //         out!(self, "{}", raw.str);
                //         if let Some(expr) = it_expr.next() {
                //             out!(self, "${{");
                //             expr.visit(self, Some(node));
                //             out!(self, "}}");
                //         }
                //     }
                // }
                // out!(self, "`");
            }
            TaggedTemplateExpression { tag, quasi } => {
                self.print_child(Some(tag), node, ChildPos::Left);
                self.print_child(Some(quasi), node, ChildPos::Right);
            }
            TemplateElement { .. } => {
                unreachable!("TemplateElement is handled in TemplateLiteral case");
            }

            Property {
                key,
                value,
                kind,
                computed,
                method,
                shorthand,
            } => {
                let mut need_sep = false;
                if kind.str != "init" {
                    out!(self, "{}", kind.str);
                    need_sep = true;
                } else if *method {
                    match &value.kind {
                        FunctionExpression {
                            generator,
                            is_async,
                            ..
                        } => {
                            if *is_async {
                                out!(self, "async");
                                need_sep = true;
                            }
                            if *generator {
                                out!(self, "*");
                                need_sep = false;
                                self.space(ForceSpace::No);
                            }
                        }
                        _ => unreachable!(),
                    };
                }
                if *computed {
                    if need_sep {
                        self.space(ForceSpace::No);
                    }
                    need_sep = false;
                    out!(self, "[");
                }
                if need_sep {
                    out!(self, " ");
                }
                key.visit(self, None);
                if *computed {
                    out!(self, "]");
                }
                if *shorthand {
                    return ();
                }
                if kind.str != "init" || *method {
                    match &value.kind {
                        FunctionExpression { params, body, .. } => {
                            self.visit_func_params_body(params, body, value);
                        }
                        _ => unreachable!(),
                    };
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    self.print_comma_expression(value, node);
                }
            }

            LogicalExpression {
                left,
                right,
                operator,
            } => {
                self.print_child(Some(left), node, ChildPos::Left);
                self.space(ForceSpace::No);
                out!(self, "{}", operator.str);
                self.space(ForceSpace::No);
                self.print_child(Some(right), node, ChildPos::Right);
            }

            ClassExpression {
                id,
                super_class,
                body,
                ..
            }
            | ClassDeclaration {
                id,
                super_class,
                body,
                ..
            } => {
                out!(self, "class");
                if let Some(id) = id {
                    self.space(ForceSpace::Yes);
                    id.visit(self, Some(node));
                }
                if let Some(super_class) = super_class {
                    out!(self, " extends ");
                    super_class.visit(self, Some(node));
                }
                self.space(ForceSpace::No);
                body.visit(self, Some(node));
            }

            ClassBody { body } => {
                if body.is_empty() {
                    out!(self, "{{}}");
                } else {
                    out!(self, "{{");
                    self.inc_indent();
                    self.newline();
                    for prop in body {
                        prop.visit(self, Some(node));
                        self.newline();
                    }
                    out!(self, "}}");
                    self.dec_indent();
                    self.newline();
                }
            }
            ClassProperty {
                key,
                value,
                computed,
                is_static,
                ..
            } => {
                if *is_static {
                    out!(self, "static ");
                }
                if *computed {
                    out!(self, "[");
                }
                key.visit(self, Some(node));
                if *computed {
                    out!(self, "]");
                }
                self.space(ForceSpace::No);
                if let Some(value) = value {
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    value.visit(self, Some(node));
                }
                out!(self, ";");
            }
            ClassPrivateProperty {
                key,
                value,
                is_static,
                ..
            } => {
                if *is_static {
                    out!(self, "static ");
                }
                out!(self, "#");
                key.visit(self, Some(node));
                self.space(ForceSpace::No);
                if let Some(value) = value {
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    value.visit(self, Some(node));
                }
                out!(self, ";");
            }
            MethodDefinition {
                key,
                value,
                kind,
                computed,
                is_static,
            } => {
                let (is_async, generator, params, body) = match &value.kind {
                    FunctionExpression {
                        generator,
                        is_async,
                        params,
                        body,
                        ..
                    } => (*is_async, *generator, params, body),
                    _ => {
                        unreachable!("Invalid method value");
                    }
                };
                if *is_static {
                    out!(self, "static ");
                }
                if is_async {
                    out!(self, "async ");
                }
                if generator {
                    out!(self, "*");
                }
                match kind.str.as_ref() {
                    "method" => {}
                    "constructor" => {
                        // Will be handled by key output.
                    }
                    "get" => {
                        out!(self, "get ");
                    }
                    "set" => {
                        out!(self, "set ");
                    }
                    _ => {
                        unreachable!("Invalid method kind");
                    }
                };
                if *computed {
                    out!(self, "[");
                }
                key.visit(self, Some(node));
                if *computed {
                    out!(self, "]");
                }
                self.visit_func_params_body(params, body, node);
            }

            ImportDeclaration {
                specifiers, source, ..
            } => {
                out!(self, "import ");
                let mut has_named_specs = false;
                for (i, spec) in specifiers.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    match &spec.kind {
                        ImportSpecifier { .. } => {
                            if !has_named_specs {
                                has_named_specs = true;
                                out!(self, "{{");
                            }
                        }
                        _ => {}
                    };
                    spec.visit(self, Some(node));
                }
                if !specifiers.is_empty() {
                    if has_named_specs {
                        out!(self, "}}");
                        self.space(ForceSpace::No);
                    } else {
                        out!(self, " ");
                    }
                    out!(self, "from ");
                }
                source.visit(self, Some(node));
                out!(self, ";");
                self.newline();
            }
            ImportSpecifier {
                imported, local, ..
            } => {
                imported.visit(self, Some(node));
                out!(self, " as ");
                local.visit(self, Some(node));
            }
            ImportDefaultSpecifier { local } => {
                local.visit(self, Some(node));
            }
            ImportNamespaceSpecifier { local } => {
                out!(self, "* as ");
                local.visit(self, Some(node));
            }

            ExportNamedDeclaration {
                declaration,
                specifiers,
                source,
                ..
            } => {
                out!(self, "export ");
                if let Some(declaration) = declaration {
                    declaration.visit(self, Some(node));
                } else {
                    out!(self, "{{");
                    for (i, spec) in specifiers.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        spec.visit(self, Some(node));
                    }
                    out!(self, "}}");
                    if let Some(source) = source {
                        out!(self, " from ");
                        source.visit(self, Some(node));
                    }
                }
                out!(self, ";");
                self.newline();
            }
            ExportSpecifier { exported, local } => {
                local.visit(self, Some(node));
                out!(self, " as ");
                exported.visit(self, Some(node));
            }
            ExportNamespaceSpecifier { exported } => {
                out!(self, "* as ");
                exported.visit(self, Some(node));
            }
            ExportDefaultDeclaration { declaration } => {
                out!(self, "export default ");
                declaration.visit(self, Some(node));
                out!(self, ";");
                self.newline();
            }

            ObjectPattern { properties, .. } => {
                self.visit_props(properties, node);
            }
            ArrayPattern { elements, .. } => {
                out!(self, "[");
                for (i, elem) in elements.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    elem.visit(self, Some(node));
                }
                out!(self, "]");
            }
            RestElement { argument } => {
                out!(self, "...");
                argument.visit(self, Some(node));
            }
            AssignmentPattern { left, right } => {
                left.visit(self, Some(node));
                self.space(ForceSpace::No);
                out!(self, "=");
                self.space(ForceSpace::No);
                right.visit(self, Some(node));
            }

            JSXIdentifier { name } => {
                out!(self, "{}", name.str);
            }
            JSXMemberExpression { object, property } => {
                object.visit(self, Some(node));
                out!(self, ".");
                property.visit(self, Some(node));
            }
            JSXNamespacedName { namespace, name } => {
                namespace.visit(self, Some(node));
                out!(self, ":");
                name.visit(self, Some(node));
            }
            JSXEmptyExpression => {}
            JSXExpressionContainer { expression } => {
                out!(self, "{{");
                expression.visit(self, Some(node));
                out!(self, "}}");
            }
            JSXSpreadChild { expression } => {
                out!(self, "{{...");
                expression.visit(self, Some(node));
                out!(self, "}}");
            }
            JSXOpeningElement {
                name,
                attributes,
                self_closing,
            } => {
                out!(self, "<");
                name.visit(self, Some(node));
                for attr in attributes {
                    self.space(ForceSpace::Yes);
                    attr.visit(self, Some(node));
                }
                if *self_closing {
                    out!(self, " />");
                } else {
                    out!(self, ">");
                }
            }
            JSXClosingElement { name } => {
                out!(self, "</");
                name.visit(self, Some(node));
                out!(self, ">");
            }
            JSXAttribute { name, value } => {
                name.visit(self, Some(node));
                if let Some(value) = value {
                    out!(self, "=");
                    value.visit(self, Some(node));
                }
            }
            JSXSpreadAttribute { argument } => {
                out!(self, "{{...");
                argument.visit(self, Some(node));
                out!(self, "}}");
            }
            JSXText { value: _, .. } => {
                unimplemented!("JSXText");
                // FIXME: Ensure escaping here works properly.
                // out!(self, "{}", value.str);
            }
            JSXElement {
                opening_element,
                children,
                closing_element,
            } => {
                opening_element.visit(self, Some(node));
                if let Some(closing_element) = closing_element {
                    self.inc_indent();
                    self.newline();
                    for child in children {
                        child.visit(self, Some(node));
                        self.newline();
                    }
                    self.dec_indent();
                    closing_element.visit(self, Some(node));
                }
            }
            JSXFragment {
                opening_fragment,
                children,
                closing_fragment,
            } => {
                opening_fragment.visit(self, Some(node));
                self.inc_indent();
                self.newline();
                for child in children {
                    child.visit(self, Some(node));
                    self.newline();
                }
                self.dec_indent();
                closing_fragment.visit(self, Some(node));
            }
            JSXOpeningFragment => {
                out!(self, "<>");
            }
            JSXClosingFragment => {
                out!(self, "</>");
            }

            ExistsTypeAnnotation => {
                out!(self, "*");
            }
            EmptyTypeAnnotation => {
                out!(self, "empty");
            }
            StringTypeAnnotation => {
                out!(self, "string");
            }
            NumberTypeAnnotation => {
                out!(self, "number");
            }
            StringLiteralTypeAnnotation { value } => {
                out!(self, "\"");
                self.print_escaped_string_literal(value, '"');
                out!(self, "\"");
            }
            NumberLiteralTypeAnnotation { value, .. } => {
                out!(self, "{}", convert::number_to_string(*value));
            }
            BooleanTypeAnnotation => {
                out!(self, "boolean");
            }
            BooleanLiteralTypeAnnotation { value, .. } => {
                out!(self, "{}", if *value { "true" } else { "false" });
            }
            NullLiteralTypeAnnotation => {
                out!(self, "null");
            }
            SymbolTypeAnnotation => {
                out!(self, "symbol");
            }
            AnyTypeAnnotation => {
                out!(self, "any");
            }
            MixedTypeAnnotation => {
                out!(self, "mixed");
            }
            VoidTypeAnnotation => {
                out!(self, "void");
            }
            FunctionTypeAnnotation {
                params,
                this,
                return_type,
                rest,
                type_parameters,
            } => {
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
                let need_parens = type_parameters.is_some() || rest.is_some() || params.len() != 1;
                if need_parens {
                    out!(self, "(");
                }
                let mut need_comma = false;
                if let Some(this) = this {
                    match &this.kind {
                        FunctionTypeParam {
                            type_annotation, ..
                        } => {
                            out!(self, "this:");
                            self.space(ForceSpace::No);
                            type_annotation.visit(self, Some(node));
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                    this.visit(self, Some(node));
                    need_comma = true;
                }
                for param in params.iter() {
                    if need_comma {
                        self.comma();
                    }
                    param.visit(self, Some(node));
                    need_comma = true;
                }
                if let Some(rest) = rest {
                    if need_comma {
                        self.comma();
                    }
                    out!(self, "...");
                    rest.visit(self, Some(node));
                }
                if need_parens {
                    out!(self, ")");
                }
                if self.pretty == Pretty::Yes {
                    out!(self, " => ");
                } else {
                    out!(self, "=>");
                }
                return_type.visit(self, Some(node));
            }
            FunctionTypeParam {
                name,
                type_annotation,
                optional,
            } => {
                if let Some(name) = name {
                    name.visit(self, Some(node));
                    if *optional {
                        out!(self, "?");
                    }
                    out!(self, ":");
                    self.space(ForceSpace::No);
                }
                type_annotation.visit(self, Some(node));
            }
            NullableTypeAnnotation { type_annotation } => {
                out!(self, "?");
                type_annotation.visit(self, Some(node));
            }
            QualifiedTypeIdentifier { qualification, id } => {
                qualification.visit(self, Some(node));
                out!(self, ".");
                id.visit(self, Some(node));
            }
            TypeofTypeAnnotation { argument } => {
                out!(self, "typeof ");
                argument.visit(self, Some(node));
            }
            TupleTypeAnnotation { types } => {
                out!(self, "[");
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    ty.visit(self, Some(node));
                }
                out!(self, "]");
            }
            ArrayTypeAnnotation { element_type } => {
                element_type.visit(self, Some(node));
                out!(self, "[]");
            }
            UnionTypeAnnotation { types } => {
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.space(ForceSpace::No);
                        out!(self, "|");
                        self.space(ForceSpace::No);
                    }
                    self.print_child(Some(ty), node, ChildPos::Anywhere);
                }
            }
            IntersectionTypeAnnotation { types } => {
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.space(ForceSpace::No);
                        out!(self, "&");
                        self.space(ForceSpace::No);
                    }
                    self.print_child(Some(ty), node, ChildPos::Anywhere);
                }
            }
            GenericTypeAnnotation {
                id,
                type_parameters,
            } => {
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
            }
            IndexedAccessType {
                object_type,
                index_type,
            } => {
                object_type.visit(self, Some(node));
                out!(self, "[");
                index_type.visit(self, Some(node));
                out!(self, "]");
            }
            OptionalIndexedAccessType {
                object_type,
                index_type,
                optional,
            } => {
                object_type.visit(self, Some(node));
                out!(self, "{}[", if *optional { "?." } else { "" });
                index_type.visit(self, Some(node));
                out!(self, "]");
            }
            InterfaceTypeAnnotation { extends, body } => {
                out!(self, "interface");
                if !extends.is_empty() {
                    out!(self, " extends ");
                    for (i, extend) in extends.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        extend.visit(self, Some(node));
                    }
                } else {
                    self.space(ForceSpace::No);
                }
                if let Some(body) = body {
                    body.visit(self, Some(node));
                }
            }

            TypeAlias {
                id,
                type_parameters,
                right,
            }
            | DeclareTypeAlias {
                id,
                type_parameters,
                right,
            } => {
                if matches!(&node.kind, DeclareTypeAlias { .. }) {
                    out!(self, "declare ");
                }
                out!(self, "type ");
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
                if self.pretty == Pretty::Yes {
                    out!(self, " = ");
                } else {
                    out!(self, "=");
                }
                right.visit(self, Some(node));
            }
            OpaqueType {
                id,
                type_parameters,
                impltype,
                supertype,
            } => {
                out!(self, "opaque type ");
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
                if let Some(supertype) = supertype {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    supertype.visit(self, Some(node));
                }
                if self.pretty == Pretty::Yes {
                    out!(self, " = ");
                } else {
                    out!(self, "=");
                }
                impltype.visit(self, Some(node));
            }
            InterfaceDeclaration {
                id,
                type_parameters,
                extends,
                body,
            }
            | DeclareInterface {
                id,
                type_parameters,
                extends,
                body,
            } => {
                self.visit_interface(
                    if matches!(node.kind, InterfaceDeclaration { .. }) {
                        "interface"
                    } else {
                        "declare interface"
                    },
                    id,
                    type_parameters.as_deref(),
                    extends,
                    body,
                    node,
                );
            }
            DeclareOpaqueType {
                id,
                type_parameters,
                impltype,
                supertype,
            } => {
                out!(self, "opaque type ");
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
                if let Some(supertype) = supertype {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    supertype.visit(self, Some(node));
                }
                if self.pretty == Pretty::Yes {
                    out!(self, " = ");
                } else {
                    out!(self, "=");
                }
                if let Some(impltype) = impltype {
                    impltype.visit(self, Some(node));
                }
            }
            DeclareClass {
                id,
                type_parameters,
                extends,
                implements,
                mixins,
                body,
            } => {
                out!(self, "declare class ");
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
                if !extends.is_empty() {
                    out!(self, " extends ");
                    for (i, extend) in extends.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        extend.visit(self, Some(node));
                    }
                }
                if !mixins.is_empty() {
                    out!(self, " mixins ");
                    for (i, mixin) in mixins.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        mixin.visit(self, Some(node));
                    }
                }
                if !implements.is_empty() {
                    out!(self, " implements ");
                    for (i, implement) in implements.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        implement.visit(self, Some(node));
                    }
                }
                self.space(ForceSpace::No);
                body.visit(self, Some(node));
            }
            DeclareFunction { id, predicate } => {
                // This AST type uses the Identifier/TypeAnnotation
                // pairing to put a name on a function header-looking construct,
                // so we have to do some deep matching to get it to come out right.
                out!(self, "declare function ");
                match &id.kind {
                    Identifier {
                        name,
                        type_annotation,
                        ..
                    } => {
                        out!(self, "{}", &name.str);
                        match type_annotation {
                            None => {
                                unimplemented!("Malformed AST: Need to handle error");
                            }
                            Some(type_annotation) => match &type_annotation.kind {
                                TypeAnnotation { type_annotation } => match &type_annotation.kind {
                                    FunctionTypeAnnotation {
                                        params,
                                        this,
                                        return_type,
                                        rest,
                                        type_parameters,
                                    } => {
                                        self.visit_func_type_params(
                                            params,
                                            this.as_deref(),
                                            rest.as_deref(),
                                            type_parameters.as_deref(),
                                            node,
                                        );
                                        out!(self, ":");
                                        self.space(ForceSpace::No);
                                        return_type.visit(self, Some(node));
                                    }
                                    _ => {
                                        unimplemented!("Malformed AST: Need to handle error");
                                    }
                                },
                                _ => {
                                    unimplemented!("Malformed AST: Need to handle error");
                                }
                            },
                        }
                        if let Some(predicate) = predicate {
                            self.space(ForceSpace::No);
                            predicate.visit(self, Some(node));
                        }
                    }
                    _ => {
                        unimplemented!("Malformed AST: Need to handle error");
                    }
                }
            }
            DeclareVariable { id } => {
                if !matches!(
                    &parent,
                    Some(Node {
                        kind: DeclareExportDeclaration { .. },
                        ..
                    })
                ) {
                    out!(self, "declare ");
                }
                id.visit(self, Some(node));
            }
            DeclareExportDeclaration {
                declaration,
                specifiers,
                source,
                default,
            } => {
                out!(self, "declare export ");
                if *default {
                    out!(self, "default ");
                }
                if let Some(declaration) = declaration {
                    declaration.visit(self, Some(node));
                } else {
                    out!(self, "{{");
                    for (i, spec) in specifiers.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        spec.visit(self, Some(node));
                    }
                    out!(self, "}}");
                    if let Some(source) = source {
                        out!(self, " from ");
                        source.visit(self, Some(node));
                    }
                }
            }
            DeclareExportAllDeclaration { source } => {
                out!(self, "declare export * from ");
                source.visit(self, Some(node));
            }
            DeclareModule { id, body, .. } => {
                out!(self, "declare module ");
                id.visit(self, Some(node));
                self.space(ForceSpace::No);
                body.visit(self, Some(node));
            }
            DeclareModuleExports { type_annotation } => {
                out!(self, "declare module.exports:");
                self.space(ForceSpace::No);
                type_annotation.visit(self, Some(node));
            }

            InterfaceExtends {
                id,
                type_parameters,
            }
            | ClassImplements {
                id,
                type_parameters,
            } => {
                id.visit(self, Some(node));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(self, Some(node));
                }
            }

            TypeAnnotation { type_annotation } => {
                type_annotation.visit(self, Some(node));
            }
            ObjectTypeAnnotation {
                properties,
                indexers,
                call_properties,
                internal_slots,
                inexact,
                exact,
            } => {
                out!(self, "{}", if *exact { "{|" } else { "{" });
                self.inc_indent();
                self.newline();

                let mut need_comma = false;

                for prop in properties {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(self, Some(node));
                    self.newline();
                    need_comma = true;
                }
                for prop in indexers {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(self, Some(node));
                    self.newline();
                    need_comma = true;
                }
                for prop in call_properties {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(self, Some(node));
                    self.newline();
                    need_comma = true;
                }
                for prop in internal_slots {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(self, Some(node));
                    self.newline();
                    need_comma = true;
                }

                if *inexact {
                    if need_comma {
                        self.comma();
                    }
                    out!(self, "...");
                }

                self.dec_indent();
                self.newline();
                out!(self, "{}", if *exact { "|}" } else { "}" });
            }
            ObjectTypeProperty {
                key,
                value,
                method,
                optional,
                is_static,
                proto,
                variance,
                ..
            } => {
                if let Some(variance) = variance {
                    variance.visit(self, Some(node));
                }
                if *is_static {
                    out!(self, "static ");
                }
                if *proto {
                    out!(self, "proto ");
                }
                key.visit(self, Some(node));
                if *optional {
                    out!(self, "?");
                }
                if *method {
                    match &value.kind {
                        FunctionTypeAnnotation {
                            params,
                            this,
                            return_type,
                            rest,
                            type_parameters,
                        } => {
                            self.visit_func_type_params(
                                params,
                                this.as_deref(),
                                rest.as_deref(),
                                type_parameters.as_deref(),
                                node,
                            );
                            out!(self, ":");
                            self.space(ForceSpace::No);
                            return_type.visit(self, Some(node));
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    value.visit(self, Some(node));
                }
            }
            ObjectTypeSpreadProperty { argument } => {
                out!(self, "...");
                argument.visit(self, Some(node));
            }
            ObjectTypeInternalSlot {
                id,
                value,
                optional,
                is_static,
                method,
            } => {
                if *is_static {
                    out!(self, "static ");
                }
                out!(self, "[[");
                id.visit(self, Some(node));
                if *optional {
                    out!(self, "?");
                }
                out!(self, "]]");
                if *method {
                    match &value.kind {
                        FunctionTypeAnnotation {
                            params,
                            this,
                            return_type,
                            rest,
                            type_parameters,
                        } => {
                            self.visit_func_type_params(
                                params,
                                this.as_deref(),
                                rest.as_deref(),
                                type_parameters.as_deref(),
                                node,
                            );
                            out!(self, ":");
                            self.space(ForceSpace::No);
                            return_type.visit(self, Some(node));
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    value.visit(self, Some(node));
                }
            }
            ObjectTypeCallProperty { value, is_static } => {
                if *is_static {
                    out!(self, "static ");
                }
                match &value.kind {
                    FunctionTypeAnnotation {
                        params,
                        this,
                        return_type,
                        rest,
                        type_parameters,
                    } => {
                        self.visit_func_type_params(
                            params,
                            this.as_deref(),
                            rest.as_deref(),
                            type_parameters.as_deref(),
                            node,
                        );
                        out!(self, ":");
                        self.space(ForceSpace::No);
                        return_type.visit(self, Some(node));
                    }
                    _ => {
                        unimplemented!("Malformed AST: Need to handle error");
                    }
                }
            }
            ObjectTypeIndexer {
                id,
                key,
                value,
                is_static,
                variance,
            } => {
                if *is_static {
                    out!(self, "static ");
                }
                if let Some(variance) = variance {
                    variance.visit(self, Some(node));
                }
                out!(self, "[");
                if let Some(id) = id {
                    id.visit(self, Some(node));
                    out!(self, ":");
                    self.space(ForceSpace::No);
                }
                key.visit(self, Some(node));
                out!(self, "]");
                out!(self, ":");
                self.space(ForceSpace::No);
                value.visit(self, Some(node));
            }
            Variance { kind } => {
                out!(
                    self,
                    "{}",
                    match kind.str.as_str() {
                        "plus" => "+",
                        "minus" => "-",
                        _ => unimplemented!("Malformed variance"),
                    }
                )
            }

            TypeParameterDeclaration { params } | TypeParameterInstantiation { params } => {
                out!(self, "<");
                for (i, param) in params.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    param.visit(self, Some(node));
                }
                out!(self, ">");
            }
            TypeParameter {
                name,
                bound,
                variance,
                default,
            } => {
                if let Some(variance) = variance {
                    variance.visit(self, Some(node));
                }
                out!(self, "{}", &name.str);
                if let Some(bound) = bound {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    bound.visit(self, Some(node));
                }
                if let Some(default) = default {
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    default.visit(self, Some(node));
                }
            }
            TypeCastExpression {
                expression,
                type_annotation,
            } => {
                // Type casts are required to have parentheses.
                out!(self, "(");
                self.print_child(Some(expression), node, ChildPos::Left);
                out!(self, ":");
                self.space(ForceSpace::No);
                self.print_child(Some(type_annotation), node, ChildPos::Right);
            }
            InferredPredicate => {
                out!(self, "%checks");
            }
            DeclaredPredicate { value } => {
                out!(self, "%checks(");
                value.visit(self, Some(node));
                out!(self, ")");
            }

            EnumDeclaration { id, body } => {
                out!(self, "enum ");
                id.visit(self, Some(node));
                body.visit(self, Some(node));
            }
            EnumStringBody {
                members,
                explicit_type,
                has_unknown_members,
            } => {
                self.visit_enum_body(
                    "string",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            EnumNumberBody {
                members,
                explicit_type,
                has_unknown_members,
            } => {
                self.visit_enum_body(
                    "number",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            EnumBooleanBody {
                members,
                explicit_type,
                has_unknown_members,
            } => {
                self.visit_enum_body(
                    "boolean",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            EnumSymbolBody {
                members,
                has_unknown_members,
            } => {
                self.visit_enum_body("symbol", members, true, *has_unknown_members, node);
            }
            EnumDefaultedMember { id } => {
                id.visit(self, Some(node));
            }
            EnumStringMember { id, init }
            | EnumNumberMember { id, init }
            | EnumBooleanMember { id, init } => {
                id.visit(self, Some(node));
                out!(
                    self,
                    "{}",
                    match self.pretty {
                        Pretty::Yes => " = ",
                        Pretty::No => "=",
                    }
                );
                init.visit(self, Some(node));
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

    /// Print one expression in a sequence separated by comma. It needs parens
    /// if its precedence is <= comma.
    fn print_comma_expression(&mut self, child: &Node, parent: &Node) {
        self.print_parens(
            child,
            parent,
            NeedParens::from(self.get_precedence(child).0 <= precedence::SEQ),
        )
    }

    fn print_parens(&mut self, child: &Node, parent: &Node, need_parens: NeedParens) {
        if need_parens == NeedParens::Yes {
            out!(self, "(");
        } else if need_parens == NeedParens::Space {
            out!(self, " ");
        }
        child.visit(self, Some(parent));
        if need_parens == NeedParens::Yes {
            out!(self, ")");
        }
    }

    fn print_escaped_string_literal(&mut self, value: &StringLiteral, esc: char) {
        for &c in &value.str {
            let c8 = char::from(c as u8);
            match c8 {
                '\\' => {
                    out!(self, "\\\\");
                    continue;
                }
                '\x08' => {
                    out!(self, "\\b");
                    continue;
                }
                '\x0c' => {
                    out!(self, "\\f");
                    continue;
                }
                '\n' => {
                    out!(self, "\\n");
                    continue;
                }
                '\r' => {
                    out!(self, "\\r");
                    continue;
                }
                '\t' => {
                    out!(self, "\\t");
                    continue;
                }
                '\x0b' => {
                    out!(self, "\\v");
                    continue;
                }
                _ => {}
            };
            if c == esc as u16 {
                out!(self, "\\");
            }
            if (0x20..=0x7f).contains(&c) {
                // Printable.
                out!(self, "{}", c8);
            } else {
                out!(self, "\\u{:04x}", c);
            }
        }
    }

    fn visit_props(&mut self, props: &[NodePtr], parent: &Node) {
        out!(self, "{{");
        for (i, prop) in props.iter().enumerate() {
            if i > 0 {
                self.comma();
            }
            prop.visit(self, Some(parent));
        }
        out!(self, "}}");
    }

    fn visit_func_params_body(&mut self, params: &[NodePtr], body: &Node, node: &Node) {
        out!(self, "(");
        for (i, param) in params.iter().enumerate() {
            if i > 0 {
                self.comma();
            }
            param.visit(self, Some(node));
        }
        out!(self, ")");
        body.visit(self, Some(node));
    }

    fn visit_func_type_params(
        &mut self,
        params: &[NodePtr],
        this: Option<&Node>,
        rest: Option<&Node>,
        type_parameters: Option<&Node>,
        node: &Node,
    ) {
        use NodeKind::*;
        if let Some(type_parameters) = type_parameters {
            type_parameters.visit(self, Some(node));
        }
        out!(self, "(");
        let mut need_comma = false;
        if let Some(this) = this {
            match &this.kind {
                FunctionTypeParam {
                    type_annotation, ..
                } => {
                    out!(self, "this:");
                    self.space(ForceSpace::No);
                    type_annotation.visit(self, Some(node));
                }
                _ => {
                    unimplemented!("Malformed AST: Need to handle error");
                }
            }
            this.visit(self, Some(node));
            need_comma = true;
        }
        for param in params.iter() {
            if need_comma {
                self.comma();
            }
            param.visit(self, Some(node));
            need_comma = true;
        }
        if let Some(rest) = rest {
            if need_comma {
                self.comma();
            }
            out!(self, "...");
            rest.visit(self, Some(node));
        }
        out!(self, ")");
    }

    fn visit_interface(
        &mut self,
        decl: &str,
        id: &Node,
        type_parameters: Option<&Node>,
        extends: &[NodePtr],
        body: &Node,
        node: &Node,
    ) {
        out!(self, "{} ", decl);
        id.visit(self, Some(node));
        if let Some(type_parameters) = type_parameters {
            type_parameters.visit(self, Some(node));
        }
        self.space(ForceSpace::No);
        if !extends.is_empty() {
            out!(self, "extends ");
            for (i, extend) in extends.iter().enumerate() {
                if i > 0 {
                    self.comma();
                }
                extend.visit(self, Some(node));
            }
            self.space(ForceSpace::No);
        }
        body.visit(self, Some(node));
    }

    /// Generate the body of a Flow enum with type `kind`.
    fn visit_enum_body(
        &mut self,
        kind: &str,
        members: &[NodePtr],
        explicit_type: bool,
        has_unknown_members: bool,
        node: &Node,
    ) {
        if explicit_type {
            out!(self, ":");
            self.space(ForceSpace::No);
            out!(self, "{}", kind);
        }
        out!(self, "{{");
        self.inc_indent();
        self.newline();

        for (i, member) in members.iter().enumerate() {
            if i > 0 {
                self.comma();
                self.newline();
            }
            member.visit(self, Some(node));
        }

        if has_unknown_members {
            if !members.is_empty() {
                self.comma();
                self.newline();
            }
            out!(self, "...");
        }

        self.dec_indent();
        self.newline();
        out!(self, "}}");
    }

    /// Visit a statement node which is the body of a loop or a clause in an if.
    /// It could be a block statement.
    /// Return true if block
    fn visit_stmt_or_block(&mut self, node: &Node, force_block: ForceBlock, parent: &Node) -> bool {
        use NodeKind::*;
        match &node.kind {
            BlockStatement { body } => {
                if body.is_empty() {
                    self.space(ForceSpace::No);
                    out!(self, "{{}}");
                    return true;
                }
                self.space(ForceSpace::No);
                out!(self, "{{");
                self.inc_indent();
                self.newline();
                self.visit_stmt_list(body, node);
                self.dec_indent();
                self.newline();
                out!(self, "}}");
                return true;
            }
            _ => {}
        };
        if force_block == ForceBlock::Yes {
            self.space(ForceSpace::No);
            out!(self, "{{");
            self.inc_indent();
            self.newline();
            self.visit_stmt_in_block(node, parent);
            self.dec_indent();
            self.newline();
            out!(self, "}}");
            true
        } else {
            self.inc_indent();
            self.newline();
            node.visit(self, Some(parent));
            self.dec_indent();
            self.newline();
            false
        }
    }

    fn visit_stmt_list(&mut self, list: &[NodePtr], parent: &Node) {
        for (i, stmt) in list.iter().enumerate() {
            if i > 0 {
                self.newline();
            }
            self.visit_stmt_in_block(stmt, parent);
        }
    }

    fn visit_stmt_in_block(&mut self, stmt: &Node, parent: &Node) {
        stmt.visit(self, Some(parent));
        if !ends_with_block(Some(stmt)) {
            out!(self, ";");
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

            ExistsTypeAnnotation
            | EmptyTypeAnnotation
            | StringTypeAnnotation
            | NumberTypeAnnotation
            | StringLiteralTypeAnnotation { .. }
            | NumberLiteralTypeAnnotation { .. }
            | BooleanTypeAnnotation
            | BooleanLiteralTypeAnnotation { .. }
            | NullLiteralTypeAnnotation
            | SymbolTypeAnnotation
            | AnyTypeAnnotation
            | MixedTypeAnnotation
            | VoidTypeAnnotation => (PRIMARY, Assoc::Ltr),
            UnionTypeAnnotation { .. } => (UNION_TYPE, Assoc::Ltr),
            IntersectionTypeAnnotation { .. } => (INTERSECTION_TYPE, Assoc::Ltr),

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

fn ends_with_block(node: Option<&Node>) -> bool {
    use NodeKind::*;
    match node {
        Some(node) => match &node.kind {
            BlockStatement { .. } | FunctionDeclaration { .. } => true,
            WhileStatement { body, .. } => ends_with_block(Some(body)),
            ForInStatement { body, .. } => ends_with_block(Some(body)),
            ForOfStatement { body, .. } => ends_with_block(Some(body)),
            WithStatement { body, .. } => ends_with_block(Some(body)),
            SwitchStatement { .. } => true,
            LabeledStatement { body, .. } => ends_with_block(Some(body)),
            TryStatement {
                finalizer, handler, ..
            } => ends_with_block(finalizer.as_deref().or(handler.as_deref())),
            CatchClause { body, .. } => ends_with_block(Some(body)),
            IfStatement {
                alternate,
                consequent,
                ..
            } => ends_with_block(alternate.as_deref().or(Some(consequent))),
            ClassDeclaration { .. } => true,
            _ => false,
        },
        None => false,
    }
}
