//==-- llvm/Support/CheckedArithmetic.h - Safe arithmetical operations *- C++ //
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains generic functions for operating on integers which
// give the indication on whether the operation has overflown.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_CHECKEDARITHMETIC_H
#define LLVM_SUPPORT_CHECKEDARITHMETIC_H

#include "llvh/ADT/APInt.h"
#include "llvh/ADT/Optional.h"

#include <type_traits>

namespace {

/// Utility function to apply a given method of \c APInt \p F to \p LHS and
/// \p RHS.
/// \return Empty optional if the operation overflows, or result otherwise.
template <typename T, typename F>
typename std::enable_if<std::is_integral<T>::value && sizeof(T) * 8 <= 64,
                        llvh::Optional<T>>::type
checkedOp(T LHS, T RHS, F Op, bool Signed = true) {
  llvh::APInt ALHS(/*BitSize=*/sizeof(T) * 8, LHS, Signed);
  llvh::APInt ARHS(/*BitSize=*/sizeof(T) * 8, RHS, Signed);
  bool Overflow;
  llvh::APInt Out = (ALHS.*Op)(ARHS, Overflow);
  if (Overflow)
    return llvh::None;
  return Signed ? Out.getSExtValue() : Out.getZExtValue();
}
}

namespace llvh {

/// Add two signed integers \p LHS and \p RHS.
/// \return Optional of sum if no signed overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_signed<T>::value, llvh::Optional<T>>::type
checkedAdd(T LHS, T RHS) {
  return checkedOp(LHS, RHS, &llvh::APInt::sadd_ov);
}

/// Multiply two signed integers \p LHS and \p RHS.
/// \return Optional of product if no signed overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_signed<T>::value, llvh::Optional<T>>::type
checkedMul(T LHS, T RHS) {
  return checkedOp(LHS, RHS, &llvh::APInt::smul_ov);
}

/// Multiply A and B, and add C to the resulting product.
/// \return Optional of result if no signed overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_signed<T>::value, llvh::Optional<T>>::type
checkedMulAdd(T A, T B, T C) {
  if (auto Product = checkedMul(A, B))
    return checkedAdd(*Product, C);
  return llvh::None;
}

/// Add two unsigned integers \p LHS and \p RHS.
/// \return Optional of sum if no unsigned overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, llvh::Optional<T>>::type
checkedAddUnsigned(T LHS, T RHS) {
  return checkedOp(LHS, RHS, &llvh::APInt::uadd_ov, /*Signed=*/false);
}

/// Multiply two unsigned integers \p LHS and \p RHS.
/// \return Optional of product if no unsigned overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, llvh::Optional<T>>::type
checkedMulUnsigned(T LHS, T RHS) {
  return checkedOp(LHS, RHS, &llvh::APInt::umul_ov, /*Signed=*/false);
}

/// Multiply unsigned integers A and B, and add C to the resulting product.
/// \return Optional of result if no unsigned overflow occurred,
/// \c None otherwise.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, llvh::Optional<T>>::type
checkedMulAddUnsigned(T A, T B, T C) {
  if (auto Product = checkedMulUnsigned(A, B))
    return checkedAddUnsigned(*Product, C);
  return llvh::None;
}

} // End llvm namespace

#endif
