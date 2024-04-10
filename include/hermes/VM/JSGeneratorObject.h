/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSGENERATOROBJECT_H
#define HERMES_VM_JSGENERATOROBJECT_H

#include "hermes/VM/Callable.h"
#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// ES6.0 25.3 Generator Objects.
/// Stores the inner function associated with the generator.
/// The inner function is stored separately from the JSGeneratorObject
/// due to the fact that it needs to store the same information as a standard
/// JSFunction, but must not be directly accessible by user code.
class JSGeneratorObject final : public JSObject {
  using Super = JSObject;
  friend void JSGeneratorObjectBuildMeta(
      const GCCell *cell,
      Metadata::Builder &mb);

  const static ObjectVTable vt;

 public:
  static constexpr CellKind getCellKind() {
    return CellKind::JSGeneratorObjectKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSGeneratorObjectKind;
  }

  static CallResult<PseudoHandle<JSGeneratorObject>> create(
      Runtime &runtime,
      Handle<Callable> innerFunction,
      Handle<JSObject> parentHandle);

  /// \return the inner function.
  static PseudoHandle<Callable> getInnerFunction(
      Runtime &runtime,
      JSGeneratorObject *self) {
    return createPseudoHandle(self->innerFunction_.get(runtime));
  }

 public:
  JSGeneratorObject(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz) {}

 private:
  /// The inner function that is called when this generator is advanced.
  GCPointer<Callable> innerFunction_;
};

} // namespace vm
} // namespace hermes

#endif
