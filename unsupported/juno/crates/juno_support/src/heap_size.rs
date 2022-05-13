/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::collections::HashMap;

/// Trait for allowing users to query how much memory a type uses in the heap.
pub trait HeapSize {
    /// Return the size of the heap allocated memory for the type, in bytes.
    fn heap_size(&self) -> usize;
}

impl HeapSize for String {
    fn heap_size(&self) -> usize {
        self.capacity()
    }
}

impl<T> HeapSize for Box<T> {
    fn heap_size(&self) -> usize {
        std::mem::size_of::<T>()
    }
}

impl<T> HeapSize for Vec<T> {
    fn heap_size(&self) -> usize {
        self.capacity() * std::mem::size_of::<T>()
    }
}

impl<K, V> HeapSize for HashMap<K, V> {
    fn heap_size(&self) -> usize {
        let entry_size: usize =
            std::mem::size_of::<u64>() + std::mem::size_of::<K>() + std::mem::size_of::<V>();
        self.capacity() * entry_size
    }
}
