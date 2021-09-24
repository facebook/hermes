/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::io::Read;
use std::os::raw::c_char;

/// An abstraction for a null-terminated buffer either read from disk, copied
/// or borrowed.
pub struct NullTerminatedBuf<'a>(Inner<'a>);

// This enum must be separate because we can't make the variants private.
enum Inner<'a> {
    Own(Vec<u8>),
    Ref(&'a [u8]),
}

impl NullTerminatedBuf<'_> {
    /// A reference to an existing NullTerminatedBuf which can be passed by value.
    pub fn as_ref_buf<'a>(buf: &'a NullTerminatedBuf<'a>) -> NullTerminatedBuf<'a> {
        NullTerminatedBuf(Inner::Ref(buf.as_bytes()))
    }

    /// Create from a reader and null terminate.
    pub fn from_reader<'a, R: Read>(
        mut reader: R,
    ) -> Result<NullTerminatedBuf<'a>, std::io::Error> {
        let mut v = Vec::<u8>::new();
        reader.read_to_end(&mut v)?;
        v.push(0);

        Ok(NullTerminatedBuf(Inner::Own(v)))
    }

    /// Create from a file and null terminate it.
    pub fn from_file<'a>(
        f: &'_ mut std::fs::File,
    ) -> Result<NullTerminatedBuf<'a>, std::io::Error> {
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
        NullTerminatedBuf(Inner::Own(v))
    }

    /// Create from a slice that may already be null-terminated. If it is,
    /// borrow it, otherwise create a null-terminated copy.
    pub fn from_slice_check(s: &[u8]) -> NullTerminatedBuf {
        if let [.., 0] = s {
            NullTerminatedBuf(Inner::Ref(s))
        } else {
            Self::from_slice_copy(s)
        }
    }

    /// Create by copying a string and appending null-termination.
    pub fn from_str_copy(s: &str) -> NullTerminatedBuf {
        Self::from_slice_copy(s.as_bytes())
    }

    /// Create from a string that may already be null-terminated. If it is,
    /// borrow it, otherwise create a null-terminated copy.
    pub fn from_str_check(s: &str) -> NullTerminatedBuf {
        Self::from_slice_check(s.as_bytes())
    }

    /// Return the length of the data including the null terminator.
    pub fn len(&self) -> usize {
        match &self.0 {
            Inner::Own(v) => v.len(),
            Inner::Ref(s) => s.len(),
        }
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

    fn as_bytes(&self) -> &[u8] {
        match &self.0 {
            Inner::Own(v) => v.as_slice(),
            Inner::Ref(s) => s,
        }
    }
}

impl AsRef<[u8]> for NullTerminatedBuf<'_> {
    fn as_ref(&self) -> &[u8] {
        self.as_bytes()
    }
}
