/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// Path to the LIT binary.
pub fn lit_path() -> &'static str {
    env!("HERMES_LIT")
}

/// Path to the FileCheck binary.
pub fn filecheck_path() -> &'static str {
    env!("HERMES_FILECHECK")
}
