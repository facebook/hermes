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

public class LocaleObjectICU4J implements ILocaleObject<ULocale> {

    // On SDK >= N .. this object keeps the icu4j's ULocale and the ULocale.Builder used for building it.
    // All the methods must ensure that the icu4jLocale, icu4jLocaleBuilder & mParsedLocaleIdentifier are in sync on exit.
    private ULocale icu4jLocale = null;
    private ULocale.Builder icu4jLocaleBuilder = null;

    private boolean mIsDirty = false;

    private LocaleObjectICU4J(ULocale uLocale) {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        icu4jLocale = uLocale;
    }

    private LocaleObjectICU4J(String localeId) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);

        icu4jLocaleBuilder = new ULocale.Builder();

        try {
            icu4jLocaleBuilder.setLanguageTag(localeId);
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    private void ensureNotDirty() throws JSRangeErrorException {
        if (mIsDirty) {
            try {
                icu4jLocale = icu4jLocaleBuilder.build();
            } catch (RuntimeException ex) {
                throw new JSRangeErrorException(ex.getMessage());
            }

            mIsDirty = false;
        }
    }

    @Override
    public ArrayList<String> getVariants() throws JSRangeErrorException {
        ensureNotDirty();

        ArrayList<String> variantList = new ArrayList<>();

        String variant = icu4jLocale.getVariant();
        if (variant != null & !variant.isEmpty())
            Collections.addAll(variantList, variant.split("-|_"));

        return variantList;
    }

    @Override
    public ArrayList<String> getUnicodeExtensions(String key) throws JSRangeErrorException {
        ensureNotDirty();

        ArrayList<String> extensionList = new ArrayList<>();
        String keywordValue = icu4jLocale.getKeywordValue(key);
        if (keywordValue != null && !keywordValue.isEmpty())
            Collections.addAll(extensionList, keywordValue.split("-|_"));

        return extensionList;
    }

    @Override
    public void setVariant(ArrayList<String> newVariant) throws JSRangeErrorException {
        ensureNotDirty();
        if (icu4jLocaleBuilder == null)
            icu4jLocaleBuilder = new ULocale.Builder().setLocale(icu4jLocale);

        icu4jLocaleBuilder.setVariant(TextUtils.join("-", newVariant));
        mIsDirty = true;
    }

    @Override
    public void setUnicodeExtensions(String key, ArrayList<String> value) throws JSRangeErrorException {
        ensureNotDirty();
        if (icu4jLocaleBuilder == null)
            icu4jLocaleBuilder = new ULocale.Builder().setLocale(icu4jLocale);

        try {
                icu4jLocaleBuilder.setUnicodeLocaleKeyword(key, TextUtils.join("-", value));
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    @Override
    public ULocale getLocale() throws JSRangeErrorException {
        ensureNotDirty();
        return icu4jLocale;
    }

    public ULocale getLocaleWithoutExtensions() throws JSRangeErrorException {
        ensureNotDirty();
        ULocale.Builder localeBuilder = new ULocale.Builder();
        localeBuilder.setLocale(icu4jLocale);

        localeBuilder.clearExtensions();
        return localeBuilder.build();
    }

    @Override
    public String toCanonicalTag() throws JSRangeErrorException {
        return getLocale().toLanguageTag();
    }

    @Override
    public String toCanonicalTagWithoutExtensions() throws JSRangeErrorException {
        return getLocaleWithoutExtensions().toLanguageTag();
    }

    public static ILocaleObject<ULocale> createFromLocaleId(String localeId) throws JSRangeErrorException {
        return new LocaleObjectICU4J(localeId);
    }

    public static ILocaleObject<ULocale> createFromULocale(ULocale uLocale) {
        return new LocaleObjectICU4J(uLocale);
    }

    public static ILocaleObject<ULocale> createDefault() {
        return new LocaleObjectICU4J(ULocale.getDefault(ULocale.Category.FORMAT));
    }
}
