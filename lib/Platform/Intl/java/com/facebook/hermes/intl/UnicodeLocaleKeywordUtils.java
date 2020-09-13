package com.facebook.hermes.intl;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class UnicodeLocaleKeywordUtils {
    // Ref: https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml#L12
    private static final Map<String, String > s_collationAliasMappings = new HashMap<String, String>() {{
        put ("dictionary", "dict");
        put ("phonebook", "phonebk");
        put ("traditional", "trad");
        put ("gb2312han", "gb2312");
    }};

    public static String resolveCollationAlias(String value) {
        if (!s_collationAliasMappings.containsKey(value)) {
            return value;
        } else {
            return s_collationAliasMappings.get(value);
        }
    }

    private static Map<String, String> s_calendarAliasMappings = new HashMap<String, String>() {{
        // https://github.com/unicode-org/cldr/blob/master/common/bcp47/calendar.xml
        put ("gregorian", "gregory");
    }};

    private static Map<String, String> s_numberSystemAliasMappings = new HashMap<String, String>() {{
        // https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml
        put ("traditional", "traditio");
    }};


    public static String resolveCalendarAlias(String value) {
        if (!s_calendarAliasMappings.containsKey(value)) {
            return value;
        } else {
            return s_calendarAliasMappings.get(value);
        }
    }

    public static String resolveNumberSystemAlias(String value) {
        if (!s_numberSystemAliasMappings.containsKey(value)) {
            return value;
        } else {
            return s_numberSystemAliasMappings.get(value);
        }
    }

    private static Map<String, String[]> s_validKeywords = new HashMap<String, String[]>() {{
        // Ref:: https://tc39.es/ecma402/#table-numbering-system-digits
        // It is a subset of https://github.com/unicode-org/cldr/blob/master/common/bcp47/number.xml
        put("nu", new String[]{"adlm", "ahom", "arab", "arabext", "bali", "beng", "bhks", "brah", "cakm", "cham", "deva", "diak", "fullwide", "gong", "gonm", "gujr", "guru", "hanidec", "hmng", "hmnp", "java", "kali", "khmr", "knda", "lana", "lanatham", "laoo", "latn", "lepc", "limb", "mathbold", "mathdbl", "mathmono", "mathsanb", "mathsans", "mlym", "modi", "mong", "mroo", "mtei", "mymr", "mymrshan", "mymrtlng", "newa", "nkoo", "olck", "orya", "osma", "rohg", "saur", "segment", "shrd", "sind", "sinh", "sora", "sund", "takr", "talu", "tamldec", "telu", "thai", "tibt", "tirh", "vaii", "wara", "wcho"});

        // https://tc39.es/ecma402/#sec-intl-collator-internal-slots
        // -- Minus "standard" & "search" which are not allowed as per spec: https://tc39.es/ecma402/#sec-intl-collator-internal-slots
        put("co", new String[]{"big5han", "compat", "dict", "direct", "ducet", "emoji", "eor", "gb2312", "phonebk", "phonetic", "pinyin", "reformed", "searchjl", "stroke", "trad", "unihan", "zhuyin"});
    }};

    public static boolean isValidKeyword(String key, String value) {
        if (s_validKeywords.containsKey(key)) {
            return Arrays.asList(s_validKeywords.get(key)).contains(value);
        }

        return true;
    }
}
