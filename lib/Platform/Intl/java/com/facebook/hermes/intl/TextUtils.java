package com.facebook.hermes.intl;

import java.util.Arrays;
import java.util.function.Predicate;

public class TextUtils {
    public static boolean isAlphaNum(CharSequence name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        if(end >= name.length())
            return false;

        int length = end - start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx = start; idx <= end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c) && !Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    public static boolean isAlpha(CharSequence name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        if(end >= name.length())
            return false;

        int length = end - start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx = start; idx <= end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c)) {
                return false;
            }
        }

        return true;
    }

    public static boolean isDigit(CharSequence name, int start, int end, int min, int max) {
        assert (start >= 0 && end >= 0 && start <= name.length() && end <= name.length());

        if(end >= name.length())
            return false;

        int length = end - start + 1;
        if (length < min || length > max) {
            return false;
        }

        for (int idx = start; idx <= end; idx++) {
            char c = name.charAt(idx);
            if (!Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    public static boolean isDigitAlphanum3(CharSequence token, int start, int end) {
        return end - start + 1 == 4 && Character.isDigit(token.charAt(start)) && isAlphaNum(token, start + 1, end, 3, 3);
    }

    public static boolean isUnicodeLanguageSubtag(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{2,3} | alpha{5,8};
        return isAlpha(token, start, end, 2, 3) || isAlpha(token, start, end, 5, 8)
                || (end - start + 1 == 4 && token.charAt(start) == 'r' && token.charAt(start+1) == 'o' && token.charAt(start+2) == 'o' && token.charAt(start+3) == 't'); // "root" is a special valid language subtag
    }

    public static boolean isExtensionSingleton(CharSequence token, int start, int end) {
        return isAlphaNum(token, start, end, 1, 1);
    }

    public static boolean isUnicodeScriptSubtag(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alpha{4};
        return isAlpha(token, start, end, 4, 4);
    }

    public static boolean isUnicodeRegionSubtag(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        //= (alpha{2} | digit{3}) ;
        return isAlpha(token, start, end, 2, 2) || isDigit(token, start, end, 3, 3);
    }

    public static boolean isUnicodeVariantSubtag(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (alphanum{5,8}
        // | digit alphanum{3}) ;
        return isAlphaNum(token, start, end, 5, 8) || isDigitAlphanum3(token, start, end);
    }

    public static boolean isUnicodeExtensionAttribute(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (alphanum{3,8}
        return isAlphaNum(token, start, end, 3, 8);
    }

    public static boolean isUnicodeExtensionKey(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alphanum alpha ;

        if (end != start + 1)
            return false;

        return Character.isLetterOrDigit(token.charAt(start)) && Character.isAlphabetic(token.charAt(end));
    }

    public static boolean isUnicodeExtensionKeyTypeItem(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = alphanum alpha ;
        return isAlphaNum(token, start, end, 3, 8);
    }

    public static boolean isTranformedExtensionTKey(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        //  	= alpha digit ;
        if (end != start + 1)
            return false;

        return Character.isLetter(token.charAt(start)) && Character.isDigit(token.charAt(end));
    }

    public static boolean isTranformedExtensionTValueItem(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (sep alphanum{3,8})+ ;
        return isAlphaNum(token, start, end, 3, 8);
    }

    public static boolean isPrivateUseExtension(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (sep alphanum{1,8})+ ;
        return isAlphaNum(token, start, end, 1, 8);
    }

    public static boolean isOtherExtension(CharSequence token, int start, int end) {
        // https://unicode.org/reports/tr35/#Unicode_Language_and_Locale_Identifiers
        // = (sep alphanum{2,8})+ ;
        return isAlphaNum(token, start, end, 2, 8);
    }

    public static boolean containsString(String[] list, final String testValue) {
        return Arrays.stream(list).anyMatch(new Predicate<String>() {
            @Override
            public boolean test(String value) {
                return value.equals(testValue);
            }
        });
    }
}
