/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_OPTIONHELPERS_H
#define HERMES_PLATFORMINTL_IMPLICU_OPTIONHELPERS_H

#include "hermes/Platform/Intl/PlatformIntl.h"
#include "llvh/ADT/ArrayRef.h"

#include <optional>
#include <string_view>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

/**
 * Returns string value of specified property from given
 * option map if found and valid. Returns given default
 * optional if not found.
 * https://tc39.es/ecma402/#sec-getoption
 * @param runtime runtime object
 * @param options option map containing all input options
 * @param property name of the option to look up
 * @param validValues valid values for this option
 * @param defaultValue default value
 * @return CallResult with a string value in optional
 * if option is found and valid, with given default
 * optional if option is not found, and with
 * ExecutionStatus.EXCEPTION otherwise.
 */
vm::CallResult<std::optional<std::u16string>> getStringOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    llvh::ArrayRef<const char16_t *> validValues,
    std::optional<std::u16string_view> defaultValue);

/**
 * Returns bool value of specified property from given
 * option map if found. Returns given default optional
 * if not found.
 * @param options option map containing all input options
 * @param property name of the option to look up
 * @param defaultValue default value
 * @return Optional with a bool value if option is found,
 * given default optional otherwise.
 */
std::optional<bool> getBoolOption(
    const Options &options,
    const std::u16string &property,
    std::optional<bool> defaultValue);

/**
 * Returns number value of specified property from given
 * option map if found and valid. Floor of the number is done.
 * Returns given default optional if not found.
 * See https://tc39.es/ecma402/#sec-defaultnumberoption.
 * @param runtime runtime object
 * @param options option map containing all input options
 * @param property name of the option to look up
 * @param minimum minimum valid number
 * @param maximum maximum valid number
 * @param defaultValue default value
 * @return CallResult with a number value in optional
 * if option is found and valid, with given default
 * optional if option is not found, and with
 * ExecutionStatus.EXCEPTION otherwise.
 */
vm::CallResult<std::optional<double>> getNumberOption(
    vm::Runtime &runtime,
    const Options &options,
    const std::u16string &property,
    double minimum,
    double maximum,
    std::optional<double> defaultValue);

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_OPTIONHELPERS_H
