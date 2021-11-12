/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_ENABLE_INTL
#include "hermes/VM/CallResult.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace hermes {
namespace platform_intl_parser {

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
  std::unordered_map<std::u16string, std::vector<std::u16string>>
      unicodeExtensionKeywords;

  ParsedLanguageIdentifier transformedLanguageIdentifier;
  std::unordered_map<std::u16string, std::vector<std::u16string>>
      transformedExtensionFields;

  std::unordered_map<char16_t, std::vector<std::u16string>> otherExtensionMap;
  std::vector<std::u16string> puExtensions;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class LanguageTagParser {
 public:
  LanguageTagParser(const std::u16string &localeId);
  LanguageTagParser();
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

} // namespace platform_intl_parser
} // namespace hermes
#endif
