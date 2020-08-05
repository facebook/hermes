package com.facebook.hermes.intl;

import java.util.Arrays;
import java.util.function.Predicate;

public class Utils {
    public static boolean containsString(String[] list, final String testValue) {
        return Arrays.stream(list).anyMatch(new Predicate<String>() {
            @Override
            public boolean test(String value) {
                return value.equals(testValue);
            }
        });
    }
}
