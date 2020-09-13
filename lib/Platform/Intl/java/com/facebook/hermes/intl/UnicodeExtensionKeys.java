package com.facebook.hermes.intl;

import java.util.HashMap;

public class UnicodeExtensionKeys {
    public static String CALENDAR = "calendar";
    public static String CALENDAR_CANON = "ca";

    public static String NUMERINGSYSTEM = "numbers";
    public static String NUMERINGSYSTEM_CANON = "nu";

    public static String HOURCYCLE = "hours";
    public static String HOURCYCLE_CANON = "hc";

    public static String COLLATION = "collation";
    public static String COLLATION_CANON = "co";

    public static String COLLATION_NUMERIC = "colnumeric";
    public static String COLLATION_NUMERIC_CANON = "kn";

    public static String COLLATION_CASEFIRST = "colcasefirst"; // TODO:: double check this
    public static String COLLATION_CASEFIRST_CANON = "kf";

    // TODO :: Build/Look-for a BiMap datastructure.
    private static HashMap<String, String> s_canonicalkey_icukey_map;
    private static HashMap<String, String> s_icukey_canonicalkey_map;

    static {
        s_canonicalkey_icukey_map = new HashMap<String, String>() {{
            put(CALENDAR_CANON, CALENDAR);
            put(NUMERINGSYSTEM_CANON, NUMERINGSYSTEM);
            put(HOURCYCLE_CANON, HOURCYCLE);
            put(COLLATION_CANON, COLLATION);
            put(COLLATION_NUMERIC_CANON, COLLATION_NUMERIC);
            put(COLLATION_CASEFIRST_CANON, COLLATION_CASEFIRST);
        }};

        s_icukey_canonicalkey_map = new HashMap<String, String>() {{
            put(CALENDAR, CALENDAR_CANON);
            put(NUMERINGSYSTEM, NUMERINGSYSTEM_CANON);
            put(HOURCYCLE, HOURCYCLE_CANON);
            put(COLLATION, COLLATION_CANON);
            put(COLLATION_NUMERIC, COLLATION_NUMERIC_CANON);
            put(COLLATION_CASEFIRST, COLLATION_CASEFIRST_CANON);
        }};
    }

    public static String CanonicalKeyToICUKey(String key) {
        if(s_canonicalkey_icukey_map.containsKey(key))
            return s_canonicalkey_icukey_map.get(key);
        else
            return key; // TODO:: Make it tighter
    }

    public static String ICUKeyToCanonicalKey(String key) {
        if(s_icukey_canonicalkey_map.containsKey(key))
            return s_icukey_canonicalkey_map.get(key);
        else
            return key; // // TODO:: Make it tighter
    }
}
