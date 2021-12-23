/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

use juno_support::atom_table::{Atom, AtomTable};
use juno_support::define_str_enum;
use libc::c_void;
use memoffset::offset_of;
use std::hash::{Hash, Hasher};
use std::ops::Deref;
use std::{
    cell::{Cell, UnsafeCell},
    fmt,
    marker::PhantomData,
    pin::Pin,
    ptr::NonNull,
    sync::atomic::{AtomicU32, Ordering},
};
use thiserror::Error;

#[macro_use]
mod def;
mod dump;
mod field;
mod kind;
mod validate;

pub use juno_support::source_manager::{SourceId, SourceLoc, SourceManager, SourceRange};

pub use field::NodeField;
pub use kind::NodeVariant;

pub use dump::{dump_json, Pretty};
pub use kind::*;
pub use validate::{validate_tree, validate_tree_pure, TreeValidationError, ValidationError};

/// ID which indicates a `StorageEntry` is free.
const FREE_ENTRY: u32 = 0;

/// A single entry in the heap.
#[derive(Debug)]
struct StorageEntry<'ctx> {
    /// ID of the context to which this entry belongs.
    /// Top bit is used as a mark bit, and flips meaning every time a GC happens.
    /// If this field is `0`, then this entry is free.
    ctx_id_markbit: Cell<u32>,

    /// Refcount of how many [`NodeRc`] point to this node.
    /// Entry may only be freed if this number is `0` and no other entries reference this entry
    /// directly.
    count: Cell<u32>,

    /// Actual node stored in this entry.
    inner: Node<'ctx>,
}

impl<'ctx> StorageEntry<'ctx> {
    unsafe fn from_node<'a>(node: &'a Node<'a>) -> &'a StorageEntry<'a> {
        let inner_offset = offset_of!(StorageEntry, inner) as isize;
        let inner = node as *const Node<'a>;
        &*(inner.offset(-inner_offset) as *const StorageEntry<'a>)
    }

    #[inline]
    fn set_markbit(&self, bit: bool) {
        let id = self.ctx_id_markbit.get();
        if bit {
            self.ctx_id_markbit.set(id | 1 << 31);
        } else {
            self.ctx_id_markbit.set(id & !(1 << 31));
        }
    }

    #[inline]
    fn markbit(&self) -> bool {
        (self.ctx_id_markbit.get() >> 31) != 0
    }

    fn is_free(&self) -> bool {
        self.ctx_id_markbit.get() == FREE_ENTRY
    }
}

/// Structure pointed to by `Context` and `NodeRc` to facilitate panicking if there are
/// outstanding `NodeRc` when the `Context` is dropped.
#[derive(Debug)]
struct NodeRcCounter {
    /// ID of the context owning the counter.
    ctx_id: u32,

    /// Number of [`NodeRc`]s allocated in this `Context`.
    /// Must be `0` when `Context` is dropped.
    count: Cell<usize>,
}

/// The storage for AST nodes.
///
/// Can be used to allocate and free nodes.
/// Nodes allocated in one `Context` must not be referenced by another `Context`'s AST.
#[derive(Debug)]
pub struct Context<'ast> {
    /// Unique number used to identify this context.
    id: u32,

    /// List of all the nodes stored in this context.
    /// Each element is a "chunk" of nodes.
    /// None of the chunks are ever resized after allocation.
    nodes: UnsafeCell<Vec<Vec<StorageEntry<'ast>>>>,

    /// First element of the free list if there is one.
    free: UnsafeCell<Vec<NonNull<StorageEntry<'ast>>>>,

    /// `NodeRc` count stored in a `Box` to ensure that `NodeRc`s can also point to it
    /// and decrement the count on drop.
    /// Placed separately to guard against `Context` moving, though relying on that behavior is
    /// technically unsafe.
    noderc_count: Pin<Box<NodeRcCounter>>,

    /// Capacity at which to allocate the next chunk.
    /// Doubles every chunk until reaching [`MAX_CHUNK_CAPACITY`].
    next_chunk_capacity: Cell<usize>,

    /// All identifiers are kept here.
    atom_tab: AtomTable,

    /// Source manager of this context.
    source_mgr: SourceManager,

    /// `true` if `1` indicates an entry is marked, `false` if `0` indicates an entry is marked.
    /// Flipped every time GC occurs.
    markbit_marked: bool,

    /// Whether strict mode has been forced.
    strict_mode: bool,

    /// Whether to warn about undefined variables in strict mode functions.
    pub warn_undefined: bool,
}

const MIN_CHUNK_CAPACITY: usize = 1 << 10;
const MAX_CHUNK_CAPACITY: usize = MIN_CHUNK_CAPACITY * (1 << 10);

impl Default for Context<'_> {
    fn default() -> Self {
        Self::new()
    }
}

impl<'ast> Context<'ast> {
    /// Allocate a new `Context` with a new ID.
    pub fn new() -> Self {
        static NEXT_ID: AtomicU32 = AtomicU32::new(FREE_ENTRY + 1);
        let id = NEXT_ID.fetch_add(1, Ordering::Relaxed);
        let result = Self {
            id,
            nodes: Default::default(),
            free: Default::default(),
            noderc_count: Pin::new(Box::new(NodeRcCounter {
                ctx_id: id,
                count: Cell::new(0),
            })),
            atom_tab: Default::default(),
            source_mgr: Default::default(),
            next_chunk_capacity: Cell::new(MIN_CHUNK_CAPACITY),
            markbit_marked: true,
            strict_mode: false,
            warn_undefined: false,
        };
        result.new_chunk();
        result
    }

    /// Allocate a new `Node` in this `Context`.
    fn alloc<'s>(&'s self, n: Node<'_>) -> &'s Node<'s> {
        let free = unsafe { &mut *self.free.get() };
        let nodes: &mut Vec<Vec<StorageEntry>> = unsafe { &mut *self.nodes.get() };
        // Transmutation is safe here, because `Node`s can only be allocated through
        // this path and only one GCLock can be made available at a time per thread.
        let node: Node<'ast> = unsafe { std::mem::transmute(n) };
        let entry = if let Some(mut entry) = free.pop() {
            let entry = unsafe { entry.as_mut() };
            debug_assert!(
                entry.ctx_id_markbit.get() == FREE_ENTRY,
                "Incorrect context ID"
            );
            debug_assert!(entry.count.get() == 0, "Freed entry has pointers to it");
            entry.ctx_id_markbit.set(self.id);
            entry.set_markbit(!self.markbit_marked);
            entry.inner = node;
            entry
        } else {
            let chunk = nodes.last().unwrap();
            if chunk.len() >= chunk.capacity() {
                self.new_chunk();
            }
            let chunk = nodes.last_mut().unwrap();
            let entry = StorageEntry {
                ctx_id_markbit: Cell::new(self.id),
                count: Cell::new(0),
                inner: node,
            };
            entry.set_markbit(!self.markbit_marked);
            chunk.push(entry);
            chunk.last().unwrap()
        };
        &entry.inner
    }

    /// Allocate a new chunk in the node storage.
    fn new_chunk(&self) {
        let nodes = unsafe { &mut *self.nodes.get() };
        let capacity = self.next_chunk_capacity.get();
        nodes.push(Vec::with_capacity(capacity));

        // Double the capacity if there's room.
        if capacity < MAX_CHUNK_CAPACITY {
            self.next_chunk_capacity.set(capacity * 2);
        }
    }

    /// Return the atom table.
    pub fn atom_table(&self) -> &AtomTable {
        &self.atom_tab
    }

    /// Add a string to the identifier table.
    #[inline]
    pub fn atom<V: Into<String> + AsRef<str>>(&self, value: V) -> Atom {
        self.atom_tab.atom(value)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str(&self, index: Atom) -> &str {
        self.atom_tab.str(index)
    }

    /// Return an immutable reference to SourceManager
    pub fn sm(&self) -> &SourceManager {
        &self.source_mgr
    }

    /// Return a mutable reference to SourceManager
    pub fn sm_mut(&mut self) -> &mut SourceManager {
        &mut self.source_mgr
    }

    /// Return true if strict mode has been forced globally.
    pub fn strict_mode(&self) -> bool {
        self.strict_mode
    }

    /// Enable strict mode. Note that it cannot be unset.
    pub fn enable_strict_mode(&mut self) {
        self.strict_mode = true;
    }

    pub fn gc(&mut self) {
        let nodes = unsafe { &mut *self.nodes.get() };
        let free = unsafe { &mut *self.free.get() };

        // Begin by collecting all the roots: entries with non-zero refcount.
        let mut roots: Vec<&StorageEntry> = vec![];
        for chunk in nodes.iter() {
            for entry in chunk.iter() {
                if entry.is_free() {
                    continue;
                }
                debug_assert!(
                    entry.markbit() != self.markbit_marked,
                    "Entry marked before start of GC: {:?}\nentry.markbit()={}\nmarkbit_marked={}",
                    &entry,
                    entry.markbit(),
                    self.markbit_marked,
                );
                if entry.count.get() > 0 {
                    roots.push(entry);
                }
            }
        }

        struct Marker {
            markbit_marked: bool,
        }

        impl<'gc> Visitor<'gc> for Marker {
            fn call(&mut self, gc: &'gc GCLock, node: &'gc Node<'gc>, _path: Option<Path<'gc>>) {
                let entry = unsafe { StorageEntry::from_node(node) };
                if entry.markbit() == self.markbit_marked {
                    // Stop visiting early if we've already marked this part,
                    // because we must have also marked all the children.
                    return;
                }
                entry.set_markbit(self.markbit_marked);
                node.visit_children(gc, self);
            }
        }

        // Use a visitor to mark every node reachable from roots.
        let mut marker = Marker {
            markbit_marked: self.markbit_marked,
        };
        {
            let gc = GCLock::new(self);
            for root in &roots {
                root.inner.visit(&gc, &mut marker, None);
            }
        }

        for chunk in nodes.iter_mut() {
            for entry in chunk.iter_mut() {
                if entry.is_free() {
                    // Skip free entries.
                    continue;
                }
                if entry.count.get() > 0 {
                    // Keep referenced entries alive.
                    continue;
                }
                if entry.markbit() == self.markbit_marked {
                    // Keep marked entries alive.
                    continue;
                }
                // Passed all checks, this entry is free.
                entry.ctx_id_markbit.set(FREE_ENTRY);
                free.push(unsafe { NonNull::new_unchecked(entry as *mut StorageEntry) });
            }
        }

        self.markbit_marked = !self.markbit_marked;
    }
}

impl Drop for Context<'_> {
    /// Ensure that there are no outstanding `NodeRc`s into this `Context` which will be
    /// invalidated once it is dropped.
    ///
    /// # Panics
    ///
    /// Will panic if there are any `NodeRc`s stored when this `Context` is dropped.
    fn drop(&mut self) {
        if self.noderc_count.count.get() > 0 {
            #[cfg(debug_assertions)]
            {
                // In debug mode, provide more information on which node was leaked.
                let nodes = unsafe { &*self.nodes.get() };
                for chunk in nodes {
                    for entry in chunk {
                        assert!(
                            entry.count.get() == 0,
                            "NodeRc must not outlive Context: {:#?}\n",
                            &entry.inner
                        );
                    }
                }
            }
            // In release mode, just panic immediately.
            panic!("NodeRc must not outlive Context");
        }
    }
}

thread_local! {
    /// Whether there exists a `GCLock` on the current thread.
    static GCLOCK_IN_USE: Cell<bool> = Cell::new(false);
}

/// A way to view the [`Context`].
///
/// Provides the user the ability to create new nodes and dereference [`NodeRc`].
///
/// **At most one is allowed to be active in any thread at any time.**
/// This is to ensure no `&Node` can be shared between `Context`s.
pub struct GCLock<'ast, 'ctx> {
    ctx: &'ctx mut Context<'ast>,
}

impl Drop for GCLock<'_, '_> {
    fn drop(&mut self) {
        GCLOCK_IN_USE.with(|flag| {
            flag.set(false);
        });
    }
}

impl<'ast, 'ctx> GCLock<'ast, 'ctx> {
    /// # Panics
    ///
    /// Will panic if there is already an active `GCLock` on this thread.
    pub fn new(ctx: &'ctx mut Context<'ast>) -> Self {
        GCLOCK_IN_USE.with(|flag| {
            if flag.get() {
                panic!("Attempt to create multiple GCLocks in a single thread");
            }
            flag.set(true);
        });
        GCLock { ctx }
    }

    /// Allocate a node in the `ctx`.
    #[inline]
    fn alloc(&self, n: Node) -> &Node {
        self.ctx.alloc(n)
    }

    /// Return a reference to the owning Context.
    pub fn ctx(&self) -> &Context<'ast> {
        self.ctx
    }

    /// Add a string to the identifier table.
    #[inline]
    pub fn atom<V: Into<String> + AsRef<str>>(&self, value: V) -> Atom {
        self.ctx.atom(value)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str(&self, index: Atom) -> &str {
        self.ctx.str(index)
    }

    /// Return an immutable reference to SourceManager.
    #[inline]
    pub fn sm(&self) -> &SourceManager {
        self.ctx.sm()
    }

    /// Return a mutable reference to SourceManager.
    #[inline]
    pub fn sm_mut(&mut self) -> &mut SourceManager {
        self.ctx.sm_mut()
    }
}

/// A wrapper around Node&, with "shallow" hashing and equality, suitable for
/// hash tables.
#[derive(Debug, Copy, Clone)]
pub struct NodePtr<'gc>(pub &'gc Node<'gc>);

impl<'gc> NodePtr<'gc> {
    pub fn from_node(node: &'gc Node<'gc>) -> Self {
        Self(node)
    }
}

impl<'gc> PartialEq for NodePtr<'gc> {
    fn eq(&self, other: &Self) -> bool {
        std::ptr::eq(self.0, other.0)
    }
}

impl Eq for NodePtr<'_> {}

impl Hash for NodePtr<'_> {
    fn hash<H: Hasher>(&self, state: &mut H) {
        (self.0 as *const Node).hash(state)
    }
}

impl<'gc> Deref for NodePtr<'gc> {
    type Target = Node<'gc>;
    fn deref(&self) -> &'gc Self::Target {
        self.0
    }
}

impl<'gc> AsRef<Node<'gc>> for NodePtr<'gc> {
    fn as_ref(&self) -> &'gc Node<'gc> {
        self.0
    }
}

impl<'gc> From<&'gc Node<'gc>> for NodePtr<'gc> {
    fn from(node: &'gc Node<'gc>) -> Self {
        NodePtr(node)
    }
}

/// Reference counted pointer to a [`Node`] in any [`Context`].
///
/// It can be used to keep references to `Node`s outside of the lifetime of a [`GCLock`],
/// but the only way to derefence and inspect the `Node` is to use a `GCLock`.
#[derive(Debug, Eq)]
pub struct NodeRc {
    /// The `NodeRcCounter` counting for the `Context` to which this belongs.
    counter: NonNull<NodeRcCounter>,

    /// Pointer to the `StorageEntry` containing the `Node`.
    /// Stored as `c_void` to avoid specifying lifetimes, as dereferencing is checked manually.
    entry: NonNull<c_void>,
}

impl Hash for NodeRc {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.entry.hash(state)
    }
}

impl PartialEq for NodeRc {
    fn eq(&self, other: &Self) -> bool {
        self.entry == other.entry
    }
}

impl Drop for NodeRc {
    fn drop(&mut self) {
        let entry = unsafe { self.entry().as_mut() };
        let c = entry.count.get();
        debug_assert!(c > 0);
        entry.count.set(c - 1);

        let noderc_count = unsafe { self.counter.as_mut() };
        let c = noderc_count.count.get();
        debug_assert!(c > 0);
        noderc_count.count.set(c - 1);
    }
}

impl Clone for NodeRc {
    /// Cloning a `NodeRc` increments refcounts on the entry and the context.
    fn clone(&self) -> Self {
        let mut cloned = NodeRc { ..*self };

        let entry = unsafe { cloned.entry().as_mut() };
        let c = entry.count.get();
        entry.count.set(c + 1);

        let noderc_count = unsafe { cloned.counter.as_mut() };
        let c = noderc_count.count.get();
        noderc_count.count.set(c + 1);

        cloned
    }
}

impl NodeRc {
    /// Turn a node reference into a `NodeRc` for storage outside `GCLock`.
    pub fn from_node<'gc>(gc: &'gc GCLock, node: &'gc Node<'gc>) -> NodeRc {
        let inner_offset = offset_of!(StorageEntry, inner) as isize;
        let inner = node as *const Node<'gc>;
        unsafe {
            let entry: &mut StorageEntry = &mut *(inner.offset(-inner_offset) as *mut StorageEntry);
            Self::from_entry(gc, entry)
        }
    }

    /// Return the actual `Node` that `self` points to.
    ///
    /// # Panics
    ///
    /// Will panic if `gc` is not for the same context as this `NodeRc` was created in.
    pub fn node<'gc>(&'_ self, gc: &'gc GCLock<'_, '_>) -> &'gc Node {
        unsafe {
            assert_eq!(
                self.counter.as_ref().ctx_id,
                gc.ctx.id,
                "Attempt to derefence NodeRc allocated context {} in context {}",
                self.counter.as_ref().ctx_id,
                gc.ctx.id
            );
            &self.entry().as_ref().inner
        }
    }

    /// Get the pointer to the `StorageEntry`.
    unsafe fn entry(&self) -> NonNull<StorageEntry> {
        let outer = self.entry.as_ptr() as *mut StorageEntry;
        NonNull::new_unchecked(outer)
    }

    unsafe fn from_entry(gc: &GCLock, entry: &StorageEntry<'_>) -> NodeRc {
        let c = entry.count.get();
        entry.count.set(c + 1);

        let c = gc.ctx.noderc_count.count.get();
        gc.ctx.noderc_count.count.set(c + 1);

        NodeRc {
            counter: NonNull::new_unchecked(gc.ctx.noderc_count.as_ref().get_ref()
                as *const NodeRcCounter
                as *mut NodeRcCounter),
            entry: NonNull::new_unchecked(entry as *const StorageEntry as *mut c_void),
        }
    }
}

/// Indicates the path to the current node.
#[derive(Debug, Copy, Clone)]
pub struct Path<'a> {
    /// Parent node.
    pub parent: &'a Node<'a>,

    /// Field name in the path node.
    pub field: NodeField,
}

impl<'a> Path<'a> {
    pub fn new(parent: &'a Node<'a>, field: NodeField) -> Path<'a> {
        Path { parent, field }
    }
}

/// Trait implemented by those who call the visit functionality.
pub trait Visitor<'gc> {
    /// Visit the Node `node` with the given `path`.
    fn call(&mut self, ctx: &'gc GCLock, node: &'gc Node<'gc>, path: Option<Path<'gc>>);
}

/// Indicates what mutation occurred to an element of the AST during [`VisitorMut`] use.
#[derive(Debug)]
pub enum TransformResult<T> {
    /// No change to the element.
    Unchanged,

    /// Element must be removed, if possible.
    /// If the element cannot be removed, it will be replaced with EmptyStatement.
    /// **Only statements may be removed from non-optional fields,
    /// as removing expressions would result in an invalid AST.**
    Removed,

    /// Element should be swapped out for the wrapped element.
    Changed(T),

    /// Element should be swapped out for multiple wrapped elements.
    Expanded(Vec<T>),
}

/// Trait implemented by those who call the visit functionality.
pub trait VisitorMut<'gc> {
    /// Visit the Node `node` with the given `path`.
    fn call(
        &mut self,
        ctx: &'gc GCLock,
        node: &'gc Node<'gc>,
        path: Option<Path<'gc>>,
    ) -> TransformResult<&'gc Node<'gc>>;
}

/// Metadata common to all AST nodes.
///
/// Stored inside [`Node`] and must not be constructed directly by users.
/// Instead it should be allocated in the GC by using the `Builder`.
/// Must not derive `Clone` or `Copy` in order to avoid `Node` being allocated
/// by callers outside this module.
#[derive(Debug)]
pub struct NodeMetadata<'a> {
    phantom: PhantomData<&'a Node<'a>>,
    pub range: SourceRange,
}

impl<'a> NodeMetadata<'a> {
    fn build_template(template: TemplateMetadata<'a>) -> NodeMetadata<'a> {
        NodeMetadata {
            phantom: template.phantom,
            range: template.range,
        }
    }
}

/// Metadata common to all AST nodes used in templates.
/// Stored inside template notes and may be constructed directly by users.
#[derive(Debug, Clone)]
pub struct TemplateMetadata<'a> {
    pub phantom: PhantomData<&'a Node<'a>>,
    pub range: SourceRange,
}

impl Default for TemplateMetadata<'_> {
    fn default() -> Self {
        Self {
            phantom: Default::default(),
            range: SourceRange {
                file: SourceId::INVALID,
                start: SourceLoc::invalid(),
                end: SourceLoc::invalid(),
            },
        }
    }
}

/// JS identifier represented as valid UTF-8.
pub type NodeLabel = Atom;

/// A list of nodes owned by a path.
pub type NodeList<'a> = Vec<&'a Node<'a>>;

/// JS string literals don't have to contain valid UTF-8,
/// so we wrap a `Vec<u16>`, which allows us to represent UTF-16 characters
/// without being subject to Rust's restrictions on [`String`].
#[derive(Clone)]
pub struct NodeString {
    pub str: Vec<u16>,
}

impl fmt::Debug for NodeString {
    /// Format the NodeString as a `u""` string to make it more readable
    /// when debugging.
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        write!(f, "u{:?}", String::from_utf16_lossy(&self.str))
    }
}

#[derive(Debug, Copy, Clone, Error)]
#[error("Invalid string property for AST node")]
pub struct TryFromStringError;

define_str_enum!(
    UnaryExpressionOperator,
    TryFromStringError,
    (Delete, "delete"),
    (Void, "void"),
    (Typeof, "typeof"),
    (Plus, "+"),
    (Minus, "-"),
    (BitNot, "~"),
    (Not, "!"),
);

define_str_enum!(
    BinaryExpressionOperator,
    TryFromStringError,
    (LooseEquals, "=="),
    (LooseNotEquals, "!="),
    (StrictEquals, "==="),
    (StrictNotEquals, "!=="),
    (Less, "<"),
    (LessEquals, "<="),
    (Greater, ">"),
    (GreaterEquals, ">="),
    (LShift, "<<"),
    (RShift, ">>"),
    (RShift3, ">>>"),
    (Plus, "+"),
    (Minus, "-"),
    (Mult, "*"),
    (Div, "/"),
    (Mod, "%"),
    (BitOr, "|"),
    (BitXor, "^"),
    (BitAnd, "&"),
    (Exp, "**"),
    (In, "in"),
    (Instanceof, "instanceof"),
);

define_str_enum!(
    LogicalExpressionOperator,
    TryFromStringError,
    (And, "&&"),
    (Or, "||"),
    (NullishCoalesce, "??"),
);

define_str_enum!(
    UpdateExpressionOperator,
    TryFromStringError,
    (Increment, "++"),
    (Decrement, "--"),
);

define_str_enum!(
    AssignmentExpressionOperator,
    TryFromStringError,
    (Assign, "="),
    (LShiftAssign, "<<="),
    (RShiftAssign, ">>="),
    (RShift3Assign, ">>>="),
    (PlusAssign, "+="),
    (MinusAssign, "-="),
    (MultAssign, "*="),
    (DivAssign, "/="),
    (ModAssign, "%="),
    (BitOrAssign, "|="),
    (BitXorAssign, "^="),
    (BitAndAssign, "&="),
    (ExpAssign, "**="),
    (LogicalOrAssign, "||="),
    (LogicalAndAssign, "&&="),
    (NullishCoalesceAssign, "??="),
);

define_str_enum!(
    VariableDeclarationKind,
    TryFromStringError,
    (Var, "var"),
    (Let, "let"),
    (Const, "const"),
);

define_str_enum!(
    PropertyKind,
    TryFromStringError,
    (Init, "init"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    MethodDefinitionKind,
    TryFromStringError,
    (Method, "method"),
    (Constructor, "constructor"),
    (Get, "get"),
    (Set, "set"),
);

define_str_enum!(
    ImportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
    (Typeof, "typeof"),
);

define_str_enum!(
    ExportKind,
    TryFromStringError,
    (Value, "value"),
    (Type, "type"),
);

impl<'gc> Node<'gc> {
    pub fn visit<V: Visitor<'gc>>(
        &'gc self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Option<Path<'gc>>,
    ) {
        visitor.call(ctx, self, path);
    }

    /// Visit this node with `visitor` and return the modified root node.
    /// If the root node is to be removed, return `None`.
    pub fn visit_mut<V: VisitorMut<'gc>>(
        &'gc self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Option<Path<'gc>>,
    ) -> Option<&'gc Node<'gc>> {
        match visitor.call(ctx, self, path) {
            TransformResult::Unchanged => Some(self),
            TransformResult::Removed => None,
            TransformResult::Changed(new_node) => Some(new_node),
            TransformResult::Expanded(_) => {
                panic!("Attempt to replace a single node with multiple");
            }
        }
    }
}

/// Trait implemented by possible child types of `NodeKind`.
trait NodeChild<'gc>
where
    Self: std::marker::Sized,
{
    type Out;

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child<V: Visitor<'gc>>(self, _ctx: &'gc GCLock, _visitor: &mut V, _path: Path<'gc>) {}

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        _ctx: &'gc GCLock,
        _visitor: &mut V,
        _path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        TransformResult::Unchanged
    }

    /// A way to "clone" children without actually implementing Clone.
    /// Not implementing Clone prevents accidental creation of `Node` references
    /// on the stack.
    /// Can't provide a default implementation here because associated type default definitions
    /// (for `Out`) are unstable.
    fn duplicate(self) -> Self::Out;
}

impl NodeChild<'_> for f64 {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for bool {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for NodeLabel {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for UnaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for BinaryExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for LogicalExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for UpdateExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for AssignmentExpressionOperator {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for VariableDeclarationKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for PropertyKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for MethodDefinitionKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ImportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for ExportKind {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl NodeChild<'_> for NodeString {
    type Out = Self;
    fn duplicate(self) -> Self::Out {
        self
    }
}
impl<'gc> NodeChild<'gc> for &NodeString {
    type Out = NodeString;
    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}
impl NodeChild<'_> for &Option<NodeString> {
    type Out = Option<NodeString>;
    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

impl<'gc, T: NodeChild<'gc> + NodeChild<'gc, Out = T>> NodeChild<'gc> for Option<T> {
    type Out = Self;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(t) = self {
            t.visit_child(ctx, visitor, path);
        }
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    panic!("Attempt to replace a single optional node with multiple");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        self.map(|inner| inner.duplicate())
    }
}

impl<'gc> NodeChild<'gc> for &Option<&'gc Node<'gc>> {
    type Out = Option<&'gc Node<'gc>>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(t) = *self {
            t.visit_child(ctx, visitor, path);
        }
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    panic!("Attempt to replace a single optional node with multiple");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        *self
    }
}

impl<'gc> NodeChild<'gc> for &'gc Node<'gc> {
    type Out = Self;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        visitor.call(ctx, self, Some(path));
    }

    /// Visit this child of the given `node`.
    /// Should be no-op for any type that doesn't contain pointers to other
    /// `Node`s.
    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        match visitor.call(ctx, self, Some(path)) {
            TransformResult::Removed => {
                TransformResult::Changed(builder::EmptyStatement::build_template(
                    ctx,
                    template::EmptyStatement {
                        metadata: TemplateMetadata {
                            phantom: Default::default(),
                            range: SourceRange {
                                file: self.range().file,
                                start: self.range().start,
                                end: self.range().start,
                            },
                        },
                    },
                ))
            }
            result => result,
        }
    }

    fn duplicate(self) -> Self::Out {
        self
    }
}

impl<'gc> NodeChild<'gc> for &NodeList<'gc> {
    type Out = NodeList<'gc>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        for child in self {
            visitor.call(ctx, *child, Some(path));
        }
    }

    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        let mut index = 0;
        let len = self.len();
        // Assume no copies to start.
        while index < len {
            let node = visitor.call(ctx, self[index], Some(path));
            if let Unchanged = node {
                index += 1;
                continue;
            }
            // First change found, either removed or changed node.
            let mut result: Self::Out = vec![];
            // Fill in the elements we skippped.
            for elem in self.iter().take(index) {
                result.push(elem);
            }
            // If the node was changed or expanded, push it.
            match node {
                Changed(new_node) => result.push(new_node),
                Expanded(new_nodes) => {
                    for node in new_nodes {
                        result.push(node);
                    }
                }
                Removed => {}
                Unchanged => {
                    unreachable!("checked for unchanged above")
                }
            };
            index += 1;
            // Fill the rest of the elements.
            while index < len {
                match visitor.call(ctx, self[index], Some(path)) {
                    Unchanged => result.push(self[index]),
                    Removed => {}
                    Changed(new_node) => result.push(new_node),
                    Expanded(new_nodes) => {
                        for node in new_nodes {
                            result.push(node);
                        }
                    }
                }
                index += 1;
            }
            return Changed(result);
        }
        Unchanged
    }

    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

impl<'gc> NodeChild<'gc> for &Option<NodeList<'gc>> {
    type Out = Option<NodeList<'gc>>;

    fn visit_child<V: Visitor<'gc>>(self, ctx: &'gc GCLock, visitor: &mut V, path: Path<'gc>) {
        if let Some(list) = self {
            for child in list {
                visitor.call(ctx, *child, Some(path));
            }
        }
    }

    fn visit_child_mut<V: VisitorMut<'gc>>(
        self,
        ctx: &'gc GCLock,
        visitor: &mut V,
        path: Path<'gc>,
    ) -> TransformResult<Self::Out> {
        use TransformResult::*;
        match self.as_ref() {
            None => Unchanged,
            Some(inner) => match inner.visit_child_mut(ctx, visitor, path) {
                Unchanged => Unchanged,
                Removed => Changed(None),
                Changed(new_node) => Changed(Some(new_node)),
                Expanded(_) => {
                    unreachable!("NodeList::visit_child_mut cannot return Expanded");
                }
            },
        }
    }

    fn duplicate(self) -> Self::Out {
        self.clone()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;

    #[test]
    fn test_string_literal() {
        assert_eq!(
            "u\"ABC\"",
            format!(
                "{:?}",
                NodeString {
                    str: vec!['A' as u16, 'B' as u16, 'C' as u16],
                }
            )
        );
    }

    #[test]
    fn test_node_ref() {
        let mut ctx = Context::new();
        let lock = GCLock::new(&mut ctx);

        let mut m = HashMap::new();

        let n1 = NodePtr::from(builder::NumericLiteral::build_template(
            &lock,
            template::NumericLiteral {
                metadata: Default::default(),
                value: 10.0,
            },
        ));
        let n2 = NodePtr::from(builder::NumericLiteral::build_template(
            &lock,
            template::NumericLiteral {
                metadata: Default::default(),
                value: 20.0,
            },
        ));
        let n25 = Box::new(n2);
        assert_ne!(n1, n2);
        assert_eq!(n2, *n25);
        m.insert(n1, 10);
        m.insert(n2, 20);

        assert_eq!(10, *m.get(&n1).unwrap());
        assert_eq!(20, *m.get(&n2).unwrap());
        assert_eq!(20, *m.get(&*n25).unwrap());
    }
}
