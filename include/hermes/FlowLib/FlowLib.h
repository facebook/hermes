/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_FLOWLIB_FLOWLIB_H
#define HERMES_FLOWLIB_FLOWLIB_H

#include "llvh/ADT/StringRef.h"

namespace hermes {

/// Get the FlowLib source code for parsing as a prelude in typed mode.
llvh::StringRef getFlowLibSource();

} // namespace hermes

#endif // HERMES_FLOWLIB_FLOWLIB_H
