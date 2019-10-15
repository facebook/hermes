/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CASTING_H
#define HERMES_VM_CASTING_H

#include "hermes/VM/GC.h"
#include "hermes/VM/HermesValue.h"

#include "llvm/Support/Casting.h"

#include <cassert>

namespace hermes {
namespace vm {

/// @name Casting
/// Simple implementation of safe static and dynamic casting based on
/// checking vtables and LLVM casts.
/// @{

/// Assert that the cell, which must be non-null, is of the specified type
/// and return the type-casted pointer.
template <class ToType>
ToType *vmcast(GCCell *cell) {
  return llvm::cast<ToType>(cell);
}

/// const version of vmcast.
template <class ToType>
const ToType *vmcast(const GCCell *cell) {
  return llvm::cast<ToType>(cell);
}

/// Assert that the HermesValue, is a non-null pointer of the specified type
/// and return the type-casted pointer.
template <class ToType>
ToType *vmcast(HermesValue val) {
  assert(val.isPointer() && "vmcast with non-pointer");
  return vmcast<ToType>(static_cast<GCCell *>(val.getPointer()));
}

/// Identical to vmcast() except a null pointer is allowed.
template <class ToType>
ToType *vmcast_or_null(GCCell *cell) {
  return llvm::cast_or_null<ToType>(cell);
}

/// Identical to vmcast() except a null pointer is allowed.
template <class ToType>
ToType *vmcast_or_null(HermesValue val) {
  assert(val.isPointer() && "vmcast with non-pointer");
  return vmcast_or_null<ToType>(static_cast<GCCell *>(val.getPointer()));
}

/// If the argument, which must be non-null, is of the specified type, cast it
/// to that type, otherwise return null.
template <class ToType>
ToType *dyn_vmcast(GCCell *cell) {
  return llvm::dyn_cast<ToType>(cell);
}

/// Const version of dyn_vmcast.
template <class ToType>
const ToType *dyn_vmcast(const GCCell *cell) {
  return llvm::dyn_cast<ToType>(cell);
}

/// If the argument, which must be a non-null pointer, is of the specified type,
/// cast it to that type, otherwise return null.
template <class ToType>
ToType *dyn_vmcast(HermesValue val) {
  return val.isPointer()
      ? dyn_vmcast<ToType>(static_cast<GCCell *>(val.getPointer()))
      : nullptr;
}

/// Identical to dyn_vmcast() except a null pointer is allowed.
template <class ToType>
ToType *dyn_vmcast_or_null(GCCell *cell) {
  return llvm::dyn_cast_or_null<ToType>(cell);
}

/// Identical to dyn_vmcast() except a null pointer is allowed.
template <class ToType>
ToType *dyn_vmcast_or_null(HermesValue val) {
  return val.isPointer()
      ? dyn_vmcast_or_null<ToType>(static_cast<GCCell *>(val.getPointer()))
      : nullptr;
}

/// Return true if the value is an instance of ToType.
template <class ToType>
bool vmisa(HermesValue val) {
  return dyn_vmcast<ToType>(val) != nullptr;
}

/// Return true if the cell is a pointer to an instance of ToType.
template <class ToType>
bool vmisa(GCCell *cell) {
  return dyn_vmcast<ToType>(cell) != nullptr;
}

/// Return true if the value, which could be nullptr, is an instance of ToType.
template <class ToType>
bool vmisa_or_null(HermesValue val) {
  return dyn_vmcast_or_null<ToType>(val) != nullptr;
}

/// Return true if the cell, which could be nullptr, is a pointer to an instance
/// of ToType.
template <class ToType>
bool vmisa_or_null(GCCell *cell) {
  return dyn_vmcast_or_null<ToType>(cell) != nullptr;
}
/// @}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CASTING_H
