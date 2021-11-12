/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/PlatformIntlBCP47Parser.h"

namespace hermes {
namespace platform_intl_parser {

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

struct ParsedLocaleIdentifier::Impl {};

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

struct LanguageTagParser::Impl {
  Impl(const std::u16string &localeId) : mLocaleId(localeId){};
  ~Impl();

  ParsedLocaleIdentifier parsedLocaleIdentifier;
  std::u16string mLocaleId;
  size_t mSubtagStart;
  size_t mSubtagEnd;
};

LanguageTagParser::LanguageTagParser(const std::u16string &localeId) : impl_(std::make_unique<Impl>(localeId)) {
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

} // hermes
} // platform intl parser
