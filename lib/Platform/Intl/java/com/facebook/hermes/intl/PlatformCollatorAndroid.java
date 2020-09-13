package com.facebook.hermes.intl;

import java.text.RuleBasedCollator;
import java.util.ArrayList;
import java.util.Locale;

import android.icu.text.Collator;
import android.icu.util.ULocale;
import android.os.Build;

import static com.facebook.hermes.intl.IPlatformCollator.Sensitivity.*;
import static com.facebook.hermes.intl.IPlatformCollator.Sensitivity.VARIANT;

public class PlatformCollatorAndroid implements IPlatformCollator{

    private RuleBasedCollator mCollator = null;
    private ILocaleObject mLocale = null;

    PlatformCollatorAndroid() throws JSRangeErrorException {
    }

    @Override
    public IPlatformCollator setLocale(ILocaleObject localeObject) throws JSRangeErrorException {
        mLocale = localeObject;

        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        mCollator = (RuleBasedCollator) RuleBasedCollator.getInstance((Locale) mLocale.getLocale());

        // TODO :: I can't find a way to set the decomposition mode.
        // mCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);

        return this;
    }

    @Override
    public int compare(String source, String target) {
        return mCollator.compare(source, target);
    }

    @Override
    public Sensitivity getSensitivity() {
        if(mCollator == null) {
            return LOCALE; // TODO: Ad-hoc default
        }

        int strength = mCollator.getStrength();
        if(strength == android.icu.text.Collator.PRIMARY) {
            return BASE;
        }

        if(strength== Collator.SECONDARY)
            return ACCENT;

        return VARIANT;
    }

    @Override
    public IPlatformCollator setSensitivity(Sensitivity sensitivity) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);

        switch (sensitivity) {
            case BASE:
                mCollator.setStrength(android.icu.text.Collator.PRIMARY);
                break;
            case ACCENT:
                mCollator.setStrength(android.icu.text.Collator.SECONDARY);
                break;
            case VARIANT:
                mCollator.setStrength(android.icu.text.Collator.TERTIARY);
                break;
        }

        return this;
    }

    @Override
    public IPlatformCollator setIgnorePunctuation(boolean ignore) {
        return this;
    }

    @Override
    public IPlatformCollator setNumericAttribute(boolean numeric) {
        return this;
    }

    @Override
    public IPlatformCollator setCaseFirstAttribute(CaseFirst caseFirst) {
        return this;
    }

    @Override
    public String[] getAvailableLocales() {

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            // Before L, Locale.toLanguageTag isn't available. Need to figure out how to get a locale id from locale object ... Currently resoring to support only en
            return new String []{"en"};
        }

        ArrayList<String> availableLocaleIds = new ArrayList<>();
        java.util.Locale[] availableLocales = java.text.Collator.getAvailableLocales();
        for(java.util.Locale locale: availableLocales) {
            availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
        }

        return availableLocaleIds.toArray(new String[availableLocaleIds.size()]);
    }

}
