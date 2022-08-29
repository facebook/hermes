/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_VTABLE_H
#define HERMES_VM_VTABLE_H

#include "hermes/VM/CellKind.h"
#include "hermes/VM/GCDecl.h"
#include "hermes/VM/HeapAlign.h"
#include "hermes/VM/HeapSnapshot.h"
#include "hermes/VM/PointerBase.h"
#include "hermes/VM/SlotAcceptor.h"

#include "llvh/Support/raw_ostream.h"

#include <cstdint>

#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif
namespace hermes {
namespace vm {

// Forward declarations.
class GCCell;

/// The "metadata" for an allocated GC cell: the kind of cell, and
/// methods to "mark" (really, to invoke a GC callback on JS values in
/// the block) and (optionally) finalize the cell.
struct VTable {
#ifdef HERMES_MEMORY_INSTRUMENTATION
  class HeapSnapshotMetadata final {
   private:
    using NameCallback = std::string(GCCell *, GC &);
    using AddEdgesCallback = void(GCCell *, GC &, HeapSnapshot &);
    using AddNodesCallback = void(GCCell *, GC &, HeapSnapshot &);
    using AddLocationsCallback = void(GCCell *, GC &, HeapSnapshot &);

   public:
    /// Construct a HeapSnapshotMetadata, that is used by the GC to decide how
    /// to print an object and add edges from it to other objects.
    ///
    /// \param name Returns the name that should be displayed in heap snapshots.
    ///   If not provided, or if the function returns an empty string, a name
    ///   based on the cell kind will be used instead.
    /// \param addEdges Adds any non-internal edges to this node. Can be used to
    ///   add custom edges that aren't well-represented by internal pointers, or
    ///   should be user-visible. Can also point to nodes created via the \p
    ///   addNodes parameter.
    /// \param addNodes Adds any nodes that are pointed to by this GCCell. Can
    ///   be used to add nodes for memory that isn't on the JS heap.
    ///   NOTE: Despite the name, edges sometimes also need to be added in
    ///   this callback (for example, if a native node points to another native
    ///   node).
    constexpr explicit HeapSnapshotMetadata(
        HeapSnapshot::NodeType nodeType,
        NameCallback *name,
        AddEdgesCallback *addEdges,
        AddNodesCallback *addNodes,
        AddLocationsCallback *addLocations)
        : nodeType_(nodeType),
          name_(name),
          addEdges_(addEdges),
          addNodes_(addNodes),
          addLocations_(addLocations) {}

    HeapSnapshot::NodeType nodeType() const {
      return nodeType_;
    }
    std::string nameForNode(GCCell *cell, GC &gc) const;
    /// Get the default name for the node, without any custom behavior.
    std::string defaultNameForNode(GCCell *cell) const;
    void addEdges(GCCell *cell, GC &gc, HeapSnapshot &snap) const;
    void addNodes(GCCell *cell, GC &gc, HeapSnapshot &snap) const;
    void addLocations(GCCell *cell, GC &gc, HeapSnapshot &snap) const;

   private:
    const HeapSnapshot::NodeType nodeType_;
    NameCallback *const name_;
    AddEdgesCallback *const addEdges_;
    AddNodesCallback *const addNodes_;
    AddLocationsCallback *const addLocations_;
  };
#endif // !defined(HERMES_MEMORY_INSTRUMENTATION)

  // Value is 64 bits to make sure it can be used as a pointer in both 32 and
  // 64-bit builds.
  // "57ab1e" == "vtable".
  // ff added at the beginning to make sure it's a kernel address (even in a
  // 32-bit build).
  static constexpr uint64_t kMagic{0xff57ab1eff57ab1e};
  // This is used to ensure a VTable is valid and is not some other value.
  // Notably, since VTable * can sometimes be a forwarding pointer, if a pointer
  // to a VTable is accidentally used as a GCCell, it will try to use the first
  // word as another VTable *. By putting in a magic number here, it will
  // SIGSEGV on a specific address, which will make it easy to know exactly
  // what went wrong.
  // This is left on even in opt builds because VTable sizes are not
  // particularly important.
  const uint64_t magic_{kMagic};
  /// The cell kind.
  const CellKind kind;
  /// `size` should be the size of the cell if it is fixed, or 0 if it is
  /// variable sized.
  /// If it is variable sized, it should inherit from \see
  /// VariableSizeRuntimeCell.
  const uint32_t size;
  /// Called during GC when an object becomes unreachable. Must not perform any
  /// allocations or access any garbage-collectable objects.  Unless an
  /// operation is documented to be safe to call from a finalizer, it probably
  /// isn't.
  using FinalizeCallback = void(GCCell *, GC &gc);
  FinalizeCallback *const finalize_;
  /// Call GC functions on weak-reference-holding objects. In a concurrent GC,
  /// guaranteed to be called while the weak ref mutex is held.
  using MarkWeakCallback = void(GCCell *, WeakRefAcceptor &);
  MarkWeakCallback *const markWeak_;
  /// Report if there is any size contribution from an object beyond the GC.
  /// Used to report any externally allocated memory for metric gathering.
  using MallocSizeCallback = size_t(GCCell *);
  MallocSizeCallback *const mallocSize_;
  /// Ask the cell what its post-trimming size will be.
  /// This should not modify the cell.
  using TrimSizeCallback = gcheapsize_t(const GCCell *);
  TrimSizeCallback *const trimSize_;

#ifdef HERMES_MEMORY_INSTRUMENTATION
  /// Any metadata associated with heap snapshots.
  const HeapSnapshotMetadata snapshotMetaData;
#endif

  /// Static array storing the VTable corresponding to each CellKind. This is
  /// initialized by buildMetadataTable.
  static std::array<const VTable *, kNumCellKinds> vtableArray;

  static const VTable *getVTable(CellKind c) {
    return vtableArray[static_cast<size_t>(c)];
  }

  constexpr explicit VTable(
      CellKind kind,
      uint32_t size,
      FinalizeCallback *finalize = nullptr,
      MarkWeakCallback *markWeak = nullptr,
      MallocSizeCallback *mallocSize = nullptr,
      TrimSizeCallback *trimSize = nullptr
#ifdef HERMES_MEMORY_INSTRUMENTATION
      ,
      HeapSnapshotMetadata snapshotMetaData =
          HeapSnapshotMetadata {
            HeapSnapshot::NodeType::Object, nullptr, nullptr, nullptr, nullptr
          }
#endif
      )
      : kind(kind),
        size(heapAlignSize(size)),
        finalize_(finalize),
        markWeak_(markWeak),
        mallocSize_(mallocSize),
        trimSize_(trimSize)
#ifdef HERMES_MEMORY_INSTRUMENTATION
        ,
        snapshotMetaData(snapshotMetaData)
#endif
  {
  }

  bool isVariableSize() const {
    return size == 0;
  }

  void finalizeIfExists(GCCell *cell, GC &gc) const {
    assert(isValid());
    if (finalize_) {
      finalize_(cell, gc);
    }
  }

  void finalize(GCCell *cell, GC &gc) const {
    assert(isValid());
    assert(
        finalize_ &&
        "Cannot unconditionally finalize if it doesn't have a finalize pointer");
    finalize_(cell, gc);
  }

  MarkWeakCallback *getMarkWeakCallback() const {
    assert(isValid());
    return markWeak_;
  }

  size_t getMallocSize(GCCell *cell) const {
    assert(isValid());
    return mallocSize_ ? mallocSize_(cell) : 0;
  }

  /// If the cell can be trimmed, return the new size of the cell after
  /// trimming. Otherwise, return \p origSize.
  gcheapsize_t getTrimmedSize(GCCell *cell, size_t origSize) const {
    const size_t trimmedSize =
        trimSize_ ? heapAlignSize(trimSize_(cell)) : origSize;
    assert(
        isValid() && trimmedSize <= origSize &&
        "Growing objects is not supported.");
    return trimmedSize;
  }

  /// \return true iff this VTable is valid.
  /// Validity is defined by:
  ///   * The magic_ field has the expected value.
  ///   * The kind that is within the range of valid CellKinds.
  bool isValid() const {
    return magic_ == kMagic && isSizeHeapAligned(size) &&
        kindInRange(
               kind, CellKind::AllCellsKind_first, CellKind::AllCellsKind_last);
  }

  /// \return true iff this VTable has the correct magic_ value, and has the
  /// given \p expectedKind.
  bool isValid(CellKind expectedKind) const {
    return magic_ == kMagic && isSizeHeapAligned(size) && kind == expectedKind;
  }
};

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const VTable &vt);

} // namespace vm
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_VM_VTABLE_H
