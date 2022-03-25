/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"

#include "llvh/ADT/STLExtras.h"

namespace hermes {
namespace platform_intl {
namespace {
bool isASCIILetter(char16_t c) {
  return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}
bool isASCIIDigit(char16_t c) {
  return (c >= '0' && c <= '9');
}
bool isASCIILetterOrDigit(char16_t c) {
  return isASCIILetter(c) || isASCIIDigit(c);
}
template <typename F>
bool isCharType(const std::u16string &str, size_t min, size_t max, F charType) {
  return str.size() >= min && str.size() <= max && llvh::all_of(str, charType);
}

// Helper functions to check subtag validity.
// https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
bool isUnicodeLanguageSubtag(const std::u16string &subtag) {
  // = alpha{2,3} | alpha{5,8};
  // root case?
  return isCharType(subtag, 2, 3, &isASCIILetter) ||
      isCharType(subtag, 5, 8, &isASCIILetter);
}
bool isUnicodeScriptSubtag(const std::u16string &subtag) {
  // = alpha{4};
  return isCharType(subtag, 4, 4, &isASCIILetter);
}
bool isUnicodeRegionSubtag(const std::u16string &subtag) {
  // = (alpha{2} | digit{3});
  return isCharType(subtag, 2, 2, &isASCIILetter) ||
      isCharType(subtag, 3, 3, &isASCIIDigit);
}
bool isUnicodeVariantSubtag(const std::u16string &subtag) {
  // = (alphanum{5,8} | digit alphanum{3});
  return isCharType(subtag, 5, 8, &isASCIILetterOrDigit) ||
      isCharType(subtag, 3, 3, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionAttribute(const std::u16string &subtag) {
  // = alphanum{3,8};
  return isCharType(subtag, 3, 8, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionKey(const std::u16string &subtag) {
  // = alphanum alpha;
  return subtag.size() == 2 && isASCIILetterOrDigit(subtag.front()) &&
      isASCIILetter(subtag.back());
}
bool isUnicodeExtensionKeyTypeItem(const std::u16string &subtag) {
  // = alphanum{3,8};
  return isCharType(subtag, 3, 8, &isASCIILetterOrDigit);
}
bool isTransformedExtensionKey(const std::u16string &subtag) {
  // = alpha digit;
  return subtag.size() == 2 && isASCIILetter(subtag.front()) &&
      isASCIIDigit(subtag.back());
}
bool isTransformedExtensionTValueItem(const std::u16string &subtag) {
  // = (sep alphanum{3,8})+;
  return isCharType(subtag, 3, 8, &isASCIILetterOrDigit);
}
bool isPrivateUseExtension(const std::u16string &subtag) {
  // = (sep alphanum{1,8})+;
  return isCharType(subtag, 1, 8, &isASCIILetterOrDigit);
}
bool isOtherExtension(const std::u16string &subtag) {
  // = (sep alphanum{2,8})+;
  return isCharType(subtag, 2, 8, &isASCIILetterOrDigit);
}
std::vector<std::u16string> splitIntoSubtags(const std::u16string &locale) {
  std::vector<std::u16string> subtags;
  auto s = locale.begin();
  const auto e = locale.end();
  while (true) {
    auto tagEnd = std::find(s, e, u'-');
    subtags.emplace_back(s, tagEnd);
    if (tagEnd == e)
      break;
    s = tagEnd + 1;
  }
  return subtags;
}
} // namespace

bool isUnicodeExtensionType(const std::u16string &str) {
  // = alphanum{3,8} (sep alphanum{3,8})*;
  return llvh::all_of(splitIntoSubtags(str), isUnicodeExtensionKeyTypeItem);
}

class LanguageTagParser {
 public:
  LanguageTagParser(const std::u16string &localeId);
  std::optional<ParsedLocaleIdentifier> parseUnicodeLocaleId();

 private:
  LLVM_NODISCARD bool parseUnicodeLanguageId(bool transformedExtensionId);
  LLVM_NODISCARD bool parseExtensions();
  LLVM_NODISCARD bool parseUnicodeExtension();
  LLVM_NODISCARD bool parseTransformedExtension();
  LLVM_NODISCARD bool parseOtherExtension(char16_t singleton);
  LLVM_NODISCARD bool parsePUExtension();

  const std::u16string &getCurrentSubtag() const;
  bool hasMoreSubtags() const;
  LLVM_NODISCARD bool nextSubtag();

  ParsedLocaleIdentifier result_;
  std::vector<std::u16string> subtags_;
};

LanguageTagParser::LanguageTagParser(const std::u16string &localeId) {
  subtags_ = splitIntoSubtags(localeId);
  std::reverse(subtags_.begin(), subtags_.end());
  // Convert all subtags to lowercase by default.
  for (std::u16string &subtag : subtags_) {
    for (char16_t &c : subtag)
      if (c <= 'Z' && c >= 'A')
        c -= 'Z' - 'z';
  }
}

// 	unicode_locale_id =
// 	  unicode_language_id
//    extensions*
//    pu_extensions? ;
std::optional<ParsedLocaleIdentifier>
LanguageTagParser::parseUnicodeLocaleId() {
  if (!parseUnicodeLanguageId(false))
    return {};
  if (!parseExtensions())
    return {};
  if (hasMoreSubtags())
    return {};
  return result_;
}

// unicode_language_id =
//   unicode_language_subtag
//   (sep unicode_script_subtag)?
//   (sep unicode_region_subtag)?
//   (sep unicode_variant_subtag)*
// NOTE: We intentionally omit tags starting with a script subtag and the "root"
// tag since they are not valid BCP-47 tags.
bool LanguageTagParser::parseUnicodeLanguageId(bool transformedExtensionId) {
  if (!transformedExtensionId && !isUnicodeLanguageSubtag(getCurrentSubtag()))
    return false;

  auto &languageId = transformedExtensionId
      ? result_.transformedLanguageIdentifier
      : result_.languageIdentifier;

  languageId.languageSubtag = getCurrentSubtag();
  if (!nextSubtag())
    return true;

  if (isUnicodeScriptSubtag(getCurrentSubtag())) {
    languageId.scriptSubtag = getCurrentSubtag();
    if (!nextSubtag())
      return true;
  }

  if (isUnicodeRegionSubtag(getCurrentSubtag())) {
    languageId.regionSubtag = getCurrentSubtag();
    if (!nextSubtag())
      return true;
  }

  while (isUnicodeVariantSubtag(getCurrentSubtag())) {
    if (!languageId.variantSubtagList.insert(getCurrentSubtag()).second)
      return false;
    if (!nextSubtag())
      return true;
  }
  return true;
}

bool LanguageTagParser::parseExtensions() {
  while (hasMoreSubtags() && getCurrentSubtag().size() == 1) {
    char16_t singleton = getCurrentSubtag()[0];
    if (!isASCIILetterOrDigit(singleton))
      return true;

    if (!nextSubtag())
      return false;

    switch (singleton) {
      case 'u':
        // unicode_locale_extensions
        if (!parseUnicodeExtension())
          return false;
        break;
      case 't':
        // transformed_extensions
        if (!parseTransformedExtension())
          return false;
        break;
      case 'x':
        // pu_extensions - must be the last extension
        return parsePUExtension();
      default:
        // other_extensions
        if (!parseOtherExtension(singleton))
          return false;
        break;
    }
  }
  return true;
}

// unicode_locale_extensions = sep [uU]
// ((sep keyword)+
// |(sep attribute)+ (sep keyword)*) ;
bool LanguageTagParser::parseUnicodeExtension() {
  if (!result_.unicodeExtensionAttributes.empty() ||
      !result_.unicodeExtensionKeywords.empty())
    return false;

  bool hasKeywordOrAttribute = false;

  while (isUnicodeExtensionAttribute(getCurrentSubtag())) {
    hasKeywordOrAttribute = true;
    // ECMA-402 specifies that attributes should be deduplicated.
    result_.unicodeExtensionAttributes.insert(getCurrentSubtag());
    if (!nextSubtag())
      return true;
  }

  while (hasMoreSubtags() && isUnicodeExtensionKey(getCurrentSubtag())) {
    hasKeywordOrAttribute = true;
    // ECMA-402 specifies that only the first occurrence of a key is kept.
    auto res =
        result_.unicodeExtensionKeywords.insert({getCurrentSubtag(), {}});
    std::u16string &uvalue = res.first->second;
    while (nextSubtag() && isUnicodeExtensionKeyTypeItem(getCurrentSubtag())) {
      if (res.second) {
        if (!uvalue.empty())
          uvalue += u'-';
        uvalue.append(getCurrentSubtag());
      }
    }
  }

  return hasKeywordOrAttribute;
}

// transformed_extensions= sep [tT]
// ((sep tlang (sep tfield)*)
// | (sep tfield)+) ;
//
// tlang = unicode_language_subtag
//  (sep unicode_script_subtag)?
//  (sep unicode_region_subtag)?
//  (sep unicode_variant_subtag)* ;
//
//  tfield = tkey tvalue;
bool LanguageTagParser::parseTransformedExtension() {
  if (!result_.transformedLanguageIdentifier.languageSubtag.empty() ||
      !result_.transformedExtensionFields.empty())
    return false;

  bool hasExtension = false;
  if (isUnicodeLanguageSubtag(getCurrentSubtag())) {
    hasExtension = true;
    if (!parseUnicodeLanguageId(true))
      return false;
  }

  while (hasMoreSubtags() && isTransformedExtensionKey(getCurrentSubtag())) {
    hasExtension = true;
    auto res =
        result_.transformedExtensionFields.insert({getCurrentSubtag(), {}});
    // IETF RFC6497: Two identical field separators MUST NOT be present in the
    // language tag.
    if (!res.second)
      return false;
    if (!nextSubtag())
      return false;
    if (!isTransformedExtensionTValueItem(getCurrentSubtag()))
      return false;

    std::u16string &tvalue = res.first->second;
    do {
      if (!tvalue.empty())
        tvalue += u'-';
      tvalue.append(getCurrentSubtag());
    } while (nextSubtag() &&
             isTransformedExtensionTValueItem(getCurrentSubtag()));
  }

  return hasExtension;
}

// pu_extensions= sep [xX]
// (sep alphanum{1,8})+ ;
bool LanguageTagParser::parsePUExtension() {
  if (!isPrivateUseExtension(getCurrentSubtag()))
    return false;

  std::u16string &values = result_.puExtensions;
  do {
    if (!values.empty())
      values += u'-';
    values.append(getCurrentSubtag());
  } while (nextSubtag() && isPrivateUseExtension(getCurrentSubtag()));
  return true;
}

// other_extensions= sep [alphanum-[tTuUxX]]
// (sep alphanum{2,8})+ ;
bool LanguageTagParser::parseOtherExtension(char16_t singleton) {
  auto res = result_.otherExtensionMap.insert({singleton, {}});
  if (!res.second || !isOtherExtension(getCurrentSubtag()))
    return false;

  std::u16string &values = res.first->second;
  do {
    if (!values.empty())
      values += u'-';
    values.append(getCurrentSubtag());
  } while (nextSubtag() && isOtherExtension(getCurrentSubtag()));
  return true;
}

bool LanguageTagParser::hasMoreSubtags() const {
  return !subtags_.empty();
}
bool LanguageTagParser::nextSubtag() {
  assert(hasMoreSubtags() && "No more remaining subtags.");
  subtags_.pop_back();
  return hasMoreSubtags();
}

const std::u16string &LanguageTagParser::getCurrentSubtag() const {
  assert(hasMoreSubtags());
  return subtags_.back();
}

/* static */ std::optional<ParsedLocaleIdentifier>
ParsedLocaleIdentifier::parse(const std::u16string &localeId) {
  LanguageTagParser parser(localeId);
  return parser.parseUnicodeLocaleId();
}

// https://unicode.org/reports/tr35/#Canonical_Unicode_Locale_Identifiers
// https://tc39.es/ecma402/#sec-canonicalizeunicodelocaleid
std::u16string ParsedLocaleIdentifier::canonicalize() const {
  std::u16string canoLocaleId;

  // Append unicode_language_id
  {
    canoLocaleId.append(languageIdentifier.languageSubtag);
    assert(!canoLocaleId.empty() && "Valid tag with empty language subtag.");
    const auto &scriptSubtag = languageIdentifier.scriptSubtag;
    auto toAsciiUpper = [](char16_t c) -> char16_t {
      return (c >= u'a' && c <= u'z') ? c - u'a' + u'A' : c;
    };
    if (!scriptSubtag.empty()) {
      // Convert script subtag into title case.
      canoLocaleId.append({u'-', toAsciiUpper(scriptSubtag[0])});
      for (size_t i = 1; i < scriptSubtag.size(); i++)
        canoLocaleId += scriptSubtag[i];
    }
    if (!languageIdentifier.regionSubtag.empty()) {
      canoLocaleId += u'-';
      // Convert region subtag into uppercase.
      for (auto c : languageIdentifier.regionSubtag)
        canoLocaleId += toAsciiUpper(c);
    }
    for (const auto &subtag : languageIdentifier.variantSubtagList)
      canoLocaleId.append(u"-").append(subtag);
  }

  // Any extensions are in alphabetical order by their singleton (eg,
  // en-t-xxx-u-yyy, not en-u-yyy-t-xxx).

  // Append other extensions from a-s
  auto oExtIt = otherExtensionMap.begin();
  while (oExtIt != otherExtensionMap.end() && oExtIt->first < 't') {
    canoLocaleId.append({u'-', oExtIt->first});
    if (!oExtIt->second.empty())
      canoLocaleId.append(u"-").append(oExtIt->second);
    oExtIt++;
  }

  // Build transformed extension
  {
    std::u16string transformedExtension;

    // Append tlang
    if (!transformedLanguageIdentifier.languageSubtag.empty())
      transformedExtension.append(u"-").append(
          transformedLanguageIdentifier.languageSubtag);
    if (!transformedLanguageIdentifier.scriptSubtag.empty())
      transformedExtension.append(u"-").append(
          transformedLanguageIdentifier.scriptSubtag);
    if (!transformedLanguageIdentifier.regionSubtag.empty())
      transformedExtension.append(u"-").append(
          transformedLanguageIdentifier.regionSubtag);
    for (const auto &subtag : transformedLanguageIdentifier.variantSubtagList)
      transformedExtension.append(u"-").append(subtag);

    // Sort tfields by the alphabetical order of its keys
    for (const auto &p : transformedExtensionFields) {
      transformedExtension.append(u"-").append(p.first);
      if (!p.second.empty())
        transformedExtension.append(u"-").append(p.second);
    }

    // Append transformed extension singleton and extension
    if (transformedExtension.length() > 0)
      canoLocaleId.append(u"-t").append(transformedExtension);
  }

  // Build Unicode extension
  {
    std::u16string unicodeExtension;

    // Append Unicode attributes.
    for (const auto &attribute : unicodeExtensionAttributes)
      unicodeExtension.append(u"-").append(attribute);

    // Append Unicode keywords
    for (const auto &p : unicodeExtensionKeywords) {
      unicodeExtension.append(u"-").append(p.first);
      // Drop uvalue if it is "true".
      if (!p.second.empty() && p.second != u"true")
        unicodeExtension.append(u"-").append(p.second);
    }

    // Append Unicode singleton
    if (!unicodeExtension.empty())
      canoLocaleId.append(u"-u").append(unicodeExtension);
  }

  // Append remaining other extensions
  while (oExtIt != otherExtensionMap.end()) {
    canoLocaleId.append({u'-', oExtIt->first});
    if (!oExtIt->second.empty())
      canoLocaleId.append(u"-").append(oExtIt->second);
    oExtIt++;
  }

  // Append private use extensions
  if (!puExtensions.empty()) {
    canoLocaleId.append(u"-x-");
    canoLocaleId.append(puExtensions);
  }

  return canoLocaleId;
}

} // namespace platform_intl
} // namespace hermes
