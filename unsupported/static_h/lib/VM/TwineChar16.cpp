/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/TwineChar16.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringPrimitive.h"

#include "llvh/Support/raw_ostream.h"

namespace hermes {
namespace vm {

TwineChar16::TwineChar16(const StringPrimitive *str)
    : rightKind_(EmptyKind), rightSize_(0) {
  assert(str != nullptr);
  leftChild_.stringPrimitive = str;
  leftKind_ = StringPrimitiveKind;
  leftSize_ = str->getStringLength();
  assert(isValid());
}

void TwineChar16::print(llvh::raw_ostream &os) const {
  assert(isValid());
  auto printChild = [&os](const Node child, const NodeKind kind, size_t size) {
    switch (kind) {
      case TwineChar16::NullKind:
      case TwineChar16::EmptyKind:
        break;
      case TwineChar16::TwineKind:
        child.twine->print(os);
        break;
      case TwineChar16::CharStrKind:
        os << llvh::StringRef(child.charStr, size);
        break;
      case TwineChar16::Char16StrKind:
        os << UTF16Ref(child.char16Str, size);
        break;
      case TwineChar16::StringPrimitiveKind: {
        SmallU16String<32> str;
        child.stringPrimitive->appendUTF16String(str);
        os << str;
        break;
      }
      case TwineChar16::Int32Kind:
        os << child.int32;
        break;
      case TwineChar16::Unsigned32Kind:
        os << child.uint32;
        break;
      case TwineChar16::DoubleKind:
        char buf[NUMBER_TO_STRING_BUF_SIZE];
        auto len = numberToString(child.flt, buf, sizeof(buf));
        assert(len < sizeof(buf));
        buf[len] = '\0';
        os << buf;
        break;
    }
  };
  printChild(leftChild_, leftKind_, leftSize_);
  printChild(rightChild_, rightKind_, rightSize_);
}

size_t TwineChar16::toChar16Str(char16_t *out, size_t maxlen) const {
  assert(isValid());
  auto childToChar16Str =
      [](char16_t *out, const Node child, const NodeKind kind, size_t size) {
        switch (kind) {
          case TwineChar16::NullKind:
          case TwineChar16::EmptyKind:
            break;
          case TwineChar16::TwineKind:
            child.twine->toChar16Str(out, size);
            break;
          case TwineChar16::CharStrKind:
            std::copy(child.charStr, child.charStr + size, out);
            break;
          case TwineChar16::Char16StrKind:
            std::copy(child.char16Str, child.char16Str + size, out);
            break;
          case TwineChar16::StringPrimitiveKind: {
            SmallU16String<32> str;
            child.stringPrimitive->appendUTF16String(str);
            std::copy(str.begin(), str.end(), out);
            break;
          }
          case TwineChar16::Int32Kind: {
            char buf[32];
            auto len = ::snprintf(buf, sizeof(buf), "%" PRId32, child.int32);
            std::copy(buf, buf + len, out);
            break;
          }
          case TwineChar16::Unsigned32Kind: {
            char buf[32];
            auto len = ::snprintf(buf, sizeof(buf), "%" PRIu32, child.uint32);
            std::copy(buf, buf + len, out);
            break;
          }
          case TwineChar16::DoubleKind: {
            char buf[NUMBER_TO_STRING_BUF_SIZE];
            auto len = numberToString(child.flt, buf, sizeof(buf));
            assert(len < sizeof(buf));
            buf[len] = '\0';
            std::copy(buf, buf + len, out);
            break;
          }
        }
      };

  auto leftUsed = std::min(maxlen, leftSize_);
  auto rightUsed = std::min(maxlen - leftUsed, rightSize_);

  childToChar16Str(out, leftChild_, leftKind_, leftUsed);
  childToChar16Str(out + leftUsed, rightChild_, rightKind_, rightUsed);

  return leftUsed + rightUsed;
}

void TwineChar16::toVector(llvh::SmallVectorImpl<char16_t> &out) const {
  assert(isValid());
  auto childToVector = [&out](
                           const Node child, const NodeKind kind, size_t size) {
    switch (kind) {
      case TwineChar16::NullKind:
      case TwineChar16::EmptyKind:
        break;
      case TwineChar16::TwineKind:
        child.twine->toVector(out);
        break;
      case TwineChar16::CharStrKind:
        out.append(child.charStr, child.charStr + size);
        break;
      case TwineChar16::Char16StrKind:
        out.append(child.char16Str, child.char16Str + size);
        break;
      case TwineChar16::StringPrimitiveKind:
        child.stringPrimitive->appendUTF16String(out);
        break;
      case TwineChar16::Int32Kind: {
        char buf[32];
        auto len = ::snprintf(buf, sizeof(buf), "%" PRId32, child.int32);
        out.append(buf, buf + len);
        break;
      }
      case TwineChar16::Unsigned32Kind: {
        char buf[32];
        auto len = ::snprintf(buf, sizeof(buf), "%" PRIu32, child.uint32);
        out.append(buf, buf + len);
        break;
      }
      case TwineChar16::DoubleKind: {
        char buf[NUMBER_TO_STRING_BUF_SIZE];
        auto len = numberToString(child.flt, buf, sizeof(buf));
        assert(len < sizeof(buf));
        buf[len] = '\0';
        out.append(buf, buf + len);
        break;
      }
    }
  };
  out.reserve(size());
  childToVector(leftChild_, leftKind_, leftSize_);
  childToVector(rightChild_, rightKind_, rightSize_);
}

} // namespace vm
} // namespace hermes
