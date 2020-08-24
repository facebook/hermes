package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.TreeMap;

public class ParsedLocaleIdentifier {

    public class ParsedLanguageIdentifier {
        String languageSubtag;
        String scriptSubtag;
        String regionSubtag;
        ArrayList<String> variantSubtagList;
    }

    String originalLocaleId;

    ParsedLanguageIdentifier languageIdentifier;

    ArrayList<CharSequence> unicodeExtensionAttributes;
    TreeMap<String, ArrayList<String>> unicodeExtensionKeywords;

    ParsedLanguageIdentifier transformedLanguageIdentifier;
    TreeMap<String, ArrayList<String>> transformedExtensionFields;

    TreeMap<Character, ArrayList<String>> otherExtensionsMap;
    ArrayList<String> puExtensions;
}
