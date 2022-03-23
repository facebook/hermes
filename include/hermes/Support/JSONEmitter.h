/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_JSONEMITTER_H
#define HERMES_SUPPORT_JSONEMITTER_H

#include <cstdint>
#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Format.h"
#include "llvh/Support/raw_ostream.h"

namespace hermes {

/// JSONEmitter is a simple, stateful class for emitting JSON to an
/// llvh::raw_ostream.
/// To emit an array, use openArray(). Then call a sequence of emitValue() or
/// emitValues(), or use openArray()/openDict() to nest a collection. Finish
/// with closeArray(). To emit a dictionary, use openDict(), a sequence of
/// emitKeyValue() or emitKey() followed by openArray()/openDict() to nest a
/// collection. Finish with closeDict(). Unbalanced dictionaries/arrays are
/// caught via assert().
///
/// JSONEmitter accepts 8-bit strings which it expects to be in UTF-8 format.
/// Invalid UTF-8 strings are fatal errors. It is the caller's responsibility
/// to ensure only valid UTF-8 is passed.
///
/// Example usage:
///  JSONEmitter json(llvh::outs());
///  json.openDict();
///  json.emitKeyValue("name", "hermes");
///  json.emitKeyValue("age", 2);
///  json.emitKey("platforms");
///  json.openArray();
///  json.emitValues({"linux", "mac", "android"});
///  json.closeArray();
///  json.closeDict();
///
class JSONEmitter {
 public:
  /// Construct a JSONEmitter to output to a stream \p OS. The emitter is
  /// initially empty. Usually you will want to invoke openDict() immediately on
  /// it.
  /// \p pretty is the option to pretty print the JSON object, with new lines
  /// and indentation. Note that since the JSONEmitter does not know when
  /// the printing will stop, the user needs to print a new line themselves
  /// when they finish.
  JSONEmitter(llvh::raw_ostream &OS, bool pretty = false)
      : OS(OS), pretty_(pretty) {}

  /// Emit a boolean value \p val.
  void emitValue(bool val);

  /// Emit an integer value \p val.
  void emitValue(short val);
  void emitValue(int val);
  void emitValue(long val);
  void emitValue(long long val);

  /// Emit an unsigned integer value \p val.
  void emitValue(unsigned short val);
  void emitValue(unsigned int val);
  void emitValue(unsigned long val);
  void emitValue(unsigned long long val);

  /// Emit a double value \p val.
  /// Note that only finite values may be so emitted. Infinities and NaNs are
  /// an error.
  void emitValue(double val);

  /// Emit a string \p val. This is not used to emit dictionary keys: use
  /// emitKey() instead.
  void emitValue(llvh::StringRef val);
  void emitValue(const char *val) {
    emitValue(llvh::StringRef(val));
  }

  /// Emit a null as value.
  void emitNullValue();

  /// Emit a dictionary key \p key. This requires that we are currently emitting
  /// a dictionary, and it expects a key (not a value).
  void emitKey(llvh::StringRef key);

  /// Emit a key \p key followed by a value \p val. This requires that we are
  /// currently emitting a dictionary and it expects a key.
  template <typename T>
  void emitKeyValue(llvh::StringRef key, const T &val) {
    emitKey(key);
    emitValue(val);
  }

  /// Emit a sequence of values \p val. This requires that we are currently
  /// emitting an array.
  template <typename T>
  void emitValues(llvh::ArrayRef<T> vals) {
    for (const T &val : vals)
      emitValue(val);
  }

  template <typename T>
  void emitValues(std::initializer_list<T> vals) {
    for (const T &val : vals)
      emitValue(val);
  }

  /// Begin emitting a dictionary.
  void openDict();

  /// Close the currently emitting dictionary.
  void closeDict();

  /// Begin emitting an array.
  void openArray();

  /// Close the currently emitting array.
  void closeArray();

  /// Terminate a JSON Lines record.
  void endJSONL();

  /// JSONEmitters hold raw_ostreams by reference.
  /// They may be moved but not copied.
  JSONEmitter(JSONEmitter &&);

 private:
  JSONEmitter(const JSONEmitter &) = delete;
  void operator=(const JSONEmitter &) = delete;
  void operator=(JSONEmitter &&) = delete;

  /// A function to unconditionally emit a string. This is used for both
  /// dictionary keys and ordinary string values.
  /// Assumes \p str is encoded in utf-8.
  /// Escapes certain control characters and non-ascii characters.
  void primitiveEmitString(llvh::StringRef str);

  /// \return whether a dictionary is currently being emitted.
  bool inDict() const {
    return !states_.empty() && states_.back().type == State::Dict;
  }

  /// \return whether an array is currently being emitted.
  bool inArray() const {
    return !states_.empty() && states_.back().type == State::Array;
  }

  /// Given that we are about to emit a value of any type (but not a dictionary
  /// key), perform housekeeping tasks such as emitting a trailing comma.
  void willEmitValue();

  /// In pretty printing mode, print a new line then indent.
  void prettyNewLine();

  /// In pretty printing mode, indent one level more.
  void indentMore();

  /// In pretty printing mode, indent one level less.
  void indentLess();

  /// A State represents the status of a single object (Dictionary or Array)
  /// being emitted.
  struct State {
    /// Whether the object is a dictionary or array.
    enum Type : uint8_t { Dict, Array } type;

    /// Whether we need a comma before emitting another value.
    bool needsComma;

    /// Whether we are a dictionary and expect a key.
    bool needsKey;

    /// Whether we expect a value (only after emitting a key in a dict).
    bool needsValue;

    /// Whether the dict or object is empty.
    bool isEmpty;

    /// Construct a State given a type \p t.
    /* implicit */ State(Type t)
        : type(t),
          needsComma(false),
          needsKey(type == Dict),
          needsValue(false),
          isEmpty(true) {}
  };

  /// The stack of states.
  llvh::SmallVector<State, 8> states_;

  /// The stream to output to.
  llvh::raw_ostream &OS;

  /// Pretty print the JSON object.
  bool pretty_{false};

  /// Number of spaces needed to indent.
  uint32_t indent_{0};
};

} // namespace hermes

#endif // HERMES_SUPPORT_JSONEMITTER_H
