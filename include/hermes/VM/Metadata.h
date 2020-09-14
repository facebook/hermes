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
#include "hermes/VM/SymbolID.h"

#include <cassert>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>

namespace hermes {
namespace vm {

/// Metadata is a the information about a class's pointers locations.
/// This is used by the Garbage collector to know where potential pointers
/// to other objects are.
struct Metadata final {
  using offset_t = std::uint16_t;
  using OffsetAndNameAndSize =
      std::pair<offset_t, std::pair<const char *, size_t>>;
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
    enum class ArrayType { Pointer, HermesValue, Symbol };
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
    /// Adds a \c Symbol field.
    void addField(const SymbolID *fieldLocation);
    void addField(const char *name, const SymbolID *fieldLocation);

    /// @}

    /// Adds an array to this class's metadata.
    /// \p startLocation The location of the first element in the array.
    /// \p lengthLocation The location of the size of the array.
    /// The \c ArrayType parameter denotes which type of elements are in the
    /// array.
    template <ArrayData::ArrayType type>
    inline void addArray(
        const void *startLocation,
        const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
        std::size_t stride);

    template <ArrayData::ArrayType type>
    inline void addArray(
        const char *name,
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

    /// Build creates a Metadata, and destroys this builder.
    Metadata build();

   private:
    /// The base of the object, used to calculate offsets.
    const char *base_;
    /// A list of offsets and within the object to its field type and name.
    std::map<offset_t, std::pair<const char *, size_t>> pointers_;
    std::map<offset_t, std::pair<const char *, size_t>> values_;
    std::map<offset_t, std::pair<const char *, size_t>> symbols_;
#ifndef NDEBUG
    /// True if [offset, offset + size) overlaps any previously added field.
    bool fieldConflicts(offset_t offset, size_t size);
    std::vector<bool> coveredOffsets_;
#endif
    /// An optional array for an object to contain.
    OptValue<ArrayData> array_;

    /// For subclasses of JSObject, the number of unused direct property slots.
    OptValue<unsigned> jsObjectOverlapSlots_;

    friend Metadata;
  };

  /// Construct an empty metadata.
  Metadata() = default;
  Metadata(Metadata &&);
  /// Construct from a builder.
  Metadata(Builder &&mb);
  ~Metadata() = default;

  Metadata &operator=(Metadata &&) = default;

  /// A mapping from an offset to a name for that field
  Fields pointers_;
  Fields values_;
  Fields symbols_;

  /// The optional array for this object to hold.
  /// NOTE: this format currently does not support multiple arrays.
  OptValue<ArrayData> array_;
};

/// @name Formatters
/// @{

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const Metadata &meta);
llvh::raw_ostream &operator<<(llvh::raw_ostream &os, Metadata::ArrayData array);
llvh::raw_ostream &operator<<(
    llvh::raw_ostream &os,
    Metadata::ArrayData::ArrayType arraytype);

/// @}

/// @name Inline implementations
/// @{

template <Metadata::ArrayData::ArrayType type>
inline void Metadata::Builder::addArray(
    const void *startLocation,
    const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
    std::size_t stride) {
  addArray<type>(nullptr, startLocation, lengthLocation, stride);
}

template <Metadata::ArrayData::ArrayType type>
inline void Metadata::Builder::addArray(
    const char *name,
    const void *startLocation,
    const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
    std::size_t stride) {
  array_ = ArrayData(
      type,
      reinterpret_cast<const char *>(startLocation) - base_,
      reinterpret_cast<const char *>(lengthLocation) - base_,
      stride);
}

/// @}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_METADATA_H
