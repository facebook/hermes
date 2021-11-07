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

// character type functions
bool isASCIILetter(char16_t c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}
bool isASCIIDigit(char16_t c) {
  return (c >= '0' && c <= '9');
}

bool isASCIILetterOrDigit(char16_t c) {
  return isASCIILetter(c) || isASCIIDigit(c);
}
bool isSubtagSeparator(char16_t c) {
  return c == '-';
}
bool isCharType(std::u16string str, int start, int end, int min, int max, bool(*charType)(char16_t)) {
  if (end >= str.length()) {
    return false;
  }
  
  int length = end - start + 1;
  if (length < min || length > max) {
    return false;
  }
  
  for (int i = start; i < end; i++) {
    if (!charType(str[i])) {
      return false;
    }
  }
  
  return true;
}

// tag type functions
bool isUnicodeLanguageSubtag(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha{2,3} | alpha{5,8};
  // root case?
  return isCharType(str, start, end, 2, 3, &isASCIILetter) ||
    isCharType(str, start, end, 5, 8, &isASCIILetter);
}
bool isUnicodeScriptSubtag(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha{4};
  return isCharType(str, start, end, 4, 4, &isASCIILetter);
}
bool isUnicodeRegionSubtag(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (alpha{2} | digit{3});
  return isCharType(str, start, end, 2, 2, &isASCIILetter) ||
    isCharType(str, start, end, 3, 3, &isASCIIDigit);
}
bool isUnicodeVariantSubtag(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (alphanum{5,8} | digit alphanum{3});
  return isCharType(str, start, end, 5, 8, &isASCIILetterOrDigit) ||
    isCharType(str, start, end, 3, 3, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionAttribute(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum{3,8};
  return isCharType(str, start, end, 3, 8, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionKey(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum alpha;
  return false;
}
bool isUnicodeExtensionType(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum alpha;
  return false;
}
bool isExtensionSingleton(std::u16string str, int start, int end) {
  return false;
}
bool isTransformedExtensionKey(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha digit;
  return false;
}
bool isTransformedExtensionTValueItem(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{3,8})+;
  return false;
}
bool isPrivateUseExtension(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{1,8})+;
  return false;
}
bool isOtherExtension(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{2,8})+;
  return false;
}

struct ParsedLanguageIdentifier {
  std::u16string languageSubtag;
  std::u16string scriptSubtag;
  std::u16string regionSubtag;
  std::vector<std::u16string> variantSubtagList;
};

class ParsedLocaleIdentifier : public vm::DecoratedObject::Decoration {
 public:
  ParsedLocaleIdentifier();
  ~ParsedLocaleIdentifier();
  // public function declaration
  static ParsedLocaleIdentifier parseLocaleId(const std::u16string &localeId);

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
struct ParsedLocaleIdentifier::Impl {
  ParsedLanguageIdentifier languageIdentifier;
  // Other extensions

  // Function implementation
  static ParsedLocaleIdentifier parseLocaleId(const std::u16string &localeId) {
    return {};
  }
};
class LanguageTagParser : public vm::DecoratedObject::Decoration {
 public:
  LanguageTagParser(std::u16string localeId);
  ~LanguageTagParser();
    
  // public function declaration
  ParsedLocaleIdentifier parseLocaleId();
  std::u16string toString();
  std::u16string getCurrentTag();
  bool hasMoreSubtags();
  bool nextSubtag();

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
struct LanguageTagParser::Impl {
  std::u16string mLocaleId;
  size_t mSubtagStart;
  size_t mSubtagEnd;
};
LanguageTagParser::LanguageTagParser(std::u16string localeId) : impl_(std::make_unique<Impl>()) {
  mLocaleId = localeId;
  mSubtagStart = 0;
  mSubtagEnd = -1;
}
LanguageTagParser::~LanguageTagParser();
bool LanguageTagParser::hasMoreSubtags() {
  return mLocaleId.length();
}

bool LanguageTagParser::nextSubtag() {
  if (!hasMoreSubtags()) {
    return false; // throw error?
  }
  
  auto length = mLocaleId.length();
  
  if (mSubtagEnd >= mSubtagStart) {
    if (!isSubtagSeparator(mLocaleId[mSubtagEnd+1])) {
      return false;
    }
    if (mSubtagEnd + 2 == length) {
      return false;
    }
    mSubtagStart = mSubtagEnd + 2;
  }
  
  for (mSubtagEnd = mSubtagStart; mSubtagEnd < length && !isSubtagSeparator(mLocaleId[mSubtagEnd]); mSubtagEnd++)
    ;
  
  if (mSubtagEnd > mSubtagStart) {
    mSubtagEnd--;
    return true;
  } else {
    return false;
  }
}
std::u16string LanguageTagParser::toString() {
  return localeId;
}
std::u16string LanguageTagParser::getCurrentTag() {
  return localeId.substr(mSubtagStart, mSubtagEnd - mSubtagStart + 1);
}
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
    auto *localeNSString = u16StringToNSString(locale);
    NSString *canonicalizedTagNSString =
        [NSLocale canonicalLocaleIdentifierFromString:localeNSString];
    auto canonicalizedTag = nsStringToU16String(canonicalizedTagNSString);
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
