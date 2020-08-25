package com.facebook.hermes.intl;

import java.util.HashMap;
import java.util.Map;

public class UnicodeLocaleKeywordUtils {
    // Ref: https://github.com/unicode-org/cldr/blob/release-37/common/bcp47/collation.xml#L12
    private static final Map<String, String > collationValueMappings = new HashMap<String, String>() {{
        put ("dictionary", "dict");
        put ("phonebook", "phonebk");
        put ("traditional", "trad");
        put ("gb2312han", "gb2312");
    }};

    public static String resolveCollationKeyword(String value) {
        if(!collationValueMappings.containsKey(value)) {
            return value;
        } else {
            return collationValueMappings.get(value);
        }
    }
}
