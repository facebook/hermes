/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_METADATA_H
#define HERMES_VM_METADATA_H

#include "hermes/ADT/OwningArray.h"
#include "hermes/Support/OptValue.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/HermesValue.h"

#include <cassert>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace hermes {
namespace vm {

// Forward declare to avoid pulling in a runtime dependency.
class GCSymbolID;

/// Metadata is a the information about a class's pointers locations.
/// This is used by the Garbage collector to know where potential pointers
/// to other objects are.
struct Metadata final {
  using offset_t = std::uint8_t;

  /// Fields is a group of both offsets and names that describe a field within
  /// an object.
  struct Fields {
    Fields() = default;
    explicit Fields(size_t numFields) : offsets(numFields), names(numFields) {}

    size_t size() const {
      return offsets.size();
    }

    bool empty() const {
      return offsets.empty();
    }

    // Invariant: these arrays are all of the same size N, and the ith elements
    // describe the ith field.

    /// The offset location of the field within the object.
    OwningArray<offset_t> offsets;
    /// The names of the fields, only used in snapshots.
    OwningArray<const char *> names;
  };

  /// The information about an array for an object.
  struct ArrayData {
    /// Which type of element the array holds.
    enum class ArrayType { Pointer, HermesValue, SmallHermesValue, Symbol };
    ArrayType type;
    /// The offset of where the array starts.
    offset_t startOffset;
    /// The offset of the field that knows how many elements are in the array.
    offset_t lengthOffset;
    /// The width of each element. For example, a pointer has a stride of
    /// sizeof(void *).
    std::uint16_t stride;

    explicit ArrayData() = default;
    explicit constexpr ArrayData(
        ArrayType type,
        offset_t startOffset,
        offset_t lengthOffset,
        std::uint16_t stride)
        : type(type),
          startOffset(startOffset),
          lengthOffset(lengthOffset),
          stride(stride) {}
    ArrayData(const ArrayData &data) = default;
    ArrayData &operator=(const ArrayData &data) = default;
  };

  /// A Builder is a way to build a metadata structure by incrementally
  /// adding fields to it.
  /// After adding the fields, call \c build in order to create the \c Metadata
  /// object.
  class Builder final {
   public:
    /// Creates a Builder for an object, from a base to be used to
    /// calculate the offsets of fields from the base.
    Builder(const void *base);

    /// @name Field adders
    /// @{
    /// Adds a field to the metadata.
    /// The version without a \p name parameter means that field will not appear
    /// in snapshots.

    /// Adds a pointer field.
    void addField(const GCPointerBase *fieldLocation);
    void addField(const char *name, const GCPointerBase *fieldLocation);
    /// Adds a \c HermesValue field.
    void addField(const GCHermesValue *fieldLocation);
    void addField(const char *name, const GCHermesValue *fieldLocation);
    void addField(const GCSmallHermesValue *fieldLocation);
    void addField(const char *name, const GCSmallHermesValue *fieldLocation);
    /// Adds a \c Symbol field.
    void addField(const GCSymbolID *fieldLocation);
    void addField(const char *name, const GCSymbolID *fieldLocation);

    /// @}

    /// Adds an array to this class's metadata.
    /// \p startLocation The location of the first element in the array.
    /// \p lengthLocation The location of the size of the array.
    void addArray(
        const GCHermesValue *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          nullptr,
          ArrayData::ArrayType::HermesValue,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const char *name,
        const GCHermesValue *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          name,
          ArrayData::ArrayType::HermesValue,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const GCSmallHermesValue *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          nullptr,
          ArrayData::ArrayType::SmallHermesValue,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const char *name,
        const GCSmallHermesValue *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          name,
          ArrayData::ArrayType::SmallHermesValue,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const GCPointerBase *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          nullptr,
          ArrayData::ArrayType::Pointer,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const char *name,
        const GCPointerBase *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          name,
          ArrayData::ArrayType::Pointer,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const GCSymbolID *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          nullptr,
          ArrayData::ArrayType::Symbol,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const char *name,
        const GCSymbolID *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride) {
      addArray(
          name,
          ArrayData::ArrayType::Symbol,
          startLocation,
          lengthLocation,
          stride);
    }

    void addArray(
        const char *name,
        ArrayData::ArrayType type,
        const void *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride);

    /// Should be called first when building metadata for a JSObject subclass.
    /// Records how many property slots are unused due to overlap.
    void addJSObjectOverlapSlots(unsigned num) {
      if (!jsObjectOverlapSlots_) {
        jsObjectOverlapSlots_ = num;
      } else {
        // Subsequent calls do nothing but assert that order of calls is sane.
        assert(
            num <= *jsObjectOverlapSlots_ &&
            "most derived class should call this method first");
      }
    }

    /// The number of initial direct property slots to exclude in the metadata
    /// for a JSObject subclass.
    unsigned getJSObjectOverlapSlots() const {
      assert(
          jsObjectOverlapSlots_ && "missing call to addJSObjectOverlapSlots");
      return *jsObjectOverlapSlots_;
    }

    void setVTable(const VTable *vtp) {
      vtp_ = vtp;
    }

    /// Build creates a Metadata, and destroys this builder.
    Metadata build();

   private:
    /// Calculate the offset of a field at \p fieldLocation relative to base_.
    offset_t getOffset(const void *fieldLocation);

    /// The base of the object, used to calculate offsets.
    const char *base_;
    /// A list of offsets and within the object to its field type and name.
    std::map<offset_t, const char *> pointers_;
    std::map<offset_t, const char *> values_;
    std::map<offset_t, const char *> smallValues_;
    std::map<offset_t, const char *> symbols_;
#ifndef NDEBUG
    /// True if [offset, offset + size) overlaps any previously added field.
    bool fieldConflicts(offset_t offset, size_t size);
    std::vector<bool> coveredOffsets_;
#endif
    /// An optional array for an object to contain.
    OptValue<ArrayData> array_;

    /// For subclasses of JSObject, the number of unused direct property slots.
    OptValue<unsigned> jsObjectOverlapSlots_;

    /// The VTable pointer for the cell that this metadata describes.
    const VTable *vtp_{nullptr};

    friend Metadata;
  };

  /// Construct from a builder.
  Metadata(Builder &&mb);

  /// A mapping from an offset to a name for that field
  Fields pointers_;
  Fields values_;
  Fields smallValues_;
  Fields symbols_;

  /// The optional array for this object to hold.
  /// NOTE: this format currently does not support multiple arrays.
  OptValue<ArrayData> array_;

  /// The VTable pointer for the cell that this metadata describes.
  const VTable *vtp_;
};

/// @name Formatters
/// @{

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const Metadata &meta);
llvh::raw_ostream &operator<<(llvh::raw_ostream &os, Metadata::ArrayData array);
llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    Metadata::ArrayData::ArrayType arraytype);

/// @}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_METADATA_H
