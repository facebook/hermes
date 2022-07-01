/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! # Semantic Analyzer
//!
//! This module performs semantic analysis of the AST. That includes resolving
//! identifiers to corresponding declarations, resolving `break`/`continue`
//! targets, and performing validation (things like assigning to a non-lvalue,
//! names disallowed in strict mode, etc).

mod decl_collector;
mod keywords;
mod known_globals;
mod resolver;
mod sem_context;

pub use resolver::resolve_module;
pub use resolver::resolve_program;
pub use sem_context::*;
