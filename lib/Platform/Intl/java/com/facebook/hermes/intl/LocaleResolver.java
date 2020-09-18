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

        if (optionLocaleMatcher.equals("lookup")) {
            localeMatchResult = LocaleMatcher.lookupMatch(requestedLocales.toArray(new String[requestedLocales.size()]), availableLocales);
        } else {
            // Default is best-fit
            // Note that we don't pass the list of available locale ids for best fit match ... to avoid re-creation of ULocale for each of the IDs again. Instead, we directly call ULocale.getAvailableLocales() at the lowest platform aware method.
            // TODO :: Avoid fetching the available locales array in the preceeding code when best-fit locale matching is desired.
            localeMatchResult = LocaleMatcher.bestFitMatch(requestedLocales.toArray(new String[requestedLocales.size()]));
        }

        Set<String> supportedExtensionAdditionKeys = new HashSet<>();

        for( String key : relevantExtensionKeys) {

            Object value = JSObjects.Null();
            if (!localeMatchResult.extensions.isEmpty()) { // 9.h.

                if (localeMatchResult.extensions.containsKey(key)) { // 9.h.i
                    String requestedValue = localeMatchResult.extensions.get(key);

                    if(!requestedValue.isEmpty()) {
                        value = requestedValue;
                    } else {
                        value = JSObjects.newBoolean(true);
                    }
                    supportedExtensionAdditionKeys.add(key);
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

            value = UnicodeExtensionKeys.resolveKnownAliases(key, value);

            if(JSObjects.isString(value) && !UnicodeExtensionKeys.isValidKeyword(key, JSObjects.getJavaString(value), localeMatchResult.matchedLocale)) {
                result.put(key, JSObjects.Null());
                continue;
            }

            result.put(key, value);
        }

        for (String supportedExtendionKey : supportedExtensionAdditionKeys) {
            ArrayList<String> valueList = new ArrayList<>();
            String keyValue = localeMatchResult.extensions.get(supportedExtendionKey);

            keyValue = JSObjects.getJavaString(UnicodeExtensionKeys.resolveKnownAliases(supportedExtendionKey, JSObjects.newString(keyValue)));

            if(JSObjects.isString(keyValue) && !UnicodeExtensionKeys.isValidKeyword(supportedExtendionKey, JSObjects.getJavaString(keyValue), localeMatchResult.matchedLocale)) {
                continue;
            }

            valueList.add(keyValue);
            localeMatchResult.matchedLocale.setUnicodeExtensions(supportedExtendionKey, valueList);
        }

        result.put("locale", localeMatchResult.matchedLocale);

        return result;
    }
}
