/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

mod convert;
mod generated_ffi;
mod hermes_parser;
mod node;
mod utf;

use crate::ast;
use thiserror::Error;

use convert::Converter;
use generated_ffi::cvt_node_ptr;
use hermes_parser::HermesParser;
use node::NodePtr;
use std::fmt::Formatter;
use utf::utf8_with_surrogates_to_string;

pub use hermes_parser::MagicCommentKind;

pub struct ParsedJS {
    parser: HermesParser,
}

impl ParsedJS {
    /// Parse the source and store an internal representation of the AST and/or a list of diagnostic
    /// messages. If at least one of the messages is an error, there is no AST.
    /// To avoid copying `source` can optionally be NUL-terminated.
    pub fn parse(source: &str) -> ParsedJS {
        ParsedJS {
            parser: HermesParser::parse(source),
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
                    col: msg.coord.column as u32,
                },
                utf8_with_surrogates_to_string(msg.message.as_slice()).unwrap(),
            )
        })
    }

    /// Create and return an external representation of the AST, or None if there were parse errors.
    pub fn to_ast(&self) -> Option<ast::NodePtr> {
        fn convert_ast(cvt: &Converter, n: NodePtr) -> ast::NodePtr {
            unsafe { cvt_node_ptr(cvt, n) }
        }
        self.parser.root().map(|root| {
            convert_ast(
                &Converter {
                    hparser: &self.parser,
                    file_id: 0,
                },
                root,
            )
        })
    }
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
pub fn parse(source: &str) -> Result<ast::NodePtr, ParseError> {
    let parsed = ParsedJS::parse(source);
    if let Some(ast) = parsed.to_ast() {
        Ok(ast)
    } else {
        let (loc, msg) = parsed.first_error().unwrap();
        Err(ParseError { loc, msg })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test1() {
        parse("function foo(p1) { var x = (10 + p1); }").expect("Parse failed");
    }
}
