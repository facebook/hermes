/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//! Garbage-collected Storage structures for AST nodes.

use crate::Node;
use crate::Path;
use crate::SourceManager;
use crate::Visitor;
use juno_support::atom_table::Atom;
use juno_support::atom_table::AtomTable;
use juno_support::atom_table::AtomU16;
use juno_support::Deque;
use juno_support::HeapSize;
use libc::c_void;
use memoffset::offset_of;
use std::cell::Cell;
use std::cell::UnsafeCell;
use std::hash::Hash;
use std::hash::Hasher;
use std::ops::Deref;
use std::pin::Pin;
use std::ptr::NonNull;
use std::sync::atomic::AtomicU32;
use std::sync::atomic::Ordering;

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

/// A single entry in the NodeList storage.
/// These are also immutable from the user's perspective, like `Node`s,
/// but they are temporarily mutated here during construction only, in order to append elements.
#[derive(Debug)]
pub(crate) struct NodeListElement<'ctx> {
    /// ID of the context to which this entry belongs.
    /// Top bit is used as a mark bit, and flips meaning every time a GC happens.
    /// If this field is `0`, then this entry is free.
    ctx_id_markbit: Cell<u32>,

    /// Actual node stored in this entry.
    /// Must not be null, because empty lists are represented as null pointers in the [`NodeList`].
    pub inner: *const Node<'ctx>,

    /// Pointer to the next element in the NodeList.
    /// Stored in a `Cell` to allow for simple appends.
    pub next: Cell<*const NodeListElement<'ctx>>,
}

impl<'ctx> NodeListElement<'ctx> {
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
    nodes: UnsafeCell<Deque<StorageEntry<'ast>>>,

    /// Free list for AST nodes.
    free_nodes: UnsafeCell<Vec<NonNull<StorageEntry<'ast>>>>,

    /// Every `NodeListElement` allocated in this context.
    /// These store the links in the linked lists.
    list_elements: UnsafeCell<Deque<NodeListElement<'ast>>>,

    /// Free list for `NodeListElement`s.
    free_list_elements: UnsafeCell<Vec<NonNull<NodeListElement<'ast>>>>,

    /// `NodeRc` count stored in a `Box` to ensure that `NodeRc`s can also point to it
    /// and decrement the count on drop.
    /// Placed separately to guard against `Context` moving, though relying on that behavior is
    /// technically unsafe.
    noderc_count: Pin<Box<NodeRcCounter>>,

    /// All identifiers are kept here.
    pub atom_table: AtomTable,

    /// Source manager of this context.
    pub source_mgr: SourceManager,

    /// `true` if `1` indicates an entry is marked, `false` if `0` indicates an entry is marked.
    /// Flipped every time GC occurs.
    markbit_marked: bool,

    /// Whether strict mode has been forced.
    strict_mode: bool,

    /// Whether to warn about undefined variables in strict mode functions.
    pub warn_undefined: bool,
}

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
        Self {
            id,
            nodes: Default::default(),
            free_nodes: Default::default(),
            list_elements: Default::default(),
            free_list_elements: Default::default(),
            noderc_count: Pin::new(Box::new(NodeRcCounter {
                ctx_id: id,
                count: Cell::new(0),
            })),
            atom_table: Default::default(),
            source_mgr: Default::default(),
            markbit_marked: true,
            strict_mode: false,
            warn_undefined: false,
        }
    }

    /// Acquire a [`GCLock`] on this `Context`.
    /// This is just a more ergonomic way to call `GCLock::new`.
    pub fn lock<'ctx>(&'ctx mut self) -> GCLock<'ast, 'ctx> {
        GCLock::new(self)
    }

    /// Allocate a new `Node` in this `Context`.
    pub(crate) fn alloc<'s>(&'s self, n: Node<'_>) -> &'s Node<'s> {
        let free = unsafe { &mut *self.free_nodes.get() };
        let nodes: &mut Deque<StorageEntry<'ast>> = unsafe { &mut *self.nodes.get() };
        let node = unsafe { std::mem::transmute(n) };
        let entry: &StorageEntry<'ast> = if let Some(mut entry) = free.pop() {
            let entry: &mut StorageEntry<'ast> = unsafe { entry.as_mut() };
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
            let entry: &StorageEntry = nodes.push(StorageEntry {
                ctx_id_markbit: Cell::new(self.id),
                count: Cell::new(0),
                inner: node,
            });
            entry.set_markbit(!self.markbit_marked);
            entry
        };
        // Transmute here to handle the fact that Cell<> is invariant over its type,
        // meaning the lifetime doesn't automatically narrow from `'ast` to `'s`.
        unsafe { std::mem::transmute(&entry.inner) }
    }

    /// Allocate a list element in the context with the provided previous element if it exists.
    /// `prev` will be updated to point to `node` as its next element.
    pub(crate) fn append_list_element<'a>(
        &'a self,
        prev: Option<&'a NodeListElement<'a>>,
        node: &'a Node<'a>,
    ) -> &'a NodeListElement<'a> {
        let elements: &mut Deque<NodeListElement<'ast>> = unsafe { &mut *self.list_elements.get() };
        let free = unsafe { &mut *self.free_list_elements.get() };
        // Transmutation is safe here, because `Node`s can only be allocated through
        // this path and only one GCLock can be made available at a time per thread.
        let node: &'ast Node<'ast> = unsafe { std::mem::transmute(node) };
        let prev: Option<&'ast NodeListElement<'ast>> = unsafe { std::mem::transmute(prev) };
        let entry = if let Some(mut entry) = free.pop() {
            let entry: &mut NodeListElement<'ast> = unsafe { entry.as_mut() };
            debug_assert!(
                entry.ctx_id_markbit.get() == FREE_ENTRY,
                "Incorrect context ID"
            );
            entry.ctx_id_markbit.set(self.id);
            entry.set_markbit(!self.markbit_marked);
            entry.inner = node;
            if let Some(prev) = prev {
                entry.next.set(std::ptr::null());
                prev.next.set(entry as *const _);
            }
            entry
        } else {
            let entry = elements.push(NodeListElement {
                ctx_id_markbit: Cell::new(self.id),
                inner: node,
                next: Cell::new(std::ptr::null()),
            });
            entry.set_markbit(!self.markbit_marked);
            if let Some(prev) = prev {
                prev.next.set(entry as *const _);
            }
            entry
        };
        debug_assert!(!entry.is_free(), "Entry must not be free");
        // Transmute here to handle the fact that Cell<> is invariant over its type,
        // meaning the lifetime doesn't automatically narrow from `'ast` to `'s`.
        unsafe { std::mem::transmute(entry) }
    }

    /// Return the atom table.
    pub fn atom_table(&self) -> &AtomTable {
        &self.atom_table
    }

    /// Add a string to the identifier table.
    #[inline]
    pub fn atom<V: Into<String> + AsRef<str>>(&self, value: V) -> Atom {
        self.atom_table.atom(value)
    }

    /// Add a string to the identifier table.
    #[inline]
    pub fn atom_u16<V: Into<Vec<u16>> + AsRef<[u16]>>(&self, value: V) -> AtomU16 {
        self.atom_table.atom_u16(value)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str(&self, index: Atom) -> &str {
        self.atom_table.str(index)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str_u16(&self, index: AtomU16) -> &[u16] {
        self.atom_table.str_u16(index)
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
        let free_nodes = unsafe { &mut *self.free_nodes.get() };

        let list_elements = unsafe { &mut *self.list_elements.get() };
        let free_list_elements = unsafe { &mut *self.free_list_elements.get() };

        {
            // Begin by collecting all the roots: entries with non-zero refcount.
            let mut roots: Vec<&StorageEntry> = vec![];
            for entry in nodes.iter() {
                if entry.is_free() {
                    continue;
                }
                debug_assert!(
                    entry.markbit() != self.markbit_marked,
                    "Entry marked before start of GC: \
                        {:?}\nentry.markbit()={}\nmarkbit_marked={}",
                    &entry,
                    entry.markbit(),
                    self.markbit_marked,
                );
                if entry.count.get() > 0 {
                    // Transmuting the lifetime here because we have to store the roots from
                    // across accesses to `nodes`, meaning we must translate
                    // from `'ast` to the lifetime of this scope.
                    roots.push(unsafe { std::mem::transmute(entry) });
                }
            }

            struct Marker {
                markbit_marked: bool,
            }

            impl<'gc> Visitor<'gc> for Marker {
                fn call(
                    &mut self,
                    gc: &'gc GCLock,
                    node: &'gc Node<'gc>,
                    _path: Option<Path<'gc>>,
                ) {
                    let entry = unsafe { StorageEntry::from_node(node) };
                    if entry.markbit() == self.markbit_marked {
                        // Stop visiting early if we've already marked this part,
                        // because we must have also marked all the children.
                        return;
                    }
                    entry.set_markbit(self.markbit_marked);
                    node.mark_lists(gc, |elem| {
                        elem.set_markbit(self.markbit_marked);
                    });
                    node.visit_children(gc, self);
                }
            }

            // Use a visitor to mark every node reachable from roots.
            let mut marker = Marker {
                markbit_marked: self.markbit_marked,
            };
            {
                let gc: GCLock = GCLock::new(self);
                for root in roots {
                    root.inner.visit(&gc, &mut marker, None);
                }
            }
        }

        for entry in nodes.iter_mut() {
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
            free_nodes.push(unsafe { NonNull::new_unchecked(entry as *mut StorageEntry) });
        }

        for element in list_elements.iter_mut() {
            if element.is_free() {
                // Skip free entries.
                continue;
            }
            if element.markbit() == self.markbit_marked {
                // Keep marked entries alive.
                continue;
            }
            // Passed all checks, this element is free.
            element.ctx_id_markbit.set(FREE_ENTRY);
            free_list_elements
                .push(unsafe { NonNull::new_unchecked(element as *mut NodeListElement) });
        }

        self.markbit_marked = !self.markbit_marked;
    }

    /// Returns the number of node slots which have been allocated.
    /// Includes nodes currently in use as well as nodes in the free list.
    pub fn num_nodes(&self) -> usize {
        let nodes = unsafe { &*self.nodes.get() };
        nodes.len()
    }

    /// Returns the approximate size of just the AST storages in bytes.
    /// Includes the allocated nodes, lists, as well as free lists for both.
    pub fn storage_size(&self) -> usize {
        let nodes = unsafe { &*self.nodes.get() };
        let free_nodes = unsafe { &*self.free_nodes.get() };
        let list_elements = unsafe { &*self.list_elements.get() };
        let free_list_elements = unsafe { &*self.free_list_elements.get() };
        let mut result = 0;
        result += nodes.heap_size();
        result += free_nodes.heap_size();
        result += list_elements.heap_size();
        result += free_list_elements.heap_size();
        result
    }
}

impl HeapSize for Context<'_> {
    fn heap_size(&self) -> usize {
        let nodes = unsafe { &*self.nodes.get() };
        let free_nodes = unsafe { &*self.free_nodes.get() };
        let list_elements = unsafe { &*self.list_elements.get() };
        let free_list_elements = unsafe { &*self.free_list_elements.get() };
        let mut result = 0;
        result += nodes.heap_size();
        result += free_nodes.heap_size();
        result += list_elements.heap_size();
        result += free_list_elements.heap_size();
        result += std::mem::size_of::<NodeRcCounter>();
        result += self.atom_table.heap_size();
        result += self.source_mgr.heap_size();
        result
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
                for entry in nodes.iter() {
                    assert!(
                        entry.count.get() == 0,
                        "NodeRc must not outlive Context: {:#?}\n",
                        &entry.inner
                    );
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
    pub(crate) fn alloc<'s>(&'s self, n: Node<'s>) -> &'s Node<'s> {
        self.ctx.alloc(n)
    }

    /// Append `node` to the `prev` element if provided, else create the element as the first
    /// element in the `NodeList`.
    #[inline]
    pub(crate) fn append_list_element<'s>(
        &'s self,
        prev: Option<&'s NodeListElement<'s>>,
        n: &'s Node<'s>,
    ) -> &'s NodeListElement<'s> {
        self.ctx.append_list_element(prev, n)
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

    /// Add a string to the identifier table.
    #[inline]
    pub fn atom_u16<V: Into<Vec<u16>> + AsRef<[u16]>>(&self, value: V) -> AtomU16 {
        self.ctx.atom_u16(value)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str(&self, index: Atom) -> &str {
        self.ctx.str(index)
    }

    /// Obtain the contents of an atom from the atom table.
    #[inline]
    pub fn str_u16(&self, index: AtomU16) -> &[u16] {
        self.ctx.str_u16(index)
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
