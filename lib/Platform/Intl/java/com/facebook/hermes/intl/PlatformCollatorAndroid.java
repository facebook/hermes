package com.facebook.hermes.intl;

import java.text.RuleBasedCollator;
import java.util.Locale;
import android.os.Build;

public class PlatformCollatorAndroid implements IPlatformCollator{

    private RuleBasedCollator mCollator = null;

    private PlatformCollatorAndroid(ILocaleObject<Locale> locale) throws JSRangeErrorException {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        mCollator = (RuleBasedCollator) RuleBasedCollator.getInstance(locale.getLocale());

        // Normalization is always on by the spec. We don't know whether the text is already normalized, hence we can't optimize as of now.
        mCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);
    }

    public static PlatformCollatorAndroid createFromLocale(ILocaleObject<Locale> locale) throws JSRangeErrorException {
        return new PlatformCollatorAndroid(locale);
    }

    @Override
    public int compare(String source, String target) {
        return mCollator.compare(source, target);
    }

    @Override
    public boolean isSensitiySupported(String sensitivity) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        if(sensitivity.equals(Constants.SENSITIVITY_CASE))
            return false;

        return true;
    }

    @Override
    public void setSensitivity(String sensitivity) {
        assert (Build.VERSION.SDK_INT < Build.VERSION_CODES.N);
        switch (sensitivity) {
            case Constants.SENSITIVITY_BASE:
                mCollator.setStrength(android.icu.text.Collator.PRIMARY);
                break;
            case Constants.SENSITIVITY_ACCENT:
                mCollator.setStrength(android.icu.text.Collator.SECONDARY);
                break;
            case Constants.SENSITIVITY_CASE:
                throw new UnsupportedOperationException("Unsupported Sensitivity option is Collator");
            case Constants.SENSITIVITY_VARIANT:
                mCollator.setStrength(android.icu.text.Collator.TERTIARY);
                break;
        }
    }

    @Override
    public void setIgnorePunctuation(boolean ignore) {
        throw new UnsupportedOperationException("IgnorePunctuation collation not supported !");
    }

    @Override
    public boolean isIgnorePunctuationSupported() {
        return false;
    }

    @Override
    public boolean isNumericCollationSupported() {
        return false;
    }

    @Override
    public void setNumericAttribute(boolean numeric) {
        throw new UnsupportedOperationException("Numeric collation not supported !");
    }

    @Override
    public boolean isCaseFirstCollationSupported() {
        return false;
    }

    @Override
    public void setCaseFirstAttribute(String caseFirst) {
        throw new UnsupportedOperationException("CaseFirst collation not supported !");
    }
}
