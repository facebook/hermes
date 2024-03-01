/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_PASSES_OPTPARENTENVIRONMENT_H
#define HERMES_BCGEN_HBC_PASSES_OPTPARENTENVIRONMENT_H

namespace hermes {
class Pass;
namespace hbc {

Pass *createOptParentEnvironment();

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_PASSES_OPTPARENTENVIRONMENT_H
