/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "hermes/VM/SerializedValue.h"
#include "jsi/jsi.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hermes::vm::experimental {

using JSONValue = ::facebook::jsi::JSONValue;

/// Encodes the JSON subset directly into Hermes' internal structured-clone
/// SerializedValue representation, without constructing a Runtime.
class JSONSerializedValueEncoder {
 public:
  SerializedValue encode(const JSONValue &value) {
    SerializedValue result;
    out_ = &result;
    stringIds_.clear();
    writeValue(value);
    out_ = nullptr;
    return result;
  }

 private:
  SerializedValue *out_{nullptr};
  std::unordered_map<std::string, uint32_t> stringIds_;

  static uint8_t typeByte(SerializedValue::Type type) {
    return static_cast<uint8_t>(type);
  }

  template <typename T>
  static void appendPod(std::vector<uint8_t> &buffer, T value) {
    const auto *bytes = reinterpret_cast<const uint8_t *>(&value);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
  }

  static void appendDouble(std::vector<uint8_t> &buffer, double value) {
    uint64_t bits;
    static_assert(sizeof(bits) == sizeof(value), "Unexpected double size");
    std::memcpy(&bits, &value, sizeof(bits));
    appendPod<uint64_t>(buffer, bits);
  }

  static bool isASCII(std::string_view value) {
    for (unsigned char c : value) {
      if (c > 0x7f) {
        return false;
      }
    }
    return true;
  }

  static void appendUTF8AsUTF16(
      std::vector<uint8_t> &buffer,
      std::string_view value) {
    auto appendCodeUnit = [&buffer](char16_t unit) {
      appendPod<char16_t>(buffer, unit);
    };

    for (size_t i = 0; i < value.size();) {
      const uint8_t c = static_cast<uint8_t>(value[i++]);
      uint32_t codePoint;
      if (c < 0x80) {
        codePoint = c;
      } else if ((c >> 5) == 0x6 && i < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x1f) << 6) | (c1 & 0x3f);
      } else if ((c >> 4) == 0xe && i + 1 < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        const uint8_t c2 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);
      } else if ((c >> 3) == 0x1e && i + 2 < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        const uint8_t c2 = static_cast<uint8_t>(value[i++]);
        const uint8_t c3 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x07) << 18) | ((c1 & 0x3f) << 12) |
            ((c2 & 0x3f) << 6) | (c3 & 0x3f);
      } else {
        throw std::invalid_argument("Invalid UTF-8 JSON string");
      }

      if (codePoint <= 0xffff) {
        appendCodeUnit(static_cast<char16_t>(codePoint));
      } else if (codePoint <= 0x10ffff) {
        codePoint -= 0x10000;
        appendCodeUnit(static_cast<char16_t>(0xd800 + (codePoint >> 10)));
        appendCodeUnit(static_cast<char16_t>(0xdc00 + (codePoint & 0x3ff)));
      } else {
        throw std::invalid_argument("Invalid UTF-8 JSON string");
      }
    }
  }

  static void appendUTF16(
      std::vector<uint8_t> &buffer,
      std::u16string_view value) {
    buffer.insert(
        buffer.end(),
        reinterpret_cast<const uint8_t *>(value.data()),
        reinterpret_cast<const uint8_t *>(value.data() + value.size()));
  }

  static uint32_t utf16LengthFromUTF8(std::string_view value) {
    uint32_t len = 0;
    for (size_t i = 0; i < value.size();) {
      const uint8_t c = static_cast<uint8_t>(value[i++]);
      uint32_t codePoint;
      if (c < 0x80) {
        codePoint = c;
      } else if ((c >> 5) == 0x6 && i < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x1f) << 6) | (c1 & 0x3f);
      } else if ((c >> 4) == 0xe && i + 1 < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        const uint8_t c2 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);
      } else if ((c >> 3) == 0x1e && i + 2 < value.size()) {
        const uint8_t c1 = static_cast<uint8_t>(value[i++]);
        const uint8_t c2 = static_cast<uint8_t>(value[i++]);
        const uint8_t c3 = static_cast<uint8_t>(value[i++]);
        codePoint = ((c & 0x07) << 18) | ((c1 & 0x3f) << 12) |
            ((c2 & 0x3f) << 6) | (c3 & 0x3f);
      } else {
        throw std::invalid_argument("Invalid UTF-8 JSON string");
      }
      len += codePoint <= 0xffff ? 1 : 2;
    }
    return len;
  }

  static std::string makeStringKey(const JSONValue &value) {
    std::string key;
    key.push_back(static_cast<char>(value.stringEncoding));
    if (value.stringEncoding == JSONValue::StringEncoding::UTF16) {
      const auto *bytes =
          reinterpret_cast<const char *>(value.utf16StringValue.data());
      key.append(bytes, value.utf16StringValue.size() * sizeof(char16_t));
    } else {
      key.append(value.stringValue);
    }
    return key;
  }

  uint32_t internString(const JSONValue &value) {
    std::string key = makeStringKey(value);
    auto it = stringIds_.find(key);
    if (it != stringIds_.end()) {
      return it->second;
    }

    const uint32_t id = out_->offsets.size();
    stringIds_.emplace(std::move(key), id);
    out_->offsets.push_back(out_->strings.size());

    const bool ascii = value.stringEncoding == JSONValue::StringEncoding::ASCII;
    out_->strings.push_back(ascii ? 1 : 0);
    const uint32_t length = ascii
        ? static_cast<uint32_t>(value.stringValue.size())
        : value.stringEncoding == JSONValue::StringEncoding::UTF16
        ? static_cast<uint32_t>(value.utf16StringValue.size())
        : utf16LengthFromUTF8(value.stringValue);
    appendPod<uint32_t>(out_->strings, length);

    if (ascii) {
      out_->strings.insert(
          out_->strings.end(),
          value.stringValue.begin(),
          value.stringValue.end());
    } else {
      while (out_->strings.size() % alignof(char16_t) != 0) {
        out_->strings.push_back(0);
      }
      if (value.stringEncoding == JSONValue::StringEncoding::UTF16) {
        appendUTF16(out_->strings, value.utf16StringValue);
      } else {
        appendUTF8AsUTF16(out_->strings, value.stringValue);
      }
    }

    return id;
  }

  uint32_t internUTF8String(std::string_view value) {
    return internString(JSONValue::utf8String(std::string{value}));
  }

  void writeValue(const JSONValue &value) {
    switch (value.kind) {
      case JSONValue::Kind::Null:
        out_->content.push_back(typeByte(SerializedValue::Type::Null));
        return;
      case JSONValue::Kind::Bool:
        out_->content.push_back(
            typeByte(SerializedValue::Type::PrimitiveBoolean));
        out_->content.push_back(value.boolValue ? 1 : 0);
        return;
      case JSONValue::Kind::Number:
        if (!std::isfinite(value.numberValue)) {
          throw std::invalid_argument("JSON numbers must be finite");
        }
        out_->content.push_back(
            typeByte(SerializedValue::Type::PrimitiveNumber));
        appendDouble(out_->content, value.numberValue);
        return;
      case JSONValue::Kind::String:
        out_->content.push_back(
            typeByte(SerializedValue::Type::PrimitiveString));
        appendPod<uint32_t>(out_->content, internString(value));
        return;
      case JSONValue::Kind::Array:
        writeArray(value.arrayValue);
        return;
      case JSONValue::Kind::Object:
        writeObject(value.objectValue);
        return;
    }
    assert(false && "Unhandled JSON kind");
  }

  uint32_t beginObjectRecord(SerializedValue::Type type) {
    const uint32_t id = out_->offsets.size();
    out_->offsets.push_back(out_->content.size());
    out_->content.push_back(typeByte(type));
    appendPod<uint32_t>(out_->content, id);
    return id;
  }

  void writeArray(const std::vector<JSONValue> &array) {
    if (array.size() > std::numeric_limits<uint32_t>::max()) {
      throw std::overflow_error("JSON array is too large");
    }
    beginObjectRecord(SerializedValue::Type::Array);
    const auto len = static_cast<uint32_t>(array.size());
    appendPod<uint32_t>(out_->content, len);
    appendPod<uint32_t>(out_->content, len);
    for (uint32_t i = 0; i < len; ++i) {
      appendPod<uint32_t>(out_->content, i);
      writeValue(array[i]);
    }
    appendPod<uint32_t>(out_->content, 0);
  }

  void writeObject(const JSONValue::Object &object) {
    if (object.size() > std::numeric_limits<uint32_t>::max()) {
      throw std::overflow_error("JSON object has too many properties");
    }
    beginObjectRecord(SerializedValue::Type::Object);
    appendPod<uint32_t>(out_->content, static_cast<uint32_t>(object.size()));
    for (const auto &[key, value] : object) {
      appendPod<uint32_t>(out_->content, internUTF8String(key));
      writeValue(value);
    }
  }
};

} // namespace hermes::vm::experimental
