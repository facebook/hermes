/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSREGEXPSTRINGITERATOR_H
#define HERMES_VM_JSREGEXPSTRINGITERATOR_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// RegExpStringIterator object.
/// See ES11 21.2.7.2 for Properties of RegExp String Iterator Instances.
class JSRegExpStringIterator : public JSObject {
  using Super = JSObject;

  friend void JSRegExpStringIteratorBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSRegExpStringIteratorKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSRegExpStringIteratorKind;
  }

  static PseudoHandle<JSRegExpStringIterator> create(
      Runtime &runtime,
      Handle<JSObject> R,
      Handle<StringPrimitive> S,
      bool global,
      bool fullUnicode);

  /// Iterate to the next element and return.
  static CallResult<HermesValue> nextElement(
      Handle<JSRegExpStringIterator> self,
      Runtime &runtime);

 public:
  JSRegExpStringIterator(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz,
      Handle<JSObject> iteratedRegExp,
      Handle<StringPrimitive> iteratedString,
      bool global,
      bool unicode)
      : JSObject(runtime, *parent, *clazz),
        iteratedRegExp_(runtime, *iteratedRegExp, runtime.getHeap()),
        iteratedString_(runtime, *iteratedString, runtime.getHeap()),
        global_(global),
        unicode_(unicode) {}

 private:
  /// [[IteratingRegExp]]
  // Note: despite what the name is suggesting, it's actually unsafe to use
  // JSRegExp here. See regExpConstructorInternal for more details.
  GCPointer<JSObject> iteratedRegExp_;

  /// [[IteratedString]]
  GCPointer<StringPrimitive> iteratedString_;

  /// [[Global]]
  bool global_;

  /// [[Unicode]]
  bool unicode_;

  /// [[Done]]
  bool done_{false};
};

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_JSREGEXPSTRINGITERATOR_H
