package com.facebook.hermes.intl;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class LocaleResolver {
    // This method corresponds to https://tc39.es/ecma402/#sec-resolvelocale
    public static HashMap<String, Object> resolveLocale(String[] availableLocales, List<String> requestedLocales, Object options, List<String> relevantExtensionKeys) throws JSRangeErrorException {

        HashMap<String, Object> result = new HashMap<>();
        String optionLocaleMatcher = JSObjects.getJavaString(JSObjects.Get(options, "localeMatcher"));
        LocaleMatcher.LocaleMatchResult localeMatchResult;

//        if (optionLocaleMatcher.equals("best fit")) {
//            // android.icu.util.ULocale[] availableLocalesArray = ULocale.getAvailableLocales();
//            localeMatchResult = LocaleMatcher2.bestFitMatch(requestedLocales.toArray(new String[requestedLocales.size()]), availableLocales);
//        } else if (optionLocaleMatcher.equals("lookup")) {

//        ArrayList<String> availableLocalesIds = new ArrayList<>();
//        for (java.util.Locale availableLocale : availableLocales)
//            availableLocalesIds.add(availableLocale.toLanguageTag()); // TODO :: This doesn;t work on Platform <= 20

        localeMatchResult = LocaleMatcher.lookupMatch(requestedLocales.toArray(new String[requestedLocales.size()]), availableLocales);

        Set<String> supportedExtensionAdditionKeys = new HashSet<>();

        for( String key : relevantExtensionKeys) {

            Object value = JSObjects.Null();
            if (!localeMatchResult.extensions.isEmpty()) { // 9.h.

                if (localeMatchResult.extensions.containsKey(key)) { // 9.h.i
                    String requestedValue = localeMatchResult.extensions.get(key);

                    if(!requestedValue.isEmpty()) {
                        value = localeMatchResult.extensions.get(key);
                        supportedExtensionAdditionKeys.add(key);
                    } else {
                        value = JSObjects.newBoolean(true);
                        supportedExtensionAdditionKeys.add(key);
                    }
                }
            }

            if (JSObjects.getJavaMap(options).containsKey(key)) { // 9.i
                Object optionsValue = JSObjects.Get(options, key);
                if(JSObjects.isString(optionsValue)) {
                    if(JSObjects.getJavaString(optionsValue).isEmpty())
                        optionsValue = JSObjects.newBoolean(true);
                }

                if (!JSObjects.isUndefined(optionsValue) && !optionsValue.equals(value)) {
                    supportedExtensionAdditionKeys.remove(key);
                    value = optionsValue;
                }
            }

            if(key.equals("ca") && JSObjects.isString(value)) {
                value = UnicodeLocaleKeywordUtils.resolveCalendarAlias((String) value);
            }

            if(key.equals("nu") && JSObjects.isString(value)) {
                value = UnicodeLocaleKeywordUtils.resolveNumberSystemAlias((String) value);
            }

            if(key.equals("co") && JSObjects.isString(value)) {
                value = UnicodeLocaleKeywordUtils.resolveCollationAlias((String) value);
            }

            if(value.equals("yes"))
                value = "true";

            if(value.equals("no"))
                value = "false";


            if(JSObjects.isString(value) && !UnicodeLocaleKeywordUtils.isValidKeyword(key, JSObjects.getJavaString(value))) {
                result.put(key, JSObjects.Null());
                continue;
            }

            result.put(key, value);
        }

        for (String supportedExtendionKey : supportedExtensionAdditionKeys) {
            ArrayList<String> valueList = new ArrayList<>();
            String keyValue = localeMatchResult.extensions.get(supportedExtendionKey);

            if(supportedExtendionKey.equals("ca")) {
                keyValue = UnicodeLocaleKeywordUtils.resolveCalendarAlias(keyValue);
            }

            if(supportedExtendionKey.equals("nu")) {
                keyValue = UnicodeLocaleKeywordUtils.resolveNumberSystemAlias(keyValue);
            }

            if(supportedExtendionKey.equals("co")) {
                keyValue = UnicodeLocaleKeywordUtils.resolveCollationAlias(keyValue);
            }

            if(JSObjects.isString(keyValue) && !UnicodeLocaleKeywordUtils.isValidKeyword(supportedExtendionKey, JSObjects.getJavaString(keyValue))) {
                continue;
            }

            // TODO:: This needs to be standardized.
            if(keyValue.equals("yes"))
                keyValue = "";

            if(keyValue.equals("no"))
                keyValue = "false";


            valueList.add(keyValue);
            localeMatchResult.matchedLocale.setUnicodeExtensions(supportedExtendionKey, valueList);
        }

        result.put("locale", localeMatchResult.matchedLocale);

        return result;
    }
}
