/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use crate::HeapSize;

/// Append-only deque which ensures the elements pushed into it never move.
/// Allocates chunks in doubling capacities.
#[derive(Debug)]
pub struct Deque<T> {
    storage: Vec<Vec<T>>,

    /// Capacity at which to allocate the next chunk.
    /// Doubles every chunk until reaching [`MAX_CHUNK_CAPACITY`].
    next_chunk_capacity: usize,
}

/// Minimum chunk capacity in the deque.
/// May be made configurable in the future.
const MIN_CHUNK_CAPACITY: usize = 1 << 10;

/// Maximum chunk capacity in the deque.
/// May be made configurable in the future.
const MAX_CHUNK_CAPACITY: usize = MIN_CHUNK_CAPACITY * (1 << 10);

impl<T> Default for Deque<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Deque<T> {
    pub fn new() -> Self {
        let mut result = Self {
            storage: Default::default(),
            next_chunk_capacity: MIN_CHUNK_CAPACITY,
        };
        result.new_chunk();
        result
    }

    /// Append an element to the deque and return a reference to it.
    /// The element will not move after it is allocated.
    pub fn push(&mut self, val: T) -> &T {
        let chunk = self.storage.last().unwrap();
        if chunk.len() >= chunk.capacity() {
            self.new_chunk();
        }
        let chunk = self.storage.last_mut().unwrap();
        debug_assert!(
            chunk.len() < chunk.capacity(),
            "Invalid attempt to expand a chunk"
        );
        chunk.push(val);
        chunk.last().unwrap()
    }

    /// Return the number of elements that have been appended to the deque.
    pub fn len(&self) -> usize {
        let mut result = 0;
        for chunk in &self.storage {
            result += chunk.len();
        }
        result
    }

    pub fn is_empty(&self) -> bool {
        self.storage.is_empty()
    }

    /// Iterator over every element of the deque.
    pub fn iter(&self) -> impl Iterator<Item = &T> {
        self.storage.iter().flatten()
    }

    /// Iterator over every element of the deque.
    pub fn iter_mut(&mut self) -> impl Iterator<Item = &mut T> {
        self.storage.iter_mut().flatten()
    }

    /// Allocate a new chunk in the node storage.
    fn new_chunk(&mut self) {
        let capacity = self.next_chunk_capacity;
        self.storage.push(Vec::with_capacity(capacity));

        // Double the capacity if there's room.
        if capacity < MAX_CHUNK_CAPACITY {
            self.next_chunk_capacity = capacity * 2;
        }
    }
}

impl<T> HeapSize for Deque<T> {
    fn heap_size(&self) -> usize {
        let mut result = 0;
        for chunk in &self.storage {
            result += chunk.heap_size();
        }
        result
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn append() {
        let mut d = Deque::new();
        d.push(1);
        d.push(2);
        assert_eq!(d.iter().count(), 2);
    }

    #[test]
    fn multi_chunks() {
        let mut d = Deque::<usize>::new();
        let count = MIN_CHUNK_CAPACITY * 2;
        let mut ptr = std::ptr::null();
        for i in 0..count {
            let elem = d.push(i);
            if i == 1000 {
                ptr = elem as *const usize;
            }
        }
        assert_eq!(d.iter().count(), count);
        // Make sure nothing in the first chunk moved around.
        assert_eq!(unsafe { *ptr }, 1000);
    }
}
