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
void toASCIILowerCase(std::u16string str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] <= 'Z' && str[i] >= 'A') {
      str[i] -= 'Z' - 'z';
    }
  }
  return;
}
void toASCIIUpperCase(std::u16string str) {
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] <= 'z' && str[i] >= 'a') {
      str[i] -= 'z' - 'Z';
    }
  }
  return;
}
void toASCIITitleCase(std::u16string str) {
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
  
  for (int i = start; i <= end; i++) {
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
bool isUnicodeExtensionKeyTypeItem(std::u16string str, int start, int end) {
  // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
  // = alphanum{3,8};
  return isCharType(str, start, end, 3, 8, &isASCIILetterOrDigit);
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
}

struct LanguageTagParser::Impl {
  Impl(const std::u16string &localeId) : mLocaleId(localeId){};
  //~Impl();

  ParsedLocaleIdentifier parsedLocaleIdentifier;
  std::u16string mLocaleId;
  int mSubtagStart;
  int mSubtagEnd;
};

LanguageTagParser::LanguageTagParser(const std::u16string &localeId) : impl_(std::make_unique<Impl>(localeId)) {
  impl_->mLocaleId = localeId;
  toASCIILowerCase(impl_->mLocaleId);
  impl_->mSubtagStart = 0;
  impl_->mSubtagEnd = -1;
}
// New is used 2 places: unicode extension keys and transformed extension tvalues
LanguageTagParser::~LanguageTagParser() = default;

ParsedLocaleIdentifier LanguageTagParser::getParsedLocaleId(){
  return impl_->parsedLocaleIdentifier;
}

bool LanguageTagParser::parseUnicodeLocaleId() {
  if (!parseUnicodeLanguageId())  {
    return false;
  }
  if (!hasMoreSubtags()) {
    return false; //true?
  }
  if (!parseExtensions()) {
    return false;
  }
  
  return true;
}

bool LanguageTagParser::parseUnicodeLanguageId() {
  if (!nextSubtag() || !isUnicodeLanguageSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    return false;
  }
  
  impl_->parsedLocaleIdentifier.languageIdentifier.languageSubtag = getCurrentSubtag();
  
  if (!nextSubtag()) {
    return true;
  }
  
  // handle extensions here? is this most common path?
  
  if (isUnicodeScriptSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    impl_->parsedLocaleIdentifier.languageIdentifier.scriptSubtag = getCurrentSubtag();
    toASCIITitleCase(impl_->parsedLocaleIdentifier.languageIdentifier.scriptSubtag);
    if (!nextSubtag()) {
      return true;
    }
  }
  
  if (isUnicodeRegionSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
    impl_->parsedLocaleIdentifier.languageIdentifier.regionSubtag = getCurrentSubtag();
    toASCIIUpperCase(impl_->parsedLocaleIdentifier.languageIdentifier.regionSubtag);
    if (!nextSubtag()) {
      return true;
    }
  }
  
  while (true) {
    if (!isUnicodeVariantSubtag(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
      return true;
    } else {
      if (!addVariantSubtag()) {
        return false;
      }
    }
    
    if (!nextSubtag()) {
      return true;
    }
  }
  
  return false;
}

bool LanguageTagParser::addVariantSubtag() {
  if (impl_->parsedLocaleIdentifier.languageIdentifier.variantSubtagList.empty()) {
    impl_->parsedLocaleIdentifier.languageIdentifier.variantSubtagList.push_back(getCurrentSubtag());
  } else {
    auto subtag = getCurrentSubtag();
    auto begin = impl_->parsedLocaleIdentifier.languageIdentifier.variantSubtagList.begin();
    auto end = impl_->parsedLocaleIdentifier.languageIdentifier.variantSubtagList.end();
    auto position = std::upper_bound(begin, end, subtag);
    if (position != std::upper_bound(begin, end, subtag)) {
      return false;
    }
    impl_->parsedLocaleIdentifier.languageIdentifier.variantSubtagList.insert(position, subtag);
  }
  return true;
}

bool LanguageTagParser::parseExtensions() {
  // if transformed extensions and next subtag is transformed extension
  // parse transformed extensions
  while (true) {
    // check if current subtag isExtensionSingleton

    if (impl_->mSubtagEnd != impl_->mSubtagStart) {
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
        if (!parseOtherExtension(singleton)) {
          return false;
        }
        break;
      }
    }
  }

  return false;
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
    std::vector<std::u16string> *extensionKeyTypes = new std::vector<std::u16string>();
    impl_->parsedLocaleIdentifier.unicodeExtensionKeywords.insert({key, extensionKeyTypes});
    
    if (!nextSubtag()) {
      return true;
    } else {
      while (isUnicodeExtensionKeyTypeItem(impl_->mLocaleId, impl_->mSubtagStart, impl_->mSubtagEnd)) {
        extensionKeyTypes->push_back(getCurrentSubtag());
        if (!nextSubtag()) {
          return true;
        }
      }
    }
  }
  
  return true;
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
// tkey =  	= alpha digit ;
//
// tvalue = (sep alphanum{3,8})+ ;
bool LanguageTagParser::parseTransformedExtension() { 
  bool hasExtension = false;
  if (isUnicodeLanguageSubtag()) {
    hasExtension = true;
    // parseUnicodeLanguageId(true);
    // tricky
    if (!hasMoreSubtags()) {
      return true;
    }
  } 

  if (isTransformedExtensionTKey()) {
    hasExtension = true;
    if (!impl_->parsedLocaleIdentifier.transformedExtensionFields.empty()) {
      return false;
    }

    while (true) {
      if (!isTransformedExtensionTKey()) {
        break;
      }
      std::u16string tkey = getCurrentSubtag();
      std::vector<std::u16string> *tvalues = new std::vector<std::u16string>();
      impl_->parsedLocaleIdentifier.transformedExtensionFields.insert({tkey, tvalues});
      
      // read key then one or more values
      if (!nextSubtag()) {
        return false;
      }
      if (!isTransformedExtensionTValueItem()) {
        return false;
      }
      tvalues->push_back(getCurrentSubtag());
      if (!nextSubtag()) {
        return true;
      }
      while (true) {
        if (!isTransformedExtensionTValueItem()) {
          break;
        }
        tvalues->push_back(getCurrentSubtag());
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
  if (!isPrivateUseExtension()) {
    return false;
  }
  impl_->parsedLocaleIdentifier.puExtensions.push_back(getCurrentSubtag());

  if (!nextSubtag()) {
    return true;
  }

  while (isPrivateUseExtension()) {
    impl_->parsedLocaleIdentifier.puExtensions.push_back(getCurrentSubtag());
    if (!nextSubtag()) {
      return true;
    }
  }

  // Tokens are not expected after pu extension
  return false;
}

// other_extensions= sep [alphanum-[tTuUxX]]
// (sep alphanum{2,8})+ ;
bool LanguageTagParser::parseOtherExtension(uchar16_t singleton) {
  std::unordered_map<char16_t, std::vector<std::u16string>*> extMap = impl_->parsedLocaleIdentifier.otherExtensionMap;
  if (extMap.find(singleton) != extMap.end()) {
    return false;
  }

  std::vector<std::u16string> *otherExtensions = new std::vector<std::u16string>();
  extMap.insert({singleton, otherExtensions});

  if (!isOtherExtension()) {
    return false;
  }
  otherExtensions->push_back(getCurrentSubtag());
  if (!nextSubtag()) {
    return true;
  }

  while (isOtherExtension()) {
    otherExtensions->push_back(getCurrentSubtag());
    if (!nextSubtag()) {
      return true;
    }
  }
  return false;
}

bool LanguageTagParser::hasMoreSubtags() {
  return impl_->mLocaleId.length() > 0 && impl_->mSubtagEnd < (int)impl_->mLocaleId.length() - 1;
}
bool LanguageTagParser::nextSubtag() {
  if (!hasMoreSubtags()) {
    return false;
  }
  
  int length = impl_->mLocaleId.length();
  
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
std::u16string LanguageTagParser::getCurrentSubtag() {
  return impl_->mLocaleId.substr(impl_->mSubtagStart, impl_->mSubtagEnd - impl_->mSubtagStart + 1);
}

} // hermes
} // platform intl parser
