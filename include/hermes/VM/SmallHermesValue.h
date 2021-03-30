/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_SMALLHERMESVALUE_H
#define HERMES_VM_SMALLHERMESVALUE_H

#include "hermes/VM/GCDecl.h"
#include "hermes/VM/HermesValue.h"
#include "hermes/VM/SymbolID.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace hermes {
namespace vm {

class StringPrimitive;
class PointerBase;
class GCCell;
class Runtime;

/// An adaptor class that provides the API of a SmallHermesValue is internally
/// just a HermesValue.
class SmallHermesValueAdaptor : protected HermesValue {
  constexpr explicit SmallHermesValueAdaptor(HermesValue hv)
      : HermesValue(hv) {}

 public:
  SmallHermesValueAdaptor() = default;
  SmallHermesValueAdaptor(const SmallHermesValueAdaptor &) = default;

#ifdef _MSC_VER
  // This is a workaround to ensure is_trivial is true in MSVC.
  SmallHermesValueAdaptor(SmallHermesValueAdaptor &&) = default;
  SmallHermesValueAdaptor &operator=(const SmallHermesValueAdaptor &hv) =
      default;
#else
  SmallHermesValueAdaptor &operator=(const SmallHermesValueAdaptor &hv) =
      delete;
#endif

  using HermesValue::getBool;
  using HermesValue::getSymbol;
  using HermesValue::isBool;
  using HermesValue::isEmpty;
  using HermesValue::isNumber;
  using HermesValue::isObject;
  using HermesValue::isPointer;
  using HermesValue::isString;
  using HermesValue::isSymbol;
  using HermesValue::isUndefined;

  HermesValue toHV(PointerBase *) const {
    return *this;
  }
  HermesValue unboxToHV(PointerBase *) const {
    return *this;
  }

  GCCell *getPointer(PointerBase *) const {
    return static_cast<GCCell *>(HermesValue::getPointer());
  }
  GCCell *getObject(PointerBase *) const {
    return static_cast<GCCell *>(HermesValue::getObject());
  }
  StringPrimitive *getString(PointerBase *) const {
    return HermesValue::getString();
  }
  double getNumber(PointerBase *) const {
    return HermesValue::getNumber();
  }
  uint32_t getRelocationID() const {
    return reinterpret_cast<uintptr_t>(HermesValue::getPointer());
  }

  inline void setInGC(SmallHermesValueAdaptor hv, GC *gc);

  SmallHermesValueAdaptor updatePointer(GCCell *ptr, PointerBase *) const {
    return SmallHermesValueAdaptor{HermesValue::updatePointer(ptr)};
  }
  void unsafeUpdateRelocationID(uint32_t id) {
    HermesValue::unsafeUpdatePointer(
        reinterpret_cast<void *>(static_cast<uintptr_t>(id)));
  }
  void unsafeUpdatePointer(GCCell *ptr, PointerBase *) {
    HermesValue::unsafeUpdatePointer(ptr);
  }

  static constexpr SmallHermesValueAdaptor
  encodeHermesValue(HermesValue hv, GC *, PointerBase *) {
    return SmallHermesValueAdaptor{hv};
  }
  static constexpr SmallHermesValueAdaptor encodeHermesValue(
      HermesValue hv,
      Runtime *) {
    return SmallHermesValueAdaptor{hv};
  }
  static SmallHermesValueAdaptor
  encodeNumberValue(double d, GC *, PointerBase *) {
    return SmallHermesValueAdaptor{HermesValue::encodeNumberValue(d)};
  }
  static SmallHermesValueAdaptor encodeNumberValue(double d, Runtime *) {
    return SmallHermesValueAdaptor{HermesValue::encodeNumberValue(d)};
  }
  static SmallHermesValueAdaptor encodeObjectValue(GCCell *ptr, PointerBase *) {
    return SmallHermesValueAdaptor{HermesValue::encodeObjectValue(ptr)};
  }
  static SmallHermesValueAdaptor encodeStringValue(
      StringPrimitive *ptr,
      PointerBase *) {
    return SmallHermesValueAdaptor{HermesValue::encodeStringValue(ptr)};
  }
  static SmallHermesValueAdaptor encodeSymbolValue(SymbolID s) {
    return SmallHermesValueAdaptor{HermesValue::encodeSymbolValue(s)};
  }
  static constexpr SmallHermesValueAdaptor encodeBoolValue(bool b) {
    return SmallHermesValueAdaptor{HermesValue::encodeBoolValue(b)};
  }

  static constexpr SmallHermesValueAdaptor encodeNullValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeNullValue()};
  }
  static constexpr SmallHermesValueAdaptor encodeUndefinedValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeUndefinedValue()};
  }
  static constexpr SmallHermesValueAdaptor encodeEmptyValue() {
    return SmallHermesValueAdaptor{HermesValue::encodeEmptyValue()};
  }
};

using SmallHermesValue = SmallHermesValueAdaptor;

static_assert(
    std::is_trivial<SmallHermesValue>::value,
    "SmallHermesValue must be trivial");

using GCSmallHermesValue = GCHermesValueBase<SmallHermesValue>;

} // end namespace vm
} // end namespace hermes

#endif // HERMES_VM_HERMESVALUE_H
