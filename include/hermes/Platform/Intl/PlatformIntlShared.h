/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_PLATFORMINTLSHARED_H
#define HERMES_PLATFORMINTL_PLATFORMINTLSHARED_H

#ifdef HERMES_ENABLE_INTL
#include "hermes/Platform/Intl/PlatformIntl.h"

namespace hermes {
namespace platform_intl {

/// https://402.ecma-international.org/8.0/#sec-todatetimeoptions
vm::CallResult<Options> toDateTimeOptions(
    vm::Runtime &runtime,
    Options options,
    std::u16string_view required,
    std::u16string_view defaults);

/// https://402.ecma-international.org/8.0/#sec-case-sensitivity-and-case-mapping
std::u16string toASCIIUppercase(std::u16string_view tz);

} // namespace platform_intl
} // namespace hermes

#endif

#endif
