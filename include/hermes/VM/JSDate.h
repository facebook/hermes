/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
  friend void DateBuildMeta(const GCCell *, Metadata::Builder &);

 public:
  static const ObjectVTable vt;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::DateKind;
  }

  static PseudoHandle<JSDate>
  create(Runtime *runtime, double value, Handle<JSObject> prototype);

  static PseudoHandle<JSDate> create(
      Runtime *runtime,
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

#ifdef HERMESVM_SERIALIZE
  explicit JSDate(Deserializer &d);

  friend void DateSerialize(Serializer &s, const GCCell *cell);
  friend void DateDeserialize(Deserializer &d, CellKind kind);
#endif

  JSDate(
      Runtime *runtime,
      double value,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, &vt.base, *parent, *clazz), primitiveValue_{value} {}

 private:
  double primitiveValue_;
};

} // namespace vm
} // namespace hermes

#endif
