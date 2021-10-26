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
// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-bestavailablelocale
llvh::Optional<std::u16string> bestAvailableLocale(
    const std::vector<std::u16string> &availableLocales,
    const std::u16string &locale) {
  // 1. Let candidate be locale
  std::u16string candidate = locale;

  // 2. Repeat
  while (true) {
    // a. If availableLocales contains an element equal to candidate, return
    // candidate.
    if (llvh::find(availableLocales, candidate) !=
        availableLocales.end()) {
      return candidate;
    }

    // b. Let pos be the character index of the last occurrence of "-" (U+002D)
    // within candidate.
    size_t pos = candidate.rfind(u'-');

    // ...If that character does not occur, return undefined.
    if (pos == std::string::npos) {
        return {};
    }

    // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate,
    // decrease pos by 2.
    if (pos >= 2 && candidate[pos - 2] == '-') {
      pos -= 2;
    }
    // d. Let candidate be the substring of candidate from position 0,
    // inclusive, to position pos, exclusive.
    candidate.resize(pos);
  }
}

// Implementer note: For more information review
// https://402.ecma-international.org/7.0/#sec-unicode-locale-extension-sequences
std::u16string toNoExtensionsLocale(const std::u16string &locale) {
  std::vector<std::u16string> subtags;
  size_t i = 0, j = 0;
  while (i < locale.size()) {
    if (locale[i] == u'-') {
      subtags.push_back(locale.substr(j, i));
      j = i + 1;
    }
    i++;
  }

  std::u16string result;
  size_t size = subtags.size();
  if (size > 0) {
    result.append(subtags.at(0));
  }
  for (size_t s = 1; s < size; s++) {
    // If next tag is a private marker and there are remaining tags
    if (subtags.at(s) != u"u" && s < size - 1) {
      // Skip those tags until you reach end or another singleton subtag
      while (s < size - 1 && subtags.at(s + 1).size() > 1) {
        s++;
      }
    } else {
      result.append(subtags.at(s));
    }
  }

  return result;
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sec-lookupmatcher
typedef struct locale_match_t {
  std::u16string locale;
  std::u16string extension;
} locale_match_t;
locale_match_t lookupMatcher(
    std::vector<std::u16string> &requestedLocales,
    std::vector<std::u16string> &availableLocales) {
  // 1. Let result be a new Record.
  locale_match_t result;
  // 2. For each element locale of requestedLocales, do
  for (std::u16string locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with
    // any Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, then
    if (!availableLocale.hasValue()) {
      // i. Set result.[[locale]] to availableLocale.
      result.locale = availableLocale.getValue();
      // ii. If locale and noExtensionsLocale are not the same String value,
      if (locale != noExtensionsLocale) {
        // then
        // 1. Let extension be the String value consisting of the substring of
        // the Unicode locale extension sequence within locale.
        // 2. Set result.[[extension]] to extension.
        result.extension = result.locale.substr(0, locale.length() - noExtensionsLocale.length());
        // iii. Return result.
        return result;
      }
    }
    // 3. Let defLocale be DefaultLocale().
    NSString *defLocale = [[NSLocale currentLocale] localeIdentifier];
    // 4. Set result.[[locale]] to defLocale.
    result.locale = nsStringToU16String(defLocale);
    // 5. Return result.
    return result;
  }
}
// Implementer note: This method corresponds roughly to
// https://402.ecma-international.org/7.0/#sec-lookupsupportedlocales
std::vector<std::u16string> lookupSupportedLocales(
    std::vector<std::u16string> &availableLocales,
    std::vector<std::u16string> &requestedLocales) {
  // 1. Let subset be a new empty List.
  std::vector<std::u16string> subset;
  // 2. For each element locale of requestedLocales in List order, do
  for (std::u16string locale : requestedLocales) {
    // a. Let noExtensionsLocale be the String value that is locale with all
    // Unicode locale extension sequences removed.
    std::u16string noExtensionsLocale = toNoExtensionsLocale(locale);
    // b. Let availableLocale be BestAvailableLocale(availableLocales,
    // noExtensionsLocale).
    llvh::Optional<std::u16string> availableLocale =
        bestAvailableLocale(availableLocales, noExtensionsLocale);
    // c. If availableLocale is not undefined, append locale to the end of
    // subset.
    if (availableLocale.hasValue()) {
      subset.push_back(locale);
    }
  }
  // 3. Return subset.
  return subset;
}
}
vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales) {
  return std::vector<std::u16string>{u"fr-FR", u"es-ES"};
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sup-string.prototype.tolocalelowercase
vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime *runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str) {
  NSString *nsStr = u16StringToNSString(str);

  // 3. Let requestedLocales be ? CanonicalizeLocaleList(locales).
  vm::CallResult<std::vector<std::u16string>> requestedLocales =
      getCanonicalLocales(runtime, locales);
  // getcano doesnt throw so should this be removed?
  if (LLVM_UNLIKELY(requestedLocales == llvh::ExecutionStatus::EXCEPTION)) {
    return llvh::ExecutionStatus::EXCEPTION;
  }

  // 4. If requestedLocales is not an empty List, then
  std::u16string requestedLocale;
  if (!requestedLocales->empty()) {
    // a. Let requestedLocale be requestedLocales[0].
    std::vector<std::u16string> val = requestedLocales.getValue();
    requestedLocale = val[0];
  } else { // 5. Else,
    // a. Let requestedLocale be DefaultLocale().
    return nsStringToU16String(
        [nsStr lowercaseStringWithLocale:[NSLocale currentLocale]]);
  }
  // 6. Let noExtensionsLocale be the String value that is requestedLocale with
  // any Unicode locale extension sequences (6.2.1) removed.
  std::u16string noExtensionsLocale = toNoExtensionsLocale(requestedLocale);

  // 7. Let availableLocales be a List with language tags that includes the
  // languages for which the Unicode Character Database contains language
  // sensitive case mappings. Implementations may add additional language tags
  // if they support case mapping for additional locales.
  NSArray<NSString *> *availableLocales = [NSLocale availableLocaleIdentifiers];

  // 8. Let locale be BestAvailableLocale(availableLocales, noExtensionsLocale).
  // Convert to C++ array for bestAvailableLocale function
  std::vector<std::u16string> availableLocalesVector;
  for (id object in availableLocales) {
    std::u16string u16StringFromVector = nsStringToU16String(object);
    availableLocalesVector.push_back(u16StringFromVector);
  }
  llvh::Optional<std::u16string> locale =
      bestAvailableLocale(availableLocalesVector, noExtensionsLocale);
  // 9. If locale is undefined, let locale be "und".
  if (!locale.hasValue()) {
    locale = u"und";
  }

  // 10. Let cpList be a List containing in order the code points of S as
  // defined in es2022, 6.1.4, starting at the first element of S.
  // 11. Let cuList be a List where the elements are the result of a lower case
  // transformation of the ordered code points in cpList according to the
  // Unicode Default Case Conversion algorithm or an implementation-defined
  // conversion algorithm. A conforming implementation's lower case
  // transformation algorithm must always yield the same cpList given the same
  // cuList and locale.
  // 12. Let L be a String whose elements are the UTF-16 Encoding (defined in
  // es2022, 6.1.4) of the code points of cuList.
  NSString *L = u16StringToNSString(locale.getValue());
  // 13. Return L.
  return nsStringToU16String([nsStr
      lowercaseStringWithLocale:[[NSLocale alloc] initWithLocaleIdentifier:L]]);
}

// Implementer note: This method corresponds roughly to
// https://tc39.es/ecma402/#sup-string.prototype.tolocaleuppercase
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
