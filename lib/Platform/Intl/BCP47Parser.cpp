/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Intl/BCP47Parser.h"

namespace hermes {
namespace bcp47_parser {
namespace {
using UTF16Ref = llvh::ArrayRef<char16_t>;

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
void toASCIILowerCase(std::u16string &str) {
  for (char16_t &c : str) {
    if (c <= 'Z' && c >= 'A') {
      c -= 'Z' - 'z';
    }
  }
  return;
}
void toASCIIUpperCase(std::u16string &str) {
  for (char16_t &c : str) {
    if (c <= 'z' && c >= 'a') {
      c -= 'z' - 'Z';
    }
  }
  return;
}
void toASCIITitleCase(std::u16string &str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (i == 0 && str[i] <= 'z' && str[i] >= 'a') {
      str[i] -= 'z' - 'Z';
    } else if (str[i] <= 'Z' && str[i] >= 'A') {
      str[i] -= 'Z' - 'z';
    }
  }
  return;
}
bool isSubtagSeparator(char16_t c) {
  return c == u'-';
}
bool isCharType(const UTF16Ref str, size_t min, size_t max, bool(*charType)(char16_t)) {
  size_t length = str.size();
  if (length < min || length > max) {
    return false;
  }
  
  for (char16_t c : str) {
    if (!charType(c)) {
      return false;
    }
  }
  return true;
}

// tag type functions
bool isUnicodeLanguageSubtag(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha{2,3} | alpha{5,8};
  // root case?
  return isCharType(subtagRef, 2, 3, &isASCIILetter) ||
    isCharType(subtagRef, 5, 8, &isASCIILetter);
}
bool isUnicodeScriptSubtag(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha{4};
  return isCharType(subtagRef, 4, 4, &isASCIILetter);
}
bool isUnicodeRegionSubtag(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (alpha{2} | digit{3});
  return isCharType(subtagRef, 2, 2, &isASCIILetter) ||
    isCharType(subtagRef, 3, 3, &isASCIIDigit);
}
bool isUnicodeVariantSubtag(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (alphanum{5,8} | digit alphanum{3});
  return isCharType(subtagRef, 5, 8, &isASCIILetterOrDigit) ||
    isCharType(subtagRef, 3, 3, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionAttribute(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum{3,8};
  return isCharType(subtagRef, 3, 8, &isASCIILetterOrDigit);
}
bool isUnicodeExtensionKey(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum alpha;
  return subtagRef.size() == 2 && isASCIILetterOrDigit(subtagRef.front()) && isASCIILetter(subtagRef.back());
}
bool isUnicodeExtensionKeyTypeItem(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum{3,8};
  return isCharType(subtagRef, 3, 8, &isASCIILetterOrDigit);
}
bool isTransformedExtensionKey(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alpha digit;
  return subtagRef.size() == 2 && isASCIILetter(subtagRef.front()) && isASCIIDigit(subtagRef.back());
}
bool isTransformedExtensionTValueItem(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{3,8})+;
  return isCharType(subtagRef, 3, 8, &isASCIILetterOrDigit);
}
bool isPrivateUseExtension(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{1,8})+;
  return isCharType(subtagRef, 1, 8, &isASCIILetterOrDigit);
}
bool isOtherExtension(const UTF16Ref subtagRef) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = (sep alphanum{2,8})+;
  return isCharType(subtagRef, 2, 8, &isASCIILetterOrDigit);
}
}

class LanguageTagParser {
 public:
  LanguageTagParser(const std::u16string &localeId);

  // public function declaration
  llvh::Optional<ParsedLocaleIdentifier> parseUnicodeLocaleId();

 private:
  // private function declaration
  bool parseUnicodeLanguageId(bool transformedExtensionId);
  bool addVariantSubtag(bool transformedExtensionId);
  bool parseExtensions();
  bool parseUnicodeExtension();
  bool parseTransformedExtension();
  bool parseOtherExtension(char16_t singleton);
  bool parsePUExtension();
  // tokenizer functions
  std::u16string getCurrentSubtag();
  UTF16Ref getCurrentSubtagRef();
  bool hasMoreSubtags();
  bool nextSubtag();
  
  ParsedLocaleIdentifier parsedLocaleIdentifier;
  std::u16string localeId_;
  int subtagStart_;
  int subtagEnd_;
};

LanguageTagParser::LanguageTagParser(const std::u16string &localeId) : localeId_{localeId}, subtagStart_(0), subtagEnd_(-1) {
  toASCIILowerCase(localeId_);
}

llvh::Optional<ParsedLocaleIdentifier> LanguageTagParser::parseUnicodeLocaleId() {
  if (!nextSubtag()) {
    return {};
  }
  if (!parseUnicodeLanguageId(false))  {
    return {};
  }
  if (!hasMoreSubtags()) {
    return parsedLocaleIdentifier;
  }
  if (!parseExtensions()) {
    return {};
  }
  if (hasMoreSubtags()) {
    return {};
  }
  return parsedLocaleIdentifier;
}

bool LanguageTagParser::parseUnicodeLanguageId(bool transformedExtensionId) {
  if (!transformedExtensionId && !isUnicodeLanguageSubtag(getCurrentSubtagRef())) {
    return false;
  }
  ParsedLanguageIdentifier &languageId = transformedExtensionId ? parsedLocaleIdentifier.transformedLanguageIdentifier : parsedLocaleIdentifier.languageIdentifier;
  
  languageId.languageSubtag = getCurrentSubtag();
  
  if (!nextSubtag()) {
    return true;
  }
  
  if (isUnicodeScriptSubtag(getCurrentSubtagRef())) {
    languageId.scriptSubtag = getCurrentSubtag();
    if (!transformedExtensionId) {
      toASCIITitleCase(languageId.scriptSubtag);
    }
    if (!nextSubtag()) {
      return true;
    }
  }

  if (isUnicodeRegionSubtag(getCurrentSubtagRef())) {
    languageId.regionSubtag = getCurrentSubtag();
    if (!transformedExtensionId) {
      toASCIIUpperCase(languageId.regionSubtag);
    }
    if (!nextSubtag()) {
      return true;
    }
  }
  
  while (true) {
    if (!isUnicodeVariantSubtag(getCurrentSubtagRef())) {
      return true;
    } else if (!addVariantSubtag(transformedExtensionId)) {
      return false;
    }
  
    if (!nextSubtag()) {
      return true;
    }
  }
}

bool LanguageTagParser::addVariantSubtag(bool transformedExtensionId) {
  ParsedLanguageIdentifier &languageId = transformedExtensionId ? parsedLocaleIdentifier.transformedLanguageIdentifier : parsedLocaleIdentifier.languageIdentifier;

  // Insert in alphabetical order
  if (languageId.variantSubtagList.empty()) {
    languageId.variantSubtagList.push_back(getCurrentSubtag());
  } else {
    auto subtag = getCurrentSubtag();
    auto begin = languageId.variantSubtagList.begin();
    auto end = languageId.variantSubtagList.end();
    auto position = std::upper_bound(begin, end, subtag);
    if (position != std::lower_bound(begin, end, subtag)) {
      return false;
    }
    languageId.variantSubtagList.insert(position, subtag);
  }
  return true;
}

bool LanguageTagParser::parseExtensions() {
  while (subtagEnd_ == subtagStart_) {
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
        // private use extension - must be the last extension
        if (!parsePUExtension()) {
          return false;
        }
        return true;
      }
      default: {
        // other extension
        if (!parseOtherExtension(singleton)) {
          return false;
        }
        break;
      }
    }
  }

  return true;
}

// unicode_locale_extensions = sep [uU]
// ((sep keyword)+
// |(sep attribute)+ (sep keyword)*) ;
//
// keyword = = key (sep type)? ;
//
// key = = alphanum alpha ;
//
// type = = alphanum{3,8}
//  (sep alphanum{3,8})* ;
//
// attribute = alphanum{3,8} ;
bool LanguageTagParser::parseUnicodeExtension() {
  if (!parsedLocaleIdentifier.unicodeExtensionAttributes.empty() ||
      !parsedLocaleIdentifier.unicodeExtensionKeywords.empty()) {
    return false;
  }
  
  bool hasKeywordOrAttribute = false;
  
  while (isUnicodeExtensionAttribute(getCurrentSubtagRef())) {
    hasKeywordOrAttribute = true;
    // Insert in sorted order
    if (parsedLocaleIdentifier.unicodeExtensionAttributes.empty()) {
      parsedLocaleIdentifier.unicodeExtensionAttributes.push_back(getCurrentSubtag());
    } else {
      auto subtag = getCurrentSubtag();
      auto begin = parsedLocaleIdentifier.unicodeExtensionAttributes.begin();
      auto end = parsedLocaleIdentifier.unicodeExtensionAttributes.end();
      auto position = std::upper_bound(begin, end, subtag);
      parsedLocaleIdentifier.unicodeExtensionAttributes.insert(position, subtag);
    }

    if (!nextSubtag()) {
      return true;
    }
  }
  
  while (isUnicodeExtensionKey(getCurrentSubtagRef())) {
    hasKeywordOrAttribute = true;
    
    std::u16string key = getCurrentSubtag();

    std::vector<std::u16string> *extensionKeyTypes = &parsedLocaleIdentifier.unicodeExtensionKeywords.insert({key, {}}).first->second;
    
    if (!nextSubtag()) {
      return true;
    } else {
      while (isUnicodeExtensionKeyTypeItem(getCurrentSubtagRef())) {
        extensionKeyTypes->push_back(getCurrentSubtag());
        if (!nextSubtag()) {
          return true;
        }
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
//
//  tkey = alpha digit ;
//
// tvalue = (sep alphanum{3,8})+ ;
bool LanguageTagParser::parseTransformedExtension() {
  if (!parsedLocaleIdentifier.transformedLanguageIdentifier.languageSubtag.empty() ||
      !parsedLocaleIdentifier.transformedExtensionFields.empty()) {
    return false;
  }
  
  bool hasExtension = false;
  if (isUnicodeLanguageSubtag(getCurrentSubtagRef())) {
    hasExtension = true;
    parseUnicodeLanguageId(true);
    if (!hasMoreSubtags()) {
      return true;
    }
  } 

  if (isTransformedExtensionKey(getCurrentSubtagRef())) {
    hasExtension = true;

    while (true) {
      if (!isTransformedExtensionKey(getCurrentSubtagRef())) {
        break;
      }
      
      std::u16string tkey = getCurrentSubtag();
      
      std::vector<std::u16string> &tvalues = parsedLocaleIdentifier.transformedExtensionFields.insert({tkey, {}}).first->second;
      
      // read key then one or more values
      if (!nextSubtag()) {
        return false;
      }
      if (!isTransformedExtensionTValueItem(getCurrentSubtagRef())) {
        return false;
      }
      tvalues.push_back(getCurrentSubtag());
      if (!nextSubtag()) {
        return true;
      }
      while (true) {
        if (!isTransformedExtensionTValueItem(getCurrentSubtagRef())) {
          break;
        }
        tvalues.push_back(getCurrentSubtag());
        if (!nextSubtag()) {
          return true;
        }
      }
    }
  } 

  return hasExtension;
}

// pu_extensions= sep [xX]
// (sep alphanum{1,8})+ ;
//
// No tokens may appear after pu_extensions
bool LanguageTagParser::parsePUExtension() {
  if (!isPrivateUseExtension(getCurrentSubtagRef())) {
    return false;
  }
  parsedLocaleIdentifier.puExtensions.push_back(getCurrentSubtag());

  if (!nextSubtag()) {
    return true;
  }

  while (isPrivateUseExtension(getCurrentSubtagRef())) {
    parsedLocaleIdentifier.puExtensions.push_back(getCurrentSubtag());
    if (!nextSubtag()) {
      return true;
    }
  }

  // Tokens are not expected after pu extension
  return false;
}

// other_extensions= sep [alphanum-[tTuUxX]]
// (sep alphanum{2,8})+ ;
bool LanguageTagParser::parseOtherExtension(char16_t singleton) {
  std::unordered_map<char16_t, std::vector<std::u16string>> &extMap = parsedLocaleIdentifier.otherExtensionMap;
  if (extMap.find(singleton) != extMap.end()) {
    return false;
  }
  
  extMap.insert({singleton, {}});
  std::vector<std::u16string> *otherExtensions = &extMap.find(singleton)->second;

  if (!isOtherExtension(getCurrentSubtagRef())) {
    return false;
  }
  otherExtensions->push_back(getCurrentSubtag());
  if (!nextSubtag()) {
    return true;
  }

  while (isOtherExtension(getCurrentSubtagRef())) {
    otherExtensions->push_back(getCurrentSubtag());
    if (!nextSubtag()) {
      return true;
    }
  }
  
  return true;
}

bool LanguageTagParser::hasMoreSubtags() {
  return localeId_.length() > 0 && subtagEnd_ < (int)localeId_.length() - 1;
}
bool LanguageTagParser::nextSubtag() {
  if (!hasMoreSubtags()) {
    return false;
  }
  
  size_t length = localeId_.length();
  
  if (subtagEnd_ >= subtagStart_) {
    if (subtagEnd_ + 2 == (int)length) {
      return false;
    }
    subtagStart_ = subtagEnd_ + 2;
  }
  
  for (subtagEnd_ = subtagStart_; subtagEnd_ < (int)length && !isSubtagSeparator(localeId_[subtagEnd_]); subtagEnd_++)
    ;
  
  if (subtagEnd_ > subtagStart_) {
    subtagEnd_--;
    return true;
  } else {
    return false;
  }
}

std::u16string LanguageTagParser::getCurrentSubtag() {
  return localeId_.substr(subtagStart_, subtagEnd_ - subtagStart_ + 1);
}
UTF16Ref LanguageTagParser::getCurrentSubtagRef() {
  return UTF16Ref(localeId_.data() + subtagStart_, subtagEnd_ - subtagStart_ + 1);
}

// Parses and returns locale id if it is a structurally valid language tag
llvh::Optional<ParsedLocaleIdentifier> parseLocaleId(const std::u16string &localeId) {
  LanguageTagParser parser(localeId);
  return parser.parseUnicodeLocaleId();
}

} // namespace bcp47_parser
} // namespace hermes
