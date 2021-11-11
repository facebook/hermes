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
  if ((unsigned)end >= str.length()) {
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
  
  if (end - start != 1) {
    return false;
  }
  
  return isASCIILetterOrDigit(str[start]) && isASCIILetter(str[end]);
}
bool isUnicodeExtensionType(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum alpha;
  if (end - start != 1) {
    return false;
  }
  
  return isASCIILetterOrDigit(str[start]) && isASCIILetter(str[end]);
}
bool isExtensionSingleton(std::u16string str, int start, int end) {
  return (unsigned)start < str.length() && end - start == 1 && str[start] == '-';
}
bool isTransformedExtensionKey(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha digit;
  if (end - start != 1) {
    return false;
  }
  
  return isASCIILetter(str[start]) && isASCIIDigit(str[end]);
}
bool isTransformedExtensionTValueItem(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{3,8})+;
  return isCharType(str, start, end, 3, 8, &isASCIILetterOrDigit);
}
bool isPrivateUseExtension(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{1,8})+;
  return isCharType(str, start, end, 1, 8, &isASCIILetterOrDigit);
}
bool isOtherExtension(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{2,8})+;
  return isCharType(str, start, end, 2, 8, &isASCIILetterOrDigit);
  return false;
}

struct ParsedLanguageIdentifier {
  std::u16string languageSubtag;
  std::u16string scriptSubtag;
  std::u16string regionSubtag;
  std::vector<std::u16string> variantSubtagList;
};

class ParsedLocaleIdentifier {
 public:
  ParsedLocaleIdentifier();
  ~ParsedLocaleIdentifier();
  
  // public methods
  std::u16string toString();
  
  // public fields
  ParsedLanguageIdentifier languageIdentifier;
  
  std::vector<std::u16string> unicodeExtensionAttributes;
  std::unordered_map<std::u16string, std::vector<std::u16string>> unicodeExtensionKeywords;
  
  ParsedLanguageIdentifier transformedLanguageIdentifier;
  std::unordered_map<std::u16string, std::vector<std::u16string>> transformedExtensionFields;
  
  std::unordered_map<char16_t, std::vector<std::u16string>> otherExtensionMap;
  std::vector<std::u16string> puExtensions;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};
struct ParsedLocaleIdentifier::Impl { };

std::u16string ParsedLocaleIdentifier::toString() {
  std::u16string res;
  
  // Append unicode_language_id
  if (!languageIdentifier.languageSubtag.empty()) {
    res += u"unicode_language_subtag: " + languageIdentifier.languageSubtag + u"\n";
  }
  if (!languageIdentifier.scriptSubtag.empty()) {
    res += u"unicode_script_subtag: " + languageIdentifier.scriptSubtag + u"\n";
  }
  if (!languageIdentifier.regionSubtag.empty()) {
    res += u"unicode_region_subtag: " + languageIdentifier.regionSubtag + u"\n";
  }
  if (!languageIdentifier.variantSubtagList.empty()) {
    res += u"unicode_variant_subtags: ";
    for (const auto &subtag : languageIdentifier.variantSubtagList) {
      res += subtag;
    }
    res += u"\n";
  }
  
  if (!unicodeExtensionAttributes.empty() ||
      !unicodeExtensionKeywords.empty()) {
    res += u"unicode_locale_extensions\n";
  }
  if (!unicodeExtensionAttributes.empty()) {
    res += u"\tattributes: ";
    for (const auto &attribute : unicodeExtensionAttributes) {
      res += attribute;
    }
    res += u"\n";
  }
  if (!unicodeExtensionKeywords.empty()) {
    res += u"\tkeywords:\n";
    for (const auto &keyword : unicodeExtensionKeywords) {
      res += u"\t\t" + keyword.first;
      for (const auto &word : keyword.second) {
        res += word;
      }
      res += u"\n";
    }
    res += u"\n";
  }
  
  // currently skipping transformed extensions
  if (!otherExtensionMap.empty()) {
    res += u"other_extensions:\n";
    for (const auto &ext : otherExtensionMap) {
      res += u"\t";
      res += ext.first; // cheap cast
      for (const auto &extVal : ext.second) {
        res += extVal;
      }
      res += u"\n";
    }
    res += u"\n";
  }
  
  if (!puExtensions.empty()) {
    res += u"pu_extensions: ";
    for (const auto &ext : puExtensions) {
      res += ext;
    }
    res += u"\n";
  }
  
  return res;
}

class LanguageTagParser {
 public:
  LanguageTagParser(const std::u16string &localeId);
  ~LanguageTagParser();
    
  // public function declaration
  ParsedLocaleIdentifier parseLocaleId();
  // tokenizer functions
  std::u16string toString();
  std::u16string toParsedString();
  
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  
  // private function declaration
  bool parseUnicodeLocaleId();
  bool parseUnicodeLanguageId();
  bool parseExtensions();
  bool parseUnicodeExtension();
  bool parseTransformedExtension();
  bool parseOtherExtension();
  bool parsePUExtension();

  // tokenizer functions
  std::u16string getCurrentSubtag();
  bool hasMoreSubtags();
  bool nextSubtag();
};
struct LanguageTagParser::Impl {
  Impl(const std::u16string &localeId)
    : mLocaleId(localeId) {};
  ~Impl();
  
  ParsedLocaleIdentifier parsedLocaleIdentifier;
  std::u16string mLocaleId;
  size_t mSubtagStart;
  size_t mSubtagEnd;
};
LanguageTagParser::LanguageTagParser(const std::u16string &localeId) : impl_(std::make_unique<Impl>()) {
  impl_->mLocaleId = localeId; // tolowercase? this should just be ascii right?
  impl_->mSubtagStart = 0;
  impl_->mSubtagEnd = -1;
}
LanguageTagParser::~LanguageTagParser() = default;

ParsedLocaleIdentifier LanguageTagParser::parseLocaleId() {
  if (!parseUnicodeLocaleId()) {
    // throw
  }
  //return impl->parsedLocaleIdentifier;
  return {};
}

bool LanguageTagParser::parseUnicodeLocaleId() {
  if (!parseUnicodeLanguageId())  {
    return false;
  }
  if (!hasMoreSubtags()) {
    return false;
  }
  if (!parseExtensions()) {
    return false;
  }
  
  return false;
}

bool LanguageTagParser::parseUnicodeLanguageId() {
  ParsedLanguageIdentifier parsedLanguageIdentifier;
  
  if (!hasMoreSubtags() || !isUnicodeLanguageSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    return false;
  }
  
  parsedLanguageIdentifier.languageSubtag = getCurrentSubtag();
  
  if (!nextSubtag()) {
    return true;
  }
  
  // handle extensions here? is this most common path?
  
  if (isUnicodeScriptSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    parsedLanguageIdentifier.scriptSubtag = getCurrentSubtag(); // to title case?
    if (!nextSubtag()) {
      return true;
    }
  }
  
  if (isUnicodeRegionSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    parsedLanguageIdentifier.regionSubtag = getCurrentSubtag(); // to upper case?
    if (!nextSubtag()) {
      return true;
    }
  }
  
  while (true) {
    if (!isUnicodeVariantSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
      return false;
    } else {
      // add variant subtag to list
    }
    
    if (!nextSubtag()) {
      return true;
    }
  }
  
  return false;
}

bool LanguageTagParser::parseExtensions() {
  // if transformed extensions and next subtag is transformed extension
  // parse transformed extensions

  while (true) {
    // check if current subtag isExtensionSingleton
    if (impl_->mSubtagEnd - impl_->mSubtagStart != 1) {
      return true;
    }
    char16_t singleton = getCurrentSubtag()[0];
    if (!isASCIILetterOrDigit(singleton)) {
      return true;
    }
    
    if (!nextSubtag()) {
      return false;
    }
    
    // duplicate subtags handled in parse functions
    switch (singleton) {
      case 'u': {
        // unicode extension
        if (!parseUnicodeExtension()) {
          return false;
        }
        break;
      }
      case 't': {
        // transformed extension
        if (!parseTransformedExtension()) {
          return false;
        }
        break;
      }
      case 'x': {
        // private use extension
        if (!parsePUExtension()) {
          return false;
        }
        return true;
      }
      default: {
        // other extension
        if (!parseOtherExtension()) {
          return false;
        }
        break;
      }
    }
  }

  return false;
}

bool LanguageTagParser::parseUnicodeExtension() {
  if (!impl_->parsedLocaleIdentifier.unicodeExtensionAttributes.empty() ||
      !impl_->parsedLocaleIdentifier.unicodeExtensionKeywords.empty()) {
    return false;
  }
  
  while (isUnicodeExtensionAttribute(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    impl_->parsedLocaleIdentifier.unicodeExtensionAttributes.push_back(getCurrentSubtag());
    if (!nextSubtag()) {
      return true;
    }
  }
  
  while (isUnicodeExtensionKey(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    std::u16string key = getCurrentSubtag();
    std::vector<std::u16string> extensionKeyTypes;
    impl_->parsedLocaleIdentifier.unicodeExtensionKeywords.insert({key, extensionKeyTypes});
    
    if (!nextSubtag()) {
      return true;
    } else {
      while (isUnicodeExtensionType(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
        extensionKeyTypes.push_back(getCurrentSubtag());
        if (!nextSubtag()) {
          return true;
        }
      }
    }
  }
  
  return true;
}

bool LanguageTagParser::parseTransformedExtension() {
  return false;
}

bool LanguageTagParser::parseOtherExtension() {
  return false;
}

bool LanguageTagParser::parsePUExtension() {
  return false;
}

bool LanguageTagParser::hasMoreSubtags() {
  return impl_->mLocaleId.length() > 0 && impl_->mSubtagEnd < impl_->mLocaleId.length() - 1;
}
bool LanguageTagParser::nextSubtag() {
  if (!hasMoreSubtags()) {
    return false; // throw error?
  }
  
  auto length = impl_->mLocaleId.length();
  
  if (impl_->mSubtagEnd >= impl_->mSubtagStart) {
    if (!isSubtagSeparator(impl_->mLocaleId[impl_->mSubtagEnd+1])) {
      return false;
    }
    if (impl_->mSubtagEnd + 2 == length) {
      return false;
    }
    impl_->mSubtagStart = impl_->mSubtagEnd + 2;
  }
  
  for (impl_->mSubtagEnd = impl_->mSubtagStart; impl_->mSubtagEnd < length && !isSubtagSeparator(impl_->mLocaleId[impl_->mSubtagEnd]); impl_->mSubtagEnd++)
    ;
  
  if (impl_->mSubtagEnd > impl_->mSubtagStart) {
    impl_->mSubtagEnd--;
    return true;
  } else {
    return false;
  }
}
std::u16string LanguageTagParser::toString() {
  return impl_->mLocaleId;
}
std::u16string LanguageTagParser::toParsedString() {
  return impl_->parsedLocaleIdentifier.toString();
}
std::u16string LanguageTagParser::getCurrentSubtag() {
  return impl_->mLocaleId.substr(impl_->mSubtagStart, impl_->mSubtagEnd - impl_->mSubtagStart + 1);
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
