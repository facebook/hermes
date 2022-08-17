/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.TreeMap;

public class ParsedLocaleIdentifier {

  public static class ParsedLanguageIdentifier {
    String languageSubtag;
    String scriptSubtag;
    String regionSubtag;
    ArrayList<String> variantSubtagList;
  }

  ParsedLanguageIdentifier languageIdentifier;

  ArrayList<CharSequence> unicodeExtensionAttributes;
  TreeMap<String, ArrayList<String>> unicodeExtensionKeywords;

  ParsedLanguageIdentifier transformedLanguageIdentifier;
  TreeMap<String, ArrayList<String>> transformedExtensionFields;

  TreeMap<Character, ArrayList<String>> otherExtensionsMap;
  ArrayList<String> puExtensions;
}
