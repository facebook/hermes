/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::collections::HashMap;

/// Type used to hold a string index internally.
type NumIndex = u32;

/// A string uniquing table - only one copy of a string is stored and all attempts
/// to add the same string again return the same atom.
#[derive(Debug, Default)]
pub struct AtomTable {
    /// Strings are added here and never removed or mutated.
    strings: Vec<String>,
    /// Maps from a reference inside [`AtomTable::strings`] to the index in [`AtomTable::strings`].
    /// Since strings are never removed or modified, the lifetime of the key
    /// is effectively static.
    map: HashMap<&'static str, NumIndex>,
}

/// This represents a unique string index in the table.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
pub struct Atom(NumIndex);

/// A special value reserved for the invalid atom.
pub const INVALID_ATOM: Atom = Atom(NumIndex::MAX);

impl AtomTable {
    /// Create a new empty atom table.
    pub fn new() -> AtomTable {
        Default::default()
    }

    /// Add a string to the table and return its atom index. The same
    /// string always returns the same index.
    pub fn add_atom<V: Into<String> + AsRef<str>>(&mut self, value: V) -> Atom {
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
    pub fn str(&self, ident: Atom) -> &str {
        self.strings[ident.0 as usize].as_str()
    }
}

impl std::ops::Index<Atom> for AtomTable {
    type Output = str;

    fn index(&self, index: Atom) -> &Self::Output {
        self.str(index)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tab() {
        let mut idtab = AtomTable::new();

        let id_foo = idtab.add_atom("foo");
        let p_foo: *const str = idtab.str(id_foo);
        let id_bar = idtab.add_atom("bar");
        assert_ne!(id_foo, id_bar);

        assert_eq!(idtab.add_atom("foo"), id_foo);
        assert_eq!(idtab.add_atom("bar"), id_bar);

        assert_eq!(idtab.add_atom(String::from("foo")), id_foo);
        assert_eq!(idtab.add_atom(String::from("bar")), id_bar);

        assert_eq!(idtab.str(id_foo), "foo");
        assert_eq!(idtab.str(id_bar), "bar");

        assert_eq!(idtab.str(id_foo) as *const str, p_foo);
    }
}
