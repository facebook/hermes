/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

use std::cell::Cell;
use std::cell::UnsafeCell;
use std::mem;
use std::mem::MaybeUninit;

use intrusive_collections::LinkedList;
use intrusive_collections::LinkedListLink;

use crate::uref::URef;
use crate::uref_intrusive_adapter;

pub(crate) struct Value {
    user_list: UnsafeCell<LinkedList<UseNodeAdapter>>,
}

/// UseNode links a "use" of a value to the value and the owner of the use.
/// A value owns a linked list of `UseNode` linking all uses of the value.
/// An owner owns a vector of `UseNode` representing the operands (values) that
/// are used by that owner.
#[derive(Default)]
pub(crate) struct UseNode {
    /// Link in the user list of ValueType.
    link: LinkedListLink,
    /// The value that is being "used".
    value: Cell<Option<URef<Value>>>,
    /// Who is using the value? (Typically an instruction)
    owner: Cell<Option<URef<Instruction>>>,
}

uref_intrusive_adapter!(UseNodeAdapter = URef<UseNode>: UseNode { link: LinkedListLink } );

#[derive(Default)]
pub(crate) struct ValueList {
    owner: Option<URef<Instruction>>,
    data: Vec<UseNode>,
}

pub(crate) struct Instruction {
    operands: UnsafeCell<ValueList>,
}

/// On destruction, ValueType clears all pointers pointing to it.
impl Drop for Value {
    fn drop(&mut self) {
        let list = self.user_list.get_mut();
        while !list.is_empty() {
            // This removes the element from the list as it clears the pointer.
            list.front().get().unwrap().set_value(None);
        }
    }
}

impl Value {
    fn construct(mem: &mut MaybeUninit<Self>) -> &mut Self {
        mem.write(Value {
            user_list: UnsafeCell::new(Default::default()),
        })
    }

    unsafe fn add_user(&self, user: &UseNode) {
        (&mut *self.user_list.get()).push_back(URef::new(user));
    }
    unsafe fn remove_user(&self, user: &UseNode) {
        (&mut *self.user_list.get())
            .cursor_mut_from_ptr(user)
            .remove();
    }

    unsafe fn replace_user(&self, old_user: &UseNode, new_user: &UseNode) {
        // TODO: this can be implemented more efficiently in a circular list.
        let mut old_cursor = (*self.user_list.get()).cursor_mut_from_ptr(old_user);
        old_cursor.insert_after(URef::new(new_user));
        old_cursor.remove();
    }
}

impl UseNode {
    /// We can't use a constructor, because the UseNode cannot be moved once it is added to a list.
    pub fn init_no_add(&mut self, value: Option<URef<Value>>, owner: Option<URef<Instruction>>) {
        self.value.set(value);
        self.owner.set(owner);
    }

    /// We can't use a constructor, because the UseNode cannot be moved once it is added to a list.
    pub fn init(&mut self, value: Option<URef<Value>>, owner: Option<URef<Instruction>>) {
        self.value.set(value);
        self.owner.set(owner);
        if let Some(value) = value {
            unsafe {
                value.add_user(self);
            }
        }
    }

    pub fn set_value(&self, value: Option<URef<Value>>) {
        if let Some(old_value) = self.value.get() {
            unsafe {
                old_value.remove_user(self);
            }
        }
        self.value.set(value);
        if let Some(value) = value {
            unsafe {
                value.add_user(self);
            }
        }
    }
}

/// Remove the UseNode from the value's user list on destruction.
impl Drop for UseNode {
    fn drop(&mut self) {
        if let Some(old_value) = self.value.get() {
            unsafe {
                old_value.remove_user(self);
            }
        }
    }
}

impl ValueList {
    const INIT_CAPACITY: usize = 4;
    const MAX_CAPACITY: usize = usize::MAX / 2 / mem::size_of::<UseNode>();

    pub fn set_owner(&mut self, owner: &Instruction) {
        self.owner = Some(URef::new(owner));
    }

    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    pub fn len(&self) -> usize {
        self.data.len()
    }

    pub fn reserve(&mut self, new_cap: usize) {
        if new_cap > self.data.capacity() {
            assert!(new_cap <= Self::MAX_CAPACITY);
            self.grow_to(new_cap);
        }
    }

    pub fn clear(&mut self) {
        self.data.clear();
    }

    pub fn push_back(&mut self, value: Option<URef<Value>>) {
        let index = self.data.len();
        if self.data.capacity() == index {
            self.grow();
        }
        debug_assert!(self.data.capacity() > index);
        self.data.push(Default::default());
        unsafe { self.data.get_unchecked_mut(index) }.init(value, self.owner);
    }

    /// Move all values from the specified list onto our list, resetting their owner.
    pub fn move_from(&mut self, other: &mut ValueList) {
        if self.data.is_empty() {
            // If we are empty, we can just swap the vecs.
            mem::swap(&mut self.data, &mut other.data);
            // Reset the owner of the stolen items.
            for elem in self.data.iter_mut() {
                elem.owner.set(self.owner);
            }
        } else {
            // Slow path.
            for elem in other.data.iter() {
                self.push_back(elem.value.get());
            }
            other.clear();
        }
    }

    /// Grow the list. This is a special operation because the reallocated nodes
    /// must be relinked into their corresponding user lists.
    fn grow(&mut self) {
        let cap = self.data.capacity();
        debug_assert!(cap == self.data.len());

        let new_cap;
        if cap == 0 {
            new_cap = Self::INIT_CAPACITY;
        } else if cap >= Self::MAX_CAPACITY {
            // Admittedly, quite unlikely...
            panic!("Maximum ValueList capacity reached");
        } else {
            new_cap = std::cmp::min(cap * 2, Self::MAX_CAPACITY);
        }

        self.grow_to(new_cap);
    }

    /// Grow the capacity to the specified value. This is a special operation
    /// because the reallocated nodes must be relinked into their corresponding user lists.
    fn grow_to(&mut self, new_cap: usize) {
        debug_assert!(new_cap > self.data.capacity());

        if self.data.capacity() == 0 {
            self.data.reserve(new_cap);
            return;
        }

        let mut new_vec = Vec::<UseNode>::with_capacity(new_cap);

        for old_node in self.data.iter() {
            // TODO: this can be optimized with MaybeUninit.
            new_vec.push(Default::default());
            let len = new_vec.len();
            let new_node = unsafe { new_vec.get_unchecked_mut(len - 1) };

            let value = old_node.value.get();
            new_node.init_no_add(value, old_node.owner.get());
            if let Some(value) = value {
                unsafe { value.replace_user(old_node, new_node) };
            }
        }

        mem::swap(&mut new_vec, &mut self.data);

        // Free the memory now stored in new_vec, without running destructors.
        let p = Box::into_raw(new_vec.into_boxed_slice());
        unsafe { Box::from_raw(p as *mut [mem::ManuallyDrop<UseNode>]) };
    }
}

impl Instruction {
    fn construct(mem: &mut MaybeUninit<Instruction>) -> &mut Self {
        let this = mem.write(Instruction {
            operands: UnsafeCell::new(Default::default()),
        });
        let operands = unsafe { &mut *this.operands.get() };
        operands.set_owner(this);
        this
    }

    fn push_operand<'a>(&'a self, value: &'a mut Value) {
        let operands = unsafe { &mut *self.operands.get() };
        operands.push_back(Some(URef::new(value)));
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::pool::Pool;

    #[test]
    fn test1() {
        let value_pool = Pool::<Value>::new();
        let owner_pool = Pool::<Instruction>::new();

        let new_value = || {
            let mut p = value_pool.alloc();
            Value::construct(unsafe { p.as_mut() })
        };
        let new_owner = || {
            let mut p = owner_pool.alloc();
            Instruction::construct(unsafe { p.as_mut() })
        };

        let o1 = new_owner();
        {
            let v1 = new_value();
            o1.push_operand(v1);
            o1.push_operand(v1);
        }
        let v2 = new_value();
        o1.push_operand(v2);

        //
        // {
        //     let o2 = new_owner();
        //     o2.push_operand(v1);
        // }
        //
    }
}
