/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::env;

fn main() {
    let hermes_build = env::var("HERMES_BUILD")
        .expect("HERMES_BUILD must point to a Hermes CMake build directory");

    println!("cargo:rustc-link-search={}/lib/Support", hermes_build);
    println!("cargo:rustc-link-search={}/external/dtoa", hermes_build);

    println!("cargo:rustc-link-lib=hermesSupport");
    println!("cargo:rustc-link-lib=dtoa");
    println!("cargo:rustc-link-lib=c++");
}
