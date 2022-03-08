/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_STRINGBUILDER_H
#define HERMES_VM_STRINGBUILDER_H

#include "hermes/ADT/SafeInt.h"
#include "hermes/VM/Casting.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringPrimitive.h"

namespace hermes {
namespace vm {

/// There are many cases we want to allocate a string with a fixed length, and
/// fill in content afterwards.
/// StringBuilder is a helper class that provides such feature.
/// It ensures that the content never exceeds the length, and we can only
/// obtain the string after we have filled in every character.
/// It supports building both ASCII and UTF16 string. In cases where it does
/// not matter and you are not sure, use default (UTF16). Only in places where
/// it is obvious that this will be an ASCII string most of the time, we create
/// an ASCII string builder. In the worst case when you start to append UTF16
/// into an ASCII string, the builder will automatically take care of that
/// and create a new UTF16 string internally.
class StringBuilder {
  /// Handle to the StringPrimitive we are constructing.
  MutableHandle<StringPrimitive> strPrim_;

  /// Current appending index.
  uint32_t index_;

  /// The runtime. We need this in the rare case when we have to create a new
  /// string when we append UTF16 string into an ASCII string.
  Runtime *runtime_;

 public:
  static CallResult<StringBuilder> createStringBuilder(
      Runtime &runtime,
      SafeUInt32 length,
      bool isASCII = false) {
    if (length.isOverflowed()) {
      return runtime.raiseRangeError("String length exceeds limit");
    }
    auto crtRes = StringPrimitive::create(runtime, *length, isASCII);
    if (LLVM_UNLIKELY(crtRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return StringBuilder(runtime, crtRes->getString());
  }

  /// Append an UTF16Ref \p str. Note that str should not point to a GC-managed
  /// memory, as this function in theory can allocate.
  void appendUTF16Ref(UTF16Ref str) {
    assert(
        index_ + str.size() <= strPrim_->getStringLength() &&
        "StringBuilder append out of bound");
    if (LLVM_UNLIKELY((*strPrim_)->isASCII())) {
      // If we are appending a UTF16 string to an ASCII, we have to recreate
      // the string. This can cause performance issues if misused.
      // The allocation can fail in theory, but in practice, since we are
      // dropping one string and replace it with another, this should be safe.
      auto strRes = runtime_->ignoreAllocationFailure(StringPrimitive::create(
          *runtime_, strPrim_->getStringLength(), /*asciiNotUTF16*/ false));
      auto currentPartialString =
          ASCIIRef(strPrim_->castToASCIIPointer(), index_);
      strPrim_ = strRes.getString();
      index_ = 0;
      appendASCIIRef(currentPartialString);
    }
    std::copy(
        str.data(),
        str.data() + str.size(),
        strPrim_->castToUTF16PointerForWrite() + index_);
    index_ += str.size();
  }

  void appendASCIIRef(ASCIIRef ascii) {
    assert(
        index_ + ascii.size() <= strPrim_->getStringLength() &&
        "StringBuilder append out of bound");
    if (LLVM_LIKELY(strPrim_->isASCII())) {
      std::copy(
          ascii.data(),
          ascii.data() + ascii.size(),
          strPrim_->castToASCIIPointerForWrite() + index_);
    } else {
      std::copy(
          ascii.data(),
          ascii.data() + ascii.size(),
          strPrim_->castToUTF16PointerForWrite() + index_);
    }
    index_ += ascii.size();
  }

  /// Append a char16_t character \p ch.
  void appendCharacter(char16_t ch) {
    assert(
        index_ + 1 <= strPrim_->getStringLength() &&
        "StringBuilder append out of bound");
    if (strPrim_->isASCII()) {
      if (ch < 128) {
        strPrim_->castToASCIIPointerForWrite()[index_++] = ch;
      } else {
        // Reuse the implementation of appendUTF16Ref, which will trigger
        // allocating a new UTF16 string.
        appendUTF16Ref(ch);
      }
    } else {
      strPrim_->castToUTF16PointerForWrite()[index_++] = ch;
    }
  }

  /// Append the first \p length characters from StringPrimitive \p other.
  void appendStringPrim(Handle<StringPrimitive> other, uint32_t length) {
    assert(
        index_ + length <= strPrim_->getStringLength() &&
        "StringBuilder append out of bound");
    if (other->isASCII()) {
      appendASCIIRef({other->castToASCIIPointer(), length});
    } else if (!strPrim_->isASCII()) {
      appendUTF16Ref({other->castToUTF16Pointer(), length});
    } else {
      // strPrim_ is ASCII, while other is UTF16. We have to recreate string.
      auto strRes = runtime_->ignoreAllocationFailure(StringPrimitive::create(
          *runtime_, strPrim_->getStringLength(), /*asciiNotUTF16*/ false));
      auto currentPartialString =
          ASCIIRef(strPrim_->castToASCIIPointer(), index_);
      // Set strPrim_ to the newly created string.
      strPrim_ = strRes.getString();
      index_ = 0;
      // Append original string and other.
      appendASCIIRef(currentPartialString);
      appendUTF16Ref({other->castToUTF16Pointer(), length});
    }
  }

  /// Append all characters from StringPrimitive \p other.
  void appendStringPrim(Handle<StringPrimitive> other) {
    return appendStringPrim(other, other->getStringLength());
  }

  /// After appending finished, return the StringPrimitive.
  Handle<StringPrimitive> getStringPrimitive() const {
    assert(
        index_ == strPrim_->getStringLength() &&
        "String content is not fully filled yet.");
    return Handle<StringPrimitive>::vmcast(strPrim_);
  }

 private:
  StringBuilder(Runtime &runtime, StringPrimitive *strPrim)
      : strPrim_(runtime, strPrim), index_(0), runtime_(&runtime) {}
};

} // namespace vm
} // namespace hermes

#endif
