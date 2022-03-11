/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::eval::*;
use juno_ast::NodeRc;

#[allow(dead_code)]
mod eval;

pub fn run(ast: &NodeRc) {
    let _ = eval::script::script_evaluation(&mut runtime::Runtime::new(), ast);
}
