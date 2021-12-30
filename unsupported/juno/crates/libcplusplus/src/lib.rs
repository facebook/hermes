/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! This crate links the correct libc++ or libstdc++.
//! The library itself contains no code, all code is in build.rs.
//! To link the C++ std library, take a dependency on this crate.

#![no_std]
