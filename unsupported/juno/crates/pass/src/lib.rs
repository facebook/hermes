/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Passes that operate on Juno's representation of JS.
//!
//! Provides transformation traits and the ability to compose them in a pipeline.

mod manager;
pub use manager::{Pass, PassManager};

mod passes;
