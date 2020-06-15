/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSREGEXP_H
#define HERMES_VM_JSREGEXP_H

#include "hermes/Regex/Compiler.h"
#include "hermes/VM/JSObject.h"
#include "hermes/VM/RegExpMatch.h"
#include "hermes/VM/SmallXString.h"

#include <memory>
#include "llvm/ADT/SmallString.h"

namespace hermes {
namespace vm {

class JSRegExp final : public JSObject {
 public:
  using Super = JSObject;
  static ObjectVTable vt;
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::RegExpKind;
  }

  /// Create a JSRegExp, with the empty string for pattern and flags
  static Handle<JSRegExp> create(Runtime *runtime, Handle<JSObject> prototype);

  /// Perform validation of the pattern and flags and throw \c SyntaxError on
  /// error. If valid, set the source and flags to the given strings, and set
  /// the standard properties of the RegExp according to the flags. Note that
  /// RegExps are not mutable (with the exception of the lastIndex property).
  /// If \p bytecode is given, initialize the regex with that bytecode.
  /// Otherwise compile the pattern and flags into a new regexp.
  static ExecutionStatus initialize(
      Handle<JSRegExp> selfHandle,
      Runtime *runtime,
      Handle<StringPrimitive> pattern,
      Handle<StringPrimitive> flags,
      OptValue<llvm::ArrayRef<uint8_t>> bytecode = llvm::None);

  /// \return the pattern string used to initialize this RegExp.
  /// Note this is not suitable for interpolation between //, nor for
  /// implementation of toString(). See 'escapePattern' to properly escape it.
  static PseudoHandle<StringPrimitive> getPattern(
      JSRegExp *self,
      PointerBase *base);

  /// \return An escaped version of the regexp pattern per ES6 21.2.3.2.4, or an
  /// exception if the string could not be created.
  static CallResult<HermesValue> escapePattern(
      Handle<StringPrimitive> pattern,
      Runtime *runtime);

  /// \return the flag bits used to initialize this RegExp
  static regex::SyntaxFlags getSyntaxFlags(JSRegExp *self) {
    return self->syntaxFlags_;
  }

  /// Searches self for a match for \str.
  /// \p searchStartOffset is the offset from which to begin searching.
  /// If searchStartOffset exceeds the length of the string, or if no match
  /// is found at or after searchStartOffset, then the result will be empty.
  /// If a match is found, the returned value will have length equal to the
  /// number of capture groups, plus one. The first element in the result will
  /// be the full match, with subsequent elements corresponding to the capture
  /// groups in order.
  /// Note that the location of matches is given relative to the beginning of
  /// the string, not the searchStartOffset.
  static CallResult<RegExpMatch> search(
      Handle<JSRegExp> selfHandle,
      Runtime *runtime,
      Handle<StringPrimitive> strHandle,
      uint32_t searchStartOffset);

 private:
#ifdef HERMESVM_SERIALIZE
  explicit JSRegExp(Deserializer &d);

  friend void RegExpSerialize(Serializer &s, const GCCell *cell);
  friend void RegExpDeserialize(Deserializer &d, CellKind kind);
#endif

  JSRegExp(Runtime *runtime, JSObject *parent, HiddenClass *clazz)
      : JSObject(runtime, &vt.base, parent, clazz) {}

  ~JSRegExp();

  /// Store a copy of the \p bytecode array.
  ExecutionStatus initializeBytecode(
      llvm::ArrayRef<uint8_t> bytecode,
      Runtime *runtime);

  uint8_t *bytecode_{};
  uint32_t bytecodeSize_{0};

  regex::SyntaxFlags syntaxFlags_ = {};

  // Finalizer to clean up stored native regex
  static void _finalizeImpl(GCCell *cell, GC *gc);
  static size_t _mallocSizeImpl(GCCell *cell);

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);
  static void _snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

  // Property storage slots.
  static constexpr inline SlotIndex patternPropIndex() {
    return numOverlapSlots<JSRegExp>() + ANONYMOUS_PROPERTY_SLOTS - 1;
  }

 public:
  // pattern
  static const PropStorage::size_type ANONYMOUS_PROPERTY_SLOTS =
      Super::ANONYMOUS_PROPERTY_SLOTS + 1;

  // lastIndex
  static const PropStorage::size_type NAMED_PROPERTY_SLOTS =
      Super::NAMED_PROPERTY_SLOTS + 1;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSREGEXP_H
