/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

fn main() {
    println!("cargo:rerun-if-changed=../../../../external");

    let dst = cmake::Config::new("../../../../")
        .build_target("FileCheck")
        .build();
    println!(
        "cargo:rustc-env=HERMES_LIT={}/build/bin/hermes-lit",
        dst.display()
    );
    println!(
        "cargo:rustc-env=HERMES_FILECHECK={}/build/bin/FileCheck",
        dst.display()
    );
}
