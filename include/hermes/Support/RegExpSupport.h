/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_REGEXPSUPPORT_H
#define HERMES_SUPPORT_REGEXPSUPPORT_H

#include "llvh/ADT/ArrayRef.h"
#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/SmallVector.h"

namespace hermes {
namespace regex {
using GroupName = llvh::SmallVector<char16_t, 5>;
// A map from identifier -> groupNum
// /(?<id_foo>a)(b)(?<id_bar>c)/
// would be represented as { id_foo -> 1, id_bar -> 3 }
using ParsedGroupNamesMapping =
    llvh::DenseMap<llvh::ArrayRef<char16_t>, uint32_t>;
} // namespace regex
} // namespace hermes

#endif // HERMES_SUPPORT_REGEXPSUPPORT_H
