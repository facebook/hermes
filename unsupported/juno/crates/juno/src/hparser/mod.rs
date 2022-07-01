/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod convert;
mod generated_cvt;

use crate::ast;
use convert::Converter;
use generated_cvt::cvt_node_ptr;
use hermes::parser::HermesParser;
use hermes::parser::NodePtr;
use hermes::utf::utf8_with_surrogates_to_string_lossy;
use juno_support::source_manager::SourceId;
use juno_support::NullTerminatedBuf;
use std::fmt::Formatter;
use thiserror::Error;

pub use hermes::parser::MagicCommentKind;
pub use hermes::parser::ParserDialect;
pub use hermes::parser::ParserFlags;

pub struct ParsedJS<'a> {
    parser: HermesParser<'a>,
}

impl<'parser> ParsedJS<'parser> {
    /// Parse the source and store an internal representation of the AST and/or a list of diagnostic
    /// messages. If at least one of the messages is an error, there is no AST.
    pub fn parse(flags: ParserFlags, source: &NullTerminatedBuf) -> ParsedJS {
        ParsedJS {
            parser: HermesParser::parse(flags, source),
        }
    }

    /// Return true if there is at least one parser error (implying there is no AST).
    pub fn has_errors(&self) -> bool {
        self.parser.has_errors()
    }

    /// Return the doc block at the top of the file if it exists.
    pub fn get_doc_block(&self) -> Option<&str> {
        self.parser.get_doc_block()
    }

    /// Return the last magic comment of the specified type (each comment overrides the previous
    /// one, so only the last is recorded).
    pub fn magic_comment(&self, kind: MagicCommentKind) -> Option<&str> {
        self.parser.magic_comment(kind)
    }

    /// This function is a temporary hack returning the first error.
    /// It returns (line, column, error_message) of the first error.
    pub fn first_error(&self) -> Option<(ast::SourceLoc, String)> {
        self.parser.first_error_index().map(|index| {
            let msg = &self.parser.messages()[index];
            (
                ast::SourceLoc {
                    line: msg.coord.line as u32,
                    col: msg.coord.offset as u32 + 1,
                },
                utf8_with_surrogates_to_string_lossy(msg.message.as_slice()),
            )
        })
    }

    /// Create and return an external representation of the AST, or None if there were parse errors.
    pub fn to_ast<'gc, 'ast: 'gc>(
        &'parser self,
        ctx: &'gc ast::GCLock<'ast, '_>,
        file_id: SourceId,
    ) -> Option<&'gc ast::Node<'gc>> {
        let mut cvt: Converter<'parser> = Converter::new(&self.parser, file_id);

        match self.parser.root() {
            None => None,
            Some(node) => {
                let ast = convert_ast(&mut cvt, ctx, node);
                if ctx.sm().num_errors() > 0 {
                    None
                } else {
                    Some(ast)
                }
            }
        }
    }
}

fn convert_ast<'parser, 'gc, 'ast: 'gc>(
    cvt: &mut Converter<'parser>,
    ctx: &'gc ast::GCLock<'ast, '_>,
    n: NodePtr,
) -> &'gc ast::Node<'gc> {
    unsafe { cvt_node_ptr(cvt, ctx, n) }
}

/// The first error encountered when parsing.
#[derive(Debug, Error)]
pub struct ParseError {
    pub loc: ast::SourceLoc,
    pub msg: String,
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}:{}:{}", self.loc.line, self.loc.col, self.msg)
    }
}

/// This is a simple function that is intended to be used mostly for testing.
/// It automatically imports the source string into the source manager.
/// When there are errors, it returns only the first error.
pub fn parse_with_flags(
    flags: ParserFlags,
    source: &str,
    ctx: &mut ast::Context,
) -> Result<ast::NodeRc, ParseError> {
    let file_id = ctx
        .sm_mut()
        .add_source("<input>", NullTerminatedBuf::from_str_check(source));
    let buf = ctx.sm().source_buffer_rc(file_id);
    let parsed = ParsedJS::parse(flags, &buf);
    let gc = ast::GCLock::new(ctx);
    if let Some(ast) = parsed.to_ast(&gc, file_id) {
        Ok(ast::NodeRc::from_node(&gc, ast))
    } else {
        match parsed.first_error() {
            Some((loc, msg)) => Err(ParseError { loc, msg }),
            None => Err(ParseError {
                loc: ast::SourceLoc::invalid(),
                msg: "invalid AST produced".into(),
            }),
        }
    }
}

/// This is a simple function that is intended to be used mostly for testing.
/// It automatically imports the source string into the source manager.
/// When there are errors, it returns only the first error.
pub fn parse(ctx: &mut ast::Context, source: &str) -> Result<ast::NodeRc, ParseError> {
    parse_with_flags(Default::default(), source, ctx)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test1() {
        let mut ctx = ast::Context::new();
        parse(&mut ctx, "function foo(p1) { var x = (10 + p1); }").expect("Parse failed");
    }
}
