/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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

  static constexpr CellKind getCellKind() {
    return kind;
  }
  static bool classof(const GCCell *cell) {
    return cell->getKind() == kind;
  }

  /// Create a SingleObject.
  static CallResult<HermesValue> create(Runtime &runtime) {
    Handle<HiddenClass> clazz = runtime.classJSMath;
    if constexpr (kind == CellKind::JSMathKind) {
      clazz = runtime.classJSMath;
    } else {
      static_assert(kind == CellKind::JSJSONKind, "Unknown SingleObject kind");
      clazz = runtime.classJSJSON;
    }

    auto *cell = runtime.makeAFixed<SingleObject>(
        runtime, runtime.objectPrototype, clazz);
    return JSObjectInit::initToHermesValue(runtime, cell);
  }

  SingleObject(
      Runtime &runtime,
      Handle<JSObject> parent,
      Handle<HiddenClass> clazz)
      : JSObject(runtime, *parent, *clazz) {}
};

template <CellKind kind>
struct IsGCObject<SingleObject<kind>> {
  static constexpr bool value = true;
};

template <CellKind kind>
const ObjectVTable SingleObject<kind>::vt = {
    VTable(kind, cellSize<SingleObject<kind>>()),
    SingleObject::_getOwnIndexedRangeImpl,
    SingleObject::_haveOwnIndexedImpl,
    SingleObject::_getOwnIndexedPropertyFlagsImpl,
    SingleObject::_getOwnIndexedImpl,
    SingleObject::_setOwnIndexedImpl,
    SingleObject::_deleteOwnIndexedImpl,
    SingleObject::_checkAllOwnIndexedImpl,
};

using JSMath = SingleObject<CellKind::JSMathKind>;
using JSJSON = SingleObject<CellKind::JSJSONKind>;

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_SINGLEOBJECT_H
