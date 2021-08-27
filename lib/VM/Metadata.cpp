/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Metadata.h"

#include "hermes/VM/SymbolID.h"

namespace hermes {
namespace vm {

using std::size_t;
using std::uintptr_t;
using ArrayData = Metadata::ArrayData;
using ArrayType = ArrayData::ArrayType;

Metadata::Metadata(Builder &&mb) : array_(std::move(mb.array_)), vtp_(mb.vtp_) {
  size_t i = 0;
  for (const auto &p : mb.pointers_) {
    offsets[i] = p.first;
    names[i] = p.second;
    i++;
  }
  pointersEnd = i;

  for (const auto &p : mb.values_) {
    offsets[i] = p.first;
    names[i] = p.second;
    i++;
  }
  valuesEnd = i;

  for (const auto &p : mb.smallValues_) {
    offsets[i] = p.first;
    names[i] = p.second;
    i++;
  }
  smallValuesEnd = i;

  for (const auto &p : mb.symbols_) {
    offsets[i] = p.first;
    names[i] = p.second;
    i++;
  }
  symbolsEnd = i;

  assert(i <= kMaxNumFields && "Number of fields exceeds max.");
  assert(vtp_->isValid() && "Must initialize VTable pointer for metadata.");
}

Metadata::Builder::Builder(const void *base)
    : base_(reinterpret_cast<const char *>(base)) {}

Metadata::offset_t Metadata::Builder::getOffset(const void *fieldLocation) {
  const size_t offset = reinterpret_cast<const char *>(fieldLocation) - base_;
  const offset_t ret = offset;
  assert(ret == offset && "Offset overflowed.");
  return ret;
}

void Metadata::Builder::addField(const GCPointerBase *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCPointerBase *fieldLocation) {
  offset_t offset = getOffset(fieldLocation);
  assert(
      !fieldConflicts(offset, sizeof(GCPointerBase)) &&
      "fields should not overlap");
  pointers_[offset] = name;
}

void Metadata::Builder::addField(const GCHermesValue *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCHermesValue *fieldLocation) {
  offset_t offset = getOffset(fieldLocation);
  assert(
      !fieldConflicts(offset, sizeof(GCHermesValue)) &&
      "fields should not overlap");
  values_[offset] = name;
}

void Metadata::Builder::addField(const GCSmallHermesValue *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCSmallHermesValue *fieldLocation) {
  offset_t offset = getOffset(fieldLocation);
  assert(
      !fieldConflicts(offset, sizeof(GCSmallHermesValue)) &&
      "fields should not overlap");
  smallValues_[offset] = name;
}

void Metadata::Builder::addField(const GCSymbolID *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCSymbolID *fieldLocation) {
  offset_t offset = getOffset(fieldLocation);
  assert(
      !fieldConflicts(offset, sizeof(GCSymbolID)) &&
      "fields should not overlap");
  symbols_[offset] = name;
}

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
  for (size_t i = 0; i < meta.symbolsEnd; ++i) {
    if (!first) {
      os << ",";
    } else {
      first = false;
    }
    os << "\n\t\t";
    os << "{ offset: " << meta.offsets[i] << ", name: " << meta.names[i] << "}";
  }
  if (!first) {
    os << "\n\t";
  }
  os << "]";
  if (meta.array_) {
    os << ",\n\tarray: " << *meta.array_ << ",\n";
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
    case ArrayType::Pointer:
      os << "Pointer";
      break;
    case ArrayType::HermesValue:
      os << "HermesValue";
      break;
    case ArrayType::SmallHermesValue:
      os << "SmallHermesValue";
      break;
    case ArrayType::Symbol:
      os << "Symbol";
      break;
  }
  return os << "}";
}

/// @}

} // namespace vm
} // namespace hermes
