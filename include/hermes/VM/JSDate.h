/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_JSDATE_H
#define HERMES_VM_JSDATE_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// Date object.
class JSDate final : public JSObject {
  using Super = JSObject;
  friend void JSDateBuildMeta(const GCCell *, Metadata::Builder &);

 public:
  static const ObjectVTable vt;

  static constexpr CellKind getCellKind() {
    return CellKind::JSDateKind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::JSDateKind;
  }

  static PseudoHandle<JSDate>
  create(Runtime &runtime, double value, Handle<JSObject> prototype);

  static PseudoHandle<JSDate> create(
      Runtime &runtime,
      Handle<JSObject> prototype) {
    return create(runtime, std::numeric_limits<double>::quiet_NaN(), prototype);
  }

  /// \return the [[PrimitiveValue]] internal property.
  double getPrimitiveValue() {
    return primitiveValue_;
  }

  /// Set the [[PrimitiveValue]] internal property.
  void setPrimitiveValue(double value) {
    primitiveValue_ = value;
  }

  JSDate(
      Runtime &runtime,
      double value,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz), primitiveValue_{value} {}

 private:
  double primitiveValue_;
};

} // namespace vm
} // namespace hermes

#endif
