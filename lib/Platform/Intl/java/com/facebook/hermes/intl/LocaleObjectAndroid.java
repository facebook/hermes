package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;

import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

public class LocaleObjectAndroid implements ILocaleObject<Locale> {

    private Locale mLocale = null;

    private ParsedLocaleIdentifier mParsedLocaleIdentifier = null;

    private boolean mIsDirty = false;

    private LocaleObjectAndroid(Locale locale) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        mLocale = locale;
    }

    private LocaleObjectAndroid(String localeId) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);

        mParsedLocaleIdentifier = LocaleIdentifier.parseLocaleId(localeId);
        reInitFromParsedLocaleIdentifier();
    }

    private LocaleObjectAndroid(ParsedLocaleIdentifier parsedLocaleIdentifier) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);

        mParsedLocaleIdentifier = parsedLocaleIdentifier;
        reInitFromParsedLocaleIdentifier();
    }


    private void ensureParsedLocaleIdentifier() throws JSRangeErrorException {
        if(mParsedLocaleIdentifier == null) {
            mParsedLocaleIdentifier = LocaleIdentifier.parseLocaleId(mLocale.toLanguageTag());
        }
    }

    private void reInitFromParsedLocaleIdentifier() throws JSRangeErrorException {

        StringBuffer localeIdBuffer = new StringBuffer();
        StringBuffer languageSubtagBuffer = new StringBuffer(), scriptSubtagBuffer = new StringBuffer(), regionSubtagBuffer = new StringBuffer();

        if (mParsedLocaleIdentifier.languageIdentifier.languageSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.languageSubtag.isEmpty())
            languageSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.languageSubtag);

        if (mParsedLocaleIdentifier.languageIdentifier.scriptSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.scriptSubtag.isEmpty())
            scriptSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.scriptSubtag);

        if (mParsedLocaleIdentifier.languageIdentifier.regionSubtag != null && !mParsedLocaleIdentifier.languageIdentifier.regionSubtag.isEmpty())
            regionSubtagBuffer.append(mParsedLocaleIdentifier.languageIdentifier.regionSubtag);

        LocaleIdentifier.replaceLanguageSubtagIfNeeded(languageSubtagBuffer, scriptSubtagBuffer, regionSubtagBuffer);

        if (languageSubtagBuffer.length() > 0)
            localeIdBuffer.append(languageSubtagBuffer.toString());

        if (scriptSubtagBuffer.length() > 0) {
            localeIdBuffer.append("-");
            localeIdBuffer.append(scriptSubtagBuffer.toString());
        }

        if (regionSubtagBuffer.length() > 0) {
            localeIdBuffer.append("-");
            localeIdBuffer.append(LocaleIdentifier.replaceRegionSubtagIfNeeded(regionSubtagBuffer));
        }

        if (mParsedLocaleIdentifier.languageIdentifier.variantSubtagList != null && !mParsedLocaleIdentifier.languageIdentifier.variantSubtagList.isEmpty()) {
            localeIdBuffer.append("-");
            localeIdBuffer.append(TextUtils.join("-", mParsedLocaleIdentifier.languageIdentifier.variantSubtagList));
        }

        // other extensions
        if (mParsedLocaleIdentifier.otherExtensionsMap != null) {
            for (Map.Entry<Character, ArrayList<String>> entry : mParsedLocaleIdentifier.otherExtensionsMap.entrySet()) {
                localeIdBuffer.append("-");
                localeIdBuffer.append(entry.getKey());
                localeIdBuffer.append("-");

                localeIdBuffer.append(TextUtils.join("-", entry.getValue()));
            }
        }

        // -t-extensions
        if (mParsedLocaleIdentifier.transformedLanguageIdentifier != null || mParsedLocaleIdentifier.transformedExtensionFields != null) {
            localeIdBuffer.append("-");
            localeIdBuffer.append('t');
            localeIdBuffer.append("-");


            StringBuffer transformedExtension = new StringBuffer();
            if (mParsedLocaleIdentifier.transformedLanguageIdentifier != null) {
                transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.languageSubtag);

                if (mParsedLocaleIdentifier.transformedLanguageIdentifier.scriptSubtag != null) {
                    transformedExtension.append("-");
                    transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.scriptSubtag);
                }

                if (mParsedLocaleIdentifier.transformedLanguageIdentifier.regionSubtag != null) {
                    transformedExtension.append("-");
                    transformedExtension.append(mParsedLocaleIdentifier.transformedLanguageIdentifier.regionSubtag);
                }

                if (mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList != null && !mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList.isEmpty()) {
                    transformedExtension.append("-");
                    transformedExtension.append(TextUtils.join("-", mParsedLocaleIdentifier.transformedLanguageIdentifier.variantSubtagList));
                }
            }

            if (mParsedLocaleIdentifier.transformedExtensionFields != null) {
                for (Map.Entry<String, ArrayList<String>> entry : mParsedLocaleIdentifier.transformedExtensionFields.entrySet()) {
                    String key = entry.getKey();
                    ArrayList<String> values = entry.getValue();

                    transformedExtension.append("-" + key);
                    for (String value : values)
                        transformedExtension.append("-" + value);
                }

                if (transformedExtension.length() > 0 && transformedExtension.charAt(0) == '-')
                    transformedExtension.deleteCharAt(0);
            }

            localeIdBuffer.append(transformedExtension.toString());

        }

        // -u-extensions
        if (mParsedLocaleIdentifier.unicodeExtensionAttributes != null || mParsedLocaleIdentifier.unicodeExtensionKeywords != null) {

            localeIdBuffer.append("-");
            localeIdBuffer.append('u');
            localeIdBuffer.append("-");

            // unicode extension attributes
            StringBuffer extension = new StringBuffer();
            if (mParsedLocaleIdentifier.unicodeExtensionAttributes != null)
                extension.append(TextUtils.join("-", mParsedLocaleIdentifier.unicodeExtensionAttributes));

            // unicode extension keywords
            if (mParsedLocaleIdentifier.unicodeExtensionKeywords != null) {
                for (Map.Entry<String, ArrayList<String>> entry : mParsedLocaleIdentifier.unicodeExtensionKeywords.entrySet()) {
                    String key = entry.getKey();
                    ArrayList<String> values = entry.getValue();

                    extension.append("-" + key);
                    for (String value : values)
                        extension.append("-" + value);
                }
            }

            if (extension.length() > 0 && extension.charAt(0) == '-')
                extension.deleteCharAt(0);

            localeIdBuffer.append(extension.toString());
        }

        // pu extension
        if (mParsedLocaleIdentifier.puExtensions != null) {
            localeIdBuffer.append("-");
            localeIdBuffer.append('x');
            localeIdBuffer.append("-");

            localeIdBuffer.append(TextUtils.join("-", mParsedLocaleIdentifier.puExtensions));
        }

        try {
            mLocale = Locale.forLanguageTag(localeIdBuffer.toString());
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = false;
    }


    private void ensureNotDirty() throws JSRangeErrorException {
        if (mIsDirty) {
            try {
                reInitFromParsedLocaleIdentifier();
            } catch (RuntimeException ex) {
                throw new JSRangeErrorException(ex.getMessage());
            }

            mIsDirty = false;
        }
    }

    @Override
    public ArrayList<String> getUnicodeExtensions(String key) throws JSRangeErrorException {
        ensureNotDirty();
        ensureParsedLocaleIdentifier();

        if(mParsedLocaleIdentifier.unicodeExtensionKeywords != null) {
            ArrayList<String> extensions = mParsedLocaleIdentifier.unicodeExtensionKeywords.get(key);
            if(extensions != null)
                return extensions;
        }
        return new ArrayList<>();
    }

    @Override
    public HashMap<String, String> getUnicodeExtensions() throws JSRangeErrorException {
        HashMap<String, String> extensions = new HashMap<>();
        if(mParsedLocaleIdentifier.unicodeExtensionKeywords != null) {
            for (String key : mParsedLocaleIdentifier.unicodeExtensionKeywords.keySet()) {
                ArrayList<String> values = mParsedLocaleIdentifier.unicodeExtensionKeywords.get(key);
                extensions.put(key, TextUtils.join("-", values));
            }
        }
        return extensions;
    }

    @Override
    public void setUnicodeExtensions(String key, ArrayList<String> value) throws JSRangeErrorException {
        ensureNotDirty();
        ensureParsedLocaleIdentifier();

        if(mParsedLocaleIdentifier.unicodeExtensionKeywords == null)
            mParsedLocaleIdentifier.unicodeExtensionKeywords = new TreeMap<>();

        if(!mParsedLocaleIdentifier.unicodeExtensionKeywords.containsKey(key))
            mParsedLocaleIdentifier.unicodeExtensionKeywords.put(key, new ArrayList<String>());

        // Remove all existing values .. TODO:: Double check whether this is correct, i.e. double check whether adding multiple values makes sense for any keys.
        mParsedLocaleIdentifier.unicodeExtensionKeywords.get(key).clear();

        mParsedLocaleIdentifier.unicodeExtensionKeywords.get(key).addAll(value);
        mIsDirty = true;
    }

    @Override
    public Locale getLocale() throws JSRangeErrorException {
        ensureNotDirty();
        return mLocale;
    }

    public Locale getLocaleWithoutExtensions() throws JSRangeErrorException {
        ensureNotDirty();
        ensureParsedLocaleIdentifier();

        ParsedLocaleIdentifier modified = new ParsedLocaleIdentifier();
        modified.languageIdentifier = mParsedLocaleIdentifier.languageIdentifier;
        return new LocaleObjectAndroid(modified).getLocale();
    }

    @Override
    public String toCanonicalTag() throws JSRangeErrorException {
        return getLocale().toLanguageTag();
    }

    @Override
    public String toCanonicalTagWithoutExtensions() throws JSRangeErrorException {
        return getLocaleWithoutExtensions().toLanguageTag();
    }

    public static ILocaleObject<Locale> createFromLocaleId(String localeId) throws JSRangeErrorException {
        return new LocaleObjectAndroid(localeId);
    }

    public static ILocaleObject<Locale> createFromLocale(Locale locale) {
        return new LocaleObjectAndroid(locale);
    }

    public static ILocaleObject<Locale> createDefault() {
        return new LocaleObjectAndroid(Locale.getDefault());
    }

    @Override
    public ILocaleObject<Locale> cloneObject() throws JSRangeErrorException {
        ensureNotDirty();
        return new LocaleObjectAndroid(mLocale);
    }
}
