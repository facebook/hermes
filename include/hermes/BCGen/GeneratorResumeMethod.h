/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_GENERATORRESUMEMETHOD_H
#define HERMES_BCGEN_GENERATORRESUMEMETHOD_H

namespace hermes {

/// Represent the method performed when resuming a generator:
/// .next(), .throw(), or .return().
enum class Action {
  Next,
  Throw,
  Return,
};

} // namespace hermes

#endif // HERMES_BCGEN_GENERATORRESUMEMETHOD_H
