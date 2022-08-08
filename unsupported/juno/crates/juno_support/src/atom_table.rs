/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::cell::Cell;
use std::cell::UnsafeCell;
use std::collections::HashMap;
use std::fmt::Formatter;
use std::ptr::null;

use crate::HeapSize;

/// Type used to hold a string index internally.
type NumIndex = u32;

/// A string uniquing table - only one copy of a string is stored and all attempts
/// to add the same string again return the same atom. This table is intended to
/// be easily shareable, so it utilizes interior mutability. UnsafeCell<> is safe
/// because we never allow reference to it to escape.
#[derive(Debug, Default)]
pub struct AtomTable(UnsafeCell<Inner>);

/// A string uniquing table - only one copy of a string is stored and all attempts
/// to add the same string again return the same atom.
#[derive(Default)]
struct Inner {
    /// Strings are added here and never removed or mutated.
    strings: Vec<String>,
    /// Maps from a reference inside [`Inner::strings`] to the index in [`Inner::strings`].
    /// Since strings are never removed or modified, the lifetime of the key
    /// is effectively static.
    map: HashMap<&'static str, NumIndex>,

    /// Strings are added here and never removed or mutated.
    strings_u16: Vec<Vec<u16>>,
    /// Maps from a reference inside [`Inner::strings_u16`] to the index in [`Inner::strings_u16`].
    /// Since strings are never removed or modified, the lifetime of the key
    /// is effectively static.
    map_u16: HashMap<&'static [u16], NumIndex>,
}

impl HeapSize for Inner {
    fn heap_size(&self) -> usize {
        self.strings.heap_size()
            + self.map.heap_size()
            + self.strings_u16.heap_size()
            + self.map_u16.heap_size()
    }
}

/// This represents a unique string index in the table.
#[derive(Copy, Clone, Eq, PartialEq, Hash)]
pub struct Atom(NumIndex);

/// This represents a unique string index in the table.
#[derive(Copy, Clone, Eq, PartialEq, Hash)]
pub struct AtomU16(NumIndex);

thread_local! {
    /// Stores the active table used for debug formatting.
    static DEBUG_TABLE: Cell<* const AtomTable> = Cell::new(null());
}

// An implementation of Debug which optionally obtains the Atom value from the
// active debug map.
impl std::fmt::Debug for Atom {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let mut t = f.debug_tuple("Atom");
        t.field(&self.0);

        // If the debug table is set and the atom is valid in it, add the value
        DEBUG_TABLE.with(|debug_table| {
            let p = debug_table.get();
            if let Some(r) = unsafe { p.as_ref() } {
                if let Some(value) = r.try_str(*self) {
                    t.field(&value);
                }
            }
        });
        t.finish()
    }
}

// An implementation of Debug which optionally obtains the Atom value from the
// active debug map.
impl std::fmt::Debug for AtomU16 {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let mut t = f.debug_tuple("Atom");
        t.field(&self.0);

        // If the debug table is set and the atom is valid in it, add the value
        DEBUG_TABLE.with(|debug_table| {
            let p = debug_table.get();
            if let Some(r) = unsafe { p.as_ref() } {
                if let Some(value) = r.try_str_u16(*self) {
                    t.field(&value);
                }
            }
        });
        t.finish()
    }
}

/// A special value reserved for the invalid atom.
pub const INVALID_ATOM: Atom = Atom(NumIndex::MAX);

impl Inner {
    /// Add a string to the table and return its atom index. The same
    /// string always returns the same index.
    fn add_atom<V: Into<String> + AsRef<str>>(&mut self, value: V) -> Atom {
        if let Some(index) = self.map.get(value.as_ref()) {
            return Atom(*index);
        }
        self.add(value.into())
    }

    /// Perform the actual addition of the owned string.
    fn add(&mut self, owned: String) -> Atom {
        // Remember the index of the new element.
        let index = self.strings.len();
        assert!(index < INVALID_ATOM.0 as usize, "More than 4GB atoms?");

        // Obtain a reference to the existing string on the heap. That reference
        // is valid while `self` is valid.
        let key: *const str = owned.as_str();

        // Push the new string.
        self.strings.push(owned);

        self.map.insert(unsafe { &*key }, index as NumIndex);
        Atom(index as NumIndex)
    }

    /// Return the contents of the specified atom.
    #[inline]
    fn str(&self, ident: Atom) -> &str {
        self.strings[ident.0 as usize].as_str()
    }

    fn try_str(&self, ident: Atom) -> Option<&str> {
        if (ident.0 as usize) < self.strings.len() {
            Some(self.str(ident))
        } else {
            None
        }
    }

    /// Add a string to the table and return its atom index. The same
    /// string always returns the same index.
    fn add_atom_u16<V: Into<Vec<u16>> + AsRef<[u16]>>(&mut self, value: V) -> AtomU16 {
        if let Some(index) = self.map_u16.get(value.as_ref()) {
            return AtomU16(*index);
        }
        self.add_u16(value.into())
    }

    /// Perform the actual addition of the owned string.
    fn add_u16(&mut self, owned: Vec<u16>) -> AtomU16 {
        // Remember the index of the new element.
        let index = self.strings_u16.len();
        assert!(index < INVALID_ATOM.0 as usize, "More than 4GB atoms?");

        // Obtain a reference to the existing string on the heap. That reference
        // is valid while `self` is valid.
        let key: *const [u16] = owned.as_slice();

        // Push the new string.
        self.strings_u16.push(owned);

        self.map_u16.insert(unsafe { &*key }, index as NumIndex);
        AtomU16(index as NumIndex)
    }

    /// Return the contents of the specified atom.
    #[inline]
    fn str_u16(&self, ident: AtomU16) -> &[u16] {
        self.strings_u16[ident.0 as usize].as_slice()
    }

    fn try_str_u16(&self, ident: AtomU16) -> Option<&[u16]> {
        if (ident.0 as usize) < self.strings_u16.len() {
            Some(self.str_u16(ident))
        } else {
            None
        }
    }
}

impl AtomTable {
    /// Create a new empty atom table.
    pub fn new() -> AtomTable {
        Default::default()
    }

    /// Add a string to the table and return its atom index. The same
    /// string always returns the same index.
    pub fn atom<V: Into<String> + AsRef<str>>(&self, value: V) -> Atom {
        unsafe { &mut *self.0.get() }.add_atom(value)
    }

    /// Return the contents of the specified atom.
    #[inline]
    pub fn str(&self, ident: Atom) -> &str {
        unsafe { &*self.0.get() }.str(ident)
    }

    #[inline]
    pub fn try_str(&self, ident: Atom) -> Option<&str> {
        unsafe { &*self.0.get() }.try_str(ident)
    }

    /// Add a string to the table and return its atom index. The same
    /// string always returns the same index.
    pub fn atom_u16<V: Into<Vec<u16>> + AsRef<[u16]>>(&self, value: V) -> AtomU16 {
        unsafe { &mut *self.0.get() }.add_atom_u16(value)
    }

    /// Return the contents of the specified atom.
    #[inline]
    pub fn str_u16(&self, ident: AtomU16) -> &[u16] {
        unsafe { &*self.0.get() }.str_u16(ident)
    }

    #[inline]
    pub fn try_str_u16(&self, ident: AtomU16) -> Option<&[u16]> {
        unsafe { &*self.0.get() }.try_str_u16(ident)
    }

    /// Execute the callback in a context where this table is used for debug
    /// printing of atoms.
    pub fn in_debug_context<R, F: FnOnce() -> R>(&self, f: F) -> R {
        DEBUG_TABLE.with(|debug_table| {
            let prev_table = debug_table.replace(self);
            let res = f();
            debug_assert!(
                debug_table.get() == self,
                "debug context unexpectedly changed"
            );
            debug_table.set(prev_table);
            res
        })
    }

    /// Set a table or nullptr as the Atom debug context. If non-null, debug
    /// printing of atoms will use it. Return the previous debug context.
    ///
    /// # Safety
    /// The table must not be destroyed or moved while it is set.
    pub unsafe fn unsafe_set_debug_context(ptr: *const Self) -> *const Self {
        DEBUG_TABLE.with(|debug_table| debug_table.replace(ptr))
    }
}

impl HeapSize for AtomTable {
    fn heap_size(&self) -> usize {
        unsafe { &*self.0.get() }.heap_size()
    }
}

impl std::ops::Index<Atom> for AtomTable {
    type Output = str;

    fn index(&self, index: Atom) -> &Self::Output {
        self.str(index)
    }
}

impl std::ops::Index<AtomU16> for AtomTable {
    type Output = [u16];

    fn index(&self, index: AtomU16) -> &Self::Output {
        self.str_u16(index)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tab() {
        let idtab = AtomTable::new();

        let id_foo = idtab.atom("foo");
        let p_foo: *const str = idtab.str(id_foo);
        let id_bar = idtab.atom("bar");
        assert_ne!(id_foo, id_bar);

        assert_eq!(idtab.atom("foo"), id_foo);
        assert_eq!(idtab.atom("bar"), id_bar);

        assert_eq!(idtab.atom(String::from("foo")), id_foo);
        assert_eq!(idtab.atom(String::from("bar")), id_bar);

        assert_eq!(idtab.str(id_foo), "foo");
        assert_eq!(idtab.str(id_bar), "bar");

        assert_eq!(idtab.str(id_foo) as *const str, p_foo);
    }
}
