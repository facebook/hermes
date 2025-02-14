/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "LocaleBCP47Object.h"

#include "hermes/Platform/Intl/BCP47Parser.h"

#include <unordered_set>

namespace hermes {
namespace platform_intl {
namespace impl_icu {

std::optional<LocaleBCP47Object> LocaleBCP47Object::forLanguageTag(
    const std::u16string &langTag) {
  auto localeId = ParsedLocaleIdentifier::parse(langTag);
  if (!localeId.has_value()) {
    return std::nullopt;
  }
  LocaleBCP47Object instance;
  instance.localeNoExt_ = getBaseName(
      localeId->languageIdentifier.languageSubtag,
      localeId->languageIdentifier.scriptSubtag,
      localeId->languageIdentifier.regionSubtag,
      localeId->languageIdentifier.variantSubtagList);
  instance.parsedLocaleIdentifier_ = localeId.value();
  return instance;
}

// https://tc39.es/ecma402/#sec-canonicalizelocalelist
// Implementation Note: Rather than returning a vector of canonicalized locale
// strings, return a vector of LocaleBCP47Object instead so they don't need to
// be parsed again later in resolveLocale() and supportedLocalesOf() functions.
vm::CallResult<std::vector<LocaleBCP47Object>>
LocaleBCP47Object::canonicalizeLocaleList(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales) noexcept {
  std::vector<LocaleBCP47Object> result;
  std::unordered_set<std::u16string> canonicalizedLocalesSeen;
  for (auto const &loc : locales) {
    auto localeBcp47ObjectOpt = forLanguageTag(loc);
    if (!localeBcp47ObjectOpt.has_value()) {
      return runtime.raiseRangeError(
          vm::TwineChar16("Invalid language tag: ") +
          vm::TwineChar16(loc.c_str()));
    }
    auto canonicalizedLocale = localeBcp47ObjectOpt->getCanonicalizedLocaleId();
    if (canonicalizedLocalesSeen.find(canonicalizedLocale) ==
        canonicalizedLocalesSeen.end()) {
      // Have not seen this canonicalized locale.
      // Add to seen set and result vector.
      canonicalizedLocalesSeen.insert(std::move(canonicalizedLocale));
      result.push_back(std::move(*localeBcp47ObjectOpt));
    }
  }
  return result;
}

void LocaleBCP47Object::updateExtensionMap(
    const std::map<std::u16string, std::u16string> &extensionMap) {
  parsedLocaleIdentifier_.unicodeExtensionKeywords = extensionMap;
}

const std::u16string &LocaleBCP47Object::getLocaleNoExt() const {
  return localeNoExt_;
}

const std::map<std::u16string, std::u16string> &
LocaleBCP47Object::getExtensionMap() const {
  return parsedLocaleIdentifier_.unicodeExtensionKeywords;
}

std::u16string LocaleBCP47Object::getCanonicalizedLocaleId() const {
  return parsedLocaleIdentifier_.canonicalize();
}

std::u16string LocaleBCP47Object::getBaseName(
    const std::u16string &languageSubtag,
    const std::u16string &scriptSubtag,
    const std::u16string &regionSubtag,
    const std::set<std::u16string> &variantSubtagList) {
  std::u16string result = languageSubtag.empty() ? u"und" : languageSubtag;
  if (!scriptSubtag.empty()) {
    result += u"-" + scriptSubtag;
  }
  if (!regionSubtag.empty()) {
    result += u"-" + regionSubtag;
  }
  for (auto const &variantSubtag : variantSubtagList) {
    result += u"-" + variantSubtag;
  }
  return result;
}

} // namespace impl_icu
} // namespace platform_intl
} // namespace hermes
