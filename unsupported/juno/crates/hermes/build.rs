/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

fn main() {
    let hermes_build = env!(
        "HERMES_BUILD",
        "HERMES_BUILD must point to a Hermes CMake build directory"
    );

    println!("cargo:rustc-link-search={}/lib/Parser", hermes_build);
    println!(
        "cargo:rustc-link-search={}/lib/Platform/Unicode",
        hermes_build
    );
    println!("cargo:rustc-link-search={}/lib/SourceMap", hermes_build);
    println!("cargo:rustc-link-search={}/lib/Support", hermes_build);
    println!("cargo:rustc-link-search={}/external/dtoa", hermes_build);
    println!(
        "cargo:rustc-link-search={}/external/llvh/lib/Support",
        hermes_build
    );

    println!("cargo:rustc-link-lib=hermesSourceMap");
    println!("cargo:rustc-link-lib=hermesParser");
    println!("cargo:rustc-link-lib=hermesPlatformUnicode");
    println!("cargo:rustc-link-lib=hermesSupport");
    println!("cargo:rustc-link-lib=LLVHSupport");
    println!("cargo:rustc-link-lib=dtoa");
    println!("cargo:rustc-link-lib=c++");
}
