/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_VM_SINGLEOBJECT_H
#define HERMES_VM_SINGLEOBJECT_H

#include "hermes/VM/JSObject.h"

namespace hermes {
namespace vm {

/// JavaScript single object, include Math and JSON.
template <CellKind kind>
class SingleObject final : public JSObject {
 public:
  using Super = JSObject;
  static const ObjectVTable vt;
  static bool classof(const GCCell *cell) {
    return cell->getKind() == kind;
  }

  /// Create a SingleObject.
  static CallResult<HermesValue> create(
      Runtime *runtime,
      Handle<JSObject> protoHandle) {
    auto propStorage =
        JSObject::createPropStorage(runtime, NEEDED_PROPERTY_SLOTS);
    if (LLVM_UNLIKELY(propStorage == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    void *mem = runtime->alloc(sizeof(SingleObject));
    return HermesValue::encodeObjectValue(new (mem) SingleObject(
        runtime,
        *protoHandle,
        runtime->getHiddenClassForPrototypeRaw(*protoHandle),
        **propStorage));
  }

 protected:
  SingleObject(
      Runtime *runtime,
      JSObject *proto,
      HiddenClass *clazz,
      JSObjectPropStorage *propStorage)
      : JSObject(runtime, &vt.base, proto, clazz, propStorage) {}
};

template <CellKind kind>
struct IsGCObject<SingleObject<kind>> {
  static constexpr bool value = true;
};

template <CellKind kind>
const ObjectVTable SingleObject<kind>::vt = {
    VTable(kind, sizeof(SingleObject<kind>), nullptr, nullptr),
    SingleObject::_getOwnIndexedRangeImpl,
    SingleObject::_haveOwnIndexedImpl,
    SingleObject::_getOwnIndexedPropertyFlagsImpl,
    SingleObject::_getOwnIndexedImpl,
    SingleObject::_setOwnIndexedImpl,
    SingleObject::_deleteOwnIndexedImpl,
    SingleObject::_checkAllOwnIndexedImpl,
};

using JSMath = SingleObject<CellKind::MathKind>;
using JSJSON = SingleObject<CellKind::JSONKind>;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SINGLEOBJECT_H
