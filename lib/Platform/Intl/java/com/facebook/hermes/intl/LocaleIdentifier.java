package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Locale;

public class LocaleIdentifier {

    private static void addSingletonSequence(String[] extensions, char singleton, StringBuffer extensionSequence) throws JSRangeErrorException{
        int singletonIndex = singleton - 'a';

        if (extensions[singletonIndex] != null) {
            throw new JSRangeErrorException("Duplicate singleton");
        }
        if (extensionSequence.length() == 0) {
            throw new JSRangeErrorException("Empty singleton");
        }

        extensions[singletonIndex] = extensionSequence.toString();
    }

    // Note this is a shallow parsing.. We don't parse the extension's kay and values.
    public static void parseExtensionSequence(StringBuffer extensionAndPrivateUseSequenceBuffer, String[] extensions) throws JSRangeErrorException{

        String[] extensionTokens = extensionAndPrivateUseSequenceBuffer.toString().split("-");
        char currentSingleton = Character.MIN_VALUE;
        StringBuffer currentExtensionSequence = new StringBuffer();
        for (int tokenIdx = 0; tokenIdx < extensionTokens.length; tokenIdx++) {
            String extensionToken = extensionTokens[tokenIdx];

            if (extensionToken.length() == 1) {
                char newSingleton = Character.toLowerCase(extensionToken.charAt(0));

                if (currentSingleton != Character.MIN_VALUE && currentSingleton != 'x') {
                    addSingletonSequence(extensions, currentSingleton, currentExtensionSequence);
                }

                currentSingleton = newSingleton;
                currentExtensionSequence.delete(0, currentExtensionSequence.length());

                if (newSingleton == 'x') {
                    // Consume the rest of the tokens.
                    tokenIdx++;
                    while (tokenIdx < extensionTokens.length) {
                        extensionToken = extensionTokens[tokenIdx];

                        if (currentExtensionSequence.length() > 0)
                            currentExtensionSequence.append('-');

                        if (!isAlphaNum(extensionToken, 0, extensionToken.length() - 1, 1, 8)) {
                            throw new JSRangeErrorException("Invalid singleton: " + extensionAndPrivateUseSequenceBuffer);
                        }

                        currentExtensionSequence.append(extensionToken.toLowerCase());
                        tokenIdx++;
                    }

                    addSingletonSequence(extensions, 'x', currentExtensionSequence);
                    break;
                }
            } else {
                if (currentExtensionSequence.length() > 0)
                    currentExtensionSequence.append('-');

                if (!isAlphaNum(extensionToken, 0, extensionToken.length() - 1, 1, 8)) {
                    throw new JSRangeErrorException("Invalid singleton: " + extensionAndPrivateUseSequenceBuffer);
                }

                currentExtensionSequence.append(extensionToken.toLowerCase());
            }
        }

        if (currentSingleton != Character.MIN_VALUE && currentSingleton != 'x') {
            addSingletonSequence(extensions, currentSingleton, currentExtensionSequence);
        }
    }

//    public static String constructLocaleIdFromSubtags(String inLocaleId, StringBuffer languageSubtagBuffer, StringBuffer scriptSubtagBuffer,
//                                                                      StringBuffer regionSubtagBuffer, ArrayList<String> variantSubtagList,
//                                                                      StringBuffer extensionAndPrivateUseSequenceBuffer) throws JSRangeErrorException{
//        // A quick comparative study with other implementations.
//        // The canonical way to canonicalize a localeId string is to roundtrip it through the icu::Locale object using forLanguageTag/toLanguageTag functions.
//        // V8 relies on icu4c implementation of the above functions, but augmented with private tables and code for handling special cases and error scenarios
//        // https://github.com/v8/v8/blob/4b9b23521e6fd42373ebbcb20ebe03bf445494f9/src/objects/intl-objects.cc
//        // Also, note that Chromium applies a few patches (https://chromium.googlesource.com/chromium/deps/icu/+/refs/heads/master/patches/) over icu, which may also result in subtle behaviour differences.
//        //
//        // Firefox doesn't seem to rely on ICU much but custom implemented most code and tables
//        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/Locale.cpp#1233
//        // Firefox has intentionally reimplemented them for performance as documented here:
//        // https://dxr.mozilla.org/mozilla-central/rev/c68fe15a81fc2dc9fc5765f3be2573519c09b6c1/js/src/builtin/intl/LanguageTag.cpp#1034
//        //
//        // Chakra official releases links to Windows.Globalization libraries available in Windows,
//        // But has an ICU variant which roundtrips the localId through icu::Locale as mentioned before with no other custom code around.
//        //
//        // Note:: icu4j is not a JNI wrapper around icu4c, but a reimplementation using Java.
//        // Even though they have similar APIs, there are subtle deviations for e.g. in error handling.
//        // Unlike the icu4c equivalents, the forLanguageTag in icu4j doesn't report any errors while parsing ..
//        // For e.g. icu4c identifies bogus locids.. and report failure when part of the loclid can't be parsed. (https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4c/source/common/locid.cpp#L816)
//        // This results in our implementation a bit more leniant (i.e. not throwing RangeError for certain invalid inputs) campared to V8/Chakra
//        // Both icu4c and icu4j implementations use private tables and mappings embedded in code for special cases such as deprecated and grandfathered  language code.
//        // icu4c: https://github.com/unicode-org/icu/blob/9219c6ae038a3556dffca880e93d2f2ca00e685a/icu4c/source/common/uloc_tag.cpp#L103
//        // icu4j: https://github.com/unicode-org/icu/blob/79fac501010d63231c258dc0d4fb9a9e87ddb8d8/icu4j/main/classes/core/src/com/ibm/icu/impl/locale/LanguageTag.java#L43
//        // Android-icu4j: https://android.googlesource.com/platform/external/icu/+/refs/heads/master/android_icu4j/
//        //
//        // Clearly icu4c implementation has a few more private tables implementated, for e.g. the deprecated languages and regions. This results in subtle behavioural differences between our implemenation and say, V8.
//        //
//        // As of now, Firefox seems to be by far the most compliant engine out there.
//        //
//        // Our current implementation is leveraging icu4j available with Android platform, but  augmented with our own structure validation and some table lookups (grandfathered tags, simple language and region replacements etc.).
//        //
//        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4j/com/ibm/icu/util/ULocale.html#forLanguageTag-java.lang.String-
//        // Ref: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1Locale.html#af76028775e37fd75a30209aaede551e2
//        // Ref: https://developer.android.com/reference/android/icu/util/ULocale.Builder
//
//        // We could have a much simpler implementation completely relying on icu4j. But that had the following deficiencies,
//        // 1. icu4j roundtripping with forLanguageTag constructor does almost no error validation. None of the negative test cases passes and many structurally invalid tags passes through.
//        // It essentially forces us to do structural validation ourselves.
//        // 2. Unicode CLDR has various mapping which needs to be applied prior to canonicalization.
//        //    icu implementations could either lookup those mappings in CLDR (not advisable) or keep those mapping privately in code. icu4c and icu4j does keep private mapping but to different degrees,
//        //    which results in subtle behaviour differences, even between different versions of ICU.
//        // 3. forLanguageTag constructor internally parses the tag again, which is wasteful as we've already parsed the tag for structure validation.
//        //    ULocale.Builder approach comes to rescue in icu4j as it allows us to create ULocale object using subtags, which we've got while parsing.
//        // 4. But, ULocale.Builder code path lacks some tag mappings, for e.g. grandfathered locale id, which forced us to add more private tables and mapping.
//        //
//        // Essentially, we ended up with slightly more complicated and less space efficient (due to added tables, but mostly static, not runtime allocations) traded against correctlness and predictability.
//
////        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
////
////            ULocale uLocale = constructLocaleFromSubtagsICU4j(inLocaleId, languageSubtagBuffer, scriptSubtagBuffer,
////                    regionSubtagBuffer, variantSubtagList,
////                    extensionAndPrivateUseSequenceBuffer);
////
////            return uLocale.toLanguageTag();
////
////        } else {
////
////            StringBuffer localeIdBuffer = new StringBuffer();
////
////            localeIdBuffer.append(languageSubtagBuffer);
////
////            if (scriptSubtagBuffer.length() > 0) {
////                localeIdBuffer.append('-');
////                localeIdBuffer.append(scriptSubtagBuffer);
////            }
////
////            if (regionSubtagBuffer.length() > 0) {
////                localeIdBuffer.append('-');
////                localeIdBuffer.append(regionSubtagBuffer);
////            }
////
////            if (!variantSubtagList.isEmpty()) {
////                localeIdBuffer.append('-');
////                localeIdBuffer.append(TextUtils.join("-", variantSubtagList));
////            }
////
////            if (extensionAndPrivateUseSequenceBuffer.length() > 0) {
////
////                if (extensionAndPrivateUseSequenceBuffer.charAt(extensionAndPrivateUseSequenceBuffer.length() - 1) == '-') {
////                    throw new JSRangeErrorException("Incomplete singleton");
////                }
////
////                String extensions[] = new String[26];
////                parseExtensionSequence(inLocaleId, extensionAndPrivateUseSequenceBuffer, extensions);
////
////                for (int ii = 0; ii < 26; ii++) {
////                    if (extensions[ii] != null && !extensions[ii].isEmpty()) {
////                        localeIdBuffer.append('-');
////                        localeIdBuffer.append((char) ('a' + ii));
////                        localeIdBuffer.append('-');
////                        localeIdBuffer.append(extensions[ii]);
////                    }
////                }
////            }
////
////            return localeIdBuffer.toString();
////        }
//
//        return LocaleObject.constructFromSubtags(languageSubtagBuffer, scriptSubtagBuffer, regionSubtagBuffer, variantSubtagList, extensionAndPrivateUseSequenceBuffer).toLocaleId();
//    }

    private static boolean isAlphaNum(String name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        return isAlphaNum(new StringBuffer(name), start, end, min, max);
    }


    private static boolean isAlphaNum(StringBuffer name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        int length = end-start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx=start; idx<=end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c) && !Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isAlpha(StringBuffer name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        int length = end-start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx=start; idx<=end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isDigit(StringBuffer name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        int length = end-start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx=start; idx<=end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isDigitAlphanum3(StringBuffer token, int start, int end) {
        return end - start + 1 == 4 && Character.isDigit(token.charAt(start)) && isAlphaNum(token, start + 1, end, 3, 3);
    }

    private static boolean isUnicodeLanguageSubtag(StringBuffer token, int[]currentTokenTerminals) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{2,3} | alpha{5,8};
        assert(currentTokenTerminals.length == 2);
        int start = currentTokenTerminals[0];
        int end = currentTokenTerminals[1];
        return isAlpha(token, start, end, 2, 3) || isAlpha(token, start, end, 5, 8);
    }

    private static boolean isExtensionSingleton(StringBuffer token, int start, int end) {
        return isAlphaNum(token, start, end, 1, 1);
    }

    private static boolean isUnicodeScriptSubtag(StringBuffer token, int[]currentTokenTerminals) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{4};
        assert(currentTokenTerminals.length == 2);
        int start = currentTokenTerminals[0];
        int end = currentTokenTerminals[1];
        return isAlpha(token, start, end, 4, 4);
    }

    private static boolean isUnicodeRegionSubtag(StringBuffer token, int[]currentTokenTerminals) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        //= (alpha{2} | digit{3}) ;
        assert(currentTokenTerminals.length == 2);
        int start = currentTokenTerminals[0];
        int end = currentTokenTerminals[1];
        return isAlpha(token, start, end,2, 2) || isDigit(token, start, end,3, 3);
    }

    private static boolean isUnicodeVariantSubtag(StringBuffer token, int[]currentTokenTerminals) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (alphanum{5,8}
        // | digit alphanum{3}) ;
        assert(currentTokenTerminals.length == 2);
        int start = currentTokenTerminals[0];
        int end = currentTokenTerminals[1];
        return isAlphaNum(token, start, end, 5, 8) || isDigitAlphanum3(token,start, end);
    }

    enum TransformType {
        ToCapital,
        ToSmall,
        ToTitle,
        None
    };

    private static void transformAndCopyString(StringBuffer destination, StringBuffer source, int start, int end, TransformType transform) {
        for(int idx=start;idx<=end; idx++) {
            switch (transform) {
                case ToCapital:
                    destination.append(Character.toUpperCase(source.charAt(idx)));
                    break;
                case ToSmall:
                    destination.append(Character.toLowerCase(source.charAt(idx)));
                    break;
                case ToTitle:
                    if(idx==start)
                        destination.append(Character.toUpperCase((source.charAt(idx))));
                    else
                        destination.append(Character.toLowerCase(source.charAt(idx)));
                    break;
                default:
                    destination.append(source.charAt(idx));
            }
        }
    }

    private static boolean isSubtagSeparator(char c) {
        // Note: LDML allows both "_" and "-" unlike BCP47 : https://unicode.org/reports/tr35/#BCP_47_Conformance

        // return c  == '-' || c  == '_';
        // Note:: Even though LDML allows '_' as separator, our reference test data doesn't allow it !
        return c  == '-';
    }

    // Populates "subTagTerminals" array with the terminal positions of next token
    // Returns false if something went unexpected.
    // Returns true if the next token is successfully identified or the end of input has reached.
    private static boolean findNextSubtag(StringBuffer tag, int[] subTagTerminals) {

        assert(subTagTerminals.length == 2);
        if (subTagTerminals[1] == tag.length() - 1) {
            subTagTerminals[0] = tag.length();
            subTagTerminals[1] = tag.length();
            return true;
        }

        // Seek forward to the start of next subtag if not the first one.
        if(subTagTerminals[1] > subTagTerminals[0]) {
            char nextChar = tag.charAt(subTagTerminals[1] + 1) ;
            if(!isSubtagSeparator(nextChar)) {
                return false;
            }

            subTagTerminals[0] = subTagTerminals[1] + 2;
        }

        int seek = subTagTerminals[0];
        for (;seek < tag.length(); seek++) {
            char currentChar = tag.charAt(seek);
            if(isSubtagSeparator(currentChar)){
                break;
            }
        }

        subTagTerminals[1] = seek-1;

        if(subTagTerminals[1] >= subTagTerminals[0])
            return true;
        else
            return false;
    }

    private static void normalizeVariantAndAddToListSorted(StringBuffer languageTag, int currentTokenTerminalStart,
                                                           int currentTokenTerminalEnd, ArrayList<String> variantSubtagList) throws JSRangeErrorException{
        StringBuffer variantSubtagBuffer = new StringBuffer();
        for(int i=currentTokenTerminalStart; i<=currentTokenTerminalEnd; i++)
            variantSubtagBuffer.append(Character.toLowerCase(languageTag.charAt(i)));

        String variantSubtag = variantSubtagBuffer.toString();
        int position = Collections.binarySearch(variantSubtagList, variantSubtag);
        if(position < 0) {
            variantSubtagList.ensureCapacity(variantSubtagList.size() + 1); // todo:: check whether this is needed ?
            variantSubtagList.add(-1 * position - 1, variantSubtag);
        } else {
            throw new JSRangeErrorException("Duplicate variant");
        }
    }

    public static void replaceLanguageSubtagIfNeeded(StringBuffer languageSubtagBuffer, StringBuffer scriptSubtagBuffer, StringBuffer regionSubtagBuffer) {

        String[] languageAliasKeys = null, languageAliasReplacements = null;
        String[] complexLanguageAliasKeys = null, complexLanguageAliasReplacementsLanguage = null, complexLanguageAliasReplacementsScript = null, complexLanguageAliasReplacementsRegion = null;

        if(languageSubtagBuffer.length() == 2) {
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
        if(found >= 0) {
            languageSubtagBuffer.delete(0, languageSubtagBuffer.length());
            languageSubtagBuffer.append(languageAliasReplacements[found]);
        } else {
            // Try complex replacement
            found = java.util.Arrays.binarySearch(complexLanguageAliasKeys, languageSubtagBuffer.toString());
            if(found >= 0) {
                String languageSubtagReplacement = complexLanguageAliasReplacementsLanguage[found];
                String scriptSubtagReplacement = complexLanguageAliasReplacementsScript[found];
                String regionSubtagReplacement = complexLanguageAliasReplacementsRegion[found];

                assert(languageSubtagReplacement != null && languageSubtagReplacement.length() > 0);
                // Overwrite languageSubtag buffer
                languageSubtagBuffer.delete(0, languageSubtagBuffer.length());
                languageSubtagBuffer.append(languageSubtagReplacement);

                if(scriptSubtagBuffer.length() == 0 && scriptSubtagReplacement != null) {
                    scriptSubtagBuffer.append(scriptSubtagReplacement);
                }

                if(regionSubtagBuffer.length() == 0 && regionSubtagReplacement != null) {
                    regionSubtagBuffer.append(regionSubtagReplacement);
                }
            }
        }
    }

    public static String replaceRegionSubtagIfNeeded(StringBuffer regionSubtag) {
        if(regionSubtag.length() == 2) {
            int found = java.util.Arrays.binarySearch(LanguageTagsGenerated.regionAliasKeys2, regionSubtag.toString());
            if(found >= 0) {
                return LanguageTagsGenerated.regionAliasReplacements2[found];
            } else {
                return regionSubtag.toString();
            }
        } else {
            int found = java.util.Arrays.binarySearch(LanguageTagsGenerated.regionAliasKeys3, regionSubtag.toString());
            if(found >= 0) {
                return LanguageTagsGenerated.regionAliasReplacements3[found];
            } else {
                return regionSubtag.toString();
            }
        }

        // Note: We don't do complex region replacement as it is expensive to do.
    }

    private static boolean noMoreTokensInLocaleId(int[] currentTokenTerminals, int tagLength){
        return currentTokenTerminals[0] == currentTokenTerminals[1] && currentTokenTerminals[0] == tagLength;
    }


    private static void copyLocaleStringToBufferNormalized(StringBuffer buffer, String locale) throws JSRangeErrorException{
        assert(buffer.length() == 0);

        // Normalize the string by
        // 1. lower casing
        // 2. Convert '_' to - [Nope: We are not doing it as test262 has tests which validates that the separator is hyphen]
        // 3. avoiding leading and trailing whitespaces [Nope; We are not doing it as test262 has tests which validates that the leading and trailing spaces should throw]

        // Seek past initial spaces.
        // int idx=0;
        // while (locale.charAt(idx) == ' ')idx++;
        if(locale.charAt(0) == ' ' || locale.charAt(locale.length()-1) == ' ')
            throw new JSRangeErrorException("Incorrect locale information provided");


        int idx=0;
        for(; idx<locale.length(); idx++) {
            char localeChar = locale.charAt(idx);

            //if(localeChar == '_') {
            //    buffer.append('-');
            //    continue;
            //}

            buffer.append(Character.toLowerCase(localeChar));
        }

        // idx=buffer.length()-1;
        // while (buffer.charAt(idx) == ' ') idx--;
        // buffer.delete(idx+1, buffer.length());
    }

    static String canonicalizeLocaleId(String inLocaleId) throws JSRangeErrorException{

//        StringBuffer languageSubtag = new StringBuffer(8);
//        StringBuffer scriptSubtag = new StringBuffer(4);
//        StringBuffer regionSubtag = new StringBuffer(4);
//        ArrayList<String> variantSubtagList = new ArrayList<>();
//        StringBuffer extensionAndPrivateUseSequence = new StringBuffer();
//
//        StringBuffer localeBuffer = new StringBuffer();
//        copyLocaleStringToBufferNormalized(localeBuffer, inLocaleId);
//
//        if(!LocaleIdentifier.canonicalizeLocaleIdIntoParts(localeBuffer, languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, extensionAndPrivateUseSequence)) {
//            throw new JSRangeErrorException(String.format("Incorrect locale information provided: %s", inLocaleId==null? "null":inLocaleId));
//        }
//
//        LocaleObject localeObject = LocaleObject.constructFromSubtags(languageSubtag, scriptSubtag, regionSubtag, variantSubtagList, extensionAndPrivateUseSequence);
//        return localeObject.toLocaleId();
        return LocaleObject.constructFromLocaleId(inLocaleId).toLocaleId();
    }

    // This method parses a given locale id to constituent subtags.
    // Note that the extension sequence is not yet parsed, but the whole extension sequence is returned as a buffer.
    // https://tc39.es/ecma402/#sec-isstructurallyvalidlanguagetag
    // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
    static boolean canonicalizeLocaleIdIntoParts(StringBuffer inLocaleId, StringBuffer outLanguageSubtagBuffer,
                                                 StringBuffer outScriptSubtagBuffer, StringBuffer outRegionSubtagBuffer,
                                                 ArrayList<String> outVariantSubtagList,
                                                 StringBuffer outExtensionSequenceBuffer) throws JSRangeErrorException{
        assert(outLanguageSubtagBuffer != null && outLanguageSubtagBuffer.length() == 0);
        assert(outScriptSubtagBuffer != null && outScriptSubtagBuffer.length() == 0);
        assert(outRegionSubtagBuffer != null && outRegionSubtagBuffer.length() ==0);
        assert(outExtensionSequenceBuffer != null && outExtensionSequenceBuffer.length() == 0);
        assert(outVariantSubtagList != null && outVariantSubtagList.isEmpty());

        // Handle grandfathered locales .. Essentially overwrite the input buffer with the alias.
        int grandfatheredIndex = java.util.Arrays.binarySearch(LanguageTagsGenerated.regularGrandfatheredKeys, inLocaleId.toString());
        if(grandfatheredIndex >= 0) {
            inLocaleId.delete(0, inLocaleId.length());
            inLocaleId.append(LanguageTagsGenerated.regularGrandfatheredReplacements[grandfatheredIndex]);
        }

        // Note: This array is essentially as iterator over the inLanguageTag
        int currentTokenTerminals[] = new int[2];

        int localeIdLength = inLocaleId.length();

        if(!findNextSubtag(inLocaleId, currentTokenTerminals))
            return false;

        // We assume the languageId starts with language subtag, but LDML allows languageId starting with script subtag: https://unicode.org/reports/tr35/#BCP_47_Conformance
        if (!isUnicodeLanguageSubtag(inLocaleId, currentTokenTerminals))
            return false;

        transformAndCopyString(outLanguageSubtagBuffer, inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1], TransformType.ToSmall);

        // Error while seeking next token
        if(!findNextSubtag(inLocaleId, currentTokenTerminals))
            return false; // Error while seeking next token

        if(noMoreTokensInLocaleId(currentTokenTerminals, localeIdLength))
            return true;

        // Note: According to BCP47, the language subtag can be followed by extlang subtags which are sequence of upto 3 3-letter alphabetic codes.
        // But unicode LDML spec disallows it: https://unicode.org/reports/tr35/#BCP_47_Conformance

        // https://en.wikipedia.org/wiki/IETF_language_tag#Extensions
        // We don;t bother the rest of the tags.
        if (isExtensionSingleton(inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1])) {
            transformAndCopyString(outExtensionSequenceBuffer, inLocaleId, currentTokenTerminals[0], localeIdLength - 1, TransformType.None);
            return true;
        }

        if (isUnicodeScriptSubtag(inLocaleId, currentTokenTerminals)) {
            transformAndCopyString(outScriptSubtagBuffer, inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1], TransformType.ToTitle);

            if(!findNextSubtag(inLocaleId, currentTokenTerminals))
                return false; // Error while seeking next token

            if(noMoreTokensInLocaleId(currentTokenTerminals, localeIdLength))
                return true;
        }

        if (isUnicodeRegionSubtag(inLocaleId, currentTokenTerminals)) {
            transformAndCopyString(outRegionSubtagBuffer, inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1], TransformType.ToCapital);

            if(!findNextSubtag(inLocaleId, currentTokenTerminals))
                return false;
        }

        do {

            if(noMoreTokensInLocaleId(currentTokenTerminals, localeIdLength))
                return true;

            if (isExtensionSingleton(inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1])) {
                transformAndCopyString(outExtensionSequenceBuffer, inLocaleId, currentTokenTerminals[0], localeIdLength - 1, TransformType.None);
                return true;
            }

            if (!isUnicodeVariantSubtag(inLocaleId, currentTokenTerminals))
                return false;
            else {
                // Canonical form of variants is lower case
                normalizeVariantAndAddToListSorted(inLocaleId, currentTokenTerminals[0], currentTokenTerminals[1], outVariantSubtagList);
            }

            if(!findNextSubtag(inLocaleId, currentTokenTerminals))
                return false;

        } while(true);
    }
}

