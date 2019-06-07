/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/HeapSnapshot.h"

#include "hermes/Support/Conversions.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/StringPrimitive.h"

#include <iomanip>
#include <sstream>

namespace hermes {
namespace vm {

namespace {
void pretty(llvm::raw_ostream &os, bool compact, const char *msg) {
  if (!compact) {
    os << msg;
  }
}
} // namespace

/// @name FacebookHeapSnapshot
/// @{
FacebookHeapSnapshot::FacebookHeapSnapshot(
    llvm::raw_ostream &os,
    bool compact,
    gcheapsize_t totalHeapSize)
    : os_(os), compact_(compact) {
  os_ << "{";
  pretty(os_, compact_, "\n\t");
  os_ << "\"version\":";
  pretty(os_, compact_, " ");
  os_ << version << ",";
  pretty(os_, compact_, "\n\t");
  os_ << "\"totalHeapSize\":";
  pretty(os_, compact_, " ");
  os_ << totalHeapSize << ",";
  pretty(os_, compact_, "\n\t");
}

FacebookHeapSnapshot::~FacebookHeapSnapshot() {
  pretty(os_, compact_, "\n");
  os_ << "}";
  pretty(os_, compact_, "\n");
}

void FacebookHeapSnapshot::beginRoots() {
  os_ << "\"roots\":";
  pretty(os_, compact_, " ");
  os_ << "[";
  firstProperty_ = true;
}

void FacebookHeapSnapshot::endRoots() {
  pretty(os_, compact_, "\n\t");
  os_ << "],";
  pretty(os_, compact_, "\n\t");
}

void FacebookHeapSnapshot::beginRefs() {
  os_ << "\"refs\":";
  pretty(os_, compact_, " ");
  os_ << "{";
  pretty(os_, compact_, "\n");
}

void FacebookHeapSnapshot::endRefs() {
  pretty(os_, compact_, "\n\t");
  os_ << "},";
  pretty(os_, compact_, "\n\t");
}

void FacebookHeapSnapshot::beginIdTable() {
  firstIdTableEntry_ = true;
  os_ << "\"idtable\":";
  pretty(os_, compact_, " ");
  os_ << "[";
}

void FacebookHeapSnapshot::addIdTableEntry(UTF16Ref entry, uint32_t id) {
  if (firstIdTableEntry_) {
    firstIdTableEntry_ = false;
  } else {
    os_ << ",";
    pretty(os_, compact_, " ");
  }
  os_ << "[";
  os_ << id;
  os_ << ",";
  pretty(os_, compact_, " ");
  os_ << "\"";
  os_ << escapeJSON(converter(entry));
  os_ << "\"]";
}

void FacebookHeapSnapshot::endIdTable() {
  os_ << "]";
}
void FacebookHeapSnapshot::addRoot(ObjectID id) {
  if (!firstProperty_) {
    os_ << ",";
  } else {
    firstProperty_ = false;
  }
  pretty(os_, compact_, "\n\t\t");
  os_ << "\"" << id << "\"";
}

void FacebookHeapSnapshot::startObject(Object &&o) {
  startObject(std::move(o), nullptr);
}

void FacebookHeapSnapshot::startObject(Object &&o, const char *value) {
  if (!firstObj_) {
    os_ << ",";
    pretty(os_, compact_, "\n");
  } else {
    firstObj_ = false;
  }
  pretty(os_, compact_, "\t\t");
  os_ << "\"" << o.id << "\":";
  pretty(os_, compact_, " ");
  os_ << "{\"size\":";
  pretty(os_, compact_, " ");
  os_ << o.size << ",";
  pretty(os_, compact_, " ");
  os_ << "\"type\":";
  pretty(os_, compact_, " ");
  os_ << "\"" << cellKindStr(o.type) << "\",";
  pretty(os_, compact_, " ");
  if (value) {
    os_ << "\"value\":";
    pretty(os_, compact_, " ");
    os_ << value << ",";
    pretty(os_, compact_, " ");
  }

  os_ << "\"props\":";
  pretty(os_, compact_, " ");
  os_ << "[";
  firstProperty_ = true;
}

void FacebookHeapSnapshot::startProperty(const char *name) {
  if (!firstProperty_) {
    os_ << ",";
    pretty(os_, compact_, " ");
  } else {
    firstProperty_ = false;
  }
  if (name) {
    os_ << "[\"" << name << "\",";
  } else {
    os_ << "[null,";
  }
  pretty(os_, compact_, " ");
}

void FacebookHeapSnapshot::addToCurrentObject(const char *name, ObjectID id) {
  startProperty(name);
  os_ << "\"" << id << "\"]";
}

void FacebookHeapSnapshot::addHermesValueToCurrentObject(
    const char *name,
    HermesValue &hv) {
  if (hv.isBool()) {
    addValueToCurrentObject(name, hv.getBool());
  } else if (hv.isNumber()) {
    addValueToCurrentObject(name, hv.getNumber());
  } else if (hv.isUndefined()) {
    addUndefinedToCurrentObject(name);
  } else if (hv.isNativeValue()) {
    addNativeValueToCurrentObject(name, hv.getNativeValue());
  } else if (hv.isEmpty()) {
    addEmptyToCurrentObject(name);
  } else if (hv.isNull()) {
    addNullToCurrentObject(name);
  }
}

void FacebookHeapSnapshot::addSymbolIdToCurrentObject(
    const char *name,
    uint32_t value) {
  startProperty(name);
  os_ << "[" << value << "]]";
}

void FacebookHeapSnapshot::addValueToCurrentObject(
    const char *name,
    double value) {
  startProperty(name);
  if (std::isfinite(value)) {
    char buff[NUMBER_TO_STRING_BUF_SIZE];
    numberToString(value, buff, sizeof(buff));
    os_ << "[" << buff << "]]";
  } else {
    // Quote non finite doubles as they aren't valid JSON
    os_ << "[\"" << value << "\"]]";
  }
}

void FacebookHeapSnapshot::addValueToCurrentObject(
    const char *name,
    bool value) {
  startProperty(name);
  os_ << "[" << value << "]]";
}

void FacebookHeapSnapshot::addNullToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[null]]";
}

void FacebookHeapSnapshot::addUndefinedToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[]]";
}

void FacebookHeapSnapshot::addNativeValueToCurrentObject(
    const char *name,
    uint64_t value) {
  startProperty(name);
  os_ << "[\"0x";
  os_.write_hex(value);
  os_ << "\"]]";
}

void FacebookHeapSnapshot::addEmptyToCurrentObject(const char *name) {
  startProperty(name);
  os_ << "[\"empty\"]]";
}

void FacebookHeapSnapshot::addInternalToCurrentObject(
    const char *name,
    uint64_t value) {
  startProperty(name);
  os_ << "[\"" << value << "\"]]";
}

void FacebookHeapSnapshot::endObject() {
  os_ << "]}";
}

/// @}

/// @name rawHeapSnapshot
/// @{

void rawHeapSnapshot(
    llvm::raw_ostream &os,
    const char *start,
    const char *end) {
  // Write the start address value first.
  os << reinterpret_cast<uintptr_t>(start);
  // Copy the raw bytes of the address range into the ostream.
  os.write(start, end - start);
}

/// @}

void V8HeapSnapshot::addObject(const Object &o) {
  dump(o);
  if (!compact_) {
    os_ << "\n";
  }
}

void V8HeapSnapshot::dump(const Object &) const {}

std::string escapeJSON(llvm::StringRef s) {
  std::ostringstream o;
  for (const unsigned char c : s) {
    switch (c) {
      case '"':
        o << "\\\"";
        break;
      case '\\':
        o << "\\\\";
        break;
      case '\b':
        o << "\\b";
        break;
      case '\f':
        o << "\\f";
        break;
      case '\n':
        o << "\\n";
        break;
      case '\r':
        o << "\\r";
        break;
      case '\t':
        o << "\\t";
        break;
      default:
        if (c <= '\x1f') {
          o << "\\u" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(c);
        } else {
          o << c;
        }
    }
  }
  return o.str();
}

std::string converter(const char *name) {
  return std::string(name);
}
std::string converter(unsigned index) {
  return oscompat::to_string(index);
}
std::string converter(int index) {
  return oscompat::to_string(index);
}
std::string converter(const StringPrimitive *str) {
  llvm::SmallVector<char16_t, 16> buf;
  str->copyUTF16String(buf);
  std::string out;
  convertUTF16ToUTF8WithReplacements(out, UTF16Ref(buf));
  return out;
}
std::string converter(const UTF16Ref ref) {
  std::string out;
  convertUTF16ToUTF8WithReplacements(out, ref);
  return out;
}

} // namespace vm
} // namespace hermes
