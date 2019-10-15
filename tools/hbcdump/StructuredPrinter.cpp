/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StructuredPrinter.h"

#include <cmath>
#include <iterator>
#include "hermes/Support/JSONEmitter.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/NativeFormatting.h"
#include "llvm/Support/raw_ostream.h"

using namespace hermes;

/// PlainTextStructuredPrinter prints data in a very simple human-readable
/// format, in which the top level entity is either a value, a dict or an array;
/// the top level array may contain dicts as values, but no other nesting of
/// dicts/arrays is allowed.
class PlainTextStructuredPrinter : public StructuredPrinter {
 public:
  PlainTextStructuredPrinter(llvm::raw_ostream &OS) : OS(OS) {}

  void emitValue(bool val) override;
  void emitValue(int32_t val) override;
  void emitValue(uint32_t val) override;
  void emitValue(double val) override;
  void emitValue(llvm::StringRef val) override;
  void emitNullValue() override;
  void emitKey(llvm::StringRef key) override;
  void openDict() override;
  void closeDict() override;
  void openArray() override;
  void closeArray() override;

  PlainTextStructuredPrinter(PlainTextStructuredPrinter &&);

 private:
  /// \return whether a dictionary is currently being emitted.
  bool inDict() const {
    return !states_.empty() && states_.back().type == State::Dict;
  }

  /// \return whether an array is currently being emitted.
  bool inArray() const {
    return !states_.empty() && states_.back().type == State::Array;
  }

  /// Given that we are about to emit a value of any type (but not a dictionary
  /// key), perform housekeeping tasks such as emitting a trailing newline.
  void willEmitValue();

  /// A State represents the status of a single object (Dictionary or Array)
  /// being emitted.
  struct State {
    /// Whether the object is a dictionary or array.
    enum Type : uint8_t { Dict, Array } type;

    /// Whether we need a newline before emitting another value.
    bool needsNewline;

    /// Whether we are a dictionary and expect a key.
    bool needsKey;

    /// Whether we expect a value (only after emitting a key in a dict).
    bool needsValue;

    /// Whether the dict or array is empty.
    bool isEmpty;

    /// Construct a State given a type \p t.
    /* implicit */ State(Type t)
        : type(t),
          needsNewline(false),
          needsKey(type == Dict),
          needsValue(false),
          isEmpty(true) {}
  };

  /// The stack of states.
  llvm::SmallVector<State, 8> states_;

  /// The stream to output to.
  llvm::raw_ostream &OS;
};

/// JSONStructuredPrinter prints data in a pretty JSON format using JSONEmitter.
class JSONStructuredPrinter : public StructuredPrinter {
 public:
  JSONStructuredPrinter(llvm::raw_ostream &os) : json_(os, true) {}

  void emitValue(bool val) override;
  void emitValue(int32_t val) override;
  void emitValue(uint32_t val) override;
  void emitValue(double val) override;
  void emitValue(llvm::StringRef val) override;
  void emitNullValue() override;
  void emitKey(llvm::StringRef key) override;
  void openDict() override;
  void closeDict() override;
  void openArray() override;
  void closeArray() override;

  JSONStructuredPrinter(JSONStructuredPrinter &&);

 private:
  /// The JSONEmitter to output to.
  JSONEmitter json_;
};

StructuredPrinter::StructuredPrinter(StructuredPrinter &&) = default;

std::unique_ptr<StructuredPrinter> StructuredPrinter::create(
    llvm::raw_ostream &OS,
    bool json) {
  std::unique_ptr<StructuredPrinter> printer;
  if (json) {
    return llvm::make_unique<JSONStructuredPrinter>(OS);
  }
  return llvm::make_unique<PlainTextStructuredPrinter>(OS);
}

void PlainTextStructuredPrinter::emitValue(bool val) {
  willEmitValue();
  OS << (val ? "true" : "false");
}

void PlainTextStructuredPrinter::emitValue(int32_t val) {
  willEmitValue();
  OS << val;
}

void PlainTextStructuredPrinter::emitValue(uint32_t val) {
  willEmitValue();
  OS << val;
}

void PlainTextStructuredPrinter::emitValue(double val) {
  assert(std::isfinite(val) && "Value is not finite");
  willEmitValue();
  OS << llvm::format("%g", val);
}

void PlainTextStructuredPrinter::emitValue(llvm::StringRef val) {
  willEmitValue();
  OS << val;
}

void PlainTextStructuredPrinter::emitNullValue() {
  willEmitValue();
  OS << "null";
}

void PlainTextStructuredPrinter::emitKey(llvm::StringRef key) {
  assert(inDict() && "Not emitting a dictionary");
  State &state = states_.back();
  assert(state.needsKey && "Not expecting a key");
  assert(!state.needsValue && "Missing a value for a key.");
  if (state.needsNewline)
    OS << '\n';
  state.needsNewline = false;
  state.needsKey = false;
  state.needsValue = true;
  OS << key << ": ";
}

void PlainTextStructuredPrinter::openDict() {
  assert(!inDict() && "Cannot nest a dictionary within a dictionary");
  willEmitValue();
  states_.push_back(State::Dict);
}

void PlainTextStructuredPrinter::closeDict() {
  assert(inDict() && "Not currently emitting a dictionary");
  assert(!states_.back().needsValue && "Missing a value for a key.");
  OS << "\n";
  states_.pop_back();
}

void PlainTextStructuredPrinter::openArray() {
  assert(
      !inArray() && !inDict() &&
      "Cannot nest an array within an array or dictionary");
  willEmitValue();
  states_.push_back(State::Array);
}

void PlainTextStructuredPrinter::closeArray() {
  assert(inArray() && "Not currently emitting an array");
  states_.pop_back();
}

void PlainTextStructuredPrinter::willEmitValue() {
  // Allow the top-level value to be set. TODO: guard against multiple
  // top-level values.
  if (states_.empty())
    return;
  State &state = states_.back();
  assert(!state.needsKey && "Expected a key");
  // Emit a newline if necessary. The next value certainly needs a newline, and
  // if we are a dictionary we expect a key next.
  if (state.needsNewline)
    OS << '\n';
  state.needsKey = (state.type == State::Dict);
  state.needsNewline = true;
  state.needsValue = false;
  state.isEmpty = false;
}

PlainTextStructuredPrinter::PlainTextStructuredPrinter(
    PlainTextStructuredPrinter &&) = default;

void JSONStructuredPrinter::emitValue(bool val) {
  json_.emitValue(val);
}

void JSONStructuredPrinter::emitValue(int32_t val) {
  json_.emitValue(val);
}

void JSONStructuredPrinter::emitValue(uint32_t val) {
  json_.emitValue(val);
}

void JSONStructuredPrinter::emitValue(double val) {
  json_.emitValue(val);
}

void JSONStructuredPrinter::emitValue(llvm::StringRef val) {
  json_.emitValue(val);
}

void JSONStructuredPrinter::emitNullValue() {
  json_.emitNullValue();
}

void JSONStructuredPrinter::emitKey(llvm::StringRef key) {
  json_.emitKey(key);
}

void JSONStructuredPrinter::openDict() {
  json_.openDict();
}

void JSONStructuredPrinter::closeDict() {
  json_.closeDict();
}

void JSONStructuredPrinter::openArray() {
  json_.openArray();
}

void JSONStructuredPrinter::closeArray() {
  json_.closeArray();
}

JSONStructuredPrinter::JSONStructuredPrinter(JSONStructuredPrinter &&) =
    default;
