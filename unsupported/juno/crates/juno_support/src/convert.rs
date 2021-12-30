/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::mem;

extern "C" {
    /// \param m the number to convert
    /// \param dest output buffer
    /// \param destSize size of dest, at least NUMBER_TO_STRING_BUF_SIZE
    /// \return the length of the generated string (excluding the terminating zero).
    fn hermes_numberToString(m: f64, dest: *mut u8, destSize: usize) -> usize;
}

/// Convert a double number to string, following ES5.1 9.8.1.
pub fn number_to_string(m: f64) -> String {
    // Size of buffer that must be passed to hermes_numberToString.
    const NUMBER_TO_STRING_BUF_SIZE: usize = 32;

    unsafe {
        // Create a mutable buffer but don't deallocate it,
        // because we will build a new `String` from it.
        let mut buf = mem::ManuallyDrop::new(vec![0u8; NUMBER_TO_STRING_BUF_SIZE]);

        let length = hermes_numberToString(m, buf.as_mut_ptr(), NUMBER_TO_STRING_BUF_SIZE);
        String::from_raw_parts(buf.as_mut_ptr(), length, NUMBER_TO_STRING_BUF_SIZE)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_number_to_string() {
        assert_eq!(number_to_string(1.0), "1");
    }
}
