/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_ABI_HERMES_ABI_H
#define HERMES_ABI_HERMES_ABI_H

struct HermesABIRuntimeConfig;
struct HermesABIRuntime;

struct HermesABIRuntimeVTable {
  /// Release the given runtime.
  void (*release)(struct HermesABIRuntime *);
};

/// An instance of a Hermes Runtime.
struct HermesABIRuntime {
  const struct HermesABIRuntimeVTable *vt;
};

struct HermesABIVTable {
  /// Create a new instance of a Hermes Runtime, and return a pointer to it. The
  /// runtime must be explicitly released when it is no longer needed.
  struct HermesABIRuntime *(*make_hermes_runtime)(
      const struct HermesABIRuntimeConfig *config);
};

#endif
