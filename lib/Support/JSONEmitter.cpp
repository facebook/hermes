/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/JSONEmitter.h"
#include <cmath>

using namespace hermes;

JSONEmitter::JSONEmitter(JSONEmitter &&) = default;

void JSONEmitter::emitValue(bool val) {
  willEmitValue();
  OS << (val ? "true" : "false");
}

void JSONEmitter::emitValue(int32_t val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(uint32_t val) {
  willEmitValue();
  OS << val;
}

void JSONEmitter::emitValue(double val) {
  assert(std::isfinite(val) && "Value is not finite");
  willEmitValue();
  OS << llvm::format("%g", val);
}

void JSONEmitter::emitValue(llvm::StringRef val) {
  willEmitValue();
  primitiveEmitString(val);
}

void JSONEmitter::emitNullValue() {
  willEmitValue();
  OS << "null";
}

void JSONEmitter::emitKey(llvm::StringRef key) {
  assert(inDict() && "Not emitting a dictionary");
  State &state = states_.back();
  assert(state.needsKey && "Not expecting a key");
  assert(!state.needsValue && "Missing a value for a key.");
  if (state.needsComma)
    OS << ',';
  state.needsComma = false;
  state.needsKey = false;
  state.needsValue = true;
  primitiveEmitString(key);
  OS << ':';
}

void JSONEmitter::openDict() {
  willEmitValue();
  OS << '{';
  states_.push_back(State::Dict);
}

void JSONEmitter::closeDict() {
  assert(inDict() && "Not currently emitting a dictionary");
  assert(!states_.back().needsValue && "Missing a value for a key.");
  OS << '}';
  states_.pop_back();
}

void JSONEmitter::openArray() {
  willEmitValue();
  OS << '[';
  states_.push_back(State::Array);
}

void JSONEmitter::closeArray() {
  assert(inArray() && "Not currently emitting an array");
  OS << ']';
  states_.pop_back();
}

void JSONEmitter::primitiveEmitString(llvm::StringRef str) {
  OS << '"';
  for (char c : str) {
    switch (c) {
      case '\\':
      case '"':
        OS << '\\' << c;
        break;
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
        OS << c;
        break;
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
}
