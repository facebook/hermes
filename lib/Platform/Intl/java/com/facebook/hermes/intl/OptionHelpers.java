package com.facebook.hermes.intl;

import java.util.Map;

public class OptionHelpers {
    public static String resolveStringOption(Map<String, Object> options, String key, String[] possibleValues, String defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            String optionValue = (String) options.get(key);
            if ((possibleValues.length == 0 ||  TextUtils.containsString(possibleValues, optionValue))
                && !optionValue.isEmpty()) {
                return optionValue;
            } else {
                throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionValue, key));
            }
        } else {
            return defaultValue;
        }
    }

    public static boolean resolveBooleanOption(Map<String, Object> options, String key, boolean defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            // Note:: Our JSI interop layer ensures that this object is indeed a boolean
            return (boolean) options.get(key);
        } else {
            return defaultValue;
        }
    }

    public static int DefaultNumberOption(int value, int minimum, int maximum, int fallback) throws JSRangeErrorException {
        if( value == -1)
            return fallback;

        if (value < minimum || value > maximum)
            throw new JSRangeErrorException("Integer option value not within range");

        return value;
    }

    public static int resolveIntegerOption(Map<String, Object> options, String key, int defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            return ((Double)options.get(key)).intValue();
        } else {
            return defaultValue;
        }
    }

    public static int resolveIntegerOption(Map<String, Object> options, String key, int minumum, int maximum, int defaultValue) throws JSRangeErrorException {
        if (options.containsKey(key)) {
            int value = ((Double)options.get(key)).intValue();
            if (value < minumum || value > maximum) {
                throw new JSRangeErrorException("Integer '" + key + "' option value not within range");
            }

            return value;
        } else {
            return defaultValue;
        }
    }


    public static <T extends Enum<?>> T searchEnum(Class<T> enumeration,
                                                   String search) {
        for (T each : enumeration.getEnumConstants()) {
            if (each.name().compareToIgnoreCase(search) == 0) {
                return each;
            }
        }
        return null;
    }
}
