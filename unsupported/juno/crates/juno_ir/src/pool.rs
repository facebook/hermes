/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! # Pool
//!
//! A pool of MaybeUninit<T>, conceptually similar to a C++ deque, except it
//! doesn't support indexing.
//! Nodes must be freed explicitly and are added to a free list.
//! This is not intended to be a user-visible API.

use std::cell::Cell;
use std::cell::UnsafeCell;
use std::mem::MaybeUninit;
use std::ptr;
use std::ptr::NonNull;

pub(crate) struct Pool<T> {
    /// List of all the nodes stored in this context.
    /// Each element is a "chunk" of nodes.
    /// None of the chunks are ever resized after allocation.
    nodes: UnsafeCell<Vec<Vec<MaybeUninit<T>>>>,

    /// List of freed elements.
    free: UnsafeCell<Vec<NonNull<MaybeUninit<T>>>>,

    /// Capacity at which to allocate the next chunk.
    /// Doubles every chunk until reaching [`MAX_CHUNK_CAPACITY`].
    next_chunk_capacity: Cell<usize>,
}

const MIN_CHUNK_CAPACITY: usize = 1 << 10;
const MAX_CHUNK_CAPACITY: usize = MIN_CHUNK_CAPACITY * (1 << 10);

impl<T> Pool<T> {
    pub fn new() -> Self {
        let result = Self {
            nodes: Default::default(),
            free: Default::default(),
            next_chunk_capacity: Cell::new(MIN_CHUNK_CAPACITY),
        };
        result.new_chunk();
        result
    }

    /// Allocate a new `Node` in this `Context`.
    pub fn alloc(&self) -> NonNull<MaybeUninit<T>> {
        let free = unsafe { &mut *self.free.get() };
        let nodes = unsafe { &mut *self.nodes.get() };
        if let Some(entry) = free.pop() {
            entry
        } else {
            let chunk = nodes.last().unwrap();
            if chunk.len() >= chunk.capacity() {
                self.new_chunk();
            }
            let chunk = nodes.last_mut().unwrap();
            chunk.push(MaybeUninit::uninit());
            let len = chunk.len();
            unsafe { NonNull::new_unchecked(chunk.get_unchecked_mut(len - 1)) }
        }
    }

    /// Allocate a new chunk in the pool storage.
    fn new_chunk(&self) {
        let nodes = unsafe { &mut *self.nodes.get() };
        let capacity = self.next_chunk_capacity.get();
        nodes.push(Vec::with_capacity(capacity));

        // Double the capacity if there's room.
        if capacity < MAX_CHUNK_CAPACITY {
            self.next_chunk_capacity.set(capacity * 2);
        }
    }

    unsafe fn free(&self, n: *const T) {
        ptr::drop_in_place(n as *mut T);
        let n = n as *mut MaybeUninit<T>;
        let free = &mut *self.free.get();
        free.push(NonNull::new_unchecked(n as *mut MaybeUninit<T>));
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test1() {
        let pool = Pool::new();
        unsafe {
            let mut n1 = pool.alloc();
            n1.as_mut().write(10);
            let mut n2 = pool.alloc();
            n2.as_mut().write(20);

            pool.free(n1.as_ref().as_ptr());
            let _n3 = pool.alloc();
        }
    }
}
