/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

macro_rules! cond {
    ($condition: expr, $_true: expr, $_false: expr) => {
        if $condition { $_true } else { $_false }
    };
}

#[allow(dead_code)]
mod cl;
#[allow(dead_code)]
mod opt;
#[allow(dead_code)]
mod parser;

pub use cl::CommandLine;
pub use opt::parse_bool;
pub use opt::parse_disallowed;
pub use opt::EnumDesc;
pub use opt::ExpectedValue;
pub use opt::Hidden;
pub use opt::Opt;
pub use opt::OptDesc;
pub use opt::OptHolder;
pub use opt::OptValue;
pub use parser::CommandLineIntent;
