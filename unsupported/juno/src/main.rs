/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use juno::gen_js;
use juno::hparser;
use std::io::Write;

fn main() {
    match hparser::parse("function foo(p1) { var x = (p1 + p1); }") {
        Err(e) => eprintln!("{}", e),
        Ok(ast) => {
            let mut out = Vec::new();
            gen_js::generate(&mut out, ast.as_ref(), gen_js::Pretty::No).unwrap();
            std::io::stdout().write_all(&out).unwrap();
        }
    }
}
