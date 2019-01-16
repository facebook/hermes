#ifndef HERMES_SUPPORT_JSONEMITTER_H
#define HERMES_SUPPORT_JSONEMITTER_H

#include <cstdint>
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {

/// JSONEmitter is a simple, stateful class for emitting JSON to an
/// llvm::raw_ostream.
/// To emit an array, use openArray(). Then call a sequence of emitValue() or
/// emitValues(), or use openArray()/openDict() to nest a collection. Finish
/// with closeArray(). To emit a dictionary, use openDict(), a sequence of
/// emitKeyValue() or emitKey() followed by openArray()/openDict() to nest a
/// collection. Finish with closeDict(). Unbalanced dictionaries/arrays are
/// caught via assert().
///
/// JSONEmitter accepts 8-bit strings which it expects to be in UTF-8 format.
///
/// Example usage:
///  JSONEmitter json(llvm::outs());
///  json.openDict();
///  json.emitKeyValue("name", "hermes");
///  json.emitKeyValue("age", 2);
///  json.emitKey("platforms");
///  json.openArray();
///  json.emitValues({"ios", "android"});
///  json.closeArray();
///  json.closeDict();
///
class JSONEmitter {
 public:
  /// Construct a JSONEmitter to output to a stream \p OS. The emitter is
  /// initially empty. Usually you will want to invoke openDict() immediately on
  /// it.
  JSONEmitter(llvm::raw_ostream &OS) : OS(OS) {}

  /// Emit a boolean value \p val.
  void emitValue(bool val);

  /// Emit an integer value \p val.
  void emitValue(int32_t val);

  /// Emit an unsigned integer value \p val.
  void emitValue(uint32_t val);

  /// Emit a double value \p val.
  /// Note that only finite values may be so emitted. Infinities and NaNs are
  /// an error.
  void emitValue(double val);

  /// Emit a string \p val. This is not used to emit dictionary keys: use
  /// emitKey() instead.
  void emitValue(llvm::StringRef val);
  void emitValue(const char *val) {
    emitValue(llvm::StringRef(val));
  }

  /// Emit a dictionary key \p key. This requires that we are currently emitting
  /// a dictionary, and it expects a key (not a value).
  void emitKey(llvm::StringRef key);

  /// Emit a key \p key followed by a value \p val. This requires that we are
  /// currently emitting a dictionary and it expects a key.
  template <typename T>
  void emitKeyValue(llvm::StringRef key, const T &val) {
    emitKey(key);
    emitValue(val);
  }

  /// Emit a sequence of values \p val. This requires that we are currently
  /// emitting an array.
  template <typename T>
  void emitValues(llvm::ArrayRef<T> vals) {
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
  void primitiveEmitString(llvm::StringRef str);

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

  /// A State represents the status of a single object (Dictionary or Array)
  /// being emitted.
  struct State {
    /// Whether the object is a dictionary or array.
    enum Type : uint8_t { Dict, Array } type;

    /// Whether we need a comma before emitting another value.
    bool needsComma;

    /// Whether we are a dictionary and expect a key.
    bool needsKey;

    /// Construct a State given a type \p t.
    /* implicit */ State(Type t)
        : type(t), needsComma(false), needsKey(type == Dict) {}
  };

  /// The stack of states.
  llvm::SmallVector<State, 8> states_;

  /// The stream to output to.
  llvm::raw_ostream &OS;
};

}; // namespace hermes

#endif // HERMES_SUPPORT_JSONEMITTER_H
