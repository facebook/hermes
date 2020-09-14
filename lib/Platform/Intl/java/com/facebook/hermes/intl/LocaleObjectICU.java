package com.facebook.hermes.intl;

import android.icu.util.ULocale;
import android.os.Build;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;

public class LocaleObjectICU implements ILocaleObject<ULocale> {

    private ULocale m_icuLocale = null;
    private ULocale.Builder m_icuLocaleBuilder = null;

    private boolean mIsDirty = false;

    private LocaleObjectICU(ULocale uLocale) {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        m_icuLocale = uLocale;
    }

    private LocaleObjectICU(String localeId) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);

        m_icuLocaleBuilder = new ULocale.Builder();

        try {
            m_icuLocaleBuilder.setLanguageTag(localeId);
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    private void ensureNotDirty() throws JSRangeErrorException {
        if (mIsDirty) {
            try {
                m_icuLocale = m_icuLocaleBuilder.build();
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
        String keywordValue = m_icuLocale.getKeywordValue(icuKey);
        if (keywordValue != null && !keywordValue.isEmpty())
            Collections.addAll(extensionList, keywordValue.split("-|_"));

        return extensionList;
    }

    @Override
    public HashMap<String, String> getUnicodeExtensions() throws JSRangeErrorException {
        ensureNotDirty();

        HashMap<String, String> keywordMap = new HashMap<>();
        Iterator<String> keywords = m_icuLocale.getKeywords();
        if(keywords != null ) {
            while (keywords.hasNext()) {
                String keyword = keywords.next();
                String canonicalKeyword = UnicodeExtensionKeys.ICUKeyToCanonicalKey(keyword);
                String value = m_icuLocale.getKeywordValue(keyword);
                keywordMap.put(canonicalKeyword, value);
            }
        }

        return keywordMap;
    }

    @Override
    public void setUnicodeExtensions(String key, ArrayList<String> value) throws JSRangeErrorException {
        ensureNotDirty();
        if (m_icuLocaleBuilder == null)
            m_icuLocaleBuilder = new ULocale.Builder().setLocale(m_icuLocale);

        try {
            m_icuLocaleBuilder.setUnicodeLocaleKeyword(key, TextUtils.join("-", value));
        } catch (RuntimeException ex) {
            throw new JSRangeErrorException(ex.getMessage());
        }

        mIsDirty = true;
    }

    @Override
    public ULocale getLocale() throws JSRangeErrorException {
        ensureNotDirty();
        return m_icuLocale;
    }

    public ULocale getLocaleWithoutExtensions() throws JSRangeErrorException {
        ensureNotDirty();
        ULocale.Builder localeBuilder = new ULocale.Builder();
        localeBuilder.setLocale(m_icuLocale);

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
        return new LocaleObjectICU(m_icuLocale);
    }

    public static ILocaleObject<ULocale> createFromLocaleId(String localeId) throws JSRangeErrorException {
        return new LocaleObjectICU(localeId);
    }

    public static ILocaleObject<ULocale> createFromULocale(ULocale uLocale) {
        return new LocaleObjectICU(uLocale);
    }

    public static ILocaleObject<ULocale> createDefault() {
        return new LocaleObjectICU(ULocale.getDefault(ULocale.Category.FORMAT));
    }
}
