/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/Metadata.h"

namespace hermes {
namespace vm {

using std::size_t;
using std::uintptr_t;
using ArrayData = Metadata::ArrayData;
using ArrayType = ArrayData::ArrayType;

Metadata::Metadata(Builder &&mb)
    : pointers_(mb.pointers_.size()),
      values_(mb.values_.size()),
      symbols_(mb.symbols_.size()),
      array_(std::move(mb.array_)) {
  auto copier =
      [](const std::map<offset_t, std::pair<const char *, size_t>> &map,
         Fields &insertionPoint) {
        std::transform(
            map.cbegin(),
            map.cend(),
            std::begin(insertionPoint.offsets),
            [](const OffsetAndNameAndSize &p) { return p.first; });
        std::transform(
            map.cbegin(),
            map.cend(),
            std::begin(insertionPoint.names),
            [](const OffsetAndNameAndSize &p) { return p.second.first; });
      };
  copier(mb.pointers_, pointers_);
  copier(mb.values_, values_);
  copier(mb.symbols_, symbols_);
}

Metadata::Metadata(Metadata &&that)
    : pointers_(std::move(that.pointers_)),
      values_(std::move(that.values_)),
      symbols_(std::move(that.symbols_)),
      array_(std::move(that.array_)) {}

Metadata::Builder::Builder(const void *base)
    : base_(reinterpret_cast<const char *>(base)),
      pointers_(),
      values_(),
      symbols_(),
      array_() {}

void Metadata::Builder::addField(const GCPointerBase *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCPointerBase *fieldLocation) {
  pointers_[reinterpret_cast<const char *>(fieldLocation) - base_] =
      std::make_pair(name, sizeof(GCPointerBase));
}

void Metadata::Builder::addField(const GCHermesValue *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const GCHermesValue *fieldLocation) {
  values_[reinterpret_cast<const char *>(fieldLocation) - base_] =
      std::make_pair(name, sizeof(GCHermesValue));
}

void Metadata::Builder::addField(const SymbolID *fieldLocation) {
  addField(nullptr, fieldLocation);
}

void Metadata::Builder::addField(
    const char *name,
    const SymbolID *fieldLocation) {
  symbols_[reinterpret_cast<const char *>(fieldLocation) - base_] =
      std::make_pair(name, sizeof(SymbolID));
}

Metadata Metadata::Builder::build() {
  return Metadata(std::move(*this));
}

/// @name Formatters
/// @{

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, const Metadata &meta) {
  os << "Metadata: {\n\tfieldsAndNames: [";
  auto printOffsetAndNameAndSizes = [](llvm::raw_ostream &os,
                                       const Metadata::Fields &vec) {
    bool first = true;
    for (size_t i = 0; i < vec.size(); ++i) {
      if (!first) {
        os << ",";
      } else {
        first = false;
      }
      os << "\n\t\t";
      os << "{ offset: " << vec.offsets[i] << ", name: " << vec.names[i] << "}";
    }
    if (!vec.empty()) {
      os << "\n\t";
    }
  };
  printOffsetAndNameAndSizes(os, meta.pointers_);
  printOffsetAndNameAndSizes(os, meta.values_);
  printOffsetAndNameAndSizes(os, meta.symbols_);
  os << "]";
  if (meta.array_) {
    os << ",\n\tarray: " << *meta.array_ << ",\n";
  } else {
    os << "\n";
  }
  return os << "}";
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, ArrayData array) {
  return os << "ArrayData: {type: {" << array.type
            << "}, lengthOffset: " << array.lengthOffset
            << ", stride: " << array.stride << "}";
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &os, ArrayType arraytype) {
  os << "ArrayType: {";
  switch (arraytype) {
    case ArrayType::Pointer:
      os << "Pointer";
      break;
    case ArrayType::HermesValue:
      os << "HermesValue";
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
