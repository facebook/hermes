/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H
#define HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H

#include "LocaleBCP47Object.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "llvh/ADT/ArrayRef.h"

#include <string>
#include <string_view>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

struct ResolvedResult {
  Options resolvedOpts;
  LocaleBCP47Object localeBcp47Object;
};

/**
 * Given a BCP47 locale priority list and options, returns the best
 * available locale and the supported options.
 * @param requestedLocales priority list of requested locales
 * @param opt requested options
 * @param relevantExtensionKeys extension keys to consider
 * @param isExtensionTypeSupported function to determine whether given
 * extension and its type are supported for given locale
 * @return resolved BCP 47 locale object.
 */
ResolvedResult resolveLocale(
    const std::vector<LocaleBCP47Object> &requestedLocales,
    const Options &opt,
    llvh::ArrayRef<std::u16string_view> relevantExtensionKeys,
    const std::function<bool(
        std::u16string_view, /* extension key */
        std::u16string_view, /* extension type */
        const LocaleBCP47Object &)> &isExtensionTypeSupported);

/**
 * @brief Search provided locales to find all the supported ones
 * https://tc39.github.io/ecma402/#sec-supportedlocales
 * @param runtime runtime object
 * @param locales locales passed from JS
 * @param options options passed from JS
 * @return CallResult with a vector of provided locales that are supported on
 * success, with ExecutionStatus.EXCEPTION on failure.
 */
vm::CallResult<std::vector<std::u16string>> supportedLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept;

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H
