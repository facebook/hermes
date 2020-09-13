package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;

public class LocaleObjectICU4J implements ILocaleObject<ULocale> {

    private ULocale m_icu4jLocale = null;
    private ULocale.Builder m_icu4jLocaleBuilder = null;

    private boolean mIsDirty = false;

    private LocaleObjectICU4J(ULocale uLocale) {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        m_icu4jLocale = uLocale;
    }

    private LocaleObjectICU4J(String localeId) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);

        m_icu4jLocaleBuilder = new ULocale.Builder();

        try {
            m_icu4jLocaleBuilder.setLanguageTag(localeId);
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    private void ensureNotDirty() throws JSRangeErrorException {
        if (mIsDirty) {
            try {
                m_icu4jLocale = m_icu4jLocaleBuilder.build();
            } catch (RuntimeException ex) {
                throw new JSRangeErrorException(ex.getMessage());
            }

            mIsDirty = false;
        }
    }

    @Override
    public ArrayList<String> getUnicodeExtensions(String key) throws JSRangeErrorException {
        ensureNotDirty();

        // nu -> numers
        // ca -> calendar
        String icuKey = UnicodeExtensionKeys.CanonicalKeyToICUKey(key);

        ArrayList<String> extensionList = new ArrayList<>();
        String keywordValue = m_icu4jLocale.getKeywordValue(icuKey);
        if (keywordValue != null && !keywordValue.isEmpty())
            Collections.addAll(extensionList, keywordValue.split("-|_"));

        return extensionList;
    }

    @Override
    public HashMap<String, String> getUnicodeExtensions() throws JSRangeErrorException {
        ensureNotDirty();

        HashMap<String, String> keywordMap = new HashMap<>();
        Iterator<String> keywords = m_icu4jLocale.getKeywords();
        if(keywords != null ) {
            while (keywords.hasNext()) {
                String keyword = keywords.next();
                String canonicalKeyword = UnicodeExtensionKeys.ICUKeyToCanonicalKey(keyword);
                String value = m_icu4jLocale.getKeywordValue(keyword);
                keywordMap.put(canonicalKeyword, value);
            }
        }

        return keywordMap;
    }

    @Override
    public void setUnicodeExtensions(String key, ArrayList<String> value) throws JSRangeErrorException {
        ensureNotDirty();
        if (m_icu4jLocaleBuilder == null)
            m_icu4jLocaleBuilder = new ULocale.Builder().setLocale(m_icu4jLocale);

        try {
            m_icu4jLocaleBuilder.setUnicodeLocaleKeyword(key, TextUtils.join("-", value));
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    @Override
    public ULocale getLocale() throws JSRangeErrorException {
        ensureNotDirty();
        return m_icu4jLocale;
    }

    public ULocale getLocaleWithoutExtensions() throws JSRangeErrorException {
        ensureNotDirty();
        ULocale.Builder localeBuilder = new ULocale.Builder();
        localeBuilder.setLocale(m_icu4jLocale);

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
        // return getLocale().getBaseName();
    }

    @Override
    public ILocaleObject<ULocale> cloneObject() throws JSRangeErrorException {
        ensureNotDirty();
        return new LocaleObjectICU4J(m_icu4jLocale);
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
