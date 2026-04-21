/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "llvh/ADT/StringRef.h"

namespace hermes {

/// Get the TypedLib source code for parsing as a prelude in typed mode.
llvh::StringRef getTypedLibSource();

} // namespace hermes
