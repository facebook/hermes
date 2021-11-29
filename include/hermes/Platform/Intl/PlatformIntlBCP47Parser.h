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
namespace bcp47_parser {

struct ParsedLanguageIdentifier {
  std::u16string languageSubtag;
  std::u16string scriptSubtag;
  std::u16string regionSubtag;
  std::vector<std::u16string> variantSubtagList;
};

struct ParsedLocaleIdentifier {
  ParsedLanguageIdentifier languageIdentifier;
  
  std::vector<std::u16string> unicodeExtensionAttributes;
  std::unordered_map<std::u16string, std::vector<std::u16string>> unicodeExtensionKeywords;
  
  ParsedLanguageIdentifier transformedLanguageIdentifier;
  std::unordered_map<std::u16string, std::vector<std::u16string>> transformedExtensionFields;

  std::unordered_map<char16_t, std::vector<std::u16string>> otherExtensionMap;
  
  std::vector<std::u16string> puExtensions;
};

llvh::Optional<ParsedLocaleIdentifier> parseLocaleId(const std::u16string &localeId);

} // namespace bcp47_parser
} // namespace hermes
#endif
