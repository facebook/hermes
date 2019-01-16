#ifndef HERMES_VM_JSDATE_H
#define HERMES_VM_JSDATE_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// Date object.
class JSDate final : public JSObject {
  using Super = JSObject;

 public:
  static ObjectVTable vt;

  /// Number of property slots the class reserves for itself. Child classes
  /// should override this value by adding to it and defining a constant with
  /// the same name.
  static const PropStorage::size_type NEEDED_PROPERTY_SLOTS =
      Super::NEEDED_PROPERTY_SLOTS + 1;

  static bool classof(const GCCell *cell) {
    return cell->getKind() == CellKind::DateKind;
  }

  static CallResult<HermesValue>
  create(Runtime *runtime, double value, Handle<JSObject> prototype);

  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> prototype) {
    return create(runtime, std::numeric_limits<double>::quiet_NaN(), prototype);
  }

  /// \return the [[PrimitiveValue]] internal property.
  static HermesValue getPrimitiveValue(JSObject *self) {
    return JSObject::getInternalProperty(self, JSDate::primitiveValueIndex);
  }

  /// Set the [[PrimitiveValue]] internal property.
  static void
  setPrimitiveValue(JSObject *self, Runtime *runtime, HermesValue value) {
    return JSObject::setInternalProperty(
        self, runtime, JSDate::primitiveValueIndex, value);
  }

 protected:
  JSDate(
      Runtime *runtime,
      JSObject *proto,
      HiddenClass *clazz,
      JSObjectPropStorage *propStorage)
      : JSObject(runtime, &vt.base, proto, clazz, propStorage) {}

 protected:
  static const SlotIndex primitiveValueIndex = 0;
};

} // namespace vm
} // namespace hermes

#endif
