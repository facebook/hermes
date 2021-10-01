/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use assert_cmd::Command;
use std::{env, fs};
use toml::Value;

#[test]
fn run_lit_tests() {
    let hermes_build = match env::var("HERMES_BUILD") {
        Ok(dir) => dir,
        Err(_) => {
            let file =
                fs::read_to_string(format!("{}/../../.hermes.toml", env!("CARGO_MANIFEST_DIR")))
                    .expect("Must provide either HERMES_BUILD env variable or .hermes.toml file");
            let value = file.parse::<Value>().unwrap();
            value["hermes_build"].as_str().unwrap().to_owned()
        }
    };

    let lit = format!("{}/bin/hermes-lit", hermes_build);
    Command::new(lit)
        .arg("-sv")
        .arg("--param")
        .arg("test_exec_root=../../target/lit")
        .arg("--param")
        .arg(format!(
            "juno={}",
            assert_cmd::cargo::cargo_bin("juno").to_str().unwrap()
        ))
        .arg("--param")
        .arg(format!("FileCheck={}/bin/FileCheck", hermes_build))
        .arg(format!("{}/../../lit/", env!("CARGO_MANIFEST_DIR")))
        .assert()
        .success();
}
