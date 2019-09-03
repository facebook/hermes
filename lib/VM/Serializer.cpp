/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifdef HERMESVM_SERIALIZE
#include "hermes/VM/Serializer.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/GCPointer.h"
#include "hermes/VM/JSArrayBuffer.h"
#include "hermes/VM/JSDataView.h"
#include "hermes/VM/JSNativeFunctions.h"
#include "hermes/VM/JSTypedArray.h"
#include "hermes/VM/JSWeakMapImpl.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"

#include "JSLib/JSLibInternal.h"

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

  // Assign relocation id for all native function without template function
  // pointers.
#define NATIVE_FUNCTION(func)                                             \
  assert(relocationMap_.count((void *)func) == 0);                        \
  LLVM_DEBUG(                                                             \
      llvm::dbgs() << currentId_ << ", " << #func << ", " << (void *)func \
                   << "\n");                                              \
  relocationMap_[(void *)func] = currentId_++;

  // Assign relocation id for native functions with template of one type
  // function pointers.
#define NATIVE_FUNCTION_TYPED(func, type)                                  \
  assert(relocationMap_.count((void *)func<type>) == 0);                   \
  relocationMap_[(void *)func<type>] = currentId_;                         \
  LLVM_DEBUG(                                                              \
      llvm::dbgs() << currentId_ << ", " << #func << "<" << #type << ">, " \
                   << (void *)func<type> << "\n");                         \
  currentId_++;

  // Assign relocation id for native functions with template of two types.
  // function pointers.
#define NATIVE_FUNCTION_TYPED_2(func, type, type2)                             \
  assert(relocationMap_.count((void *)func<type, type2>) == 0);                \
  relocationMap_[(void *)func<type, type2>] = currentId_;                      \
  LLVM_DEBUG(                                                                  \
      llvm::dbgs() << currentId_ << ", " << #func << "<" << #type << ", "      \
                   << #type2 << ">, " << ((void *)func<type, type2>) << "\n"); \
  currentId_++;

  // Assign relocation id for all constructor function without template function
  // pointers.
  using CreatorFunction = CallResult<HermesValue>(Runtime *, Handle<JSObject>);
  CreatorFunction *funcPtr;
#define NATIVE_CONSTRUCTOR(func)                                             \
  funcPtr = func;                                                            \
  assert(relocationMap_.count((void *)funcPtr) == 0);                        \
  relocationMap_[(void *)funcPtr] = currentId_;                              \
  LLVM_DEBUG(                                                                \
      llvm::dbgs() << currentId_ << ", " << #func << ", " << (void *)funcPtr \
                   << "\n");                                                 \
  currentId_++;

  // Assign relocation id for constructor functions with template,
#define NATIVE_CONSTRUCTOR_TYPED(classname, type, type2, func)                 \
  funcPtr = classname<type, type2>::func;                                      \
  assert(relocationMap_.count((void *)funcPtr) == 0);                          \
  relocationMap_[(void *)funcPtr] = currentId_;                                \
  LLVM_DEBUG(                                                                  \
      llvm::dbgs() << currentId_ << ", " << #classname << "<" << #type << ", " \
                   << #type2 << "::" << #func << ", " << (void *)funcPtr       \
                   << "\n");                                                   \
  currentId_++;

#include "hermes/VM/NativeFunctions.def"
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

  header.heapSize = runtime_->getHeap().size();

#ifndef NDEBUG
  header.isDebug = true;
#endif
#ifdef HERMES_ENABLE_DEBUGGER
  header.isEnableDebugger = true;
#endif

  runtime_->populateHeaderRuntimeConfig(header);
  writeData(&header, sizeof(SerializeHeader));
}

void Serializer::writeCurrentOffset() {
  writeInt<uint32_t>(writtenBytes_);
}

} // namespace vm
} // namespace hermes
#endif
