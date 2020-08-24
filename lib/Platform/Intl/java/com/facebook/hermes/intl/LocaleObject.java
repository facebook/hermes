package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;

// A class which is supposed to wrap various represenations of a "locale".
// It is not designed as a representation of the Intl::Locale object as defined by https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Intl/Locale/Locale
public class LocaleObject {
    private ULocale icu4jLocale = null;
    private Locale legacylocale = null;

    private ParsedLocaleIdentifier mParsedLocaleIdentifier = null;

    private LocaleObject(ULocale uLocale) {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        icu4jLocale = uLocale;
    }

    private static boolean isAlphaNum(String name, int min, int max) {

        int length = name.length();
        if (length < min || length > max) {
            return false;
        }

        for (int idx = 0; idx < name.length(); idx++) {
            char c = name.charAt(idx);
            if (!Character.isLetter(c) && !Character.isDigit(c)) {
                return false;
            }
        }

        return true;
    }

    private void ensureParsedLocaleIdentifier() throws JSRangeErrorException {
        if(mParsedLocaleIdentifier == null) {
            String localeId = null;
            if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
                localeId = icu4jLocale.toLanguageTag();
            } else {
                localeId = legacylocale.toLanguageTag();
            }

            mParsedLocaleIdentifier = LocaleIdentifier.parseLocaleId(localeId);
        }
    }

    public TreeMap<String, ArrayList<String>>  getUnicodeExtensions() throws JSRangeErrorException {
        ensureParsedLocaleIdentifier();
        return mParsedLocaleIdentifier.unicodeExtensionKeywords;
    }

    private static boolean isUnicodeExtensionAttribute(String name) {
        return isAlphaNum(name, 3, 8);
    }

    // key = alphanum alpha;
    private static boolean isUnicodeExtensionkeywordKey(String name) {
        return name.length() == 2 && (Character.isLetter(name.charAt(0)) || Character.isDigit(name.charAt(0))) && Character.isLetter(name.charAt(1));
    }

    // type = alphanum{3,8}
    private static boolean isUnicodeExtensionkeywordType(String name) {
        return isAlphaNum(name, 3, 8);
    }

    public void addUnicodeExtension(String key, String type) throws JSRangeErrorException {
        ensureParsedLocaleIdentifier();

        if(mParsedLocaleIdentifier.unicodeExtensionKeywords == null)
            mParsedLocaleIdentifier.unicodeExtensionKeywords = new TreeMap<>();

        if(!mParsedLocaleIdentifier.unicodeExtensionKeywords.containsKey(key))
            mParsedLocaleIdentifier.unicodeExtensionKeywords.put(key, new ArrayList<String>());

        mParsedLocaleIdentifier.unicodeExtensionKeywords.get(key).add(type);

        // TODO :: This is an expensive way to add an extension ..
        reInitFromParsedLocaleIdentifier();
    }

    private void reInitFromParsedLocaleIdentifier() throws JSRangeErrorException {
        
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {

            ULocale.Builder localeBuilder = new ULocale.Builder();


            StringBuffer languageSubtagBuffer = new StringBuffer(), scriptSubtagBuffer = new StringBuffer(), regionSubtagBuffer = new StringBuffer();

            if(mParsedLocaleIdentifier.languageIdentifier.languageSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.languageSubtag.isEmpty())
                languageSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.languageSubtag);

            if (mParsedLocaleIdentifier.languageIdentifier.scriptSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.scriptSubtag.isEmpty())
                scriptSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.scriptSubtag);

            if (mParsedLocaleIdentifier.languageIdentifier.regionSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.regionSubtag.isEmpty())
                regionSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.regionSubtag);

            LocaleIdentifier.replaceLanguageSubtagIfNeeded(languageSubtagBuffer, scriptSubtagBuffer, regionSubtagBuffer);

            if(languageSubtagBuffer.length() > 0)
                localeBuilder.setLanguage(languageSubtagBuffer.toString());

            if (scriptSubtagBuffer.length() > 0)
                localeBuilder.setScript(scriptSubtagBuffer.toString());

            if (regionSubtagBuffer.length() > 0)
                localeBuilder.setRegion(LocaleIdentifier.replaceRegionSubtagIfNeeded(regionSubtagBuffer));

            if (mParsedLocaleIdentifier.languageIdentifier.variantSubtagList != null && !mParsedLocaleIdentifier.languageIdentifier.variantSubtagList.isEmpty()) {
                localeBuilder.setVariant(TextUtils.join("-", mParsedLocaleIdentifier.languageIdentifier.variantSubtagList));
            }

            if(mParsedLocaleIdentifier.unicodeExtensionAttributes != null) {
                for (CharSequence unicodeAttribute : mParsedLocaleIdentifier.unicodeExtensionAttributes) {
                    localeBuilder.addUnicodeLocaleAttribute(unicodeAttribute.toString());
                }
            }

            // unicode extension attributes
            StringBuffer extension = new StringBuffer();
            if(mParsedLocaleIdentifier.unicodeExtensionAttributes != null)
                extension.append(TextUtils.join("-", mParsedLocaleIdentifier.unicodeExtensionAttributes));

            // unicode extension keywords
            if(mParsedLocaleIdentifier.unicodeExtensionKeywords != null) {
                for (Map.Entry<String, ArrayList<String>> entry : mParsedLocaleIdentifier.unicodeExtensionKeywords.entrySet()) {
                    String key = entry.getKey();
                    ArrayList<String> values = entry.getValue();

                    extension.append("-" + key);
                    for (String value : values)
                        extension.append("-" + value);
                }
            }

            if(extension.length() > 0 && extension.charAt(0) == '-')
                extension.deleteCharAt(0);

            localeBuilder.setExtension('u', extension.toString());

            // -t-extensions
            StringBuffer transformedExtension = new StringBuffer();
            if(mParsedLocaleIdentifier.transformedLanguageIdentifier != null) {
                transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.languageSubtag);

                if(mParsedLocaleIdentifier.transformedLanguageIdentifier.scriptSubtag != null) {
                    transformedExtension.append("-");
                    transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.scriptSubtag);
                }

                if(mParsedLocaleIdentifier.transformedLanguageIdentifier.regionSubtag != null) {
                    transformedExtension.append("-");
                    transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.regionSubtag);
                }

                if (mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList != null && !mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList.isEmpty()) {
                    transformedExtension.append("-");
                    transformedExtension.append(TextUtils.join("-", mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList));
                }
            }

            if(mParsedLocaleIdentifier.transformedExtensionFields != null) {
                for (Map.Entry<String, ArrayList<String>> entry : mParsedLocaleIdentifier.transformedExtensionFields.entrySet()) {
                    String key = entry.getKey();
                    ArrayList<String> values = entry.getValue();

                    transformedExtension.append("-" + key);
                    for (String value : values)
                        transformedExtension.append("-" + value);
                }

                if(transformedExtension.length() > 0 && transformedExtension.charAt(0) == '-')
                    transformedExtension.deleteCharAt(0);
            }

            localeBuilder.setExtension('t', transformedExtension.toString());


            // pu extension
            if(mParsedLocaleIdentifier.puExtensions != null) {
                localeBuilder.setExtension('x', TextUtils.join("-", mParsedLocaleIdentifier.puExtensions));
            }

            // other extensions
            if(mParsedLocaleIdentifier.otherExtensionsMap != null) {
                for (Map.Entry<Character, ArrayList<String>> entry : mParsedLocaleIdentifier.otherExtensionsMap.entrySet()) {
                    localeBuilder.setExtension(entry.getKey(), TextUtils.join("-", entry.getValue()));
                }
            }

            try {
                icu4jLocale = localeBuilder.build();
            } catch (Exception ex) {
                throw new JSRangeErrorException("Unknown error parsing locale id.");
            }

        } else {

            StringBuffer localeIdBuffer = new StringBuffer();

            localeIdBuffer.append(mParsedLocaleIdentifier.languageIdentifier.languageSubtag);

            if (mParsedLocaleIdentifier.languageIdentifier.scriptSubtag.length() > 0) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(mParsedLocaleIdentifier.languageIdentifier.scriptSubtag);
            }

            if (mParsedLocaleIdentifier.languageIdentifier.regionSubtag.length() > 0) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(mParsedLocaleIdentifier.languageIdentifier.regionSubtag);
            }

            if (!mParsedLocaleIdentifier.languageIdentifier.variantSubtagList.isEmpty()) {
                localeIdBuffer.append('-');
                localeIdBuffer.append(TextUtils.join("-", mParsedLocaleIdentifier.languageIdentifier.variantSubtagList));
            }

            legacylocale = Locale.forLanguageTag(localeIdBuffer.toString());
        }
    }

    public LocaleObject(ParsedLocaleIdentifier parsedLocaleIdentifier) throws JSRangeErrorException {
        mParsedLocaleIdentifier = parsedLocaleIdentifier;
        reInitFromParsedLocaleIdentifier();
    }

    private LocaleObject(Locale locale) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        legacylocale = locale;
    }

    public static LocaleObject constructFromLocaleId(String localeId) throws JSRangeErrorException {
        ParsedLocaleIdentifier parsedLocaleIdentifier = LocaleIdentifier.parseLocaleId(localeId);
        return LocaleObject.constructFromParsedLocaleId(parsedLocaleIdentifier);
    }

    public static LocaleObject constructFromICU4jLocale(ULocale uLocale) {
        return new LocaleObject(uLocale);
    }

    public static LocaleObject constructFromLegacyLocale(Locale locale) {
        return new LocaleObject(locale);
    }

    public static LocaleObject constructFromParsedLocaleId(ParsedLocaleIdentifier parsedLocaleIdentifier) throws JSRangeErrorException {

        return new LocaleObject(parsedLocaleIdentifier);
    }

    // Construct a LocaleObject corresponding to the current "default" locale for the user ..
    public static LocaleObject constructDefault() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return new LocaleObject(ULocale.getDefault(ULocale.Category.FORMAT));
        } else {
            return new LocaleObject(Locale.getDefault(Locale.Category.FORMAT));
        }
    }

    public String toCanonicalLocaleId() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return icu4jLocale.toLanguageTag();
        } else {
            return legacylocale.toLanguageTag();
        }
    }

    public String toCanonicalLocaleIdForLocaleFiltering() throws JSRangeErrorException {

        // This is based on the following assumption
        // 1. The list of available locales returned by <locale|collator>.getAvailableULocales()
        // 1.a. don't contain variants and extensions
        // 1.b. are canonicalized .. i.e. <langsubtag in lower case>-<script in title case>-<region in capital>

        StringBuffer canonical = new StringBuffer();

        ensureParsedLocaleIdentifier();
        if(mParsedLocaleIdentifier.languageIdentifier.languageSubtag == null)
            throw new JSRangeErrorException ("LocaleId without language subtag !");

        canonical.append(mParsedLocaleIdentifier.languageIdentifier.languageSubtag);

        if(mParsedLocaleIdentifier.languageIdentifier.scriptSubtag != null) {
            canonical.append("-");
            canonical.append(mParsedLocaleIdentifier.languageIdentifier.scriptSubtag);
        }

        if(mParsedLocaleIdentifier.languageIdentifier.regionSubtag != null) {
            canonical.append("-");
            canonical.append(mParsedLocaleIdentifier.languageIdentifier.regionSubtag);
        }

        return canonical.toString();
    }

    public boolean matches(LocaleObject candidate) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && icu4jLocale.compareTo(candidate.icu4jLocale) == 0) {
            return true;
        }

        return false;
    }

    public ULocale getICU4jLocale() {
        if (Build.VERSION.SDK_INT <= Build.VERSION_CODES.N) {
            throw new UnsupportedOperationException("ICU4j locale object not available !");
        }

        assert (icu4jLocale != null);
        return icu4jLocale;
    }

    public Locale getLegacyLocale() {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.N) {
            throw new UnsupportedOperationException("ICU4j locale should be used instead of legacy Localy !");
        }

        assert (legacylocale != null);
        return legacylocale;
    }
}
