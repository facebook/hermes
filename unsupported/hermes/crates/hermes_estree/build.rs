/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use hermes_estree_codegen::estree;

// Example custom build script.
fn main() {
    // Re-run if the codegen files change
    println!("cargo:rerun-if-changed=../hermes_estree_codegen/src/codegen.rs");
    println!("cargo:rerun-if-changed=../hermes_estree_codegen/src/lib.rs");
    println!("cargo:rerun-if-changed=../hermes_estree_codegen/src/ecmascript.json");
    println!("cargo:rerun-if-changed=../hermes_estree_codegen");

    let src = estree();
    std::fs::write("src/generated.rs", src).unwrap();
}
