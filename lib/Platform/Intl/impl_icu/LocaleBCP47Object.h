/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_PLATFORMINTL_IMPLICU_LOCALEBCP47OBJECT_H
#define HERMES_PLATFORMINTL_IMPLICU_LOCALEBCP47OBJECT_H

#include "hermes/Platform/Intl/BCP47Parser.h"
#include "hermes/VM/CallResult.h"
#include "hermes/VM/Runtime.h"

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl#locale_identification_and_negotiation
// A locale identifier is a string that consists of:
// 1. a language subtag,
// 2. (optionally) a script subtag,
// 3. (optionally) a region (or country) subtag,
// 4. (optionally) one or more variant subtags (all of which must be unique),
// 5. (optionally) one or more BCP 47 extension sequences, and
// 6. (optionally) a private-use extension sequence
class LocaleBCP47Object {
 public:
  /**
   * Default constructor. Creates an instance without an associated locale.
   */
  LocaleBCP47Object() = default;

  /**
   * Creates and initializes a BCP47 locale object.
   * @param locale a BCP47 language tag
   * @return a BCP47 locale object in optional on success, empty optional on
   * failure
   */
  static std::optional<LocaleBCP47Object> forLanguageTag(
      const std::u16string &langTag);

  /**
   * Returns a vector of unique BCP47 locale objects from given vector of
   * BCP47 language tags.
   *
   * Corresponds to https://tc39.es/ecma402/#sec-canonicalizelocalelist.
   *
   * @param runtime runtime object
   * @param locales vector of BCP47 language tags
   * @return CallResult with a vector of unique BCP47 locale objects on success,
   * with ExecutionStatus.EXCEPTION on failure.
   */
  static vm::CallResult<std::vector<LocaleBCP47Object>> canonicalizeLocaleList(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales) noexcept;

  /**
   * Updates the extension map of BCP47 locale object.
   * @param extensionMap map that contains extension keys and types.
   */
  void updateExtensionMap(
      const std::map<std::u16string, std::u16string> &extensionMap);

  /**
   * Returns the BCP47 locale string without extensions.
   * @return BCP47 locale string without extensions.
   */
  const std::u16string &getLocaleNoExt() const;

  /**
   * Returns the map that contains extension keys and types.
   * @return map that contains extension keys and types.
   */
  const std::map<std::u16string, std::u16string> &getExtensionMap() const;

  /**
   * Returns the canonicalized BCP47 locale string.
   * @return canonicalized BCP47 locale string.
   */
  std::u16string getCanonicalizedLocaleId() const;

 private:
  /**
   * Construct a base name in BCP47 locale format
   * @param languageSubtag languageSubtag
   * @param scriptSubtag scriptSubtag
   * @param regionSubtag regionSubtag
   * @param variantSubtagList variantSubtagList
   * @return a BCP47 locale base name string
   */
  static std::u16string getBaseName(
      const std::u16string &languageSubtag,
      const std::u16string &scriptSubtag,
      const std::u16string &regionSubtag,
      const std::set<std::u16string> &variantSubtagList);

  std::u16string localeNoExt_;
  ParsedLocaleIdentifier parsedLocaleIdentifier_;
};

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORMINTL_IMPLICU_LOCALEBCP47OBJECT_H
