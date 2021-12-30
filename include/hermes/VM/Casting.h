/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_VM_CASTING_H
#define HERMES_VM_CASTING_H

#include "hermes/VM/GCCell.h"
#include "hermes/VM/HermesValue.h"

#include "llvh/Support/Casting.h"

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
  return llvh::cast<ToType>(cell);
}

/// const version of vmcast.
template <class ToType>
const ToType *vmcast(const GCCell *cell) {
  return llvh::cast<ToType>(cell);
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
#ifndef NDEBUG
  return llvh::cast_or_null<ToType>(cell);
#else
  // In opt builds, avoid doing a branch for the null case.
  return static_cast<ToType *>(cell);
#endif
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
  return llvh::dyn_cast<ToType>(cell);
}

/// Const version of dyn_vmcast.
template <class ToType>
const ToType *dyn_vmcast(const GCCell *cell) {
  return llvh::dyn_cast<ToType>(cell);
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
  return llvh::dyn_cast_or_null<ToType>(cell);
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
  return val.isPointer() &&
      llvh::isa<ToType>(static_cast<GCCell *>(val.getPointer()));
}

/// Return true if the cell is a pointer to an instance of ToType.
template <class ToType>
bool vmisa(GCCell *cell) {
  return llvh::isa<ToType>(cell);
}

/// Const version of vmisa.
template <class ToType>
bool vmisa(const GCCell *cell) {
  return llvh::isa<ToType>(cell);
}

/// @}

} // namespace vm
} // namespace hermes

#endif // HERMES_VM_CASTING_H
