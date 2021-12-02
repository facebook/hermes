/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntl.h"

#import <Foundation/Foundation.h>

namespace hermes {
namespace platform_intl {
namespace {
NSString *u16StringToNSString(std::u16string src) {
  auto size = src.size();
  const auto *cString = (const unichar *)src.c_str();
  return [NSString stringWithCharacters:cString length:size];
}
std::u16string nsStringToU16String(NSString *src) {
  auto size = src.length;
  std::u16string result;
  result.resize(size);
  [src getCharacters:(unichar *)&result[0] range:NSMakeRange(0, size)];
  return result;
}
}

// https://unicode.org/reports/tr35/#Canonical_Unicode_Locale_Identifiers
// https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid
std::u16string canonicalizeLocaleId(const std::u16string &localeId) {
  llvh::Optional<bcp47_parser::ParsedLocaleIdentifier> parserOpt = bcp47_parser::parseLocaleId(localeId);

  if (!parserOpt.hasValue()) {
    // Not structurally valid language tag
    return localeId;
  }

  std::u16string canoLocaleId;
  canoLocaleId.reserve(localeId.length());

  // Casing, variant, and attribute sorting are handled in parsing
  bcp47_parser::ParsedLocaleIdentifier parsedId = parserOpt.getValue();
  
  // Append unicode_language_id
  if (!parsedId.languageIdentifier.languageSubtag.empty()) {
    canoLocaleId += parsedId.languageIdentifier.languageSubtag;
  }
  if (!parsedId.languageIdentifier.scriptSubtag.empty()) {
    if (!canoLocaleId.empty()) {
      canoLocaleId += u"-";
    }
    canoLocaleId += parsedId.languageIdentifier.scriptSubtag;
  }
  if (!parsedId.languageIdentifier.regionSubtag.empty()) {
    if (!canoLocaleId.empty()) {
      canoLocaleId += u"-";
    }
    canoLocaleId += parsedId.languageIdentifier.regionSubtag;
  }
  for (const auto &subtag : parsedId.languageIdentifier.variantSubtagList) {
    if (!canoLocaleId.empty()) {
      canoLocaleId += u"-";
    }
    canoLocaleId += subtag;
  }

  // Any extensions are in alphabetical order by their singleton (eg, en-t-xxx-u-yyy, not en-u-yyy-t-xxx).

  // Sort other extensions by singleton
  std::vector<std::u16string> otherExtensions;
  for (auto it = parsedId.otherExtensionMap.begin(); it != parsedId.otherExtensionMap.end(); it++) {
    std::u16string oExt;
    oExt += it->first;
    for (const auto &ext : it->second) {
      oExt += u"-" + ext;
    }
    otherExtensions.push_back(u"-" + oExt);
  }
  std::sort(otherExtensions.begin(), otherExtensions.end());

  // Append other extensions from a-s
  size_t oExtIndex;
  for (oExtIndex = 0; oExtIndex < otherExtensions.size(); oExtIndex++) {
    const auto &oExt = otherExtensions[oExtIndex];
    if (oExt[0] > 's') {
      break;
    }
    canoLocaleId += oExt;
  }

  // Build transformed extension
  std::u16string transformedExtension;

  // Append tlang
  if (!parsedId.transformedLanguageIdentifier.languageSubtag.empty()) {
    transformedExtension += u"-" + parsedId.transformedLanguageIdentifier.languageSubtag;
  }

  if (!parsedId.transformedLanguageIdentifier.scriptSubtag.empty()) {
    transformedExtension += u"-" + parsedId.transformedLanguageIdentifier.scriptSubtag;
  }
  if (!parsedId.transformedLanguageIdentifier.regionSubtag.empty()){
    transformedExtension += u"-" + parsedId.transformedLanguageIdentifier.regionSubtag;
  }
  for (const auto &subtag : parsedId.transformedLanguageIdentifier.variantSubtagList) {
    transformedExtension += u"-" + subtag;
  }
  
  // Sort tfields by the alphabetical order of its keys
  std::vector<std::u16string> tFields;
  for (auto it = parsedId.transformedExtensionFields.begin(); it != parsedId.transformedExtensionFields.end(); it++) {
    // Any type or tfield value "true" is removed.
    if (it->second.size() == 1 && it->second[0] == u"true") {
      continue;
    }
    
    std::u16string tField;
    tField += it->first;
    
    for (const auto &tValue : it->second) {
      tField += u"-" + tValue;
    }
    
    tFields.push_back(tField);
  }
  std::sort(tFields.begin(), tFields.end());

  // Append tfields
  for (const auto &tField : tFields) {
    transformedExtension += u"-" + tField;
  }

  // Append transformed extension singleton
  if (transformedExtension.length() > 0) {
    canoLocaleId += u"-t" + transformedExtension;
  }
  
  // Build Unicode extension
  std::u16string unicodeExtension;
  
  // Append sorted Unicode attributes
  for (const auto &attribute : parsedId.unicodeExtensionAttributes) {
    unicodeExtension += u"-" + attribute;
  }

  // Sort Unicode keywords  by the alphabetical order of its keys
  std::vector<std::u16string> keywords;
  for (auto it = parsedId.unicodeExtensionKeywords.begin(); it != parsedId.unicodeExtensionKeywords.end(); it++) {
    // Any type or tfield value "true" is removed.
    if (it->second.size() == 1 && it->second[0] == u"true") {
      continue;
    }
    
    std::u16string keyword;
    keyword += it->first;
    for (const auto &value : it->second) {
      keyword += u"-" + value;
    }

    keywords.push_back(keyword);
  }
  std::sort(keywords.begin(), keywords.end());

  // Append Unicode keywords
  for (const auto &keyword : keywords) {
    unicodeExtension += u"-" + keyword;
  }
  
  // Append Unicode singleton
  if (unicodeExtension.length() > 0) {
    canoLocaleId += u"-u" + unicodeExtension;
  }

  // Append remaining other extensions
  for (; oExtIndex < otherExtensions.size(); oExtIndex++) {
    canoLocaleId += otherExtensions[oExtIndex];
  }

  // Append private use extensions
  if (!parsedId.puExtensions.empty()) {
    canoLocaleId += u"-x";
    for (const auto &puExt : parsedId.puExtensions) {
      canoLocaleId +=  u"-" + puExt;
    }
  }
  
  return canoLocaleId;
}

// Implementation of https://tc39.es/ecma402/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> canonicalizeLocaleList(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  // 1. If locales is undefined, then
  //   a. Return a new empty List
  // Not needed, this validation occurs closer to VM in 'normalizeLocales'.
  // 2. Let seen be a new empty List.
  std::vector<std::u16string> seen;
  // 3. If Type(locales) is String or Type(locales) is Object and locales has an
  // [[InitializedLocale]] internal slot, then
  // 4. Else
  // We don't yet support Locale object -
  // https://tc39.es/ecma402/#locale-objects As of now, 'locales' can only be a
  // string list/array. Validation occurs in normalizeLocaleList, so this
  // function just takes a vector of strings.
  // 5. Let len be ? ToLength(? Get(O, "length")).
  // 6. Let k be 0.
  // 7. Repeat, while k < len
  for (std::u16string locale : locales) {
    // TODO - BCP 47 tag validation
    // 7.c.vi. Let canonicalizedTag be CanonicalizeUnicodeLocaleId(tag).

    auto canonicalizedTag = canonicalizeLocaleId(locale);
    // 7.c.vii. If canonicalizedTag is not an element of seen, append
    // canonicalizedTag as the last element of seen.
    if (std::find(seen.begin(), seen.end(), canonicalizedTag) == seen.end()) {
      seen.push_back(std::move(canonicalizedTag));
    }
  }
  return seen;
}

// https://tc39.es/ecma402/#sec-canonicalizelocalelist
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return canonicalizeLocaleList(runtime, locales);
}

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"lowered");
}
vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  return std::u16string(u"uppered");
}

struct Collator::Impl {
  std::u16string locale;
};

Collator::Collator() : impl_(std::make_unique<Impl>()) {}
Collator::~Collator() {}

vm::CallResult<std::vector<std::u16string>> Collator::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus Collator::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options Collator::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

double Collator::compare(
    const std::u16string &x,
    const std::u16string &y) noexcept {
  return x.compare(y);
}

struct DateTimeFormat::Impl {
  std::u16string locale;
};

DateTimeFormat::DateTimeFormat() : impl_(std::make_unique<Impl>()) {}
DateTimeFormat::~DateTimeFormat() {}

vm::CallResult<std::vector<std::u16string>> DateTimeFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus DateTimeFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options DateTimeFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string DateTimeFormat::format(double jsTimeValue) noexcept {
  auto s = std::to_string(jsTimeValue);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
DateTimeFormat::formatToParts(double jsTimeValue) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(jsTimeValue);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

struct NumberFormat::Impl {
  std::u16string locale;
};

NumberFormat::NumberFormat() : impl_(std::make_unique<Impl>()) {}
NumberFormat::~NumberFormat() {}

vm::CallResult<std::vector<std::u16string>> NumberFormat::supportedLocalesOf(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  return std::vector<std::u16string>{u"en-CA", u"de-DE"};
}

vm::ExecutionStatus NumberFormat::initialize(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const Options &options) noexcept {
  impl_->locale = u"en-US";
  return vm::ExecutionStatus::RETURNED;
}

Options NumberFormat::resolvedOptions() noexcept {
  Options options;
  options.emplace(u"locale", Option(impl_->locale));
  options.emplace(u"numeric", Option(false));
  return options;
}

std::u16string NumberFormat::format(double number) noexcept {
  auto s = std::to_string(number);
  return std::u16string(s.begin(), s.end());
}

std::vector<std::unordered_map<std::u16string, std::u16string>>
NumberFormat::formatToParts(double number) noexcept {
  std::unordered_map<std::u16string, std::u16string> part;
  part[u"type"] = u"integer";
  // This isn't right, but I didn't want to do more work for a stub.
  std::string s = std::to_string(number);
  part[u"value"] = {s.begin(), s.end()};
  return std::vector<std::unordered_map<std::u16string, std::u16string>>{part};
}

} // namespace platform_intl
} // namespace hermes
