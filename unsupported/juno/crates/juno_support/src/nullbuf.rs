/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::io::Read;
use std::os::raw::c_char;

/// A null terminated memory buffer.
#[derive(Debug)]
pub struct NullTerminatedBuf(Vec<u8>);

impl NullTerminatedBuf {
    /// Create from a reader and null terminate.
    pub fn from_reader(reader: &mut dyn Read) -> Result<NullTerminatedBuf, std::io::Error> {
        let mut v = Vec::<u8>::new();
        reader.read_to_end(&mut v)?;
        v.push(0);

        Ok(NullTerminatedBuf(v))
    }

    /// Create from a file and null terminate it.
    pub fn from_file(f: &'_ mut std::fs::File) -> Result<NullTerminatedBuf, std::io::Error> {
        // TODO: this is an extremely naive implementation, it can be optimized in multiple ways:
        //       - obtain the size of the file and perform a single allocation and few syscalls
        //       - memory map the file
        //       - just use LLVM's MemoryBuffer
        //       One problem is that there isn't an obvious way in Rust to check portably whether
        //       something has a fixed size and is memory mappable (i.e. is not a pipe).

        Self::from_reader(f)
    }

    /// Create by copying a slice and appending null-termination.
    pub fn from_slice_copy(s: &[u8]) -> NullTerminatedBuf {
        let mut v = Vec::with_capacity(s.len() + 1);
        v.extend_from_slice(s);
        v.push(0);
        NullTerminatedBuf(v)
    }

    /// Create from a slice that may already be null-terminated.
    pub fn from_slice_check(s: &[u8]) -> NullTerminatedBuf {
        Self::from_slice_copy(if let [head @ .., 0] = s { head } else { s })
    }

    /// Create by copying a string and appending null-termination.
    pub fn from_str_copy(s: &str) -> NullTerminatedBuf {
        Self::from_slice_copy(s.as_bytes())
    }

    /// Create from a string that may already be null-terminated.
    pub fn from_str_check(s: &str) -> NullTerminatedBuf {
        Self::from_slice_check(s.as_bytes())
    }

    /// Return the length of the data including the null terminator.
    pub fn len(&self) -> usize {
        self.0.len()
    }

    /// Just a placeholder always returning `true`, since the there is always
    /// at least a null terminator.
    pub fn is_empty(&self) -> bool {
        false
    }

    /// A pointer to the start of the slice.
    /// # Safety
    /// It is not really unsafe, but is intended to be used in an unsafe context.
    pub unsafe fn as_ptr(&self) -> *const u8 {
        self.as_bytes().as_ptr()
    }

    /// Convenience wrapper returning C `const char *`.
    /// # Safety
    /// It is not really unsafe, but is intended to be used in an unsafe context.
    pub unsafe fn as_c_char_ptr(&self) -> *const c_char {
        self.as_ptr() as *const c_char
    }

    pub fn as_bytes(&self) -> &[u8] {
        self.0.as_slice()
    }
}

impl AsRef<[u8]> for NullTerminatedBuf {
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}
