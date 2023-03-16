/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_FRONTENDDEFS_NATIVEERRORTYPES_H
#define HERMES_FRONTENDDEFS_NATIVEERRORTYPES_H

namespace hermes {

enum class NativeErrorTypes : unsigned char {
#define ALL_ERROR_TYPE(name) name,
#include "NativeErrorTypes.def"
};

} // namespace hermes

#endif // HERMES_FRONTENDDEFS_NATIVEERRORTYPES_H
