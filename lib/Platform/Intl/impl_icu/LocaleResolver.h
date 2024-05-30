/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H
#define HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H

#include "../LocaleBCP47Object.h"
#include "hermes/Platform/Intl/PlatformIntl.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

class LocaleResolver {
 public:
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
  static LocaleResolver::ResolvedResult resolveLocale(
      const std::vector<LocaleBCP47Object> &requestedLocales,
      const Options &opt,
      const std::unordered_set<std::u16string> &relevantExtensionKeys,
      const std::function<bool(
          const std::u16string &, /* extension key */
          const std::u16string &, /* extension type */
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
  static vm::CallResult<std::vector<std::u16string>> supportedLocales(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

 private:
  struct AvailableLocale {
    std::vector<const char *> icu;
    std::unordered_set<std::string> bcp47Lowercase;
  };

  /**
   * Returns all available locales from ICU.
   * @return all available locales in BCP47 and ICU format.
   */
  static AvailableLocale getAvailableLocales();

  /**
   * Returns a BCP47 locale object for the default locale.
   * @return a BCP47 locale object for the default locale on success,
   * without an associated locale on failure.
   */
  static LocaleBCP47Object getDefaultLocale();

  /**
   * Returns a BCP47 locale object for the given locale and unicode
   * extensions
   * @param locale a BCP47 locale identifier
   * @param extensions unicode extensions
   * @return a BCP47 locale object for the given locale and unicode
   * extensions on success, without an associated locale on failure.
   */
  static LocaleBCP47Object toLocaleBCP47ObjectWithExtensions(
      const std::string &locale,
      const std::unordered_map<std::u16string, std::u16string> &extensions);

  /**
   * Returns the "lookup" available locale for the given requested locale.
   * https://tc39.es/ecma402/#sec-bestavailablelocale
   * @param locale a structurally valid and canonicalized BCP47 locale
   * identifier in lower case
   * @return the longest non-empty prefix of locale that is an element of
   * availableLocales, or empty string if there is no such element.
   */
  static std::string bestAvailableLocale(const std::string &locale);

  /**
   * Returns the "lookup" available locale given the priority list of
   * requested locales.
   * https://tc39.es/ecma402/#sec-lookupmatcher
   * @param requestedLocales requested locales to compare
   * @return the best matched locale or default locale if there is not a good
   * match.
   */
  static LocaleBCP47Object lookupMatcher(
      const std::vector<LocaleBCP47Object> &requestedLocales);

  /**
   * Returns the "best fit" available locale given the priority list of
   * requested locales.
   * https://tc39.es/ecma402/#sec-bestfitmatcher
   * @param requestedLocales requested locales to compare.
   * @return the best matched locale or default locale if there is not a good
   * match.
   */
  static LocaleBCP47Object bestFitMatcher(
      const std::vector<LocaleBCP47Object> &requestedLocales);

  /**
   * Returns the "best fit" available locale for the given requested locale.
   * @param localeBCP47Object a structurally valid Unicode BCP 47
   * locale identifier
   * @return the best matched locale or empty string if there is not a good
   * match.
   */
  static std::string bestFitBestAvailableLocale(
      const LocaleBCP47Object &localeBCP47Object);

  /**
   * Returns the supported extensions specified in given locale.
   * @param localeBCP47Object locale to check
   * @param relevantExtensionKeys extension keys to consider
   * @param isExtensionTypeSupported function to determine whether given
   * extension and its type are supported for given locale
   * @return the supported extensions specified in given locale.
   */
  static std::unordered_map<std::u16string, std::u16string>
  getSupportedExtensions(
      const LocaleBCP47Object &localeBCP47Object,
      const std::unordered_set<std::u16string> &relevantExtensionKeys,
      const std::function<bool(
          const std::u16string &, /* extension key */
          const std::u16string &, /* extension type */
          const LocaleBCP47Object &)> &isExtensionTypeSupported);

  static const AvailableLocale availableLocales_;
};

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_LOCALERESOLVER_H
