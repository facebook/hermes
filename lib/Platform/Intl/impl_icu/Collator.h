/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_COLLATOR_H
#define HERMES_PLATFORMINTL_IMPLICU_COLLATOR_H

#include "LocaleBCP47Object.h"
#include "hermes/Platform/Intl/PlatformIntl.h"
#include "hermes/Platform/Unicode/icu.h"

#include <string_view>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

class Collator : public platform_intl::Collator {
 public:
  /**
   * @brief Creates a Collator.
   */
  Collator() = default;

  /**
   * @brief Destructs the Collator.
   */
  ~Collator() override;

  /**
   * Initializes the Collator.
   *
   * See https://tc39.es/ecma402/#sec-initializecollator.
   *
   * @param runtime runtime object
   * @param locales locales pass from JS
   * @param options options pass from JS
   * @return ExecutionStatus.RETURNED on success, ExecutionStatus.EXCEPTION
   * on failure.
   */
  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  /**
   * Returns the resolved options.
   *
   * See https://tc39.es/ecma402/#sec-intl.collator.prototype.resolvedoptions.
   *
   * @return a new options object with properties reflecting the locale and
   * collation options computed during initialization of collator.
   */
  Options resolvedOptions() noexcept;

  /**
   * Compares two strings according to the sort order.
   *
   * See https://tc39.es/ecma402/#sec-collator-comparestrings.
   *
   * @param x first string for comparasion
   * @param y second string for comparasion
   * @return a negative value if `x` comes before `y`;
   *         a positive value if `x` comes after `y`;
   *         0 if they are considered equal.
   */
  double compare(const std::u16string &x, const std::u16string &y) noexcept;

  /**
   * Returns provided locales that Collator supports.
   *
   * See https://tc39.es/ecma402/#sec-intl.collator.supportedlocalesof.
   *
   * @param runtime runtime object
   * @param locales locales pass from JS
   * @param options options pass from JS
   * @return CallResult with a vector of provided locales that are supported on
   * success, with ExecutionStatus.EXCEPTION on failure.
   */
  static vm::CallResult<std::vector<std::u16string>> supportedLocalesOf(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

 private:
  /**
   * Closes and disposes ICU collator member from memory.
   */
  void close();

  // https://tc39.es/ecma402/#sec-initializecollator
  vm::ExecutionStatus initializeCollator(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  /**
   * Sets collator attributes for numeric, caseFirst, sensitivity and
   * ignorePunctuation
   */
  void setAttributes();

  static bool isExtensionTypeSupported(
      std::u16string_view extensionKey,
      std::u16string_view extensionType,
      const LocaleBCP47Object &localeBCP47Object);

  std::u16string resolvedLocale_;
  std::u16string resolvedInternalLocale_;
  std::u16string resolvedUsageValue_;
  std::u16string resolvedSensitivityValue_;
  bool resolvedIgnorePunctuationValue_ = false;
  std::u16string resolvedCollationValue_;
  bool resolvedNumericValue_ = false;
  std::u16string resolvedCaseFirstValue_;
  UCollator *coll_ = nullptr;
};

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_COLLATOR_H
