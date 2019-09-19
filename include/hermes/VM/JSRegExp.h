/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_JSREGEXP_H
#define HERMES_VM_JSREGEXP_H

#include "hermes/VM/CopyableVector.h"
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

  // Struct representing flags which may be used when constructing the RegExp
  struct FlagBits {
    uint8_t ignoreCase : 1;
    uint8_t multiline : 1;
    uint8_t global : 1;

    /// \return a string representing the flags
    /// The characters are returned in the order given in ES 5.1 15.10.6.4
    /// (specifically global, ignoreCase, multiline)
    /// Note this may differ in order from the string passed in construction
    llvm::SmallString<3> toString() const {
      llvm::SmallString<3> result;
      if (global)
        result.push_back('g');
      if (ignoreCase)
        result.push_back('i');
      if (multiline)
        result.push_back('m');
      return result;
    }

    /// Given a flags string \p str, generate the corresponding FlagBits
    /// \return the flags if the string is valid, an empty optional otherwise
    /// See ES 5.1 15.10.4.1 for description of the validation
    static OptValue<FlagBits> fromString(StringView str);
  };

  /// Create a JSRegExp, with the empty string for pattern and flags
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototype);

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
  static FlagBits getFlagBits(JSRegExp *self) {
    return self->flagBits_;
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

  CopyableVector<uint8_t> bytecode_;

  FlagBits flagBits_ = {};

  // Finalizer to clean up stored native regex
  static void _finalizeImpl(GCCell *cell, GC *gc);
  static size_t _mallocSizeImpl(GCCell *cell);

  static std::string _snapshotNameImpl(GCCell *cell, GC *gc);
  static void _snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);
  static void _snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap);

  // Property storage slots.
  enum RegExpSlotIndexes { pattern, lastIndex, COUNT };
  static constexpr SlotIndex sourceValueIndex = 0;
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + RegExpSlotIndexes::COUNT;
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSREGEXP_H
