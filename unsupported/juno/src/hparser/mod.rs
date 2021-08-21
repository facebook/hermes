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

use generated_ffi::cvt_node_ptr;
use hermes_parser::HermesParser;
use node::NodePtr;
use std::fmt::Formatter;
use utf::utf8_with_surrogates_to_string;

fn convert_ast(n: NodePtr) -> ast::NodePtr {
    unsafe { cvt_node_ptr(n) }
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
    let parser = HermesParser::parse(source);

    if let Some(root) = parser.root() {
        return Ok(convert_ast(root));
    }

    let msg = &parser.messages()[parser
        .first_error_index()
        .expect("at least one error expected when no root")];

    Err(ParseError {
        loc: ast::SourceLoc {
            line: msg.coord.line as u32,
            col: msg.coord.column as u32,
        },
        msg: utf8_with_surrogates_to_string(msg.message.as_slice()).unwrap(),
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test1() {
        parse("function foo(p1) { var x = (10 + p1); }").expect("Parse failed");
    }
}
