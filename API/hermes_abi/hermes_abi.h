/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_H
#define HERMES_ABI_HERMES_ABI_H

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>

#ifndef HERMES_EXPORT
#ifdef _MSC_VER
#define HERMES_EXPORT __declspec(dllexport)
#else // _MSC_VER
#define HERMES_EXPORT __attribute__((visibility("default")))
#endif // _MSC_VER
#endif // !defined(HERMES_EXPORT)

#ifdef __cplusplus
extern "C" {
#endif

struct HermesABIContext;

struct HermesABIVTable {
  /// Create a new instance of a Hermes Runtime, and return a pointer to its
  /// associated context. The context must be released with
  /// release_hermes_runtime when it is no longer needed.
  HermesABIContext *(*make_hermes_runtime)();
  /// Release the Hermes Runtime associated with the given context.
  void (*release_hermes_runtime)(HermesABIContext *);
};

HERMES_EXPORT const HermesABIVTable *get_hermes_abi_vtable();

#ifdef __cplusplus
}
#endif

#endif
