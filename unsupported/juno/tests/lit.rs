/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use assert_cmd::Command;

#[test]
fn run_lit_tests() {
    let hermes_build = env!(
        "HERMES_BUILD",
        "HERMES_BUILD must point to a Hermes CMake build directory"
    );

    let lit = format!("{}/bin/hermes-lit", hermes_build);
    Command::new(lit)
        .arg("-sv")
        .arg("--param")
        .arg("test_exec_root=./target/lit")
        .arg("--param")
        .arg(format!(
            "juno={}",
            assert_cmd::cargo::cargo_bin("juno").to_str().unwrap()
        ))
        .arg("--param")
        .arg(format!("FileCheck={}/bin/FileCheck", hermes_build))
        .arg(format!("{}/lit/", env!("CARGO_MANIFEST_DIR")))
        .assert()
        .success();
}
