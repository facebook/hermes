/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

macro_rules! declare_opaque_id {
    ($name:ident) => {
        #[derive(Copy, Clone, Debug, Eq, PartialEq, Hash)]
        pub struct $name(std::num::NonZeroU32);
        impl $name {
            #[inline]
            fn new(v: usize) -> Self {
                debug_assert!(v < u32::MAX as usize);
                unsafe { Self::new_unchecked(v) }
            }
            #[inline]
            pub const unsafe fn new_unchecked(v: usize) -> Self {
                Self(std::num::NonZeroU32::new_unchecked((v + 1) as u32))
            }
            pub fn as_usize(self) -> usize {
                (self.0.get() - 1) as usize
            }
        }
    };
}
