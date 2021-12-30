/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::{env, fs};
use toml::Value;

fn main() {
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
}
