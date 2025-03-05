/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_NUMBERINGSYSTEM_H
#define HERMES_PLATFORMINTL_IMPLICU_NUMBERINGSYSTEM_H

#include <string>
#include <string_view>
#include <unordered_set>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

std::u16string getDefaultNumberingSystem(const std::string &localeICU);

const std::unordered_set<std::u16string> &getAvailableNumberingSystems();

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_NUMBERINGSYSTEM_H
