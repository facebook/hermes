package com.facebook.hermes.intl;

import android.icu.text.NumberingSystem;
import android.os.Build;

import java.math.RoundingMode;
import java.text.AttributedCharacterIterator;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Currency;
import java.util.Locale;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;

public class PlatformNumberFormatterAndroid implements IPlatformNumberFormatter {

    private java.text.Format  mFinalFormat;
    private DecimalFormat mDecimalFormat;
    private LocaleObjectAndroid mLocaleObject;
    private IPlatformNumberFormatter.Style mStyle;

    PlatformNumberFormatterAndroid() {}

    private void initialize(DecimalFormat decimalFormat, ILocaleObject localeObject, IPlatformNumberFormatter.Style style) {
        mDecimalFormat = decimalFormat;
        mFinalFormat = decimalFormat;
        mLocaleObject = (LocaleObjectAndroid) localeObject;
        mStyle = style;
    }

    @Override
    public PlatformNumberFormatterAndroid setCurrency(String currencyCode, CurrencyDisplay currencyDisplay) throws JSRangeErrorException {
        if (mStyle == CURRENCY) {

            Currency currency = Currency.getInstance(currencyCode);
            mDecimalFormat.setCurrency(currency);

            String currencySymbol;
            switch (currencyDisplay) {
                case NAME:
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
                        currencySymbol = currency.getDisplayName((Locale) mLocaleObject.getLocale());
                    } else {
                        currencySymbol = currency.getSymbol((Locale) mLocaleObject.getLocale());
                    }
                    break;
                case CODE:
                    currencySymbol = currencyCode;
                    break;
                case SYMBOL:
                case NARROWSYMBOL:
                default:
                    currencySymbol = currency.getSymbol((Locale) mLocaleObject.getLocale());
                    break;
            }

            DecimalFormatSymbols symbols = mDecimalFormat.getDecimalFormatSymbols();
            symbols.setCurrencySymbol(currencySymbol);
            mDecimalFormat.setDecimalFormatSymbols(symbols);

            mDecimalFormat.getDecimalFormatSymbols().setCurrencySymbol(currencySymbol);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid setGrouping(boolean mGroupingUsed) {
        mDecimalFormat.setGroupingUsed(mGroupingUsed);
        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid setMinIntergerDigits(int minimumIntegerDigits) {
        if (minimumIntegerDigits != -1)
            mDecimalFormat.setMinimumIntegerDigits(minimumIntegerDigits);

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid setSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException {
        // Not supported.
        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid setFractionDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits) {
        if (roundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {
            if (minimumFractionDigits >= 0)
                mDecimalFormat.setMinimumFractionDigits(minimumFractionDigits);

            if (maximumFractionDigits >= 0)
                mDecimalFormat.setMaximumFractionDigits(maximumFractionDigits);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid setSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay) {
        DecimalFormatSymbols symbols = mDecimalFormat.getDecimalFormatSymbols();

        switch (signDisplay) {
            case NEVER:
                mDecimalFormat.setPositivePrefix("");
                mDecimalFormat.setPositiveSuffix("");

                mDecimalFormat.setNegativePrefix("");
                mDecimalFormat.setNegativeSuffix("");

                break;
        }

        return this;
    }

    public static int getCurrencyDigits (String currencyCode) throws JSRangeErrorException {
        try {
            return Currency.getInstance(currencyCode).getDefaultFractionDigits();
        } catch (IllegalArgumentException ex) {
            throw new JSRangeErrorException("Invalid currency code !");
        }
    }

    @Override
    public String format(double n) {
        String result = mFinalFormat.format(n);
        return result;
    }

    @Override
    public String fieldToString(AttributedCharacterIterator.Attribute attribute, double x) {
        // Report unsupported/unexpected number fields as literal.
        return "literal";
    }

    @Override
    public AttributedCharacterIterator formatToParts(double n) {
        AttributedCharacterIterator result = mFinalFormat.formatToCharacterIterator(n);
        return result;
    }

    @Override
    public PlatformNumberFormatterAndroid setUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException {
        // Not supported.
        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configure(ILocaleObject localeObject, String numberingSystem, IPlatformNumberFormatter.Style style,
                                                    IPlatformNumberFormatter.CurrencySign currencySign,
                                                    IPlatformNumberFormatter.Notation notation,
                                                    IPlatformNumberFormatter.CompactDisplay compactDisplay) throws JSRangeErrorException {
        if (!numberingSystem.isEmpty()) {

            NumberingSystem numberingSystemObject;
            try {
                numberingSystemObject = NumberingSystem.getInstanceByName(JSObjects.getJavaString(numberingSystem));
            } catch (RuntimeException ex) {
                throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);
            }

            if (numberingSystemObject == null)
                throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);

            ArrayList<String> numberingSystemList = new ArrayList<>();
            numberingSystemList.add(JSObjects.getJavaString(numberingSystem));

            localeObject.setUnicodeExtensions("nu", numberingSystemList);
        }

        NumberFormat numberFormat = NumberFormat.getInstance((Locale)localeObject.getLocale());
        numberFormat.setRoundingMode(RoundingMode.HALF_UP);

        initialize((DecimalFormat) numberFormat, localeObject, style);

        return this;
    }


    public static String configureNumberingSystem(String inNumberingSystem, ILocaleObject locale) throws JSRangeErrorException {
        return "latn";
    }

    @Override
    public String[] getAvailableLocales() {

        if(Build.VERSION.SDK_INT < Build.VERSION_CODES.LOLLIPOP) {
            // Before L, Locale.toLanguageTag isn't available. Need to figure out how to get a locale id from locale object ... Currently resoring to support only en
            return new String []{"en"};
        }

        ArrayList<String> availableLocaleIds = new ArrayList<>();
        java.util.Locale[] availableLocales = NumberFormat.getAvailableLocales();
        for(java.util.Locale locale: availableLocales) {
            availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
        }

        return availableLocaleIds.toArray(new String[availableLocaleIds.size()]);
    }

    @Override
    public String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException {
        return "latn";
    }
}
