/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::ast::*;
use crate::sema::DeclKind;
use crate::sema::Resolution;
use crate::sema::SemContext;
use juno_support::convert;
use juno_support::source_manager::SourceLoc;
use sourcemap::RawToken;
use sourcemap::SourceMap;
use sourcemap::SourceMapBuilder;
use std::fmt;
use std::io;
use std::io::BufWriter;
use std::io::Write;
use std::rc::Rc;

/// Options for JS generation.
pub struct Opt<'s> {
    /// Whether to pretty-print the generated JS.
    pub pretty: Pretty,

    /// How to annotate the generated source.
    pub annotation: Annotation<'s>,

    /// Whether to force a space after the `async` keyword in arrow functions.
    pub force_async_arrow_space: bool,

    /// If `Some`, doc block to print at the top of the file.
    pub doc_block: Option<Rc<String>>,

    /// Delimiter to use for string literals.
    pub quote: QuoteChar,
}

impl Default for Opt<'_> {
    fn default() -> Self {
        Opt {
            pretty: Pretty::Yes,
            annotation: Annotation::No,
            force_async_arrow_space: true,
            doc_block: None,
            quote: QuoteChar::Single,
        }
    }
}

impl Opt<'_> {
    pub fn new() -> Self {
        Default::default()
    }
}

/// Whether to pretty-print the generated JS.
/// Does not do full formatting of the source, but does add indentation and
/// some extra spaces to make source more readable.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Pretty {
    No,
    Yes,
}

/// Delimiter to use for string literals.
#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum QuoteChar {
    Single,
    Double,
}

impl QuoteChar {
    /// The character representation of the quote.
    #[inline]
    fn as_char(self) -> char {
        match self {
            Self::Single => '\'',
            Self::Double => '"',
        }
    }
}

/// Generate JS for `root` and print it to `out`.
/// FIXME: This currently only returns an empty SourceMap.
pub fn generate(
    out: &mut dyn Write,
    ctx: &mut Context,
    root: &NodeRc,
    opt: Opt,
) -> io::Result<SourceMap> {
    let gc = GCLock::new(ctx);
    GenJS::gen_root(out, &gc, root.node(&gc), opt)
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
    use crate::ast::BinaryExpressionOperator;
    use crate::ast::LogicalExpressionOperator;

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

    pub fn get_binary_precedence(op: BinaryExpressionOperator) -> Precedence {
        use BinaryExpressionOperator::*;
        (match op {
            Exp => 12,
            Mult => 11,
            Mod => 11,
            Div => 11,
            Plus => 10,
            Minus => 10,
            LShift => 9,
            RShift => 9,
            RShift3 => 9,
            Less => 8,
            Greater => 8,
            LessEquals => 8,
            GreaterEquals => 8,
            LooseEquals => 7,
            LooseNotEquals => 7,
            StrictEquals => 7,
            StrictNotEquals => 7,
            BitAnd => 6,
            BitXor => 5,
            BitOr => 4,
            In => 8,
            Instanceof => 8,
        }) + BIN_START
    }

    pub fn get_logical_precedence(op: LogicalExpressionOperator) -> Precedence {
        use LogicalExpressionOperator::*;
        (match op {
            And => 3,
            Or => 2,
            NullishCoalesce => 1,
        }) + BIN_START
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
    /// No pathheses needed.
    No,
    /// Pathheses required.
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

pub enum Annotation<'s> {
    No,
    Sem(&'s SemContext),
}

/// Generator for output JS. Walks the AST to output real JS.
struct GenJS<'s, 'w> {
    /// Where to write the generated JS.
    out: BufWriter<&'w mut dyn Write>,

    /// Options for generating JS.
    opt: Opt<'s>,

    /// Size of the indentation step.
    /// May be configurable in the future.
    indent_step: usize,

    /// Current indentation level, used in pretty mode.
    indent: usize,

    /// Current position of the writer.
    position: SourceLoc,

    /// Raw token tracking the most recent node.
    cur_token: Option<RawToken>,

    /// Build a source map as we go along.
    sourcemap: SourceMapBuilder,

    /// Some(err) if an error has occurred when writing, else None.
    error: Option<io::Error>,
}

/// Print to the output stream if no errors have been seen so far.
/// `$gen_js` is a mutable reference to the GenJS struct.
/// `$arg` arguments follow the format pattern used by `format!`.
/// The output must be ASCII and contain no newlines.
macro_rules! out {
    ($gen_js:expr, $($arg:tt)*) => {{
        $gen_js.write_ascii(format_args!($($arg)*));
    }}
}

/// Emit a source mapping token at this point with the given node, and call `out!()`.
/// Call this macro instead of `out!()` directly if emitting the start of an AST node.
macro_rules! out_token {
    ($gen_js:expr, $node:expr, $($arg:tt)*) => {{
        $gen_js.add_segment($node);
        out!($gen_js, $($arg)*);
    }}
}

impl GenJS<'_, '_> {
    /// Generate JS for `root` and flush the output.
    /// If at any point, JS generation resulted in an error, return `Err(err)`,
    /// otherwise return `Ok(())`.
    fn gen_root<'gc>(
        writer: &mut dyn Write,
        ctx: &'gc GCLock,
        root: &'gc Node<'gc>,
        opt: Opt,
    ) -> io::Result<SourceMap> {
        let mut gen_js = GenJS {
            out: BufWriter::new(writer),
            opt,
            indent_step: 2,
            indent: 0,
            position: SourceLoc { line: 1, col: 1 },
            cur_token: None,
            // FIXME: Pass in file name here.
            sourcemap: SourceMapBuilder::new(None),
            error: None,
        };
        for i in 0..ctx.sm().num_sources() {
            gen_js
                .sourcemap
                .add_source(ctx.sm().source_name(SourceId(i as u32)));
        }

        if let Some(doc_block) = gen_js.opt.doc_block.clone() {
            let mut buf = [0u8; 4];
            for c in doc_block.chars() {
                if c == '\n' {
                    gen_js.force_newline_without_indent();
                } else {
                    gen_js.write_char(c, &mut buf);
                }
            }
        }

        root.visit(ctx, &mut gen_js, None);
        gen_js.force_newline();

        gen_js.flush_cur_token();
        match gen_js.error {
            None => gen_js
                .out
                .flush()
                .and(Ok(gen_js.sourcemap.into_sourcemap())),
            Some(err) => Err(err),
        }
    }

    /// Write to the `out` writer if we haven't seen any errors.
    /// If we have seen any errors, do nothing.
    /// Used via the `out!` macro.
    /// The output must be ASCII and contain no newlines.
    fn write_ascii(&mut self, args: fmt::Arguments<'_>) {
        if self.error.is_none() {
            let buf = format!("{}", args);
            debug_assert!(buf.is_ascii(), "Output must be ASCII");
            debug_assert!(!buf.contains('\n'), "Output must have no newlines");
            if let Err(e) = self.out.write_all(buf.as_bytes()) {
                self.error = Some(e);
            }
            self.position.col += buf.len() as u32;
        }
    }

    /// Write a single unicode character to the `out` writer if we haven't seen any errors.
    /// Character must not be a newline.
    /// Use `dst` as a temporary buffer.
    /// If we have seen any errors, do nothing.
    fn write_char(&mut self, ch: char, dst: &mut [u8]) {
        debug_assert!(ch != '\n', "Output must not contain newlines");
        if self.error.is_none() {
            if let Err(e) = self.out.write_all(ch.encode_utf8(dst).as_bytes()) {
                self.error = Some(e);
            }
            self.position.col += 1;
        }
    }

    /// Write unicode to the `out` writer if we haven't seen any errors.
    /// If we have seen any errors, do nothing.
    /// The output must contain no newlines.
    fn write_utf8(&mut self, s: &str) {
        debug_assert!(
            !s.chars().any(|c| c == '\n'),
            "Output must not contain newlines"
        );
        if self.error.is_none() {
            if let Err(e) = self.out.write_all(s.as_bytes()) {
                self.error = Some(e);
            }
        }
        self.position.col += s.chars().count() as u32;
    }

    /// Generate the JS for each node kind.
    fn gen_node<'gc>(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>) {
        match node {
            Node::Empty(_) => {}
            Node::Metadata(_) => {}

            Node::Program(Program { metadata: _, body }) => {
                self.visit_stmt_list(ctx, body, Path::new(node, NodeField::body));
            }
            Node::Module(Module { metadata: _, body }) => {
                self.visit_stmt_list(ctx, body, Path::new(node, NodeField::body));
            }

            Node::FunctionExpression(FunctionExpression {
                metadata: _,
                id,
                params,
                body,
                type_parameters,
                return_type,
                predicate,
                generator,
                is_async,
            })
            | Node::FunctionDeclaration(FunctionDeclaration {
                metadata: _,
                id,
                params,
                body,
                type_parameters,
                return_type,
                predicate,
                generator,
                is_async,
            }) => {
                if *is_async {
                    out_token!(self, node, "async function");
                } else {
                    out_token!(self, node, "function");
                }
                if *generator {
                    out!(self, "*");
                    if id.is_some() {
                        self.space(ForceSpace::No);
                    }
                } else if id.is_some() {
                    self.space(ForceSpace::Yes);
                }
                if let Some(id) = id {
                    id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                }
                self.visit_func_params_body(
                    ctx,
                    params,
                    *type_parameters,
                    *return_type,
                    *predicate,
                    *body,
                    node,
                );
            }

            Node::ArrowFunctionExpression(ArrowFunctionExpression {
                metadata: _,
                id: _,
                params,
                body,
                type_parameters,
                return_type,
                predicate,
                expression,
                is_async,
            }) => {
                let mut need_sep = false;
                if *is_async {
                    out!(self, "async");
                    if self.opt.force_async_arrow_space || self.opt.pretty == Pretty::Yes {
                        // Force a space to work with certain transforms that match on `async`
                        // followed by whitespace to detect async functions.
                        self.space(ForceSpace::Yes);
                    } else {
                        need_sep = true;
                    }
                }
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                    need_sep = false;
                }
                // Single parameter without type info doesn't need parens.
                // But only in expression mode, otherwise it is ugly.
                if params.len() == 1
                    && type_parameters.is_none()
                    && return_type.is_none()
                    && predicate.is_none()
                    && matches!(
                        params.head().unwrap(),
                        Node::Identifier(Identifier {
                            type_annotation: None,
                            optional: false,
                            ..
                        })
                    )
                    && (*expression || self.opt.pretty == Pretty::No)
                {
                    if need_sep {
                        out!(self, " ");
                    }
                    params.head().unwrap().visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::params)),
                    );
                } else {
                    out!(self, "(");
                    for (i, param) in params.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        param.visit(ctx, self, Some(Path::new(node, NodeField::params)));
                    }
                    out!(self, ")");
                }
                if return_type.is_some() || predicate.is_some() {
                    out!(self, ":");
                }
                if let Some(return_type) = return_type {
                    self.space(ForceSpace::No);
                    self.print_child(
                        ctx,
                        Some(return_type),
                        Path::new(node, NodeField::return_type),
                        ChildPos::Anywhere,
                    );
                }
                if let Some(predicate) = predicate {
                    self.space(ForceSpace::Yes);
                    predicate.visit(ctx, self, Some(Path::new(node, NodeField::predicate)));
                }
                self.space(ForceSpace::No);
                out!(self, "=>");
                self.space(ForceSpace::No);
                match &body {
                    Node::BlockStatement(_) => {
                        body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
                    }
                    _ => {
                        self.print_child(
                            ctx,
                            Some(*body),
                            Path::new(node, NodeField::body),
                            ChildPos::Right,
                        );
                    }
                }
            }

            Node::WhileStatement(WhileStatement {
                metadata: _,
                body,
                test,
            }) => {
                out!(self, "while");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(ctx, self, Some(Path::new(node, NodeField::test)));
                out!(self, ")");
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
            }
            Node::DoWhileStatement(DoWhileStatement {
                metadata: _,
                body,
                test,
            }) => {
                out!(self, "do ");
                let block = self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
                if block {
                    self.space(ForceSpace::No);
                } else {
                    self.newline();
                }
                out!(self, "while");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(ctx, self, Some(Path::new(node, NodeField::test)));
                out!(self, ")");
            }

            Node::ForInStatement(ForInStatement {
                metadata: _,
                left,
                right,
                body,
            }) => {
                out!(self, "for(");
                left.visit(ctx, self, Some(Path::new(node, NodeField::left)));
                out!(self, " in ");
                right.visit(ctx, self, Some(Path::new(node, NodeField::right)));
                out!(self, ")");
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
            }
            Node::ForOfStatement(ForOfStatement {
                metadata: _,
                left,
                right,
                body,
                is_await,
            }) => {
                out!(self, "for{}(", if *is_await { " await" } else { "" });
                left.visit(ctx, self, Some(Path::new(node, NodeField::left)));
                out!(self, " of ");
                right.visit(ctx, self, Some(Path::new(node, NodeField::right)));
                out!(self, ")");
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
            }
            Node::ForStatement(ForStatement {
                metadata: _,
                init,
                test,
                update,
                body,
            }) => {
                out!(self, "for(");
                self.print_child(ctx, *init, Path::new(node, NodeField::init), ChildPos::Left);
                out!(self, ";");
                if let Some(test) = test {
                    self.space(ForceSpace::No);
                    test.visit(ctx, self, Some(Path::new(node, NodeField::test)));
                }
                out!(self, ";");
                if let Some(update) = update {
                    self.space(ForceSpace::No);
                    update.visit(ctx, self, Some(Path::new(node, NodeField::update)));
                }
                out!(self, ")");
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
            }

            Node::DebuggerStatement(_) => {
                out!(self, "debugger");
            }
            Node::EmptyStatement(_) => {}

            Node::BlockStatement(BlockStatement { metadata: _, body }) => {
                if body.is_empty() {
                    out!(self, "{{}}");
                } else {
                    out!(self, "{{");
                    self.inc_indent();
                    self.newline();
                    self.visit_stmt_list(ctx, body, Path::new(node, NodeField::body));
                    self.dec_indent();
                    self.newline();
                    out!(self, "}}");
                }
            }

            Node::BreakStatement(BreakStatement { metadata: _, label }) => {
                out!(self, "break");
                if let Some(label) = label {
                    self.space(ForceSpace::Yes);
                    label.visit(ctx, self, Some(Path::new(node, NodeField::label)));
                }
            }
            Node::ContinueStatement(ContinueStatement { metadata: _, label }) => {
                out!(self, "continue");
                if let Some(label) = label {
                    self.space(ForceSpace::Yes);
                    label.visit(ctx, self, Some(Path::new(node, NodeField::label)));
                }
            }

            Node::ThrowStatement(ThrowStatement {
                metadata: _,
                argument,
            }) => {
                out_token!(self, node, "throw ");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
            }
            Node::ReturnStatement(ReturnStatement {
                metadata: _,
                argument,
            }) => {
                out_token!(self, node, "return");
                if let Some(argument) = argument {
                    out!(self, " ");
                    argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
                }
            }
            Node::WithStatement(WithStatement {
                metadata: _,
                object,
                body,
            }) => {
                out_token!(self, node, "with");
                self.space(ForceSpace::No);
                out!(self, "(");
                object.visit(ctx, self, Some(Path::new(node, NodeField::object)));
                out!(self, ")");
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::No,
                    Path::new(node, NodeField::body),
                );
            }

            Node::SwitchStatement(SwitchStatement {
                metadata: _,
                discriminant,
                cases,
            }) => {
                out_token!(self, node, "switch");
                self.space(ForceSpace::No);
                out!(self, "(");
                discriminant.visit(ctx, self, Some(Path::new(node, NodeField::discriminant)));
                out!(self, ")");
                self.space(ForceSpace::No);
                out!(self, "{{");
                self.newline();
                for case in cases.iter() {
                    case.visit(ctx, self, Some(Path::new(node, NodeField::cases)));
                    self.newline();
                }
                out!(self, "}}");
            }
            Node::SwitchCase(SwitchCase {
                metadata: _,
                test,
                consequent,
            }) => {
                match test {
                    Some(test) => {
                        out_token!(self, node, "case ");
                        test.visit(ctx, self, Some(Path::new(node, NodeField::test)));
                    }
                    None => {
                        out_token!(self, node, "default");
                    }
                };
                out!(self, ":");
                if !consequent.is_empty() {
                    self.inc_indent();
                    self.newline();
                    self.visit_stmt_list(ctx, consequent, Path::new(node, NodeField::consequent));
                    self.dec_indent();
                }
            }

            Node::LabeledStatement(LabeledStatement {
                metadata: _,
                label,
                body,
            }) => {
                label.visit(ctx, self, Some(Path::new(node, NodeField::label)));
                out!(self, ":");
                self.newline();
                body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
            }

            Node::ExpressionStatement(ExpressionStatement {
                metadata: _,
                expression,
                directive: _,
            }) => {
                self.print_child(
                    ctx,
                    Some(*expression),
                    Path::new(node, NodeField::expression),
                    ChildPos::Anywhere,
                );
            }

            Node::TryStatement(TryStatement {
                metadata: _,
                block,
                handler,
                finalizer,
            }) => {
                out_token!(self, node, "try");
                self.visit_stmt_or_block(
                    ctx,
                    *block,
                    ForceBlock::Yes,
                    Path::new(node, NodeField::block),
                );
                if let Some(handler) = handler {
                    handler.visit(ctx, self, Some(Path::new(node, NodeField::handler)));
                }
                if let Some(finalizer) = finalizer {
                    out!(self, "finally");
                    self.space(ForceSpace::No);
                    self.visit_stmt_or_block(
                        ctx,
                        *finalizer,
                        ForceBlock::Yes,
                        Path::new(node, NodeField::finalizer),
                    );
                }
            }

            Node::IfStatement(IfStatement {
                metadata: _,
                test,
                consequent,
                alternate,
            }) => {
                out_token!(self, node, "if");
                self.space(ForceSpace::No);
                out!(self, "(");
                test.visit(ctx, self, Some(Path::new(node, NodeField::test)));
                out!(self, ")");
                let force_block = if alternate.is_some() && is_if_without_else(consequent) {
                    ForceBlock::Yes
                } else {
                    ForceBlock::No
                };
                self.visit_stmt_or_block(
                    ctx,
                    *consequent,
                    force_block,
                    Path::new(node, NodeField::consequent),
                );
                if let Some(alternate) = alternate {
                    out!(self, "else");
                    self.space(if matches!(alternate, Node::BlockStatement(_)) {
                        ForceSpace::No
                    } else {
                        ForceSpace::Yes
                    });
                    self.visit_stmt_or_block(
                        ctx,
                        *alternate,
                        ForceBlock::No,
                        Path::new(node, NodeField::alternate),
                    );
                }
            }

            Node::BooleanLiteral(BooleanLiteral { metadata: _, value }) => {
                out_token!(self, node, "{}", if *value { "true" } else { "false" });
            }
            Node::NullLiteral(_) => {
                out_token!(self, node, "null");
            }
            Node::StringLiteral(StringLiteral { metadata: _, value }) => {
                out_token!(self, node, "{}", self.opt.quote.as_char());
                self.print_escaped_string_literal(ctx, *value, self.opt.quote.as_char());
                out!(self, "{}", self.opt.quote.as_char());
            }
            Node::NumericLiteral(NumericLiteral { metadata: _, value }) => {
                out_token!(self, node, "{}", convert::number_to_string(*value));
            }
            Node::BigIntLiteral(BigIntLiteral {
                metadata: _,
                bigint,
            }) => {
                self.add_segment(node);
                self.write_utf8(ctx.str(*bigint));
            }
            Node::RegExpLiteral(RegExpLiteral {
                metadata: _,
                pattern,
                flags,
            }) => {
                out_token!(self, node, "/");
                // Parser doesn't handle escapes when lexing RegExp,
                // so we don't need to do any manual escaping here.
                self.write_utf8(ctx.str(*pattern));
                out!(self, "/");
                self.write_utf8(ctx.str(*flags));
            }
            Node::ThisExpression(_) => {
                out_token!(self, node, "this");
            }
            Node::Super(_) => {
                out_token!(self, node, "super");
            }

            Node::SequenceExpression(SequenceExpression {
                metadata: _,
                expressions,
            }) => {
                out!(self, "(");
                for (i, expr) in expressions.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_child(
                        ctx,
                        Some(expr),
                        Path::new(node, NodeField::expressions),
                        if i == 1 {
                            ChildPos::Left
                        } else {
                            ChildPos::Right
                        },
                    );
                }
                out!(self, ")");
            }

            Node::ObjectExpression(ObjectExpression {
                metadata: _,
                properties,
            }) => {
                self.visit_props(ctx, properties, Path::new(node, NodeField::properties));
            }
            Node::ArrayExpression(ArrayExpression {
                metadata: _,
                elements,
                trailing_comma,
            }) => {
                out_token!(self, node, "[");
                for (i, elem) in elements.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    match elem {
                        Node::SpreadElement(_) => {
                            elem.visit(ctx, self, Some(Path::new(node, NodeField::elements)));
                        }
                        Node::Empty(_) => {}
                        _ => {
                            self.print_comma_expression(
                                ctx,
                                elem,
                                Path::new(node, NodeField::elements),
                            );
                        }
                    }
                }
                if *trailing_comma {
                    self.comma();
                }
                out!(self, "]");
            }

            Node::SpreadElement(SpreadElement {
                metadata: _,
                argument,
            }) => {
                out_token!(self, node, "...");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
            }

            Node::NewExpression(NewExpression {
                metadata: _,
                callee,
                type_arguments,
                arguments,
            }) => {
                out_token!(self, node, "new ");
                self.print_child(
                    ctx,
                    Some(*callee),
                    Path::new(node, NodeField::callee),
                    ChildPos::Left,
                );
                if let Some(type_arguments) = type_arguments {
                    type_arguments.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_arguments)),
                    );
                }
                out!(self, "(");
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_child(
                        ctx,
                        Some(arg),
                        Path::new(node, NodeField::arguments),
                        ChildPos::Anywhere,
                    );
                }
                out!(self, ")");
            }
            Node::YieldExpression(YieldExpression {
                metadata: _,
                argument,
                delegate,
            }) => {
                out_token!(self, node, "yield");
                if *delegate {
                    out!(self, "*");
                    self.space(ForceSpace::No);
                } else if argument.is_some() {
                    out!(self, " ");
                }
                self.print_child(
                    ctx,
                    *argument,
                    Path::new(node, NodeField::argument),
                    ChildPos::Right,
                );
            }
            Node::AwaitExpression(AwaitExpression {
                metadata: _,
                argument,
            }) => {
                out!(self, "await ");
                self.print_child(
                    ctx,
                    Some(*argument),
                    Path::new(node, NodeField::argument),
                    ChildPos::Right,
                );
            }

            Node::ImportExpression(ImportExpression {
                metadata: _,
                source,
                attributes,
            }) => {
                out_token!(self, node, "import(");
                source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
                if let Some(attributes) = attributes {
                    out!(self, ",");
                    self.space(ForceSpace::No);
                    attributes.visit(ctx, self, Some(Path::new(node, NodeField::attributes)));
                }
                out!(self, ")");
            }

            Node::CallExpression(CallExpression {
                metadata: _,
                callee,
                type_arguments,
                arguments,
            }) => {
                self.print_child(
                    ctx,
                    Some(*callee),
                    Path::new(node, NodeField::callee),
                    ChildPos::Left,
                );
                if let Some(type_arguments) = type_arguments {
                    type_arguments.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_arguments)),
                    );
                }
                out!(self, "(");
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_child(
                        ctx,
                        Some(arg),
                        Path::new(node, NodeField::arguments),
                        ChildPos::Anywhere,
                    );
                }
                out!(self, ")");
            }
            Node::OptionalCallExpression(OptionalCallExpression {
                metadata: _,
                callee,
                type_arguments,
                arguments,
                optional,
            }) => {
                self.print_child(
                    ctx,
                    Some(*callee),
                    Path::new(node, NodeField::callee),
                    ChildPos::Left,
                );
                if let Some(type_arguments) = type_arguments {
                    type_arguments.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_arguments)),
                    );
                }
                out!(self, "{}(", if *optional { "?." } else { "" });
                for (i, arg) in arguments.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    self.print_child(
                        ctx,
                        Some(arg),
                        Path::new(node, NodeField::arguments),
                        ChildPos::Anywhere,
                    );
                }
                out!(self, ")");
            }

            Node::AssignmentExpression(AssignmentExpression {
                metadata: _,
                operator,
                left,
                right,
            }) => {
                self.print_child(
                    ctx,
                    Some(*left),
                    Path::new(node, NodeField::left),
                    ChildPos::Left,
                );
                self.space(ForceSpace::No);
                out!(self, "{}", operator.as_str());
                self.space(ForceSpace::No);
                self.print_child(
                    ctx,
                    Some(*right),
                    Path::new(node, NodeField::right),
                    ChildPos::Right,
                );
            }
            Node::UnaryExpression(UnaryExpression {
                metadata: _,
                operator,
                argument,
                prefix,
            }) => {
                let ident = operator.as_str().chars().next().unwrap().is_alphabetic();
                if *prefix {
                    out!(self, "{}", operator.as_str());
                    if ident {
                        out!(self, " ");
                    }
                    self.print_child(
                        ctx,
                        Some(*argument),
                        Path::new(node, NodeField::argument),
                        ChildPos::Right,
                    );
                } else {
                    self.print_child(
                        ctx,
                        Some(*argument),
                        Path::new(node, NodeField::argument),
                        ChildPos::Left,
                    );
                    if ident {
                        out!(self, " ");
                    }
                    out!(self, "{}", operator.as_str());
                }
            }
            Node::UpdateExpression(UpdateExpression {
                metadata: _,
                operator,
                argument,
                prefix,
            }) => {
                if *prefix {
                    out!(self, "{}", operator.as_str());
                    self.print_child(
                        ctx,
                        Some(*argument),
                        Path::new(node, NodeField::argument),
                        ChildPos::Right,
                    );
                } else {
                    self.print_child(
                        ctx,
                        Some(*argument),
                        Path::new(node, NodeField::argument),
                        ChildPos::Left,
                    );
                    out!(self, "{}", operator.as_str());
                }
            }
            Node::MemberExpression(MemberExpression {
                metadata: _,
                object,
                property,
                computed,
            }) => {
                match object {
                    Node::NumericLiteral(NumericLiteral { value, .. }) => {
                        // Account for possible `50..toString()`.
                        let string = convert::number_to_string(*value);
                        // If there is an `e` or a decimal point, no need for an extra `.`.
                        let suffix = string.find::<&[char]>(&['E', 'e', '.']).map_or(".", |_| "");
                        out_token!(self, node, "{}{}", string, suffix);
                    }
                    _ => {
                        self.print_child(
                            ctx,
                            Some(*object),
                            Path::new(node, NodeField::object),
                            ChildPos::Left,
                        );
                    }
                }
                if *computed {
                    out!(self, "[");
                } else {
                    out!(self, ".");
                }
                self.print_child(
                    ctx,
                    Some(*property),
                    Path::new(node, NodeField::property),
                    ChildPos::Right,
                );
                if *computed {
                    out!(self, "]");
                }
            }
            Node::OptionalMemberExpression(OptionalMemberExpression {
                metadata: _,
                object,
                property,
                computed,
                optional,
            }) => {
                self.print_child(
                    ctx,
                    Some(*object),
                    Path::new(node, NodeField::object),
                    ChildPos::Left,
                );
                if *computed {
                    out!(self, "{}[", if *optional { "?." } else { "" });
                } else {
                    out!(self, "{}.", if *optional { "?" } else { "" });
                }
                self.print_child(
                    ctx,
                    Some(*property),
                    Path::new(node, NodeField::property),
                    ChildPos::Right,
                );
                if *computed {
                    out!(self, "]");
                }
            }

            Node::BinaryExpression(BinaryExpression {
                metadata: _,
                left,
                right,
                operator,
            }) => {
                let ident = operator.as_str().chars().next().unwrap().is_alphabetic();
                self.print_child(
                    ctx,
                    Some(*left),
                    Path::new(node, NodeField::left),
                    ChildPos::Left,
                );
                self.space(if ident {
                    ForceSpace::Yes
                } else {
                    ForceSpace::No
                });
                out!(self, "{}", operator.as_str());
                self.space(if ident {
                    ForceSpace::Yes
                } else {
                    ForceSpace::No
                });
                self.print_child(
                    ctx,
                    Some(*right),
                    Path::new(node, NodeField::right),
                    ChildPos::Right,
                );
            }

            Node::Directive(Directive { metadata: _, value }) => {
                value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
            }
            Node::DirectiveLiteral(DirectiveLiteral { metadata: _, .. }) => {
                unimplemented!("No escaping for directive literals");
            }

            Node::ConditionalExpression(ConditionalExpression {
                metadata: _,
                test,
                consequent,
                alternate,
            }) => {
                self.print_child(
                    ctx,
                    Some(*test),
                    Path::new(node, NodeField::test),
                    ChildPos::Left,
                );
                self.space(ForceSpace::No);
                out!(self, "?");
                self.space(ForceSpace::No);
                self.print_child(
                    ctx,
                    Some(*consequent),
                    Path::new(node, NodeField::consequent),
                    ChildPos::Anywhere,
                );
                self.space(ForceSpace::No);
                out!(self, ":");
                self.space(ForceSpace::No);
                self.print_child(
                    ctx,
                    Some(*alternate),
                    Path::new(node, NodeField::alternate),
                    ChildPos::Right,
                );
            }

            Node::Identifier(Identifier {
                metadata: _,
                name,
                type_annotation,
                optional,
            }) => {
                self.add_segment(node);
                self.write_utf8(ctx.str(*name).as_ref());
                self.annotate_identifier(ctx, node);
                if *optional {
                    out!(self, "?");
                }
                if let Some(type_annotation) = type_annotation {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
            }
            Node::PrivateName(PrivateName { metadata: _, id }) => {
                out_token!(self, node, "#");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
            }
            Node::MetaProperty(MetaProperty {
                metadata: _,
                meta,
                property,
            }) => {
                meta.visit(ctx, self, Some(Path::new(node, NodeField::meta)));
                out!(self, ".");
                property.visit(ctx, self, Some(Path::new(node, NodeField::property)));
            }

            Node::CatchClause(CatchClause {
                metadata: _,
                param,
                body,
            }) => {
                self.space(ForceSpace::No);
                out_token!(self, node, "catch");
                if let Some(param) = param {
                    self.space(ForceSpace::No);
                    out!(self, "(");
                    param.visit(ctx, self, Some(Path::new(node, NodeField::param)));
                    out!(self, ")");
                }
                self.visit_stmt_or_block(
                    ctx,
                    *body,
                    ForceBlock::Yes,
                    Path::new(node, NodeField::body),
                );
            }

            Node::VariableDeclaration(VariableDeclaration {
                metadata: _,
                kind,
                declarations,
            }) => {
                out_token!(self, node, "{} ", kind.as_str());
                for (i, decl) in declarations.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    decl.visit(ctx, self, Some(Path::new(node, NodeField::declarations)));
                }
            }
            Node::VariableDeclarator(VariableDeclarator {
                metadata: _,
                init,
                id,
            }) => {
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(init) = init {
                    out!(
                        self,
                        "{}",
                        match self.opt.pretty {
                            Pretty::Yes => " = ",
                            Pretty::No => "=",
                        }
                    );
                    init.visit(ctx, self, Some(Path::new(node, NodeField::init)));
                }
            }

            Node::TemplateLiteral(TemplateLiteral {
                metadata: _,
                quasis,
                expressions,
            }) => {
                out_token!(self, node, "`");
                let mut it_expr = expressions.iter();
                for quasi in quasis.iter() {
                    if let Node::TemplateElement(TemplateElement {
                        metadata: _,
                        raw,
                        tail: _,
                        cooked: _,
                    }) = quasi
                    {
                        let mut buf = [0u8; 4];
                        for char in ctx.str(*raw).chars() {
                            if char == '\n' {
                                self.force_newline_without_indent();
                                continue;
                            }
                            self.write_char(char, &mut buf);
                        }
                        if let Some(expr) = it_expr.next() {
                            out!(self, "${{");
                            expr.visit(ctx, self, Some(Path::new(node, NodeField::expressions)));
                            out!(self, "}}");
                        }
                    }
                }
                out!(self, "`");
            }
            Node::TaggedTemplateExpression(TaggedTemplateExpression {
                metadata: _,
                tag,
                quasi,
            }) => {
                self.print_child(
                    ctx,
                    Some(*tag),
                    Path::new(node, NodeField::tag),
                    ChildPos::Left,
                );
                self.print_child(
                    ctx,
                    Some(*quasi),
                    Path::new(node, NodeField::quasi),
                    ChildPos::Right,
                );
            }
            Node::TemplateElement(_) => {
                unreachable!("TemplateElement is handled in TemplateLiteral case");
            }

            Node::Property(Property {
                metadata: _,
                key,
                value,
                kind,
                computed,
                method,
                shorthand,
            }) => {
                let mut need_sep = false;
                if *kind != PropertyKind::Init {
                    out_token!(self, node, "{}", kind.as_str());
                    need_sep = true;
                } else if *method {
                    match value {
                        Node::FunctionExpression(FunctionExpression {
                            metadata: _,
                            generator,
                            is_async,
                            ..
                        }) => {
                            if *is_async {
                                out!(self, "async");
                                need_sep = true;
                            }
                            if *generator {
                                out!(self, "*");
                                need_sep = false;
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
                if *shorthand {
                    value.visit(ctx, self, None);
                } else {
                    key.visit(ctx, self, None);
                }
                if *computed {
                    out!(self, "]");
                }
                if *shorthand {
                    return;
                }
                if *kind != PropertyKind::Init || *method {
                    match value {
                        Node::FunctionExpression(FunctionExpression {
                            metadata: _,
                            // Name is handled by the property key.
                            id: _,
                            params,
                            body,
                            return_type,
                            predicate,
                            type_parameters,
                            // Handled above.
                            generator: _,
                            is_async: _,
                        }) => {
                            self.visit_func_params_body(
                                ctx,
                                params,
                                *type_parameters,
                                *return_type,
                                *predicate,
                                *body,
                                *value,
                            );
                        }
                        _ => unreachable!(),
                    };
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
            }

            Node::LogicalExpression(LogicalExpression {
                metadata: _,
                left,
                right,
                operator,
            }) => {
                self.print_child(
                    ctx,
                    Some(*left),
                    Path::new(node, NodeField::left),
                    ChildPos::Left,
                );
                self.space(ForceSpace::No);
                out!(self, "{}", operator.as_str());
                self.space(ForceSpace::No);
                self.print_child(
                    ctx,
                    Some(*right),
                    Path::new(node, NodeField::right),
                    ChildPos::Right,
                );
            }

            Node::ClassExpression(ClassExpression {
                metadata: _,
                id,
                type_parameters,
                super_class,
                super_type_parameters,
                implements,
                decorators,
                body,
            })
            | Node::ClassDeclaration(ClassDeclaration {
                metadata: _,
                id,
                type_parameters,
                super_class,
                super_type_parameters,
                implements,
                decorators,
                body,
            }) => {
                if !decorators.is_empty() {
                    for decorator in decorators.iter() {
                        decorator.visit(ctx, self, Some(Path::new(node, NodeField::decorators)));
                        self.force_newline();
                    }
                    out!(self, "class");
                } else {
                    out_token!(self, node, "class");
                }
                if let Some(id) = id {
                    self.space(ForceSpace::Yes);
                    id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                }
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                if let Some(super_class) = super_class {
                    out!(self, " extends ");
                    super_class.visit(ctx, self, Some(Path::new(node, NodeField::super_class)));
                }
                if let Some(super_type_parameters) = super_type_parameters {
                    super_type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::super_type_parameters)),
                    );
                }
                if !implements.is_empty() {
                    out!(self, " implements ");
                    for (i, implement) in implements.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        implement.visit(ctx, self, Some(Path::new(node, NodeField::implements)));
                    }
                }

                self.space(ForceSpace::No);
                body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
            }

            Node::ClassBody(ClassBody { metadata: _, body }) => {
                if body.is_empty() {
                    out!(self, "{{}}");
                } else {
                    out!(self, "{{");
                    self.inc_indent();
                    self.newline();
                    for prop in body.iter() {
                        prop.visit(ctx, self, Some(Path::new(node, NodeField::body)));
                        self.newline();
                    }
                    out!(self, "}}");
                    self.dec_indent();
                    self.newline();
                }
            }
            Node::ClassProperty(ClassProperty {
                metadata: _,
                key,
                value,
                computed,
                is_static,
                declare,
                optional,
                variance,
                type_annotation,
            }) => {
                if *declare {
                    out!(self, "declare ");
                }
                if *is_static {
                    out!(self, "static ");
                }
                if let Some(variance) = variance {
                    variance.visit(ctx, self, Some(Path::new(node, NodeField::variance)));
                }
                if *computed {
                    out!(self, "[");
                }
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                if *computed {
                    out!(self, "]");
                }
                if *optional {
                    out!(self, "?");
                }
                if let Some(type_annotation) = type_annotation {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
                if let Some(value) = value {
                    self.space(ForceSpace::No);
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
                out!(self, ";");
            }
            Node::ClassPrivateProperty(ClassPrivateProperty {
                metadata: _,
                key,
                value,
                is_static,
                declare,
                optional,
                variance,
                type_annotation,
            }) => {
                if let Some(variance) = variance {
                    variance.visit(ctx, self, Some(Path::new(node, NodeField::variance)));
                }
                if *is_static {
                    out!(self, "static ");
                }
                if *declare {
                    out!(self, "static ");
                }
                out!(self, "#");
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                if *optional {
                    out!(self, "?");
                }
                if let Some(type_annotation) = type_annotation {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
                self.space(ForceSpace::No);
                if let Some(value) = value {
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
                out!(self, ";");
            }
            Node::MethodDefinition(MethodDefinition {
                metadata: _,
                key,
                value,
                kind,
                computed,
                is_static,
            }) => {
                let (is_async, generator, params, body, return_type, predicate, type_parameters) =
                    match value {
                        Node::FunctionExpression(FunctionExpression {
                            metadata: _,
                            id: _,
                            generator,
                            is_async,
                            params,
                            body,
                            return_type,
                            predicate,
                            type_parameters,
                        }) => (
                            *is_async,
                            *generator,
                            params,
                            body,
                            return_type,
                            predicate,
                            type_parameters,
                        ),
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
                match *kind {
                    MethodDefinitionKind::Method => {}
                    MethodDefinitionKind::Constructor => {
                        // Will be handled by key output.
                    }
                    MethodDefinitionKind::Get => {
                        out!(self, "get ");
                    }
                    MethodDefinitionKind::Set => {
                        out!(self, "set ");
                    }
                };
                if *computed {
                    out!(self, "[");
                }
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                if *computed {
                    out!(self, "]");
                }
                self.visit_func_params_body(
                    ctx,
                    params,
                    *type_parameters,
                    *return_type,
                    *predicate,
                    *body,
                    node,
                );
            }

            Node::ImportDeclaration(ImportDeclaration {
                metadata: _,
                specifiers,
                source,
                assertions,
                import_kind,
            }) => {
                out_token!(self, node, "import ");
                if *import_kind != ImportKind::Value {
                    out!(self, "{} ", import_kind.as_str());
                }
                let mut has_named_specs = false;
                for (i, spec) in specifiers.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    if let Node::ImportSpecifier(_) = spec {
                        if !has_named_specs {
                            has_named_specs = true;
                            out!(self, "{{");
                        }
                    }
                    spec.visit(ctx, self, Some(Path::new(node, NodeField::specifiers)));
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
                source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
                if let Some(assertions) = assertions {
                    if !assertions.is_empty() {
                        out!(self, " assert {{");
                        for (i, attribute) in assertions.iter().enumerate() {
                            if i > 0 {
                                self.comma();
                            }
                            attribute.visit(
                                ctx,
                                self,
                                Some(Path::new(node, NodeField::assertions)),
                            );
                        }
                        out!(self, "}}");
                    }
                }
            }
            Node::ImportSpecifier(ImportSpecifier {
                metadata: _,
                imported,
                local,
                import_kind,
            }) => {
                if *import_kind != ImportKind::Value {
                    out!(self, "{} ", import_kind.as_str());
                }
                imported.visit(ctx, self, Some(Path::new(node, NodeField::imported)));
                out!(self, " as ");
                local.visit(ctx, self, Some(Path::new(node, NodeField::local)));
            }
            Node::ImportDefaultSpecifier(ImportDefaultSpecifier { metadata: _, local }) => {
                local.visit(ctx, self, Some(Path::new(node, NodeField::local)));
            }
            Node::ImportNamespaceSpecifier(ImportNamespaceSpecifier { metadata: _, local }) => {
                out!(self, "* as ");
                local.visit(ctx, self, Some(Path::new(node, NodeField::local)));
            }
            Node::ImportAttribute(ImportAttribute {
                metadata: _,
                key,
                value,
            }) => {
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                out!(self, ":");
                self.space(ForceSpace::No);
                value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
            }

            Node::ExportNamedDeclaration(ExportNamedDeclaration {
                metadata: _,
                declaration,
                specifiers,
                source,
                export_kind,
            }) => {
                out_token!(self, node, "export ");
                if let Some(declaration) = declaration {
                    declaration.visit(ctx, self, Some(Path::new(node, NodeField::declaration)));
                } else {
                    if *export_kind != ExportKind::Value {
                        out!(self, "{} ", export_kind.as_str());
                    }
                    out!(self, "{{");
                    for (i, spec) in specifiers.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        spec.visit(ctx, self, Some(Path::new(node, NodeField::specifiers)));
                    }
                    out!(self, "}}");
                    if let Some(source) = source {
                        out!(self, " from ");
                        source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
                    }
                }
            }
            Node::ExportSpecifier(ExportSpecifier {
                metadata: _,
                exported,
                local,
            }) => {
                local.visit(ctx, self, Some(Path::new(node, NodeField::local)));
                out!(self, " as ");
                exported.visit(ctx, self, Some(Path::new(node, NodeField::exported)));
            }
            Node::ExportNamespaceSpecifier(ExportNamespaceSpecifier {
                metadata: _,
                exported,
            }) => {
                out!(self, "* as ");
                exported.visit(ctx, self, Some(Path::new(node, NodeField::exported)));
            }
            Node::ExportDefaultDeclaration(ExportDefaultDeclaration {
                metadata: _,
                declaration,
            }) => {
                out_token!(self, node, "export default ");
                declaration.visit(ctx, self, Some(Path::new(node, NodeField::declaration)));
            }
            Node::ExportAllDeclaration(ExportAllDeclaration {
                metadata: _,
                source,
                export_kind,
            }) => {
                out_token!(self, node, "export ");
                if *export_kind != ExportKind::Value {
                    out!(self, "{} ", export_kind.as_str());
                }
                out!(self, "* from ");
                source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
            }

            Node::ObjectPattern(ObjectPattern {
                metadata: _,
                properties,
                type_annotation,
            }) => {
                self.visit_props(ctx, properties, Path::new(node, NodeField::properties));
                if let Some(type_annotation) = type_annotation {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
            }
            Node::ArrayPattern(ArrayPattern {
                metadata: _,
                elements,
                type_annotation,
            }) => {
                out!(self, "[");
                for (i, elem) in elements.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    elem.visit(ctx, self, Some(Path::new(node, NodeField::elements)));
                }
                out!(self, "]");
                if let Some(type_annotation) = type_annotation {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
            }
            Node::RestElement(RestElement {
                metadata: _,
                argument,
            }) => {
                out!(self, "...");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
            }
            Node::AssignmentPattern(AssignmentPattern {
                metadata: _,
                left,
                right,
            }) => {
                left.visit(ctx, self, Some(Path::new(node, NodeField::left)));
                self.space(ForceSpace::No);
                out!(self, "=");
                self.space(ForceSpace::No);
                right.visit(ctx, self, Some(Path::new(node, NodeField::right)));
            }

            Node::JSXIdentifier(JSXIdentifier { metadata: _, name }) => {
                out_token!(self, node, "{}", ctx.str(*name));
            }
            Node::JSXMemberExpression(JSXMemberExpression {
                metadata: _,
                object,
                property,
            }) => {
                object.visit(ctx, self, Some(Path::new(node, NodeField::object)));
                out!(self, ".");
                property.visit(ctx, self, Some(Path::new(node, NodeField::property)));
            }
            Node::JSXNamespacedName(JSXNamespacedName {
                metadata: _,
                namespace,
                name,
            }) => {
                namespace.visit(ctx, self, Some(Path::new(node, NodeField::namespace)));
                out!(self, ":");
                name.visit(ctx, self, Some(Path::new(node, NodeField::name)));
            }
            Node::JSXEmptyExpression(_) => {}
            Node::JSXExpressionContainer(JSXExpressionContainer {
                metadata: _,
                expression,
            }) => {
                out!(self, "{{");
                expression.visit(ctx, self, Some(Path::new(node, NodeField::expression)));
                out!(self, "}}");
            }
            Node::JSXSpreadChild(JSXSpreadChild {
                metadata: _,
                expression,
            }) => {
                out!(self, "{{...");
                expression.visit(ctx, self, Some(Path::new(node, NodeField::expression)));
                out!(self, "}}");
            }
            Node::JSXOpeningElement(JSXOpeningElement {
                metadata: _,
                name,
                attributes,
                self_closing,
            }) => {
                out!(self, "<");
                name.visit(ctx, self, Some(Path::new(node, NodeField::name)));
                for attr in attributes.iter() {
                    self.space(ForceSpace::Yes);
                    attr.visit(ctx, self, Some(Path::new(node, NodeField::attributes)));
                }
                if *self_closing {
                    out!(self, " />");
                } else {
                    out!(self, ">");
                }
            }
            Node::JSXClosingElement(JSXClosingElement { metadata: _, name }) => {
                out!(self, "</");
                name.visit(ctx, self, Some(Path::new(node, NodeField::name)));
                out!(self, ">");
            }
            Node::JSXAttribute(JSXAttribute {
                metadata: _,
                name,
                value,
            }) => {
                name.visit(ctx, self, Some(Path::new(node, NodeField::name)));
                if let Some(value) = value {
                    out!(self, "=");
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
            }
            Node::JSXSpreadAttribute(JSXSpreadAttribute {
                metadata: _,
                argument,
            }) => {
                out!(self, "{{...");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
                out!(self, "}}");
            }
            Node::JSXStringLiteral(JSXStringLiteral {
                metadata: _,
                value: _,
                raw,
            }) => {
                let mut buf = [0u8; 4];
                for char in ctx.str(*raw).chars() {
                    if char == '\n' {
                        self.force_newline_without_indent();
                        continue;
                    }
                    self.write_char(char, &mut buf);
                }
            }
            Node::JSXText(JSXText {
                metadata: _,
                value: _,
                raw,
            }) => {
                let mut buf = [0u8; 4];
                for char in ctx.str(*raw).chars() {
                    if char == '\n' {
                        self.force_newline_without_indent();
                        continue;
                    }
                    self.write_char(char, &mut buf);
                }
            }
            Node::JSXElement(JSXElement {
                metadata: _,
                opening_element,
                children,
                closing_element,
            }) => {
                opening_element.visit(ctx, self, Some(Path::new(node, NodeField::opening_element)));
                if let Some(closing_element) = closing_element {
                    for child in *children {
                        child.visit(ctx, self, Some(Path::new(node, NodeField::children)));
                    }
                    closing_element.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::closing_element)),
                    );
                }
            }
            Node::JSXFragment(JSXFragment {
                metadata: _,
                opening_fragment,
                children,
                closing_fragment,
            }) => {
                opening_fragment.visit(
                    ctx,
                    self,
                    Some(Path::new(node, NodeField::opening_fragment)),
                );
                for child in *children {
                    child.visit(ctx, self, Some(Path::new(node, NodeField::children)));
                }
                closing_fragment.visit(
                    ctx,
                    self,
                    Some(Path::new(node, NodeField::closing_fragment)),
                );
            }
            Node::JSXOpeningFragment(_) => {
                out_token!(self, node, "<>");
            }
            Node::JSXClosingFragment(_) => {
                out_token!(self, node, "</>");
            }

            Node::ExistsTypeAnnotation(_) => {
                out_token!(self, node, "*");
            }
            Node::EmptyTypeAnnotation(_) => {
                out_token!(self, node, "empty");
            }
            Node::StringTypeAnnotation(_) => {
                out_token!(self, node, "string");
            }
            Node::NumberTypeAnnotation(_) => {
                out_token!(self, node, "number");
            }
            Node::StringLiteralTypeAnnotation(StringLiteralTypeAnnotation {
                metadata: _,
                value,
                raw,
            }) => {
                let s = ctx.str_u16(*raw);
                let quote = s[0] as u8 as char;
                out_token!(self, node, "{}", quote);
                self.print_escaped_string_literal(ctx, *value, quote);
                out!(self, "{}", quote);
            }
            Node::NumberLiteralTypeAnnotation(NumberLiteralTypeAnnotation {
                metadata: _,
                value,
                ..
            }) => {
                out_token!(self, node, "{}", convert::number_to_string(*value));
            }
            Node::BigIntLiteralTypeAnnotation(BigIntLiteralTypeAnnotation { metadata: _, raw }) => {
                self.add_segment(node);
                self.write_utf8(ctx.str(*raw));
            }
            Node::BooleanTypeAnnotation(_) => {
                out_token!(self, node, "boolean");
            }
            Node::BooleanLiteralTypeAnnotation(BooleanLiteralTypeAnnotation {
                metadata: _,
                value,
                ..
            }) => {
                out_token!(self, node, "{}", if *value { "true" } else { "false" });
            }
            Node::NullLiteralTypeAnnotation(_) => {
                out_token!(self, node, "null");
            }
            Node::SymbolTypeAnnotation(_) => {
                out_token!(self, node, "symbol");
            }
            Node::AnyTypeAnnotation(_) => {
                out_token!(self, node, "any");
            }
            Node::MixedTypeAnnotation(_) => {
                out_token!(self, node, "mixed");
            }
            Node::VoidTypeAnnotation(_) => {
                out_token!(self, node, "void");
            }
            Node::FunctionTypeAnnotation(FunctionTypeAnnotation {
                metadata: _,
                params,
                this,
                return_type,
                rest,
                type_parameters,
            }) => {
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                out!(self, "(");
                let mut need_comma = false;
                if let Some(this) = this {
                    match this {
                        Node::FunctionTypeParam(FunctionTypeParam {
                            metadata: _,
                            type_annotation,
                            ..
                        }) => {
                            out!(self, "this:");
                            self.space(ForceSpace::No);
                            type_annotation.visit(
                                ctx,
                                self,
                                Some(Path::new(node, NodeField::type_annotation)),
                            );
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                    need_comma = true;
                }
                for param in params.iter() {
                    if need_comma {
                        self.comma();
                    }
                    param.visit(ctx, self, Some(Path::new(node, NodeField::param)));
                    need_comma = true;
                }
                if let Some(rest) = rest {
                    if need_comma {
                        self.comma();
                    }
                    out!(self, "...");
                    rest.visit(ctx, self, Some(Path::new(node, NodeField::rest)));
                }
                out!(self, ")");
                if self.opt.pretty == Pretty::Yes {
                    out!(self, " => ");
                } else {
                    out!(self, "=>");
                }
                return_type.visit(ctx, self, Some(Path::new(node, NodeField::return_type)));
            }
            Node::FunctionTypeParam(FunctionTypeParam {
                metadata: _,
                name,
                type_annotation,
                optional,
            }) => {
                if let Some(name) = name {
                    name.visit(ctx, self, Some(Path::new(node, NodeField::name)));
                    if *optional {
                        out!(self, "?");
                    }
                    out!(self, ":");
                    self.space(ForceSpace::No);
                }
                type_annotation.visit(ctx, self, Some(Path::new(node, NodeField::type_annotation)));
            }
            Node::NullableTypeAnnotation(NullableTypeAnnotation {
                metadata: _,
                type_annotation,
            }) => {
                out!(self, "?");
                self.print_child(
                    ctx,
                    Some(type_annotation),
                    Path::new(node, NodeField::type_annotation),
                    ChildPos::Right,
                );
            }
            Node::QualifiedTypeIdentifier(QualifiedTypeIdentifier {
                metadata: _,
                qualification,
                id,
            }) => {
                qualification.visit(ctx, self, Some(Path::new(node, NodeField::qualification)));
                out!(self, ".");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
            }
            Node::TypeofTypeAnnotation(TypeofTypeAnnotation {
                metadata: _,
                argument,
            }) => {
                out!(self, "typeof ");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
            }
            Node::TupleTypeAnnotation(TupleTypeAnnotation { metadata: _, types }) => {
                out!(self, "[");
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    ty.visit(ctx, self, Some(Path::new(node, NodeField::types)));
                }
                out!(self, "]");
            }
            Node::ArrayTypeAnnotation(ArrayTypeAnnotation {
                metadata: _,
                element_type,
            }) => {
                element_type.visit(ctx, self, Some(Path::new(node, NodeField::element_type)));
                out!(self, "[]");
            }
            Node::UnionTypeAnnotation(UnionTypeAnnotation { metadata: _, types }) => {
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.space(ForceSpace::No);
                        out!(self, "|");
                        self.space(ForceSpace::No);
                    }
                    self.print_child(
                        ctx,
                        Some(ty),
                        Path::new(node, NodeField::types),
                        ChildPos::Anywhere,
                    );
                }
            }
            Node::IntersectionTypeAnnotation(IntersectionTypeAnnotation { metadata: _, types }) => {
                for (i, ty) in types.iter().enumerate() {
                    if i > 0 {
                        self.space(ForceSpace::No);
                        out!(self, "&");
                        self.space(ForceSpace::No);
                    }
                    self.print_child(
                        ctx,
                        Some(ty),
                        Path::new(node, NodeField::types),
                        ChildPos::Anywhere,
                    );
                }
            }
            Node::GenericTypeAnnotation(GenericTypeAnnotation {
                metadata: _,
                id,
                type_parameters,
            }) => {
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
            }
            Node::IndexedAccessType(IndexedAccessType {
                metadata: _,
                object_type,
                index_type,
            }) => {
                object_type.visit(ctx, self, Some(Path::new(node, NodeField::object_type)));
                out!(self, "[");
                index_type.visit(ctx, self, Some(Path::new(node, NodeField::index_type)));
                out!(self, "]");
            }
            Node::OptionalIndexedAccessType(OptionalIndexedAccessType {
                metadata: _,
                object_type,
                index_type,
                optional,
            }) => {
                object_type.visit(ctx, self, Some(Path::new(node, NodeField::object_type)));
                out!(self, "{}[", if *optional { "?." } else { "" });
                index_type.visit(ctx, self, Some(Path::new(node, NodeField::index_type)));
                out!(self, "]");
            }
            Node::InterfaceTypeAnnotation(InterfaceTypeAnnotation {
                metadata: _,
                extends,
                body,
            }) => {
                out!(self, "interface");
                if !extends.is_empty() {
                    out!(self, " extends ");
                    for (i, extend) in extends.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        extend.visit(ctx, self, Some(Path::new(node, NodeField::extends)));
                    }
                } else {
                    self.space(ForceSpace::No);
                }
                if let Some(body) = body {
                    body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
                }
            }

            Node::TypeAlias(TypeAlias {
                metadata: _,
                id,
                type_parameters,
                right,
            })
            | Node::DeclareTypeAlias(DeclareTypeAlias {
                metadata: _,
                id,
                type_parameters,
                right,
            }) => {
                if matches!(&node, Node::DeclareTypeAlias(_)) {
                    out_token!(self, node, "declare type ");
                } else {
                    out_token!(self, node, "type ");
                }
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                if self.opt.pretty == Pretty::Yes {
                    out!(self, " = ");
                } else {
                    out!(self, "=");
                }
                right.visit(ctx, self, Some(Path::new(node, NodeField::right)));
            }
            Node::OpaqueType(OpaqueType {
                metadata: _,
                id,
                type_parameters,
                impltype,
                supertype,
            }) => {
                out_token!(self, node, "opaque type ");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                if let Some(supertype) = supertype {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    supertype.visit(ctx, self, Some(Path::new(node, NodeField::supertype)));
                }
                if self.opt.pretty == Pretty::Yes {
                    out!(self, " = ");
                } else {
                    out!(self, "=");
                }
                impltype.visit(ctx, self, Some(Path::new(node, NodeField::impltype)));
            }
            Node::InterfaceDeclaration(InterfaceDeclaration {
                metadata: _,
                id,
                type_parameters,
                extends,
                body,
            })
            | Node::DeclareInterface(DeclareInterface {
                metadata: _,
                id,
                type_parameters,
                extends,
                body,
            }) => {
                self.visit_interface(
                    ctx,
                    if matches!(node, Node::InterfaceDeclaration(_)) {
                        "interface"
                    } else {
                        "declare interface"
                    },
                    *id,
                    *type_parameters,
                    extends,
                    *body,
                    node,
                );
            }
            Node::DeclareOpaqueType(DeclareOpaqueType {
                metadata: _,
                id,
                type_parameters,
                impltype,
                supertype,
            }) => {
                if matches!(path,
                    Some(path) if !matches!(path.parent, Node::DeclareExportDeclaration(_)))
                    || path.is_none()
                {
                    out!(self, "declare ");
                }
                out_token!(self, node, "opaque type ");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                if let Some(supertype) = supertype {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    supertype.visit(ctx, self, Some(Path::new(node, NodeField::supertype)));
                }
                if let Some(impltype) = impltype {
                    if self.opt.pretty == Pretty::Yes {
                        out!(self, " = ");
                    } else {
                        out!(self, "=");
                    }
                    impltype.visit(ctx, self, Some(Path::new(node, NodeField::impltype)));
                }
            }
            Node::DeclareClass(DeclareClass {
                metadata: _,
                id,
                type_parameters,
                extends,
                implements,
                mixins,
                body,
            }) => {
                match path {
                    Some(path) if !matches!(path.parent, Node::DeclareExportDeclaration(_)) => {
                        out!(self, "declare ");
                    }
                    None => {
                        out!(self, "declare ");
                    }
                    _ => {}
                };
                out_token!(self, node, "class ");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
                if !extends.is_empty() {
                    out!(self, " extends ");
                    for (i, extend) in extends.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        extend.visit(ctx, self, Some(Path::new(node, NodeField::extends)));
                    }
                }
                if !mixins.is_empty() {
                    out!(self, " mixins ");
                    for (i, mixin) in mixins.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        mixin.visit(ctx, self, Some(Path::new(node, NodeField::mixins)));
                    }
                }
                if !implements.is_empty() {
                    out!(self, " implements ");
                    for (i, implement) in implements.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        implement.visit(ctx, self, Some(Path::new(node, NodeField::implements)));
                    }
                }
                self.space(ForceSpace::No);
                body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
            }
            Node::DeclareFunction(DeclareFunction {
                metadata: _,
                id,
                predicate,
            }) => {
                if matches!(path,
                    Some(path) if !matches!(path.parent, Node::DeclareExportDeclaration(_)))
                    || path.is_none()
                {
                    out_token!(self, node, "declare function ");
                } else {
                    out_token!(self, node, "function ");
                }
                // This AST type uses the Identifier/TypeAnnotation
                // pairing to put a name on a function header-looking construct,
                // so we have to do some deep matching to get it to come out right.
                match id {
                    Node::Identifier(Identifier {
                        metadata: _,
                        name,
                        type_annotation,
                        ..
                    }) => {
                        out!(self, "{}", &ctx.str(*name));
                        match type_annotation {
                            Some(Node::TypeAnnotation(TypeAnnotation {
                                metadata: _,
                                type_annotation:
                                    Node::FunctionTypeAnnotation(FunctionTypeAnnotation {
                                        metadata: _,
                                        params,
                                        this,
                                        return_type,
                                        rest,
                                        type_parameters,
                                    }),
                            })) => {
                                self.visit_func_type_params(
                                    ctx,
                                    params,
                                    *this,
                                    *rest,
                                    *type_parameters,
                                    node,
                                );
                                out!(self, ":");
                                self.space(ForceSpace::No);
                                return_type.visit(
                                    ctx,
                                    self,
                                    Some(Path::new(node, NodeField::return_type)),
                                );
                            }
                            _ => {
                                unimplemented!("Malformed AST: Need to handle error");
                            }
                        }
                        if let Some(predicate) = predicate {
                            self.space(ForceSpace::No);
                            predicate.visit(ctx, self, Some(Path::new(node, NodeField::predicate)));
                        }
                    }
                    _ => {
                        unimplemented!("Malformed AST: Need to handle error");
                    }
                }
            }
            Node::DeclareVariable(DeclareVariable { metadata: _, id }) => {
                if let Some(path) = path {
                    if !matches!(path.parent, Node::DeclareExportDeclaration(_)) {
                        out!(self, "declare ");
                    }
                    out!(self, "var ");
                }
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
            }
            Node::DeclareExportDeclaration(DeclareExportDeclaration {
                metadata: _,
                declaration,
                specifiers,
                source,
                default,
            }) => {
                out_token!(self, node, "declare export ");
                if *default {
                    out!(self, "default ");
                }
                if let Some(declaration) = declaration {
                    declaration.visit(ctx, self, Some(Path::new(node, NodeField::declaration)));
                } else {
                    out!(self, "{{");
                    for (i, spec) in specifiers.iter().enumerate() {
                        if i > 0 {
                            self.comma();
                        }
                        spec.visit(ctx, self, Some(Path::new(node, NodeField::specifiers)));
                    }
                    out!(self, "}}");
                    if let Some(source) = source {
                        out!(self, " from ");
                        source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
                    }
                }
            }
            Node::DeclareExportAllDeclaration(DeclareExportAllDeclaration {
                metadata: _,
                source,
            }) => {
                out_token!(self, node, "declare export * from ");
                source.visit(ctx, self, Some(Path::new(node, NodeField::source)));
            }
            Node::DeclareModule(DeclareModule {
                metadata: _,
                id,
                body,
                ..
            }) => {
                out!(self, "declare module ");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                self.space(ForceSpace::No);
                body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
            }
            Node::DeclareModuleExports(DeclareModuleExports {
                metadata: _,
                type_annotation,
            }) => {
                out!(self, "declare module.exports:");
                self.space(ForceSpace::No);
                type_annotation.visit(ctx, self, Some(Path::new(node, NodeField::type_annotation)));
            }

            Node::InterfaceExtends(InterfaceExtends {
                metadata: _,
                id,
                type_parameters,
            })
            | Node::ClassImplements(ClassImplements {
                metadata: _,
                id,
                type_parameters,
            }) => {
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                if let Some(type_parameters) = type_parameters {
                    type_parameters.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_parameters)),
                    );
                }
            }

            Node::TypeAnnotation(TypeAnnotation {
                metadata: _,
                type_annotation,
            }) => {
                type_annotation.visit(ctx, self, Some(Path::new(node, NodeField::type_annotation)));
            }
            Node::ObjectTypeAnnotation(ObjectTypeAnnotation {
                metadata: _,
                properties,
                indexers,
                call_properties,
                internal_slots,
                inexact,
                exact,
            }) => {
                out!(self, "{}", if *exact { "{|" } else { "{" });
                self.inc_indent();
                self.newline();

                let mut need_comma = false;

                for prop in *properties {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(ctx, self, Some(Path::new(node, NodeField::properties)));
                    self.newline();
                    need_comma = true;
                }
                for prop in *indexers {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(ctx, self, Some(Path::new(node, NodeField::indexers)));
                    self.newline();
                    need_comma = true;
                }
                for prop in *call_properties {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(ctx, self, Some(Path::new(node, NodeField::call_properties)));
                    self.newline();
                    need_comma = true;
                }
                for prop in *internal_slots {
                    if need_comma {
                        self.comma();
                    }
                    prop.visit(ctx, self, Some(Path::new(node, NodeField::internal_slots)));
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
            Node::ObjectTypeProperty(ObjectTypeProperty {
                metadata: _,
                key,
                value,
                method,
                optional,
                is_static,
                proto,
                variance,
                ..
            }) => {
                if let Some(variance) = variance {
                    variance.visit(ctx, self, Some(Path::new(node, NodeField::variance)));
                }
                if *is_static {
                    out!(self, "static ");
                }
                if *proto {
                    out!(self, "proto ");
                }
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                if *optional {
                    out!(self, "?");
                }
                if *method {
                    match value {
                        Node::FunctionTypeAnnotation(FunctionTypeAnnotation {
                            metadata: _,
                            params,
                            this,
                            return_type,
                            rest,
                            type_parameters,
                        }) => {
                            self.visit_func_type_params(
                                ctx,
                                params,
                                *this,
                                *rest,
                                *type_parameters,
                                node,
                            );
                            out!(self, ":");
                            self.space(ForceSpace::No);
                            return_type.visit(
                                ctx,
                                self,
                                Some(Path::new(node, NodeField::return_type)),
                            );
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
            }
            Node::ObjectTypeSpreadProperty(ObjectTypeSpreadProperty {
                metadata: _,
                argument,
            }) => {
                out!(self, "...");
                argument.visit(ctx, self, Some(Path::new(node, NodeField::argument)));
            }
            Node::ObjectTypeInternalSlot(ObjectTypeInternalSlot {
                metadata: _,
                id,
                value,
                optional,
                is_static,
                method,
            }) => {
                if *is_static {
                    out!(self, "static ");
                }
                out!(self, "[[");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                out!(self, "]]");
                if *optional {
                    out!(self, "?");
                }
                if *method {
                    match value {
                        Node::FunctionTypeAnnotation(FunctionTypeAnnotation {
                            metadata: _,
                            params,
                            this,
                            return_type,
                            rest,
                            type_parameters,
                        }) => {
                            self.visit_func_type_params(
                                ctx,
                                params,
                                *this,
                                *rest,
                                *type_parameters,
                                node,
                            );
                            out!(self, ":");
                            self.space(ForceSpace::No);
                            return_type.visit(
                                ctx,
                                self,
                                Some(Path::new(node, NodeField::return_type)),
                            );
                        }
                        _ => {
                            unimplemented!("Malformed AST: Need to handle error");
                        }
                    }
                } else {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                }
            }
            Node::ObjectTypeCallProperty(ObjectTypeCallProperty {
                metadata: _,
                value,
                is_static,
            }) => {
                if *is_static {
                    out!(self, "static ");
                }
                match value {
                    Node::FunctionTypeAnnotation(FunctionTypeAnnotation {
                        metadata: _,
                        params,
                        this,
                        return_type,
                        rest,
                        type_parameters,
                    }) => {
                        self.visit_func_type_params(
                            ctx,
                            params,
                            *this,
                            *rest,
                            *type_parameters,
                            node,
                        );
                        out!(self, ":");
                        self.space(ForceSpace::No);
                        return_type.visit(ctx, self, Some(Path::new(node, NodeField::return_type)));
                    }
                    _ => {
                        unimplemented!("Malformed AST: Need to handle error");
                    }
                }
            }
            Node::ObjectTypeIndexer(ObjectTypeIndexer {
                metadata: _,
                id,
                key,
                value,
                is_static,
                variance,
            }) => {
                if *is_static {
                    out!(self, "static ");
                }
                if let Some(variance) = variance {
                    variance.visit(ctx, self, Some(Path::new(node, NodeField::variance)));
                }
                out!(self, "[");
                if let Some(id) = id {
                    id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                    out!(self, ":");
                    self.space(ForceSpace::No);
                }
                key.visit(ctx, self, Some(Path::new(node, NodeField::key)));
                out!(self, "]");
                out!(self, ":");
                self.space(ForceSpace::No);
                value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
            }
            Node::Variance(Variance { metadata: _, kind }) => {
                out_token!(
                    self,
                    node,
                    "{}",
                    match ctx.str(*kind) {
                        "plus" => "+",
                        "minus" => "-",
                        _ => unimplemented!("Malformed variance"),
                    }
                )
            }

            Node::TypeParameterDeclaration(TypeParameterDeclaration {
                metadata: _,
                params,
            })
            | Node::TypeParameterInstantiation(TypeParameterInstantiation {
                metadata: _,
                params,
            }) => {
                out!(self, "<");
                for (i, param) in params.iter().enumerate() {
                    if i > 0 {
                        self.comma();
                    }
                    param.visit(ctx, self, Some(Path::new(node, NodeField::params)));
                }
                out!(self, ">");
            }
            Node::TypeParameter(TypeParameter {
                metadata: _,
                name,
                bound,
                variance,
                default,
            }) => {
                if let Some(variance) = variance {
                    variance.visit(ctx, self, Some(Path::new(node, NodeField::variance)));
                }
                out!(self, "{}", ctx.str(*name));
                if let Some(bound) = bound {
                    out!(self, ":");
                    self.space(ForceSpace::No);
                    bound.visit(ctx, self, Some(Path::new(node, NodeField::bound)));
                }
                if let Some(default) = default {
                    out!(self, "=");
                    self.space(ForceSpace::No);
                    default.visit(ctx, self, Some(Path::new(node, NodeField::default)));
                }
            }
            Node::TypeCastExpression(TypeCastExpression {
                metadata: _,
                expression,
                type_annotation,
            }) => {
                // Type casts are required to have pathheses.
                out!(self, "(");
                self.print_child(
                    ctx,
                    Some(*expression),
                    Path::new(node, NodeField::expression),
                    ChildPos::Left,
                );
                out!(self, ":");
                self.space(ForceSpace::No);
                self.print_child(
                    ctx,
                    Some(*type_annotation),
                    Path::new(node, NodeField::type_annotation),
                    ChildPos::Right,
                );
                out!(self, ")");
            }
            Node::InferredPredicate(_) => {
                out_token!(self, node, "%checks");
            }
            Node::DeclaredPredicate(DeclaredPredicate { metadata: _, value }) => {
                out_token!(self, node, "%checks(");
                value.visit(ctx, self, Some(Path::new(node, NodeField::value)));
                out!(self, ")");
            }

            Node::EnumDeclaration(EnumDeclaration {
                metadata: _,
                id,
                body,
            }) => {
                out_token!(self, node, "enum ");
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
            }
            Node::EnumStringBody(EnumStringBody {
                metadata: _,
                members,
                explicit_type,
                has_unknown_members,
            }) => {
                self.visit_enum_body(
                    ctx,
                    "string",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            Node::EnumNumberBody(EnumNumberBody {
                metadata: _,
                members,
                explicit_type,
                has_unknown_members,
            }) => {
                self.visit_enum_body(
                    ctx,
                    "number",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            Node::EnumBooleanBody(EnumBooleanBody {
                metadata: _,
                members,
                explicit_type,
                has_unknown_members,
            }) => {
                self.visit_enum_body(
                    ctx,
                    "boolean",
                    members,
                    *explicit_type,
                    *has_unknown_members,
                    node,
                );
            }
            Node::EnumSymbolBody(EnumSymbolBody {
                metadata: _,
                members,
                has_unknown_members,
            }) => {
                self.visit_enum_body(ctx, "symbol", members, true, *has_unknown_members, node);
            }
            Node::EnumDefaultedMember(EnumDefaultedMember { metadata: _, id }) => {
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
            }
            Node::EnumStringMember(EnumStringMember {
                metadata: _,
                id,
                init,
            })
            | Node::EnumNumberMember(EnumNumberMember {
                metadata: _,
                id,
                init,
            })
            | Node::EnumBooleanMember(EnumBooleanMember {
                metadata: _,
                id,
                init,
            }) => {
                id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
                out!(
                    self,
                    "{}",
                    match self.opt.pretty {
                        Pretty::Yes => " = ",
                        Pretty::No => "=",
                    }
                );
                init.visit(ctx, self, Some(Path::new(node, NodeField::init)));
            }

            _ => {
                unimplemented!("Cannot generate node kind: {}", node.name());
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
            match self.opt.pretty {
                Pretty::No => ",",
                Pretty::Yes => ", ",
            }
        )
    }

    /// Print a ' ' if forced by ForceSpace::Yes or pretty mode.
    fn space(&mut self, force: ForceSpace) {
        if self.opt.pretty == Pretty::Yes || force == ForceSpace::Yes {
            out!(self, " ");
        }
    }

    /// Print a newline and indent if pretty.
    fn newline(&mut self) {
        if self.opt.pretty == Pretty::Yes {
            self.force_newline();
        }
    }

    /// Print a newline and indent.
    fn force_newline(&mut self) {
        self.force_newline_without_indent();
        out!(self, "{:indent$}", "", indent = self.indent as usize);
    }

    /// Print a newline without any indent after.
    fn force_newline_without_indent(&mut self) {
        if self.error.is_none() {
            if let Err(e) = self.out.write(&[b'\n']) {
                self.error = Some(e);
            }
        }
        self.position.line += 1;
        self.position.col = 1;
    }

    /// Print the child of a `path` node at the position `child_pos`.
    fn print_child<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        child: Option<&'gc Node<'gc>>,
        path: Path<'gc>,
        child_pos: ChildPos,
    ) {
        if let Some(child) = child {
            self.print_parens(
                ctx,
                child,
                path,
                self.need_parens(ctx, path, child, child_pos),
            );
        }
    }

    /// Print one expression in a sequence separated by comma. It needs parens
    /// if its precedence is <= comma.
    fn print_comma_expression<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        child: &'gc Node<'gc>,
        path: Path<'gc>,
    ) {
        self.print_parens(
            ctx,
            child,
            path,
            NeedParens::from(self.get_precedence(child).0 <= precedence::SEQ),
        )
    }

    fn print_parens<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        child: &'gc Node<'gc>,
        path: Path<'gc>,
        need_parens: NeedParens,
    ) {
        if need_parens == NeedParens::Yes {
            out!(self, "(");
        } else if need_parens == NeedParens::Space {
            out!(self, " ");
        }
        child.visit(ctx, self, Some(path));
        if need_parens == NeedParens::Yes {
            out!(self, ")");
        }
    }

    fn print_escaped_string_literal<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        value: NodeString,
        esc: char,
    ) {
        let str = ctx.str_u16(value);
        for &c in str {
            if c <= u8::MAX as u16 {
                match char::from(c as u8) {
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
            }
            if c == esc as u16 {
                out!(self, "\\");
            }
            if (0x20..=0x7f).contains(&c) {
                // Printable.
                out!(self, "{}", char::from(c as u8));
            } else {
                out!(self, "\\u{:04x}", c);
            }
        }
    }

    fn visit_props<'gc>(&mut self, ctx: &'gc GCLock, props: &'gc NodeList<'gc>, path: Path<'gc>) {
        out!(self, "{{");
        for (i, prop) in props.iter().enumerate() {
            if i > 0 {
                self.comma();
            }
            prop.visit(ctx, self, Some(path));
        }
        out!(self, "}}");
    }

    #[allow(clippy::too_many_arguments)]
    fn visit_func_params_body<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        params: &'gc NodeList<'gc>,
        type_parameters: Option<&'gc Node<'gc>>,
        return_type: Option<&'gc Node<'gc>>,
        predicate: Option<&'gc Node<'gc>>,
        body: &'gc Node<'gc>,
        node: &'gc Node<'gc>,
    ) {
        if let Some(type_parameters) = type_parameters {
            type_parameters.visit(ctx, self, Some(Path::new(node, NodeField::type_parameters)));
        }
        out!(self, "(");
        for (i, param) in params.iter().enumerate() {
            if i > 0 {
                self.comma();
            }
            param.visit(ctx, self, Some(Path::new(node, NodeField::param)));
        }
        out!(self, ")");
        if return_type.is_some() || predicate.is_some() {
            out!(self, ":");
        }
        if let Some(return_type) = return_type {
            self.space(ForceSpace::No);
            return_type.visit(ctx, self, Some(Path::new(node, NodeField::return_type)));
        }
        if let Some(predicate) = predicate {
            self.space(ForceSpace::Yes);
            predicate.visit(ctx, self, Some(Path::new(node, NodeField::predicate)));
        }
        self.space(ForceSpace::No);
        body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
    }

    fn visit_func_type_params<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        params: &'gc NodeList<'gc>,
        this: Option<&'gc Node<'gc>>,
        rest: Option<&'gc Node<'gc>>,
        type_parameters: Option<&'gc Node<'gc>>,
        node: &'gc Node<'gc>,
    ) {
        if let Some(type_parameters) = type_parameters {
            type_parameters.visit(ctx, self, Some(Path::new(node, NodeField::type_parameters)));
        }
        out!(self, "(");
        let mut need_comma = false;
        if let Some(this) = this {
            match this {
                Node::FunctionTypeParam(FunctionTypeParam {
                    metadata: _,
                    type_annotation,
                    ..
                }) => {
                    out!(self, "this:");
                    self.space(ForceSpace::No);
                    type_annotation.visit(
                        ctx,
                        self,
                        Some(Path::new(node, NodeField::type_annotation)),
                    );
                }
                _ => {
                    unimplemented!("Malformed AST: Need to handle error");
                }
            }
            need_comma = true;
        }
        for param in params.iter() {
            if need_comma {
                self.comma();
            }
            param.visit(ctx, self, Some(Path::new(node, NodeField::param)));
            need_comma = true;
        }
        if let Some(rest) = rest {
            if need_comma {
                self.comma();
            }
            out!(self, "...");
            rest.visit(ctx, self, Some(Path::new(node, NodeField::rest)));
        }
        out!(self, ")");
    }

    #[allow(clippy::too_many_arguments)]
    fn visit_interface<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        decl: &str,
        id: &'gc Node<'gc>,
        type_parameters: Option<&'gc Node<'gc>>,
        extends: &'gc NodeList<'gc>,
        body: &'gc Node<'gc>,
        node: &'gc Node<'gc>,
    ) {
        out!(self, "{} ", decl);
        id.visit(ctx, self, Some(Path::new(node, NodeField::id)));
        if let Some(type_parameters) = type_parameters {
            type_parameters.visit(ctx, self, Some(Path::new(node, NodeField::type_parameters)));
        }
        self.space(ForceSpace::No);
        if !extends.is_empty() {
            out!(self, " extends ");
            for (i, extend) in extends.iter().enumerate() {
                if i > 0 {
                    self.comma();
                }
                extend.visit(ctx, self, Some(Path::new(node, NodeField::extends)));
            }
            self.space(ForceSpace::No);
        }
        body.visit(ctx, self, Some(Path::new(node, NodeField::body)));
    }

    /// Generate the body of a Flow enum with type `kind`.
    fn visit_enum_body<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        kind: &str,
        members: &'gc NodeList<'gc>,
        explicit_type: bool,
        has_unknown_members: bool,
        node: &'gc Node<'gc>,
    ) {
        if explicit_type {
            out!(self, " of {}", kind);
        }
        self.space(ForceSpace::No);
        out!(self, "{{");
        self.inc_indent();
        self.newline();

        for (i, member) in members.iter().enumerate() {
            if i > 0 {
                self.comma();
                self.newline();
            }
            member.visit(ctx, self, Some(Path::new(node, NodeField::members)));
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
    fn visit_stmt_or_block<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        node: &'gc Node<'gc>,
        force_block: ForceBlock,
        path: Path<'gc>,
    ) -> bool {
        if let Node::BlockStatement(BlockStatement { metadata: _, body }) = &node {
            if body.is_empty() {
                self.space(ForceSpace::No);
                out!(self, "{{}}");
                return true;
            }
            self.space(ForceSpace::No);
            out!(self, "{{");
            self.inc_indent();
            self.newline();
            self.visit_stmt_list(ctx, body, Path::new(node, NodeField::body));
            self.dec_indent();
            self.newline();
            out!(self, "}}");
            return true;
        }
        if force_block == ForceBlock::Yes {
            self.space(ForceSpace::No);
            out!(self, "{{");
            self.inc_indent();
            self.newline();
            self.visit_stmt_in_block(ctx, node, path);
            self.dec_indent();
            self.newline();
            out!(self, "}}");
            self.newline();
            true
        } else {
            self.inc_indent();
            self.newline();
            self.visit_stmt_in_block(ctx, node, path);
            self.dec_indent();
            false
        }
    }

    fn visit_stmt_list<'gc>(&mut self, ctx: &'gc GCLock, list: &NodeList<'gc>, path: Path<'gc>) {
        for (i, stmt) in list.iter().enumerate() {
            if i > 0 {
                self.newline();
            }
            self.visit_stmt_in_block(ctx, stmt, path);
        }
    }

    fn visit_stmt_in_block<'gc>(
        &mut self,
        ctx: &'gc GCLock,
        stmt: &'gc Node<'gc>,
        path: Path<'gc>,
    ) {
        stmt.visit(ctx, self, Some(path));
        if !stmt_skip_semi(ctx, Some(stmt)) {
            out!(self, ";");
        }
    }

    /// Return the precedence and associativity of `node`.
    fn get_precedence(&self, node: &Node<'_>) -> (precedence::Precedence, Assoc) {
        // Precedence order taken from
        // https://github.com/facebook/flow/blob/master/src/parser_utils/output/js_layout_generator.ml
        use precedence::*;
        match &node {
            Node::Identifier(_)
            | Node::NullLiteral(_)
            | Node::BooleanLiteral(_)
            | Node::StringLiteral(_)
            | Node::NumericLiteral(_)
            | Node::RegExpLiteral(_)
            | Node::ThisExpression(_)
            | Node::Super(_)
            | Node::ArrayExpression(_)
            | Node::ObjectExpression(_)
            | Node::ObjectPattern(_)
            | Node::FunctionExpression(_)
            | Node::ClassExpression(_)
            | Node::TemplateLiteral(_)
            | Node::JSXElement(_)
            | Node::JSXFragment(_)
            | Node::TypeCastExpression(_) => (PRIMARY, Assoc::Ltr),
            Node::MemberExpression(_)
            | Node::OptionalMemberExpression(_)
            | Node::MetaProperty(_)
            | Node::CallExpression(_)
            | Node::OptionalCallExpression(_) => (MEMBER, Assoc::Ltr),
            Node::NewExpression(NewExpression {
                metadata: _,
                arguments,
                ..
            }) => {
                // `new foo()` has higher precedence than `new foo`. In pretty mode we
                // always append the `()`, but otherwise we must check the number of args.
                if self.opt.pretty == Pretty::Yes || !arguments.is_empty() {
                    (MEMBER, Assoc::Ltr)
                } else {
                    (NEW_NO_ARGS, Assoc::Ltr)
                }
            }
            Node::TaggedTemplateExpression(_) | Node::ImportExpression(_) => {
                (TAGGED_TEMPLATE, Assoc::Ltr)
            }
            Node::UpdateExpression(UpdateExpression {
                metadata: _,
                prefix,
                ..
            }) => {
                if *prefix {
                    (POST_UPDATE, Assoc::Ltr)
                } else {
                    (UNARY, Assoc::Rtl)
                }
            }
            Node::UnaryExpression(_) => (UNARY, Assoc::Rtl),
            Node::BinaryExpression(BinaryExpression {
                metadata: _,
                operator,
                ..
            }) => (get_binary_precedence(*operator), Assoc::Ltr),
            Node::LogicalExpression(LogicalExpression {
                metadata: _,
                operator,
                ..
            }) => (get_logical_precedence(*operator), Assoc::Ltr),
            Node::ConditionalExpression(_) => (COND, Assoc::Rtl),
            Node::AssignmentExpression(_) => (ASSIGN, Assoc::Rtl),
            Node::ArrowFunctionExpression(_) => (ARROW, Assoc::Ltr),
            Node::YieldExpression(_) => (YIELD, Assoc::Ltr),
            Node::SequenceExpression(_) => (SEQ, Assoc::Rtl),

            Node::ExistsTypeAnnotation(_)
            | Node::EmptyTypeAnnotation(_)
            | Node::StringTypeAnnotation(_)
            | Node::NumberTypeAnnotation(_)
            | Node::StringLiteralTypeAnnotation(_)
            | Node::NumberLiteralTypeAnnotation(_)
            | Node::BooleanTypeAnnotation(_)
            | Node::BooleanLiteralTypeAnnotation(_)
            | Node::NullLiteralTypeAnnotation(_)
            | Node::SymbolTypeAnnotation(_)
            | Node::AnyTypeAnnotation(_)
            | Node::MixedTypeAnnotation(_)
            | Node::VoidTypeAnnotation(_) => (PRIMARY, Assoc::Ltr),
            Node::NullableTypeAnnotation(_) => (UNARY, Assoc::Ltr),
            Node::UnionTypeAnnotation(_) => (UNION_TYPE, Assoc::Ltr),
            Node::IntersectionTypeAnnotation(_) => (INTERSECTION_TYPE, Assoc::Ltr),

            _ => (ALWAYS_PAREN, Assoc::Ltr),
        }
    }

    /// Return whether pathheses are needed around the `child` node,
    /// which is situated at `child_pos` position in relation to its `path`.
    fn need_parens<'gc>(
        &self,
        ctx: &'gc GCLock,
        path: Path<'gc>,
        child: &'gc Node<'gc>,
        child_pos: ChildPos,
    ) -> NeedParens {
        #[allow(clippy::if_same_then_else)]
        if matches!(path.parent, Node::ArrowFunctionExpression(_)) {
            // (x) => ({x: 10}) needs parens to avoid confusing it with a block and a
            // labelled statement.
            if child_pos == ChildPos::Right
                && self.expr_starts_with(ctx, child, Some(path), |n| {
                    matches!(n, Node::ObjectExpression(_))
                })
            {
                return NeedParens::Yes;
            }
        } else if matches!(path.parent, Node::ForStatement(_)) {
            // for((a in b);..;..) needs parens to avoid confusing it with for(a in b).
            return NeedParens::from(match &child {
                Node::BinaryExpression(BinaryExpression {
                    metadata: _,
                    operator,
                    ..
                }) => *operator == BinaryExpressionOperator::In,
                _ => false,
            });
        } else if matches!(path.parent, Node::NewExpression(_)) {
            // `new(fn())` needs parens to avoid confusing it with `new fn()`.
            // Need to check the entire subtree to ensure there isn't a call anywhere in it,
            // because if there is, it would take precedence and terminate the `new` early.
            // As an example, see the difference between
            // `new(foo().bar)` (which gets `bar` on `foo()`)
            // and
            // `new foo().bar` (which gets `bar` on `new foo()`)
            if child_pos == ChildPos::Left && contains_call(ctx, child) {
                return NeedParens::Yes;
            }
            // It's illegal to place parens around spread arguments.
            if matches!(child, Node::SpreadElement(_)) {
                return NeedParens::No;
            }
        } else if matches!(path.parent, Node::ExpressionStatement(_)) {
            // Expression statement like (function () {} + 1) needs parens.
            return NeedParens::from(self.root_starts_with(ctx, child, |kind| -> bool {
                matches!(
                    kind,
                    Node::FunctionExpression(_)
                        | Node::ClassExpression(_)
                        | Node::ObjectExpression(_)
                        | Node::ObjectPattern(_)
                )
            }));
        } else if (is_unary_op(path.parent, UnaryExpressionOperator::Minus)
            && self.root_starts_with(ctx, child, check_minus))
            || (is_unary_op(path.parent, UnaryExpressionOperator::Plus)
                && self.root_starts_with(ctx, child, check_plus))
            || (child_pos == ChildPos::Right
                && is_binary_op(path.parent, BinaryExpressionOperator::Minus)
                && self.root_starts_with(ctx, child, check_minus))
            || (child_pos == ChildPos::Right
                && is_binary_op(path.parent, BinaryExpressionOperator::Plus)
                && self.root_starts_with(ctx, child, check_plus))
        {
            // -(-x) or -(--x) or -(-5)
            // +(+x) or +(++x)
            // a-(-x) or a-(--x) or a-(-5)
            // a+(+x) or a+(++x)
            return if self.opt.pretty == Pretty::Yes {
                NeedParens::Yes
            } else {
                NeedParens::Space
            };
        } else if matches!(
            path.parent,
            Node::MemberExpression(_) | Node::CallExpression(_)
        ) && matches!(
            child,
            Node::OptionalMemberExpression(_) | Node::OptionalCallExpression(_)
        ) && child_pos == ChildPos::Left
        {
            // When optional chains are terminated by non-optional member/calls,
            // we need the left hand side to be pathhesized.
            // Avoids confusing `(a?.b).c` with `a?.b.c`.
            return NeedParens::Yes;
        } else if (check_and_or(path.parent) && check_nullish(child))
            || (check_nullish(path.parent) && check_and_or(child))
        {
            // Nullish coalescing always requires parens when mixed with any
            // other logical operations.
            return NeedParens::Yes;
        } else if matches!(
            path.parent,
            Node::CallExpression(_) | Node::OptionalCallExpression(_)
        ) && matches!(child, Node::SpreadElement(_))
        {
            // It's illegal to place parens around spread arguments.
            return NeedParens::No;
        } else if matches!(path.parent, Node::AssignmentExpression(_))
            && matches!(child, Node::ObjectPattern(_) | Node::ArrayPattern(_))
            && child_pos == ChildPos::Left
        {
            // Avoid parentheses for destructuring patterns.
            return NeedParens::No;
        }

        let (child_prec, _child_assoc) = self.get_precedence(child);
        if child_prec == precedence::ALWAYS_PAREN {
            return NeedParens::Yes;
        }

        let (path_prec, path_assoc) = self.get_precedence(path.parent);

        if child_prec < path_prec {
            // Child is definitely a danger.
            return NeedParens::Yes;
        }
        if child_prec > path_prec {
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
        NeedParens::from(if path_assoc == Assoc::Rtl {
            child_pos == ChildPos::Left
        } else {
            child_pos == ChildPos::Right
        })
    }

    fn root_starts_with<'gc, F: Fn(&'gc Node<'gc>) -> bool>(
        &self,
        ctx: &'gc GCLock,
        expr: &'gc Node<'gc>,
        pred: F,
    ) -> bool {
        self.expr_starts_with(ctx, expr, None, pred)
    }

    fn expr_starts_with<'gc, F: Fn(&'gc Node<'gc>) -> bool>(
        &self,
        ctx: &'gc GCLock,
        expr: &'gc Node<'gc>,
        path: Option<Path<'gc>>,
        pred: F,
    ) -> bool {
        if let Some(path) = path {
            if self.need_parens(ctx, path, expr, ChildPos::Left) == NeedParens::Yes {
                return false;
            }
        }

        if pred(expr) {
            return true;
        }

        // Ensure the recursive calls are the last things to run,
        // hopefully the compiler makes this into a loop.
        match expr {
            Node::CallExpression(CallExpression {
                metadata: _,
                callee,
                ..
            }) => {
                self.expr_starts_with(ctx, *callee, Some(Path::new(expr, NodeField::callee)), pred)
            }
            Node::OptionalCallExpression(OptionalCallExpression {
                metadata: _,
                callee,
                ..
            }) => {
                self.expr_starts_with(ctx, *callee, Some(Path::new(expr, NodeField::callee)), pred)
            }
            Node::BinaryExpression(BinaryExpression {
                metadata: _, left, ..
            }) => self.expr_starts_with(ctx, *left, Some(Path::new(expr, NodeField::left)), pred),
            Node::LogicalExpression(LogicalExpression {
                metadata: _, left, ..
            }) => self.expr_starts_with(ctx, *left, Some(Path::new(expr, NodeField::left)), pred),
            Node::ConditionalExpression(ConditionalExpression {
                metadata: _, test, ..
            }) => self.expr_starts_with(ctx, *test, Some(Path::new(expr, NodeField::test)), pred),
            Node::AssignmentExpression(AssignmentExpression {
                metadata: _, left, ..
            }) => self.expr_starts_with(ctx, *left, Some(Path::new(expr, NodeField::left)), pred),
            Node::UpdateExpression(UpdateExpression {
                metadata: _,
                prefix,
                argument,
                ..
            }) => {
                !*prefix
                    && self.expr_starts_with(
                        ctx,
                        *argument,
                        Some(Path::new(expr, NodeField::argument)),
                        pred,
                    )
            }
            Node::UnaryExpression(UnaryExpression {
                metadata: _,
                prefix,
                argument,
                ..
            }) => {
                !*prefix
                    && self.expr_starts_with(
                        ctx,
                        *argument,
                        Some(Path::new(expr, NodeField::argument)),
                        pred,
                    )
            }
            Node::MemberExpression(MemberExpression {
                metadata: _,
                object,
                ..
            })
            | Node::OptionalMemberExpression(OptionalMemberExpression {
                metadata: _,
                object,
                ..
            }) => {
                self.expr_starts_with(ctx, *object, Some(Path::new(expr, NodeField::object)), pred)
            }
            Node::TaggedTemplateExpression(TaggedTemplateExpression {
                metadata: _, tag, ..
            }) => self.expr_starts_with(ctx, *tag, Some(Path::new(expr, NodeField::tag)), pred),
            _ => false,
        }
    }

    /// Adds the current location as a segment pointing to the start of `node`.
    fn add_segment(&mut self, node: &Node) {
        // Convert from 1-indexed to 0-indexed as expected by source map.
        // Use `wrapping_sub` in case the line/col are invalid (0) to ensure
        // the overflow goes to `u32::MAX`.
        let new_token = Some(RawToken {
            dst_line: self.position.line.wrapping_sub(1),
            dst_col: self.position.col.wrapping_sub(1),
            src_line: node.range().start.line.wrapping_sub(1),
            src_col: node.range().start.col.wrapping_sub(1),
            src_id: 0,
            name_id: !0,
        });
        self.flush_cur_token();
        self.cur_token = new_token;
    }

    /// Add the `cur_token` to the sourcemap and set `cur_token` to `None`.
    fn flush_cur_token(&mut self) {
        if let Some(cur) = self.cur_token {
            self.sourcemap.add_raw(
                cur.dst_line,
                cur.dst_col,
                cur.src_line,
                cur.src_col,
                if cur.src_id != !0 {
                    Some(cur.src_id)
                } else {
                    None
                },
                if cur.name_id != !0 {
                    Some(cur.name_id)
                } else {
                    None
                },
            );
        }
        self.cur_token = None;
    }

    /// Add an "@" and some information tagging an identifier with its declaration ID.
    fn annotate_identifier<'gc>(&mut self, lock: &'gc GCLock, node: &'gc Node<'gc>) {
        if let Annotation::Sem(sem) = &self.opt.annotation {
            match sem.ident_decl(&NodeRc::from_node(lock, node)) {
                Some(Resolution::Decl(decl_id)) => {
                    match sem.decl(decl_id).kind {
                        DeclKind::Let
                        | DeclKind::Const
                        | DeclKind::Class
                        | DeclKind::Import
                        | DeclKind::ES5Catch
                        | DeclKind::FunctionExprName
                        | DeclKind::ScopedFunction
                        | DeclKind::Var
                        | DeclKind::Parameter => {
                            out!(self, "@D{}", decl_id);
                        }
                        DeclKind::GlobalProperty => {
                            out!(self, "@global");
                        }
                        DeclKind::UndeclaredGlobalProperty => {
                            out!(self, "@uglobal");
                        }
                    };
                }
                Some(Resolution::Unresolvable) => {
                    out!(self, "@unresolvable");
                }
                _ => {}
            }
        };
    }
}

impl<'gc> Visitor<'gc> for GenJS<'_, '_> {
    fn call(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>) {
        self.gen_node(ctx, node, path);
    }
}

fn is_unary_op(node: &Node, op: UnaryExpressionOperator) -> bool {
    match node {
        Node::UnaryExpression(UnaryExpression {
            metadata: _,
            operator,
            ..
        }) => *operator == op,
        _ => false,
    }
}

fn is_update_prefix(node: &Node, op: UpdateExpressionOperator) -> bool {
    match node {
        Node::UpdateExpression(UpdateExpression {
            metadata: _,
            prefix,
            operator,
            ..
        }) => *prefix && *operator == op,
        _ => false,
    }
}

fn is_negative_number(node: &Node) -> bool {
    match node {
        Node::NumericLiteral(NumericLiteral {
            metadata: _, value, ..
        }) => *value < 0f64,
        _ => false,
    }
}

fn is_binary_op(node: &Node, op: BinaryExpressionOperator) -> bool {
    match node {
        Node::BinaryExpression(BinaryExpression {
            metadata: _,
            operator,
            ..
        }) => *operator == op,
        _ => false,
    }
}

fn is_if_without_else(node: &Node) -> bool {
    match node {
        Node::IfStatement(IfStatement {
            metadata: _,
            alternate,
            ..
        }) => alternate.is_none(),
        _ => false,
    }
}

fn check_plus(node: &Node) -> bool {
    is_unary_op(node, UnaryExpressionOperator::Plus)
        || is_update_prefix(node, UpdateExpressionOperator::Increment)
}

fn check_minus(node: &Node) -> bool {
    is_unary_op(node, UnaryExpressionOperator::Minus)
        || is_update_prefix(node, UpdateExpressionOperator::Decrement)
        || is_negative_number(node)
}

fn check_and_or(node: &Node) -> bool {
    matches!(
        node,
        Node::LogicalExpression(LogicalExpression {
            metadata: _,
            operator: LogicalExpressionOperator::And | LogicalExpressionOperator::Or,
            ..
        })
    )
}

fn check_nullish(node: &Node) -> bool {
    matches!(
        node,
        Node::LogicalExpression(LogicalExpression {
            metadata: _,
            operator: LogicalExpressionOperator::NullishCoalesce,
            ..
        })
    )
}

/// Whether to skip the semicolon at the end of `node`.
/// Block statements don't need semicolons at the end, but other statements which contain
/// statements don't need them either.
/// For example:
/// ```js
/// if (x)
///   y();
/// ```
/// The semicolon will be emitted as part of emitting `y()`, which is an `ExpressionStatement`,
/// so the `IfStatement` does not need to emit a semicolon.
fn stmt_skip_semi<'gc>(ctx: &'gc GCLock, node: Option<&'gc Node<'gc>>) -> bool {
    match node {
        Some(node) => match &node {
            Node::BlockStatement(_)
            | Node::FunctionDeclaration(_)
            | Node::WhileStatement(_)
            | Node::ForStatement(_)
            | Node::ForInStatement(_)
            | Node::ForOfStatement(_)
            | Node::IfStatement(_)
            | Node::WithStatement(_) => true,
            Node::InterfaceDeclaration(_)
            | Node::DeclareInterface(_)
            | Node::DeclareClass(_)
            | Node::DeclareModule(_) => true,
            Node::DeclareExportDeclaration(DeclareExportDeclaration {
                declaration,
                source: None,
                ..
            }) => stmt_skip_semi(ctx, *declaration),
            Node::SwitchStatement(_) => true,
            Node::LabeledStatement(LabeledStatement { body, .. }) => {
                stmt_skip_semi(ctx, Some(*body))
            }
            Node::TryStatement(TryStatement {
                metadata: _,
                finalizer,
                handler,
                ..
            }) => stmt_skip_semi(ctx, finalizer.or(*handler)),
            Node::CatchClause(CatchClause { body, .. }) => stmt_skip_semi(ctx, Some(*body)),
            Node::ClassDeclaration(_) => true,
            Node::ExportDefaultDeclaration(ExportDefaultDeclaration {
                metadata: _,
                declaration,
            }) => stmt_skip_semi(ctx, Some(*declaration)),
            Node::ExportNamedDeclaration(ExportNamedDeclaration {
                metadata: _,
                declaration,
                ..
            }) => stmt_skip_semi(ctx, *declaration),
            Node::EnumDeclaration(_) => true,
            _ => false,
        },
        None => false,
    }
}

/// Return true if `node` contains a `CallExpression`.
fn contains_call<'gc>(gc: &'gc GCLock, node: &'gc Node<'gc>) -> bool {
    struct CallFinder {
        found: bool,
    }
    impl<'gc> Visitor<'gc> for CallFinder {
        fn call(&mut self, gc: &'gc GCLock, node: &'gc Node<'gc>, _path: Option<Path<'gc>>) {
            match node {
                Node::CallExpression(_)
                | Node::OptionalCallExpression(OptionalCallExpression {
                    optional: false, ..
                }) => {
                    self.found = true;
                }
                _ => {
                    node.visit_children(gc, self);
                }
            };
        }
    }
    let mut finder = CallFinder { found: false };
    node.visit(gc, &mut finder, None);
    finder.found
}
