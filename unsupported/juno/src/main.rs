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
    let res = hparser::parse("function foo(p1) { var x = (p1 + p1); }").unwrap();
    let mut out = Vec::new();
    gen_js::generate(&mut out, res.as_ref(), gen_js::Pretty::No).unwrap();
    std::io::stdout().write_all(&out).unwrap();
}
