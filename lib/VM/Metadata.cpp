/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Metadata.h"

#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

using std::size_t;
using ArrayData = Metadata::ArrayData;
using ArrayType = ArrayData::ArrayType;

std::array<Metadata, kNumCellKinds> Metadata::metadataTable{};

Metadata::Metadata(Builder &&mb) : vtp(mb.vtp_) {
  offsets.array = mb.array_;
  size_t i = 0;

#define SLOT_TYPE(type)                   \
  for (const auto &p : mb.map##type##_) { \
    offsets.fields[i] = p.first;          \
    names[i] = p.second;                  \
    i++;                                  \
  }                                       \
  offsets.end##type = i;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

  assert(i <= kMaxNumFields && "Number of fields exceeds max.");
  assert(vtp->isValid() && "Must initialize VTable pointer for metadata.");
}

Metadata::Builder::Builder(const void *base)
    : base_(reinterpret_cast<const char *>(base)) {}

Metadata::offset_t Metadata::Builder::getOffset(const void *fieldLocation) {
  const size_t offset = reinterpret_cast<const char *>(fieldLocation) - base_;
  const offset_t ret = offset;
  assert(ret == offset && "Offset overflowed.");
  return ret;
}

#define SLOT_TYPE(type)                                         \
  void Metadata::Builder::addField(const type *fieldLocation) { \
    addField(nullptr, fieldLocation);                           \
  }
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

#define SLOT_TYPE(type)                                                        \
  void Metadata::Builder::addField(                                            \
      const char *name, const type *fieldLocation) {                           \
    offset_t offset = getOffset(fieldLocation);                                \
    assert(                                                                    \
        !fieldConflicts(offset, sizeof(type)) && "fields should not overlap"); \
    map##type##_[offset] = name;                                               \
  }
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE

void Metadata::Builder::addArray(
    const char *name,
    ArrayData::ArrayType type,
    const void *startLocation,
    const AtomicIfConcurrentGC<uint32_t> *lengthLocation,
    std::size_t stride) {
  const uint8_t stride8 = stride;
  assert(stride8 == stride && "Stride overflowed");
  array_ = ArrayData(
      type, getOffset(startLocation), getOffset(lengthLocation), stride8);
}

Metadata Metadata::Builder::build() {
  return Metadata(std::move(*this));
}

#ifndef NDEBUG
bool Metadata::Builder::fieldConflicts(offset_t offset, size_t size) {
  size_t end = offset + size;
  coveredOffsets_.resize(std::max(coveredOffsets_.size(), end));
  for (offset_t i = offset; i < end; ++i) {
    if (coveredOffsets_[i])
      return true;
    coveredOffsets_[i] = true;
  }
  return false;
}
#endif

/// @name Formatters
/// @{

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, const Metadata &meta) {
  os << "Metadata: {\n\tfieldsAndNames: [";
  bool first = true;
  size_t end;
#define SLOT_TYPE(type) end = meta.offsets.end##type;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  for (size_t i = 0; i < end; ++i) {
    if (!first) {
      os << ",";
    } else {
      first = false;
    }
    os << "\n\t\t";
    os << "{ offset: " << meta.offsets.fields[i] << ", name: " << meta.names[i]
       << "}";
  }
  if (!first) {
    os << "\n\t";
  }
  os << "]";
  if (meta.offsets.array) {
    os << ",\n\tarray: " << *meta.offsets.array << ",\n";
  } else {
    os << "\n";
  }
  return os << "}";
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, ArrayData array) {
  return os << "ArrayData: {type: {" << array.type
            << "}, lengthOffset: " << array.lengthOffset
            << ", stride: " << array.stride << "}";
}

llvh::raw_ostream &operator<<(llvh::raw_ostream &os, ArrayType arraytype) {
  os << "ArrayType: {";
  switch (arraytype) {
#define SLOT_TYPE(type) \
  case ArrayType::type: \
    os << #type;        \
    break;
#include "hermes/VM/SlotKinds.def"
#undef SLOT_TYPE
  }
  return os << "}";
}

/// @}

} // namespace vm
} // namespace hermes
