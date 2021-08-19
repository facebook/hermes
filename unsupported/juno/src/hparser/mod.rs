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

use generated_ffi::cvt_node_ptr;
use hermes_parser::HermesParser;
use node::NodePtr;

fn convert_ast(n: NodePtr) -> ast::NodePtr {
    unsafe { cvt_node_ptr(n) }
}

pub fn parse(source: &str) -> Result<ast::NodePtr, String> {
    match HermesParser::parse(source) {
        Err(s) => Err(s),
        Ok(parser) => Ok(convert_ast(parser.root())),
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
