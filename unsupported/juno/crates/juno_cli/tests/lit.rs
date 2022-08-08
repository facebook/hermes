/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::env;

use assert_cmd::Command;

#[test]
fn run_lit_tests() {
    let lit = lit::lit_path();
    Command::new(lit)
        .arg("-sv")
        .arg("--param")
        .arg("test_exec_root=../../target/lit/juno")
        .arg("--param")
        .arg(format!(
            "juno={}",
            assert_cmd::cargo::cargo_bin("juno").to_str().unwrap()
        ))
        .arg("--param")
        .arg(format!("FileCheck={}", lit::filecheck_path()))
        .arg(format!("{}/../../lit/juno", env!("CARGO_MANIFEST_DIR")))
        .assert()
        .success();
}
