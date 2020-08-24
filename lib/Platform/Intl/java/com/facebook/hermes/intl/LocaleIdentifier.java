package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.TreeMap;

public class LocaleIdentifier {

    private static void addVariantSubtag(String variantSubtag, ParsedLocaleIdentifier.ParsedLanguageIdentifier parsedLanguageIdentifier) throws JSRangeErrorException {
        if (parsedLanguageIdentifier.variantSubtagList != null) {
            int position = Collections.binarySearch(parsedLanguageIdentifier.variantSubtagList, variantSubtag);
            if (position < 0) {
                // parsedLocaleIdentifier.languageIdentifier.variantSubtagList.ensureCapacity(variantSubtagList.size() + 1); // todo:: check whether this is needed ?
                parsedLanguageIdentifier.variantSubtagList.add(-1 * position - 1, variantSubtag);
            } else {
                throw new JSRangeErrorException("Duplicate variant");
            }
        } else {
            parsedLanguageIdentifier.variantSubtagList = new ArrayList<>();
            parsedLanguageIdentifier.variantSubtagList.add(variantSubtag);
        }
    }

    public static void replaceLanguageSubtagIfNeeded(StringBuffer languageSubtagBuffer, StringBuffer scriptSubtagBuffer, StringBuffer regionSubtagBuffer) {

        // If mappings are not available .. return early.
        if(LanguageTagsGenerated.languageAliasKeys2 == null)
            return;

        String[] languageAliasKeys = null, languageAliasReplacements = null;
        String[] complexLanguageAliasKeys = null, complexLanguageAliasReplacementsLanguage = null, complexLanguageAliasReplacementsScript = null, complexLanguageAliasReplacementsRegion = null;

        if (languageSubtagBuffer.length() == 2) {
            languageAliasKeys = LanguageTagsGenerated.languageAliasKeys2;
            languageAliasReplacements = LanguageTagsGenerated.languageAliasReplacements2;

            complexLanguageAliasKeys = LanguageTagsGenerated.complexLanguageAliasKeys2;
            complexLanguageAliasReplacementsLanguage = LanguageTagsGenerated.complexLanguageAliasReplacementsLanguage2;
            complexLanguageAliasReplacementsScript = LanguageTagsGenerated.complexLanguageAliasReplacementsScript2;
            complexLanguageAliasReplacementsRegion = LanguageTagsGenerated.complexLanguageAliasReplacementsRegion2;
        } else {
            languageAliasKeys = LanguageTagsGenerated.languageAliasKeys3;
            languageAliasReplacements = LanguageTagsGenerated.languageAliasReplacements3;

            complexLanguageAliasKeys = LanguageTagsGenerated.complexLanguageAliasKeys3;
            complexLanguageAliasReplacementsLanguage = LanguageTagsGenerated.complexLanguageAliasReplacementsLanguage3;
            complexLanguageAliasReplacementsScript = LanguageTagsGenerated.complexLanguageAliasReplacementsScript3;
            complexLanguageAliasReplacementsRegion = LanguageTagsGenerated.complexLanguageAliasReplacementsRegion3;
        }

        int found = java.util.Arrays.binarySearch(languageAliasKeys, languageSubtagBuffer.toString());
        if (found >= 0) {
            languageSubtagBuffer.delete(0, languageSubtagBuffer.length());
            languageSubtagBuffer.append(languageAliasReplacements[found]);
        } else {
            // Try complex replacement
            found = java.util.Arrays.binarySearch(complexLanguageAliasKeys, languageSubtagBuffer.toString());
            if (found >= 0) {
                String languageSubtagReplacement = complexLanguageAliasReplacementsLanguage[found];
                String scriptSubtagReplacement = complexLanguageAliasReplacementsScript[found];
                String regionSubtagReplacement = complexLanguageAliasReplacementsRegion[found];

                assert (languageSubtagReplacement != null && languageSubtagReplacement.length() > 0);
                // Overwrite languageSubtag buffer
                languageSubtagBuffer.delete(0, languageSubtagBuffer.length());
                languageSubtagBuffer.append(languageSubtagReplacement);

                if (scriptSubtagBuffer.length() == 0 && scriptSubtagReplacement != null) {
                    scriptSubtagBuffer.append(scriptSubtagReplacement);
                }

                if (regionSubtagBuffer.length() == 0 && regionSubtagReplacement != null) {
                    regionSubtagBuffer.append(regionSubtagReplacement);
                }
            }
        }
    }

    public static String replaceRegionSubtagIfNeeded(StringBuffer regionSubtag) {
        if(LanguageTagsGenerated.regionAliasKeys2 == null)
            return regionSubtag.toString();

        if (regionSubtag.length() == 2) {
            int found = java.util.Arrays.binarySearch(LanguageTagsGenerated.regionAliasKeys2, regionSubtag.toString());
            if (found >= 0) {
                return LanguageTagsGenerated.regionAliasReplacements2[found];
            } else {
                return regionSubtag.toString();
            }
        } else {
            int found = java.util.Arrays.binarySearch(LanguageTagsGenerated.regionAliasKeys3, regionSubtag.toString());
            if (found >= 0) {
                return LanguageTagsGenerated.regionAliasReplacements3[found];
            } else {
                return regionSubtag.toString();
            }
        }

        // Note: We don't do complex region replacement as it is expensive to do.
    }

    static String canonicalizeLocaleId(String inLocaleId) throws JSRangeErrorException {

        // A quick comparative study with other implementations.
        // The canonical way to canonicalize a localeId string is to roundtrip it through the icu::Locale object using forLanguageTag/toLanguageTag functions.
        // V8 relies on icu4c implementation of the above functions, but augmented with private tables and code for handling special cases and error scenarios
        // https://github.com/v8/v8/blob/4b9b23521e6fd42373ebbcb20ebe03bf445494f9/src/objects/intl-objects.cc
        // Also, note that Chromium applies a few patches (https://chromium.googlesource.com/chromium/deps/icu/+/refs/heads/master/patches/) over icu, which may also result in subtle behaviour differences.
        //
        // Firefox doesn't seem to rely on ICU much but custom implemented most code and tables
        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/Locale.cpp#1233
        // Firefox has intentionally reimplemented them for performance as documented here:
        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/LanguageTag.cpp#1034
        //
        // Chakra official releases links to Windows.Globalization libraries available in Windows,
        // But has an ICU variant which roundtrips the localId through icu::Locale as mentioned before with no other custom code around.
        //
        // Note:: icu4j is not a JNI wrapper around icu4c, but a reimplementation using Java.
        // Even though they have similar APIs, there are subtle deviations for e.g. in error handling.
        // Unlike the icu4c equivalents, the forLanguageTag in icu4j doesn't report any errors while parsing ..
        // For e.g. icu4c identifies bogus locids.. and report failure when part of the loclid can't be parsed. (https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4c/source/common/locid.cpp#L816)
        // This results in our implementation a bit more leniant (i.e. not throwing RangeError for certain invalid inputs) campared to V8/Chakra
        // Both icu4c and icu4j implementations use private tables and mappings embedded in code for special cases such as deprecated and grandfathered  language code.
        // icu4c: https://github.com/unicode-org/icu/blob/9219c6ae038a3556dffca880e93d2f2ca00e685a/icu4c/source/common/uloc_tag.cpp#L103
        // icu4j: https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4j/main/classes/core/src/com/ibm/icu/impl/locale/LanguageTag.java#L43
        // Android-icu4j: https://android.googlesource.com/platform/external/icu/+/refs/heads/master/android_icu4j/
        //
        // Clearly icu4c implementation has a few more private tables implementated, for e.g. the deprecated languages and regions. This results in subtle behavioural differences between our implemenation and say, V8.
        //
        // As of now, Firefox seems to be by far the most compliant engine out there.
        //
        // Our current implementation is leveraging icu4j available with Android platform, but  augmented with our own structure validation and some table lookups (grandfathered tags, simple language and region replacements etc.).
        //
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/ULocale.html#forLanguageTag-java.lang.String-
        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1Locale.html#af76028775e37fd75a30209aaede551e2
        // Ref: https://developer.android.com/reference/android/icu/util/ULocale.Builder

        // We could have a much simpler implementation completely relying on icu4j. But that had the following deficiencies,
        // 1. icu4j roundtripping with forLanguageTag constructor does almost no error validation. None of the negative test cases passes and many structurally invalid tags passes through.
        // It essentially forces us to do structural validation ourselves.
        // 2. Unicode CLDR has various mapping which needs to be applied prior to canonicalization.
        //    icu implementations could either lookup those mappings in CLDR (not advisable) or keep those mapping privately in code. icu4c and icu4j does keep private mapping but to different degrees,
        //    which results in subtle behaviour differences, even between different versions of ICU.
        // 3. forLanguageTag constructor internally parses the tag again, which is wasteful as we've already parsed the tag for structure validation.
        //    ULocale.Builder approach comes to rescue in icu4j as it allows us to create ULocale object using subtags, which we've got while parsing.
        // 4. But, ULocale.Builder code path lacks some tag mappings, for e.g. grandfathered locale id, which forced us to add more private tables and mapping.
        //
        // Essentially, we ended up with slightly more complicated and less space efficient (due to added tables, but mostly static, not runtime allocations) traded against correctlness and predictability.

        return LocaleObject.constructFromLocaleId(inLocaleId).toCanonicalLocaleId();
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
    static void parseUnicodeExtensions(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer,
                                       ParsedLocaleIdentifier parsedLocaleIdentifier) throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {
        if (!localeIdTokenizer.hasMoreSubtags())
            throw new JSRangeErrorException("Extension sequence expected.");

        LocaleIdTokenizer.LocaleIdSubtag nextSubtag = localeIdTokenizer.nextSubtag();

        if(parsedLocaleIdentifier.unicodeExtensionAttributes != null || parsedLocaleIdentifier.unicodeExtensionKeywords != null) {
            throw new JSRangeErrorException(String.format("Duplicate unicode extension sequence in [%s]", inLocaleId));
        }

        // Read out all attributes first ..
        while (nextSubtag.isUnicodeExtensionAttribute()) {
            if (parsedLocaleIdentifier.unicodeExtensionAttributes == null)
                parsedLocaleIdentifier.unicodeExtensionAttributes = new ArrayList<>();

            parsedLocaleIdentifier.unicodeExtensionAttributes.add(nextSubtag.toString());

            if (!localeIdTokenizer.hasMoreSubtags())
                return;

            nextSubtag = localeIdTokenizer.nextSubtag();
        }

        if (nextSubtag.isUnicodeExtensionKey()) {

            if (parsedLocaleIdentifier.unicodeExtensionKeywords == null) {
                parsedLocaleIdentifier.unicodeExtensionKeywords = new TreeMap<>();
            }

            do {
                String key = nextSubtag.toString();
                ArrayList<String> extensionKeyTypes = new ArrayList<>();
                parsedLocaleIdentifier.unicodeExtensionKeywords.put(key, extensionKeyTypes);

                // Read out all key types
                if (!localeIdTokenizer.hasMoreSubtags()) {
                    return;
                } else {
                    nextSubtag = localeIdTokenizer.nextSubtag();

                    while (nextSubtag.isUnicodeExtensionKeyTypeItem()) {
                        extensionKeyTypes.add(nextSubtag.toString());

                        if (!localeIdTokenizer.hasMoreSubtags())
                            return;

                        nextSubtag = localeIdTokenizer.nextSubtag();
                    }

                }
            } while (nextSubtag.isUnicodeExtensionKey());
        }

        if (nextSubtag.isExtensionSingleton()) {
            parseExtensions(inLocaleId, nextSubtag, localeIdTokenizer, parsedLocaleIdentifier);
            return;
        } else {
            throw new JSRangeErrorException("Malformed sequence expected.");
        }
    }

    static void parseTransformedExtensionFields(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, LocaleIdTokenizer.LocaleIdSubtag nextSubtag,
                                           ParsedLocaleIdentifier parsedLocaleIdentifier) throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if(nextSubtag.isTranformedExtensionTKey()) {

            if(parsedLocaleIdentifier.transformedExtensionFields != null) {
                throw new JSRangeErrorException(String.format("Duplicate transformed extension sequence in [%s]", inLocaleId));
            }

            if (parsedLocaleIdentifier.transformedExtensionFields == null) {
                parsedLocaleIdentifier.transformedExtensionFields = new TreeMap<>();
            }

            do {
                String tkey = nextSubtag.toString();
                ArrayList<String> tValues = new ArrayList<>();
                parsedLocaleIdentifier.transformedExtensionFields.put(tkey, tValues);

                // Read out all key types
                if (localeIdTokenizer.hasMoreSubtags()) {
                    nextSubtag = localeIdTokenizer.nextSubtag();

                    while (nextSubtag.isTranformedExtensionTValueItem()) {
                        tValues.add(nextSubtag.toString());

                        if (!localeIdTokenizer.hasMoreSubtags())
                            return;

                        nextSubtag = localeIdTokenizer.nextSubtag();
                    }

                } else {
                    throw new JSRangeErrorException(String.format("Malformated transformed key in : %s", inLocaleId));
                }

            } while (nextSubtag.isTranformedExtensionTKey());
        }

        if (nextSubtag.isExtensionSingleton()) {
            parseExtensions(inLocaleId, nextSubtag, localeIdTokenizer, parsedLocaleIdentifier);
            return;
        } else {
            throw new JSRangeErrorException("Malformed extension sequence.");
        }
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
    static void parseTransformedExtensions(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, ParsedLocaleIdentifier parsedLocaleIdentifier)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if (!localeIdTokenizer.hasMoreSubtags())
            throw new JSRangeErrorException("Extension sequence expected.");

        LocaleIdTokenizer.LocaleIdSubtag nextSubtag = localeIdTokenizer.nextSubtag();

        if(nextSubtag.isUnicodeLanguageSubtag()) {
            parseLanguageId(inLocaleId, localeIdTokenizer, nextSubtag, true, parsedLocaleIdentifier);
            // nextSubtag = localeIdTokenizer.currentSubtag();
        } else if (nextSubtag.isTranformedExtensionTKey()) {
            parseTransformedExtensionFields(inLocaleId, localeIdTokenizer, nextSubtag, parsedLocaleIdentifier);
        } else {
            throw new JSRangeErrorException(String.format("Unexpected token [%s] in transformed extension sequence [%s]", nextSubtag.toString(), inLocaleId));
        }
    }

    static void parsePrivateUseExtensions(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, ParsedLocaleIdentifier parsedLocaleIdentifier)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if (!localeIdTokenizer.hasMoreSubtags())
            throw new JSRangeErrorException("Extension sequence expected.");

        LocaleIdTokenizer.LocaleIdSubtag nextSubtag = localeIdTokenizer.nextSubtag();

        if (parsedLocaleIdentifier.puExtensions == null) {
            parsedLocaleIdentifier.puExtensions = new ArrayList<>();
        }

        while (nextSubtag.isPrivateUseExtension()) {

            parsedLocaleIdentifier.puExtensions.add(nextSubtag.toString());

            if (!localeIdTokenizer.hasMoreSubtags())
                return;

            nextSubtag = localeIdTokenizer.nextSubtag();
        }

        throw new JSRangeErrorException("Tokens are not expected after pu extension.");
    }

    static void parseOtherExtensions(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, ParsedLocaleIdentifier parsedLocaleIdentifier, char singleton)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if (!localeIdTokenizer.hasMoreSubtags())
            throw new JSRangeErrorException("Extension sequence expected.");

        LocaleIdTokenizer.LocaleIdSubtag nextSubtag = localeIdTokenizer.nextSubtag();

        if (parsedLocaleIdentifier.otherExtensionsMap == null) {
            parsedLocaleIdentifier.otherExtensionsMap = new TreeMap<>();
        }

        ArrayList<String> otherExtensions = new ArrayList<>();
        parsedLocaleIdentifier.otherExtensionsMap.put(new Character(singleton), otherExtensions);

        while (nextSubtag.isOtherExtension()) {

            otherExtensions.add(nextSubtag.toString());

            if (!localeIdTokenizer.hasMoreSubtags())
                return;

            nextSubtag = localeIdTokenizer.nextSubtag();
        }

        if (nextSubtag.isExtensionSingleton()) {
            parseExtensions(inLocaleId, nextSubtag, localeIdTokenizer, parsedLocaleIdentifier);
            return;
        } else {
            throw new JSRangeErrorException("Malformed sequence expected.");
        }
    }

    static void parseExtensions(CharSequence inLocaleId, LocaleIdTokenizer.LocaleIdSubtag singletonSubtag, LocaleIdTokenizer localeIdTokenizer, ParsedLocaleIdentifier parsedLocaleIdentifier)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if (!localeIdTokenizer.hasMoreSubtags())
            throw new JSRangeErrorException("Extension sequence expected.");

        char singleton = singletonSubtag.toString().charAt(0);

        if (singleton == 'u') {
            parseUnicodeExtensions(inLocaleId, localeIdTokenizer, parsedLocaleIdentifier);
        } else if (singleton == 't') {
            parseTransformedExtensions(inLocaleId, localeIdTokenizer, parsedLocaleIdentifier);
        } else if (singleton == 'x') {
            parsePrivateUseExtensions(inLocaleId, localeIdTokenizer, parsedLocaleIdentifier);
        } else {
            parseOtherExtensions(inLocaleId, localeIdTokenizer, parsedLocaleIdentifier, singleton);
        }
    }

    static boolean handleExtensions(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, LocaleIdTokenizer.LocaleIdSubtag nextSubtag, boolean transformedExtMode, ParsedLocaleIdentifier parsedLocaleIdentifier)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        if(transformedExtMode && nextSubtag.isTranformedExtensionTKey()) {
                parseTransformedExtensionFields(inLocaleId, localeIdTokenizer, nextSubtag, parsedLocaleIdentifier);
                return true;
            }

        // https://en.wikipedia.org/wiki/IETF_language_tag#Extensions
        if (nextSubtag.isExtensionSingleton()) {
            if(!transformedExtMode) {
                parseExtensions(inLocaleId, nextSubtag, localeIdTokenizer, parsedLocaleIdentifier);
                return true;
            } else {
                throw new JSRangeErrorException(String.format("Extension singletons in transformed extension language tag: %s", inLocaleId));
            }
        }

        return false;
    }

    static void parseLanguageId(CharSequence inLocaleId, LocaleIdTokenizer localeIdTokenizer, LocaleIdTokenizer.LocaleIdSubtag nextSubtag, boolean transformedExtMode, ParsedLocaleIdentifier parsedLocaleIdentifier)
            throws JSRangeErrorException, LocaleIdTokenizer.LocaleIdSubtagIterationFailed {

        ParsedLocaleIdentifier.ParsedLanguageIdentifier parsedLanguageIdentifier = parsedLocaleIdentifier.new ParsedLanguageIdentifier();

        if(transformedExtMode)
            parsedLocaleIdentifier.transformedLanguageIdentifier = parsedLanguageIdentifier;
        else
            parsedLocaleIdentifier.languageIdentifier = parsedLanguageIdentifier;

        try {
            // We expect at least one subtag.
            // We assume the languageId starts with language subtag, but LDML allows languageId starting with script subtag: https://unicode.org/reports/tr35/#BCP_47_Conformance
            if (!nextSubtag.isUnicodeLanguageSubtag())
                throw new JSRangeErrorException(String.format("Language subtag expected at %s: %s", nextSubtag.toString(), inLocaleId));

            parsedLanguageIdentifier.languageSubtag = nextSubtag.toLowerString();

            if (!localeIdTokenizer.hasMoreSubtags()) {
                return;
            }

            nextSubtag = localeIdTokenizer.nextSubtag();

            // Note: According to BCP47, the language subtag can be followed by extlang subtags which are sequence of upto 3 3-letter alphabetic codes.
            // But unicode LDML spec disallows it: https://unicode.org/reports/tr35/#BCP_47_Conformance

            if(handleExtensions(inLocaleId, localeIdTokenizer, nextSubtag, transformedExtMode, parsedLocaleIdentifier))
                return;

            if (nextSubtag.isUnicodeScriptSubtag()) {
                parsedLanguageIdentifier.scriptSubtag = nextSubtag.toTitleString();

                if (!localeIdTokenizer.hasMoreSubtags()) {
                    return;
                }

                nextSubtag = localeIdTokenizer.nextSubtag();
            }

            if (nextSubtag.isUnicodeRegionSubtag()) {
                parsedLanguageIdentifier.regionSubtag = nextSubtag.toUpperString();

                if (!localeIdTokenizer.hasMoreSubtags()) {
                    return;
                }
                nextSubtag = localeIdTokenizer.nextSubtag();
            }

            do {
                if(handleExtensions(inLocaleId, localeIdTokenizer, nextSubtag, transformedExtMode, parsedLocaleIdentifier))
                    return;

                if (!nextSubtag.isUnicodeVariantSubtag())
                    throw new JSRangeErrorException(String.format("Unknown token [%s] found in locale id: %s", nextSubtag.toString(), inLocaleId));
                else {
                    addVariantSubtag(nextSubtag.toString(), parsedLanguageIdentifier);
                }

                if (!localeIdTokenizer.hasMoreSubtags()) {
                    return;
                }
                nextSubtag = localeIdTokenizer.nextSubtag();

            } while (true);

        } catch (LocaleIdTokenizer.LocaleIdSubtagIterationFailed localeIdSubtagIterationFailed) {
            throw new JSRangeErrorException(String.format("Locale Identifier subtag iteration failed: %s", inLocaleId));
        }
    }

    static ParsedLocaleIdentifier parseLocaleId(String inLocaleId, LocaleIdTokenizer localeIdTokenizer) throws JSRangeErrorException {
        ParsedLocaleIdentifier parsedLocaleIdentifier = new ParsedLocaleIdentifier();

        try {

            if (!localeIdTokenizer.hasMoreSubtags())
                throw new JSRangeErrorException(String.format("Language subtag not found: %s", inLocaleId));

            LocaleIdTokenizer.LocaleIdSubtag nextSubtag = localeIdTokenizer.nextSubtag();

            parseLanguageId(inLocaleId, localeIdTokenizer, nextSubtag, false, parsedLocaleIdentifier);

            return parsedLocaleIdentifier;

        } catch (LocaleIdTokenizer.LocaleIdSubtagIterationFailed localeIdSubtagIterationFailed) {
            throw new JSRangeErrorException(String.format("Locale Identifier subtag iteration failed: %s", inLocaleId));
        }
    }

    // This method parses a given locale id to constituent subtags.
    // Note that the extension sequence is not yet parsed, but the whole extension sequence is returned as a buffer.
    // https://tc39.es/ecma402/#sec-isstructurallyvalidlanguagetag
    // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
    static ParsedLocaleIdentifier parseLocaleId(String inLocaleId) throws JSRangeErrorException {

        // Handle grandfathered locales ..
        int grandfatheredIndex = java.util.Arrays.binarySearch(LanguageTagsGenerated.regularGrandfatheredKeys, inLocaleId.toString());
        if (grandfatheredIndex >= 0) {
            inLocaleId = LanguageTagsGenerated.regularGrandfatheredReplacements[grandfatheredIndex];
        }

        // Normalize input to lower case.
        inLocaleId = inLocaleId.toLowerCase();

        LocaleIdTokenizer localeIdTokenizer = new LocaleIdTokenizer(inLocaleId);
        return parseLocaleId(inLocaleId, localeIdTokenizer);
    }
}

