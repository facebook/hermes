/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Juno abstract syntax tree.
//!
//! Provides a transformable AST which is stored in a garbage-collected heap.
//! All nodes are stored in [`Context`], which handles memory management of the nodes
//! and exposes a safe API.
//!
//! Allocation and viewing of nodes must be done via the use of a [`GCLock`],
//! **of which there must be only one active per thread at any time**,
//! to avoid accidentally mixing `Node`s between `Context`.
//! The `GCLock` will provide `&'gc Node<'gc>`,
//! i.e. a `Node` that does not outlive the `GCLock` and which references other `Node`s which
//! also do not outlive the `GCLock`.
//!
//! Nodes are allocated and cloned/modified by using the various `Builder` structs,
//! for example [`NumericLiteralBuilder`].
//! Builder structs have `build_template` functions that take "template" structs,
//! which have the same general structure as the various node kinds, but are only used
//! for building/allocating nodes in the `Context`.
//!
//! Visitor patterns are provided by [`Visitor`] and [`VisitorMut`].

#[macro_use]
mod def;

mod context;
mod dump;
mod field;
mod kind;
mod node_child;
mod node_enums;
mod validate;
mod visitor;

pub use context::Context;
pub use context::GCLock;
pub use context::NodePtr;
pub use context::NodeRc;
pub use dump::dump_json;
pub use dump::Pretty;
pub use field::NodeField;
pub use juno_support::source_manager::SourceId;
pub use juno_support::source_manager::SourceLoc;
pub use juno_support::source_manager::SourceManager;
pub use juno_support::source_manager::SourceRange;
pub use kind::NodeVariant;
pub use kind::*;
pub use node_child::NodeLabel;
pub use node_child::NodeList;
pub use node_child::NodeMetadata;
pub use node_child::NodeString;
pub use node_child::TemplateMetadata;
pub use node_enums::*;
pub use validate::validate_tree;
pub use validate::validate_tree_pure;
pub use validate::TreeValidationError;
pub use validate::ValidationError;
pub use visitor::Path;
pub use visitor::TransformResult;
pub use visitor::Visitor;
pub use visitor::VisitorMut;
