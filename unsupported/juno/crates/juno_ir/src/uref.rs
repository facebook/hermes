/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! # URef
//!
//! A re-implementation of intrusive_collections::UnsafeRef that implements
//! the `Copy` trait.

use std::borrow::Borrow;
use std::fmt;
use std::marker::PhantomData;
use std::ops::Deref;
use std::ptr::NonNull;

use intrusive_collections::PointerOps;

pub(crate) struct URef<T: ?Sized> {
    ptr: NonNull<T>,
}

impl<T: ?Sized> Copy for URef<T> {}

impl<T: ?Sized> Clone for URef<T> {
    fn clone(&self) -> Self {
        Self { ptr: self.ptr }
    }
}

impl<T: ?Sized> URef<T> {
    pub fn new(val: &T) -> Self {
        unsafe {
            URef {
                ptr: NonNull::new_unchecked(val as *const T as *mut T),
            }
        }
    }

    /// Creates an `URef` from a raw pointer
    #[inline]
    pub unsafe fn from_raw(val: *const T) -> URef<T> {
        URef {
            ptr: NonNull::new_unchecked(val as *mut _),
        }
    }

    /// Converts an `URef` into a raw pointer
    #[inline]
    pub fn into_raw(ptr: Self) -> *mut T {
        ptr.ptr.as_ptr()
    }
}

impl<T: ?Sized> Deref for URef<T> {
    type Target = T;

    #[inline]
    fn deref(&self) -> &T {
        self.as_ref()
    }
}

impl<T: ?Sized> AsRef<T> for URef<T> {
    #[inline]
    fn as_ref(&self) -> &T {
        unsafe { self.ptr.as_ref() }
    }
}

impl<T: ?Sized> Borrow<T> for URef<T> {
    #[inline]
    fn borrow(&self) -> &T {
        self.as_ref()
    }
}

impl<T: fmt::Debug + ?Sized> fmt::Debug for URef<T> {
    #[inline]
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        fmt::Debug::fmt(self.as_ref(), f)
    }
}

unsafe impl<T: ?Sized + Send> Send for URef<T> {}

unsafe impl<T: ?Sized + Sync> Sync for URef<T> {}

pub(crate) struct URefPointerOps<T: ?Sized>(PhantomData<T>);

impl<Pointer> URefPointerOps<Pointer> {
    /// Constructs an instance of `DefaultPointerOps`.
    #[inline]
    pub const fn new() -> URefPointerOps<Pointer> {
        URefPointerOps(PhantomData)
    }
}

impl<Pointer> Clone for URefPointerOps<Pointer> {
    #[inline]
    fn clone(&self) -> Self {
        *self
    }
}

impl<Pointer> Copy for URefPointerOps<Pointer> {}

unsafe impl<T: ?Sized> PointerOps for URefPointerOps<URef<T>> {
    type Value = T;
    type Pointer = URef<T>;

    #[inline]
    unsafe fn from_raw(&self, raw: *const T) -> URef<T> {
        URef::from_raw(raw as *mut T)
    }

    #[inline]
    fn into_raw(&self, ptr: URef<T>) -> *const T {
        URef::into_raw(ptr) as *const T
    }
}

#[macro_export]
macro_rules! uref_intrusive_adapter {
    (@impl
        $(#[$attr:meta])* $vis:vis $name:ident ($($args:tt),*)
        = $pointer:ty: $value:path { $field:ident: $link:ty } $($where_:tt)*
    ) => {
        #[allow(explicit_outlives_requirements)]
        $(#[$attr])*
        $vis struct $name<$($args),*> $($where_)* {
            link_ops: <$link as ::intrusive_collections::DefaultLinkOps>::Ops,
            pointer_ops: $crate::uref::URefPointerOps<$pointer>,
        }
        unsafe impl<$($args),*> Send for $name<$($args),*> $($where_)* {}
        unsafe impl<$($args),*> Sync for $name<$($args),*> $($where_)* {}
        impl<$($args),*> Copy for $name<$($args),*> $($where_)* {}
        impl<$($args),*> Clone for $name<$($args),*> $($where_)* {
            #[inline]
            fn clone(&self) -> Self {
                *self
            }
        }
        impl<$($args),*> Default for $name<$($args),*> $($where_)* {
            #[inline]
            fn default() -> Self {
                Self::NEW
            }
        }
        #[allow(dead_code)]
        impl<$($args),*> $name<$($args),*> $($where_)* {
            pub const NEW: Self = $name {
                link_ops: <$link as ::intrusive_collections::DefaultLinkOps>::NEW,
                pointer_ops: $crate::uref::URefPointerOps::<$pointer>::new(),
            };
            #[inline]
            pub fn new() -> Self {
                Self::NEW
            }
        }
        #[allow(dead_code, unsafe_code)]
        unsafe impl<$($args),*> ::intrusive_collections::Adapter for $name<$($args),*> $($where_)* {
            type LinkOps = <$link as ::intrusive_collections::DefaultLinkOps>::Ops;
            type PointerOps = $crate::uref::URefPointerOps<$pointer>;

            #[inline]
            unsafe fn get_value(&self, link: <Self::LinkOps as ::intrusive_collections::LinkOps>::LinkPtr) -> *const <Self::PointerOps as ::intrusive_collections::PointerOps>::Value {
                ::intrusive_collections::container_of!(link.as_ptr(), $value, $field)
            }
            #[inline]
            unsafe fn get_link(&self, value: *const <Self::PointerOps as ::intrusive_collections::PointerOps>::Value) -> <Self::LinkOps as ::intrusive_collections::LinkOps>::LinkPtr {
                // We need to do this instead of just accessing the field directly
                // to strictly follow the stack borrow rules.
                let ptr = (value as *const u8).add(::intrusive_collections::offset_of!($value, $field));
                core::ptr::NonNull::new_unchecked(ptr as *mut _)
            }
            #[inline]
            fn link_ops(&self) -> &Self::LinkOps {
                &self.link_ops
            }
            #[inline]
            fn link_ops_mut(&mut self) -> &mut Self::LinkOps {
                &mut self.link_ops
            }
            #[inline]
            fn pointer_ops(&self) -> &Self::PointerOps {
                &self.pointer_ops
            }
        }
    };
    (@find_generic
        $(#[$attr:meta])* $vis:vis $name:ident ($($prev:tt)*) > $($rest:tt)*
    ) => {
        uref_intrusive_adapter!(@impl
            $(#[$attr])* $vis $name ($($prev)*) $($rest)*
        );
    };
    (@find_generic
        $(#[$attr:meta])* $vis:vis $name:ident ($($prev:tt)*) $cur:tt $($rest:tt)*
    ) => {
        uref_intrusive_adapter!(@find_generic
            $(#[$attr])* $vis $name ($($prev)* $cur) $($rest)*
        );
    };
    (@find_if_generic
        $(#[$attr:meta])* $vis:vis $name:ident < $($rest:tt)*
    ) => {
        uref_intrusive_adapter!(@find_generic
            $(#[$attr])* $vis $name () $($rest)*
        );
    };
    (@find_if_generic
        $(#[$attr:meta])* $vis:vis $name:ident $($rest:tt)*
    ) => {
        uref_intrusive_adapter!(@impl
            $(#[$attr])* $vis $name () $($rest)*
        );
    };
    ($(#[$attr:meta])* $vis:vis $name:ident $($rest:tt)*) => {
        uref_intrusive_adapter!(@find_if_generic
            $(#[$attr])* $vis $name $($rest)*
        );
    };
}
