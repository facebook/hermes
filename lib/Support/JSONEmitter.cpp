/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Support/JSONEmitter.h"

#include <cmath>
#include <iterator>
#include "hermes/Support/Conversions.h"
#include "hermes/Support/ErrorHandling.h"
#include "hermes/Support/UTF8.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/Support/NativeFormatting.h"

using namespace hermes;

JSONEmitter::JSONEmitter(JSONEmitter &&) = default;

void JSONEmitter::emitValue(bool val) {
  willEmitValue();
  OS << (val ? "true" : "false");
}

void JSONEmitter::emitValue(short val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(int val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(long val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(long long val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(unsigned short val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(unsigned int val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(unsigned long val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(unsigned long long val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(double val) {
  willEmitValue();
  if (std::isfinite(val)) {
    char buf8[hermes::NUMBER_TO_STRING_BUF_SIZE];
    (void)hermes::numberToString(val, buf8, sizeof(buf8));
    OS << buf8;
  } else {
    OS << "null";
  }
}

void JSONEmitter::emitValue(llvh::StringRef val) {
  willEmitValue();
  primitiveEmitString(val);
}

void JSONEmitter::emitNullValue() {
  willEmitValue();
  OS << "null";
}

void JSONEmitter::emitKey(llvh::StringRef key) {
  assert(inDict() && "Not emitting a dictionary");
  State &state = states_.back();
  assert(state.needsKey && "Not expecting a key");
  assert(!state.needsValue && "Missing a value for a key.");
  if (state.needsComma)
    OS << ',';
  prettyNewLine();
  state.needsComma = false;
  state.needsKey = false;
  state.needsValue = true;
  primitiveEmitString(key);
  OS << ':';
  if (pretty_) {
    OS << ' ';
  }
}

void JSONEmitter::openDict() {
  willEmitValue();
  OS << '{';
  indentMore();
  states_.push_back(State::Dict);
}

void JSONEmitter::closeDict() {
  assert(inDict() && "Not currently emitting a dictionary");
  assert(!states_.back().needsValue && "Missing a value for a key.");
  indentLess();
  if (!states_.back().isEmpty) {
    prettyNewLine();
  }
  OS << '}';
  states_.pop_back();
}

void JSONEmitter::openArray() {
  willEmitValue();
  indentMore();
  OS << '[';
  states_.push_back(State::Array);
}

void JSONEmitter::closeArray() {
  assert(inArray() && "Not currently emitting an array");
  indentLess();
  if (!states_.back().isEmpty) {
    prettyNewLine();
  }
  OS << ']';
  states_.pop_back();
}

void JSONEmitter::primitiveEmitString(llvh::StringRef str) {
  OS << '"';
  const char *begin8 = str.begin();
  const char *end8 = str.end();
  auto errorHandler = [](const llvh::Twine &) {
    hermes_fatal("invalid UTF-8");
  };
  while (begin8 != end8) {
    uint32_t cp = decodeUTF8<true>(begin8, errorHandler);
    // Escape non-ascii characters.
    if (cp > 0x7F) {
      llvh::SmallVector<char16_t, 2> utf16Chars;
      auto insert = std::back_inserter(utf16Chars);
      encodeUTF16(insert, cp);
      for (auto &c : utf16Chars) {
        OS << "\\u";
        llvh::write_hex(OS, c, llvh::HexPrintStyle::Lower, 4);
      }
      continue;
    }
    if (cp == '\"' || cp == '\\' || cp == '/') {
      // escape quotation mark, slash, forward slash by adding a '\'.
      OS << '\\';
    }
    if (cp >= 0x20) {
      OS << (char)cp;
      continue;
    }
    switch (cp) {
      case '\b':
        OS << "\\b";
        break;
      case '\f':
        OS << "\\f";
        break;
      case '\n':
        OS << "\\n";
        break;
      case '\r':
        OS << "\\r";
        break;
      case '\t':
        OS << "\\t";
        break;
      default:
        OS << "\\u";
        llvh::write_hex(OS, cp, llvh::HexPrintStyle::Lower, 4);
    }
  }
  OS << '"';
}

void JSONEmitter::endJSONL() {
  assert(states_.empty() && "Previous object was not terminated.");
  OS << "\n";
}

void JSONEmitter::willEmitValue() {
  // Allow the top-level value to be set. TODO: guard against multiple
  // top-level values.
  if (states_.empty())
    return;
  State &state = states_.back();
  assert(!state.needsKey && "Expected a key");
  // Emit a comma if necessary. The next value certainly needs a comma, and if
  // we are a dictionary we expect a key next.
  if (state.needsComma)
    OS << ',';
  state.needsKey = (state.type == State::Dict);
  state.needsComma = true;
  state.needsValue = false;
  state.isEmpty = false;
  if (state.type == State::Array) {
    prettyNewLine();
  }
}

void JSONEmitter::prettyNewLine() {
  if (!pretty_)
    return;
  OS << "\n";
  for (uint32_t i = 0; i < indent_; ++i) {
    OS << " ";
  }
}

void JSONEmitter::indentMore() {
  if (!pretty_)
    return;
  indent_ += 2;
}

void JSONEmitter::indentLess() {
  if (!pretty_)
    return;
  assert(indent_ >= 2 && "Unbalanced indentation.");
  indent_ -= 2;
}
