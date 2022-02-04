/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

fn main() {
    println!("cargo:rerun-if-changed=../../../../include");
    println!("cargo:rerun-if-changed=../../../../lib");
    println!("cargo:rerun-if-changed=../../../../external");
    println!("cargo:rerun-if-changed=../../../../cmake");
    println!("cargo:rerun-if-changed=../../../../CMakeLists.txt");
    let dst = cmake::Config::new("../../../../")
        .build_target("hermesSupport")
        .build();
    println!(
        "cargo:rustc-link-search={}/build/lib/Support",
        dst.display()
    );
    println!("cargo:rustc-link-lib=hermesSupport");
    println!(
        "cargo:rustc-link-search={}/build/external/dtoa",
        dst.display()
    );
    println!("cargo:rustc-link-lib=dtoa");
}
