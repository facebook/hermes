/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use assert_cmd::Command;
use std::env;

#[test]
fn run_lit_tests() {
    let lit = lit::lit_path();
    Command::new(lit)
        .arg("-sv")
        .arg("--param")
        .arg("test_exec_root=../../target/lit/flow_native")
        .arg("--param")
        .arg(format!(
            "fnc={}",
            assert_cmd::cargo::cargo_bin("fnc").to_str().unwrap()
        ))
        .arg("--param")
        .arg(format!("FileCheck={}", lit::filecheck_path()))
        .arg(format!(
            "{}/../../lit/flow_native",
            env!("CARGO_MANIFEST_DIR")
        ))
        .assert()
        .success();
}
