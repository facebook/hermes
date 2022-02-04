/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/// Converts a `snake_case` ASCII input string to `camelCase`.
pub fn ascii_snake_to_camel(input: &str) -> String {
    debug_assert!(input.is_ascii(), "no Unicode support");
    let mut output = String::new();
    let mut need_upper = false;
    for c in input.chars() {
        match c {
            '_' => {
                need_upper = true;
            }
            _ => {
                output.push(if need_upper {
                    c.to_ascii_uppercase()
                } else {
                    c
                });
                need_upper = false;
            }
        }
    }
    output
}
