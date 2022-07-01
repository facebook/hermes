/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! ScopedHashMap
//!
//! Conceptually ScopedHashMap provides a stack of HashMap-s, where we can:
//! - Push and pop maps (we call them scopes)
//! - Insert elements in the top HashMap
//! - Search elements from top to bottom.
//!
//! The performance of a naive implementation would be:
//! - Push/pop a scope: O(1)
//! - Search: O(number of scopes)
//! It optimizes for pushing and popping scopes instead of searching.
//!
//! Instead this implementation instead optimizes for search:
//! - Push a scope: O(1)
//! - Pop a scope: O(number of elements in the scope)
//! - Search: O(1)
//!
//! Scopes usually have only a few elements, can be deeply nested, and searching
//! is very frequent. To that end, instead of a "stack of HashMap-s" we use a
//! "HashMap of stacks". In other words, every element of the single HashMap is
//! a stack of entries from different scopes. When we perform a lookup, we can
//! get to the topmost entry in O(1). When we pop a scope, we need to pop each
//! individual element belonging to it.
//!
//! All map entries are separately heap allocated, connected through pointers.
//! Every has a pointer to a "shadowed" entry in a previous scope, and a
//! next entry in the same scope. The hash table itself points to the top-most
//! node. Additionally a stack of scopes points to the first node in every
//! scope.
//!
//! If we perform the following operations:
//! ```[rust]
//! m.push_scope();
//! m.insert("a", 1);
//! m.insert("c", 3);
//! m.push_scope();
//! m.insert("b", 20);
//! m.insert("c", 30);
//! m.push_scope();
//! m.insert("a", 100);
//! m.insert("d", 400);
//! ```
//! We will get the following state in memory:
//! ```[text]
//!
//! Scope 1: --> "a":1 ------------------> "c": 3
//!               ^                         ^
//!               |                         |
//! Scope 2: -----+----------> "b":20 ---> "c": 30
//!               |             ^           ^
//!               |             |           |
//! Scope 3: --> "a":100 -------+-----------+--------> "d":400
//!               ^             |           |           ^
//!               |             |           |           |
//! HashTab:    ["a"]         ["b"]       ["c"]       ["d"]
//! ```

use std::borrow::Borrow;
use std::cmp::Ordering;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::fmt::Debug;
use std::fmt::Formatter;
use std::hash::Hash;
use std::ptr::null_mut;

/// A heap allocated node representing a value inserted in a scope.
struct Node<K, V> {
    key: K,
    value: V,
    /// A node with the same key in a previous scope, or null if no previous.
    prev_shadowed: *mut Node<K, V>,
    /// The previous node in the same scope, or null if this is the first one.
    prev_in_scope: *mut Node<K, V>,
    /// Level of the scope this node belongs to.
    depth: usize,
}

/// The scope is the head of a singly linked list of nodes belonging to the
/// scope, chained through the [Node::prev_in_scope] pointer.
struct Scope<K, V> {
    /// The last node inserted into the scope, null initially.
    last: *mut Node<K, V>,
}

impl<K, V> Debug for Scope<K, V>
where
    K: Debug,
    V: Debug,
{
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        let mut dm = f.debug_map();
        let mut cur_node = self.last;
        while !cur_node.is_null() {
            let n = unsafe { &*cur_node };
            dm.entry(&n.key, &n.value);
            cur_node = n.prev_in_scope;
        }
        dm.finish()
    }
}

#[derive(Debug)]
pub struct ScopedHashMap<K, V>
where
    K: Eq + Hash + Clone,
{
    /// Maps from keys to most current definition. All Nodes are owned by this.
    /// They are allocated by insertIntoScope and deleted by clearCurrentScope
    /// The pointers will never be null, since we remove the map entry if we
    /// pop the last node.
    map: HashMap<K, *mut Node<K, V>>,

    /// Stack of scopes.
    scopes: Vec<Scope<K, V>>,
}

impl<K, V> Default for ScopedHashMap<K, V>
where
    K: Eq + Hash + Clone,
{
    fn default() -> Self {
        let mut res = ScopedHashMap {
            map: Default::default(),
            scopes: Default::default(),
        };
        res.push_scope();
        res
    }
}

impl<K, V> ScopedHashMap<K, V>
where
    K: Eq + Hash + Clone,
{
    pub fn new() -> Self {
        Default::default()
    }

    /// Insert a key/value pair into the scope at the specified depth if
    /// possible, return an error otherwise.
    /// If the depth is smaller than the current depth, it is not guaranteed to
    /// succeed, because values may already be present "shadowing" the value
    /// that is being inserted.
    /// If a value is already present at the specified depth, it is replaced.
    pub fn insert_into_scope(
        &mut self,
        scope_depth: usize,
        key: K,
        value: V,
    ) -> Result<(), &'static str> {
        let scope = unsafe {
            assert!(scope_depth < self.scopes.len(), "scope_index out of range");
            self.scopes.get_unchecked_mut(scope_depth)
        };
        let mut entry = self.map.entry(key.clone());
        let next_shadowed = if let Entry::Occupied(o) = &mut entry {
            let node = unsafe { &mut **o.get() };
            match node.depth.cmp(&scope_depth) {
                Ordering::Greater => return Err("Value to be inserted is already shadowed"),
                Ordering::Equal => {
                    node.value = value;
                    return Ok(());
                }
                Ordering::Less => node,
            }
        } else {
            null_mut()
        };

        // All Nodes allocated here.
        let node = Box::new(Node {
            key,
            value,
            prev_shadowed: next_shadowed,
            prev_in_scope: scope.last,
            depth: scope_depth,
        });

        let vref = entry.or_insert(null_mut());
        let ptr = Box::into_raw(node);
        *vref = ptr;
        scope.last = ptr;
        Ok(())
    }

    /// Insert a key/value pair into the current scope. If the key already exists,
    /// the value is overwritten.
    pub fn insert(&mut self, key: K, value: V) {
        self.insert_into_scope(self.scopes.len() - 1, key, value)
            .unwrap();
    }

    pub fn contains_key<Q: ?Sized>(&self, k: &Q) -> bool
    where
        K: Borrow<Q>,
        Q: Hash + Eq,
    {
        self.map.contains_key(k)
    }

    pub fn get<Q: ?Sized>(&self, k: &Q) -> Option<&V>
    where
        K: Borrow<Q>,
        Q: Hash + Eq,
    {
        self.map.get(k).map(|n| unsafe { &(**n).value })
    }

    pub fn get_mut<Q: ?Sized>(&mut self, k: &Q) -> Option<&mut V>
    where
        K: Borrow<Q>,
        Q: Hash + Eq,
    {
        self.map.get_mut(k).map(|n| unsafe { &mut (**n).value })
    }

    pub fn value(&self, k: K) -> Option<&V> {
        self.get(&k)
    }

    pub fn value_mut(&mut self, k: K) -> Option<&mut V> {
        self.get_mut(&k)
    }

    /// Push a new scope, run the specified callback in it, then pop the scope.
    pub fn in_new_scope<R, F: FnOnce(&mut Self) -> R>(&mut self, f: F) -> R {
        self.push_scope();
        let res = f(self);
        self.pop_scope();
        res
    }

    /// Return the depth of the current scope (the initial scope is depth 0).
    /// It can be used in order to call ['ScopedHashMap::insert_into_scope()'].
    pub fn current_scope_depth(&self) -> usize {
        debug_assert!(!self.scopes.is_empty(), "We don't allow popping of scope 0");
        self.scopes.len() - 1
    }

    pub fn push_scope(&mut self) {
        self.scopes.push(Scope { last: null_mut() });
    }

    pub fn pop_scope(&mut self) {
        assert!(self.scopes.len() > 1, "Cannot pop the root scope");
        self.pop_scope_impl();
    }

    fn pop_scope_impl(&mut self) {
        assert!(!self.scopes.is_empty(), "No current scope to clear");
        let cur_depth = self.scopes.len() - 1;
        unsafe {
            let mut current = self.scopes.pop().unwrap().last;
            while !current.is_null() {
                debug_assert!((*current).depth == cur_depth, "Bad scope link");
                let popped = self.pop_node(&(*current).key);
                debug_assert!(current == popped, "Unexpected innermost value for key");
                current = (*current).prev_in_scope;
                // All nodes deallocated here.
                std::mem::drop(Box::from_raw(popped));
            }
        }
    }

    /// Unlinks the innermost Node for a key and returns it.
    fn pop_node(&mut self, key: &K) -> *mut Node<K, V> {
        unsafe {
            let entry = self.map.get_mut(key).unwrap();
            let ptr = *entry;
            let next_shadowed = (*ptr).prev_shadowed;
            if !next_shadowed.is_null() {
                *entry = next_shadowed;
            } else {
                self.map.remove(key);
            }
            ptr
        }
    }
}

impl<K, V> Drop for ScopedHashMap<K, V>
where
    K: Eq + Hash + Clone,
{
    fn drop(&mut self) {
        while !self.scopes.is_empty() {
            self.pop_scope_impl();
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test1() {
        let mut map = ScopedHashMap::<i32, i32>::new();

        map.insert(1, 10);
        map.insert(2, 20);

        assert_eq!(*map.value(1).unwrap(), 10);
        assert_eq!(*map.value(2).unwrap(), 20);
        assert!(map.value(3).is_none());
    }

    #[test]
    fn test2() {
        let mut map = ScopedHashMap::<i32, i32>::new();

        map.insert(1, 10);
        map.insert(2, 20);

        assert_eq!(*map.value(1).unwrap(), 10);
        assert_eq!(*map.value(2).unwrap(), 20);
        assert!(map.value(3).is_none());

        map.in_new_scope(|map| {
            map.insert(1, 11);
            map.insert(3, 31);
            assert_eq!(*map.value(1).unwrap(), 11);
            assert_eq!(*map.value(2).unwrap(), 20);
            assert_eq!(*map.value(3).unwrap(), 31);
            map.in_new_scope(|map| {
                map.insert(1, 12);
                map.insert(3, 32);
                assert_eq!(*map.value(1).unwrap(), 12);
                assert_eq!(*map.value(2).unwrap(), 20);
                assert_eq!(*map.value(3).unwrap(), 32);
            });
            assert_eq!(*map.value(1).unwrap(), 11);
            assert_eq!(*map.value(2).unwrap(), 20);
            assert_eq!(*map.value(3).unwrap(), 31);
        });

        assert_eq!(*map.value(1).unwrap(), 10);
        assert_eq!(*map.value(2).unwrap(), 20);
        assert!(map.value(3).is_none());
    }
}
