/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod convert;
mod generated_cvt;

use crate::ast;
use convert::Converter;
use generated_cvt::cvt_node_ptr;
use hermes::parser::{HermesParser, NodePtr};
use hermes::utf::utf8_with_surrogates_to_string_lossy;
use std::fmt::Formatter;
use support::NullTerminatedBuf;
use thiserror::Error;

pub use hermes::parser::{MagicCommentKind, ParserDialect, ParserFlags};

pub struct ParsedJS<'a> {
    parser: HermesParser<'a>,
}

impl ParsedJS<'_> {
    /// Parse the source and store an internal representation of the AST and/or a list of diagnostic
    /// messages. If at least one of the messages is an error, there is no AST.
    /// To avoid copying `source` can optionally be NUL-terminated.
    pub fn parse<'a>(flags: ParserFlags, source: &'a NullTerminatedBuf) -> ParsedJS<'a> {
        ParsedJS {
            parser: HermesParser::parse(flags, source),
        }
    }

    /// Return true if there is at least one parser error (implying there is no AST).
    pub fn has_errors(&self) -> bool {
        self.parser.has_errors()
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
    pub fn to_ast(&self, file_id: u32) -> Option<ast::NodePtr> {
        let mut cvt = Converter {
            hparser: &self.parser,
            file_id,
        };

        self.parser.root().map(|root| convert_ast(&mut cvt, root))
    }
}

fn convert_ast(cvt: &mut Converter, n: NodePtr) -> ast::NodePtr {
    unsafe { cvt_node_ptr(cvt, n) }
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
/// When there ar errors, it returns only the first error.
/// It checks if the input is already null-terminated and avoids making the copy in that case.
/// Note that if the null terminator is truly present in the input, it would parse successfully
/// what ought to be an error.
pub fn parse_with_file_id(
    flags: ParserFlags,
    source: &str,
    file_id: u32,
) -> Result<ast::NodePtr, ParseError> {
    let buf = NullTerminatedBuf::from_str_check(source);
    let parsed = ParsedJS::parse(flags, &buf);
    if let Some(ast) = parsed.to_ast(file_id) {
        Ok(ast)
    } else {
        let (loc, msg) = parsed.first_error().unwrap();
        Err(ParseError { loc, msg })
    }
}

/// This is a simple function that is intended to be used mostly for testing.
/// When there ar errors, it returns only the first error.
/// It checks if the input is already null-terminated and avoids making the copy in that case.
/// Note that if the null terminator is truly present in the input, it would parse successfully
/// what ought to be an error.
pub fn parse(source: &str) -> Result<ast::NodePtr, ParseError> {
    parse_with_file_id(Default::default(), source, 0)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test1() {
        parse("function foo(p1) { var x = (10 + p1); }").expect("Parse failed");
    }
}
