/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_HELPERS_H
#define HERMES_ABI_HERMES_ABI_HELPERS_H

#include "hermes_abi/hermes_abi.h"

namespace facebook::hermes::abi {

#define DECLARE_HERMES_ABI_POINTER_HELPERS(name)                             \
  static inline HermesABI##name create##name(HermesABIManagedPointer *ptr) { \
    return {ptr};                                                            \
  }                                                                          \
  static inline HermesABI##name##OrError create##name##OrError(              \
      HermesABIManagedPointer *ptr) {                                        \
    return {(uintptr_t)ptr};                                                 \
  }                                                                          \
  static inline HermesABI##name##OrError create##name##OrError(              \
      HermesABIErrorCode err) {                                              \
    return {static_cast<uintptr_t>((err << 2) | 1)};                         \
  }
HERMES_ABI_POINTER_TYPES(DECLARE_HERMES_ABI_POINTER_HELPERS)
#undef DECLARE_HERMES_ABI_POINTER_HELPERS

#define DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_HELPERS(name, type)            \
  static inline HermesABI##name##OrError create##name##OrError(type val) { \
    HermesABI##name##OrError res;                                          \
    res.is_error = false;                                                  \
    res.data.val = val;                                                    \
    return res;                                                            \
  }                                                                        \
  static inline HermesABI##name##OrError create##name##OrError(            \
      HermesABIErrorCode err) {                                            \
    HermesABI##name##OrError res;                                          \
    res.is_error = true;                                                   \
    res.data.error = err;                                                  \
    return res;                                                            \
  }
HERMES_ABI_TRIVIAL_OR_ERROR_TYPES(DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_HELPERS)
#undef DECLARE_HERMES_ABI_TRIVIAL_OR_ERROR_HELPERS

static inline HermesABIVoidOrError createVoidOrError(void) {
  HermesABIVoidOrError res;
  res.is_error = false;
  return res;
}
static inline HermesABIVoidOrError createVoidOrError(HermesABIErrorCode err) {
  HermesABIVoidOrError res;
  res.is_error = true;
  res.error = err;
  return res;
}

static inline HermesABIValue createUndefinedValue() {
  HermesABIValue val;
  val.kind = HermesABIValueKindUndefined;
  return val;
}
static inline HermesABIValue createNullValue() {
  HermesABIValue val;
  val.kind = HermesABIValueKindNull;
  return val;
}
static inline HermesABIValue createBoolValue(bool b) {
  HermesABIValue val;
  val.kind = HermesABIValueKindBoolean;
  val.data.boolean = b;
  return val;
}
static inline HermesABIValue createErrorValue(HermesABIErrorCode err) {
  HermesABIValue val;
  val.kind = HermesABIValueKindError;
  val.data.error = err;
  return val;
}
static inline HermesABIValue createNumberValue(double d) {
  HermesABIValue val;
  val.kind = HermesABIValueKindNumber;
  val.data.number = d;
  return val;
}
static inline HermesABIValue createObjectValue(HermesABIManagedPointer *ptr) {
  HermesABIValue val;
  val.kind = HermesABIValueKindObject;
  val.data.pointer = ptr;
  return val;
}
static inline HermesABIValue createObjectValue(const HermesABIObject &obj) {
  return createObjectValue(obj.pointer);
}
static inline HermesABIValue createStringValue(HermesABIManagedPointer *ptr) {
  HermesABIValue val;
  val.kind = HermesABIValueKindString;
  val.data.pointer = ptr;
  return val;
}
static inline HermesABIValue createStringValue(const HermesABIString &str) {
  return createStringValue(str.pointer);
}
static inline HermesABIValue createBigIntValue(HermesABIManagedPointer *ptr) {
  HermesABIValue val;
  val.kind = HermesABIValueKindBigInt;
  val.data.pointer = ptr;
  return val;
}
static inline HermesABIValue createBigIntValue(const HermesABIBigInt &bi) {
  return createBigIntValue(bi.pointer);
}
static inline HermesABIValue createSymbolValue(HermesABIManagedPointer *ptr) {
  HermesABIValue val;
  val.kind = HermesABIValueKindSymbol;
  val.data.pointer = ptr;
  return val;
}
static inline HermesABIValue createSymbolValue(const HermesABISymbol &sym) {
  return createSymbolValue(sym.pointer);
}

static inline HermesABIValueKind getValueKind(const HermesABIValue &val) {
  return val.kind;
}

static inline bool isUndefinedValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindUndefined;
}
static inline bool isNullValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindNull;
}
static inline bool isBoolValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindBoolean;
}
static inline bool isErrorValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindError;
}
static inline bool isNumberValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindNumber;
}
static inline bool isObjectValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindObject;
}
static inline bool isStringValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindString;
}
static inline bool isBigIntValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindBigInt;
}
static inline bool isSymbolValue(const HermesABIValue &val) {
  return getValueKind(val) == HermesABIValueKindSymbol;
}

static inline bool getBoolValue(const HermesABIValue &val) {
  assert(isBoolValue(val));
  return val.data.boolean;
}
static inline HermesABIErrorCode getErrorValue(const HermesABIValue &val) {
  assert(isErrorValue(val));
  return val.data.error;
}
static inline double getNumberValue(const HermesABIValue &val) {
  assert(isNumberValue(val));
  return val.data.number;
}
static inline HermesABIObject getObjectValue(const HermesABIValue &val) {
  assert(isObjectValue(val));
  return createObject(val.data.pointer);
}
static inline HermesABIString getStringValue(const HermesABIValue &val) {
  assert(isStringValue(val));
  return createString(val.data.pointer);
}
static inline HermesABIBigInt getBigIntValue(const HermesABIValue &val) {
  assert(isBigIntValue(val));
  return createBigInt(val.data.pointer);
}
static inline HermesABISymbol getSymbolValue(const HermesABIValue &val) {
  assert(isSymbolValue(val));
  return createSymbol(val.data.pointer);
}
static inline HermesABIManagedPointer *getPointerValue(
    const HermesABIValue &val) {
  assert(getValueKind(val) | HERMES_ABI_POINTER_MASK);
  return val.data.pointer;
}

static inline void releaseValue(const HermesABIValue &val) {
  if (getValueKind(val) & HERMES_ABI_POINTER_MASK)
    getPointerValue(val)->vtable->invalidate(val.data.pointer);
}

} // namespace facebook::hermes::abi

#endif // HERMES_ABI_HERMES_ABI_HELPERS_H
