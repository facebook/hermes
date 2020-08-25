package com.facebook.hermes.intl;

import android.icu.text.RuleBasedCollator;
import android.icu.util.ULocale;
import android.os.Build;

public class PlatformCollatorICU4J implements IPlatformCollator{

    private android.icu.text.RuleBasedCollator mCollator = null;

    private PlatformCollatorICU4J(ILocaleObject<ULocale> locale) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N);
        mCollator = (RuleBasedCollator) RuleBasedCollator.getInstance(locale.getLocale());

        // Normalization is always on by the spec. We don't know whether the text is already normalized, hence we can't optimize as of now.
        mCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);
    }

    public static PlatformCollatorICU4J createFromLocale(ILocaleObject<ULocale> locale) throws JSRangeErrorException {
        return new PlatformCollatorICU4J(locale);
    }

    @Override
    public int compare(String source, String target) {
        return mCollator.compare(source, target);
    }

    @Override
    public boolean isSensitiySupported(String sensitivity) {
        return true;
    }

    @Override
    public void setSensitivity(String sensitivity) {
        switch (sensitivity) {
            case Constants.SENSITIVITY_BASE:
                mCollator.setStrength(android.icu.text.Collator.PRIMARY);
                break;
            case Constants.SENSITIVITY_ACCENT:
                mCollator.setStrength(android.icu.text.Collator.SECONDARY);
                break;
            case Constants.SENSITIVITY_CASE:
                mCollator.setStrength(android.icu.text.Collator.PRIMARY);
                mCollator.setCaseLevel(true);
                break;
            case Constants.SENSITIVITY_VARIANT:
                mCollator.setStrength(android.icu.text.Collator.TERTIARY);
                break;
        }
    }

    @Override
    public void setIgnorePunctuation(boolean ignore) {
        // TODO:: According to documentation, it should take effect only when the strength is se to "QUATERNARY". Need to test it.
        mCollator.setAlternateHandlingShifted(ignore);
    }

    @Override
    public boolean isIgnorePunctuationSupported() {
        return true;
    }

    @Override
    public boolean isNumericCollationSupported() {
        return true;
    }

    @Override
    public void setNumericAttribute(boolean numeric) {
        mCollator.setNumericCollation(numeric);
    }

    @Override
    public boolean isCaseFirstCollationSupported() {
        return true;
    }

    @Override
    public void setCaseFirstAttribute(String caseFirst) {
        switch (caseFirst) {
            case "upper":
                mCollator.setUpperCaseFirst(true);
                break;

            case "lower":
                mCollator.setLowerCaseFirst(true);
                break;

            case "false":
            default:
                mCollator.setCaseFirstDefault();
                break;
        }
    }
}
