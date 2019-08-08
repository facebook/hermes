/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/VM/Serializer.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/Runtime.h"

#include "llvm/Support/Debug.h"

#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

using SerializeCallBack = void(Serializer &s, const GCCell *cell);

static SerializeCallBack *serializeImpl[] = {
#define CELL_KIND(name) name##Serialize,
#include "hermes/VM/CellKinds.def"
#undef CELL_KIND
};

Serializer::Serializer(llvm::raw_ostream &OS, Runtime *runtime)
    : os_(OS), runtime_(runtime) {
  //  Write the header here.
  writeHeader();

  // Always map nullptr to 0, so it can be changed back when deserialize.
  relocationMap_[0] = 0;
}

uint32_t Serializer::endObject(const void *object) {
  uint32_t res = writeRelocation(object);
  LLVM_DEBUG(
      llvm::dbgs() << "[endObject] " << object << ", id " << res << "\n");
  return res;
}

void Serializer::flushCharBufs() {
  if (charBufOffset_ > 0) {
    // Write charBuf_.
    assert(
        charBufOffset_ == charBuf_.size() &&
        "Illegal offset for string buffer");
    os_.write(charBuf_.data(), charBufOffset_);
    writtenBytes_ += charBufOffset_;
  }
  // Write size of charBuf_.
  writeInt<uint32_t>(charBufOffset_);

  if (char16BufOffset_ > 0) {
    // Write char16Buf_.
    assert(
        char16BufOffset_ == char16Buf_.size() * 2 &&
        "Illegal offset for string buffer");
    os_.write(
        reinterpret_cast<char *>(char16Buf_.data()), char16BufOffset_ * 2);
    writtenBytes_ += char16BufOffset_ * 2;
  }
  // Write size of char16Buf_.
  writeInt<uint32_t>(char16BufOffset_ * 2);
}

void Serializer::serializeCell(const GCCell *cell) {
  serializeImpl[(uint8_t)cell->getKind()](*this, cell);
}

void Serializer::writeHeader() {
  // Default value all false.
  SerializeHeader header;

  header.maxHeapSize = runtime_->getHeap().maxSize();

#ifndef NDEBUG
  header.isDebug = true;
#endif
#ifdef HERMES_ENABLE_DEBUGGER
  header.isEnableDebugger = true;
#endif
  writeData(&header, sizeof(SerializeHeader));
}

void Serializer::writeCurrentOffset() {
  writeInt<uint32_t>(writtenBytes_);
}

} // namespace vm
} // namespace hermes
