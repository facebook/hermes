/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_TOOLS_HBCDUMP_STRUCTUREDPRINTER_H
#define HERMES_TOOLS_HBCDUMP_STRUCTUREDPRINTER_H

#include <cstdint>
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {

/// StructuredPrinter is an interface for printing simple structured data.
/// Implementations of StructuredPrinter apply formatting logic (e.g. plain text
/// vs JSON) and may impose specific constraints on the structure of the data.
class StructuredPrinter {
 public:
  /// Create a printer that prints to the specified output stream and optionally
  /// prints in JSON format.
  static std::unique_ptr<StructuredPrinter> create(
      llvm::raw_ostream &OS,
      bool json);

  /// Emit a boolean value \p val.
  virtual void emitValue(bool val) = 0;

  /// Emit an integer value \p val.
  virtual void emitValue(int32_t val) = 0;

  /// Emit an unsigned integer value \p val.
  virtual void emitValue(uint32_t val) = 0;

  /// Emit a double value \p val.
  virtual void emitValue(double val) = 0;

  /// Emit a string \p val. This is not used to emit dictionary keys: use
  /// emitKey() instead.
  virtual void emitValue(llvm::StringRef val) = 0;
  void emitValue(const char *val) {
    emitValue(llvm::StringRef(val));
  }

  /// Emit a null as value.
  virtual void emitNullValue() = 0;

  /// Emit a dictionary key \p key. This requires that we are currently emitting
  /// a dictionary, and it expects a key (not a value).
  virtual void emitKey(llvm::StringRef key) = 0;

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
  virtual void openDict() = 0;

  /// Close the currently emitting dictionary.
  virtual void closeDict() = 0;

  /// Begin emitting an array.
  virtual void openArray() = 0;

  /// Close the currently emitting array.
  virtual void closeArray() = 0;

  StructuredPrinter() = default;

  /// StructuredPrinter subclasses hold raw_ostreams by reference.
  /// They may be moved but not copied.
  StructuredPrinter(StructuredPrinter &&);

  virtual ~StructuredPrinter() = default;

 private:
  StructuredPrinter(const StructuredPrinter &) = delete;
  void operator=(const StructuredPrinter &) = delete;
  void operator=(StructuredPrinter &&) = delete;
};

}; // namespace hermes

#endif // HERMES_TOOLS_HBCDUMP_STRUCTUREDPRINTER_H
