package com.facebook.hermes.intl;

import java.math.RoundingMode;
import java.text.AttributedCharacterIterator;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.NumberFormat;
import java.util.Currency;
import java.util.Locale;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;

public class PlatformNumberFormatterAndroid implements IPlatformNumberFormatter {

    private java.text.Format  mFinalFormat;
    private DecimalFormat mDecimalFormat;
    private ILocaleObject mLocaleObject;
    private IPlatformNumberFormatter.Style mStyle;

    private PlatformNumberFormatterAndroid(DecimalFormat decimalFormat, ILocaleObject localeObject, IPlatformNumberFormatter.Style style) {
        mDecimalFormat = decimalFormat;
        mFinalFormat = decimalFormat;
        mLocaleObject = localeObject;
        mStyle = style;
    }

    @Override
    public PlatformNumberFormatterAndroid configureCurrency(String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay) throws JSRangeErrorException {
        if (mStyle == CURRENCY) {

            Currency currency = Currency.getInstance(currencyCode);
            mDecimalFormat.setCurrency(currency);

            String currencySymbol;
            switch (currencyDisplay) {
                case NAME:
                    currencySymbol = currency.getDisplayName((Locale) mLocaleObject.getLocale());
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

            mDecimalFormat.getDecimalFormatSymbols().setCurrencySymbol(currencySymbol);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configureGrouping(boolean mGroupingUsed) {
        mDecimalFormat.setGroupingUsed(mGroupingUsed);
        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configureMinIntergerDigits(int minimumIntegerDigits) {
        if (minimumIntegerDigits != -1)
            mDecimalFormat.setMinimumIntegerDigits(minimumIntegerDigits);

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configureSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException {
        // Not supported.
        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configureFractinDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits) {
        if (roundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {
            if (minimumFractionDigits >= 0)
                mDecimalFormat.setMinimumFractionDigits(minimumFractionDigits);

            if (maximumFractionDigits >= 0)
                mDecimalFormat.setMaximumFractionDigits(maximumFractionDigits);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterAndroid configureSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay) {
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

    public PlatformNumberFormatterAndroid configureUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException {
        // Not supported.
        return this;
    }

    public static PlatformNumberFormatterAndroid createDecimalFormat(ILocaleObject localeObject, IPlatformNumberFormatter.Style style,
                                                                     IPlatformNumberFormatter.CurrencySign currencySign,
                                                                     IPlatformNumberFormatter.Notation notation,
                                                                     IPlatformNumberFormatter.CompactDisplay compactDisplay) throws JSRangeErrorException {

        NumberFormat numberFormat = NumberFormat.getInstance((Locale)localeObject.getLocale());
        numberFormat.setRoundingMode(RoundingMode.HALF_UP);

        return new PlatformNumberFormatterAndroid((DecimalFormat) numberFormat, localeObject, style);
    }


    public static String configureNumberingSystem(String inNumberingSystem, ILocaleObject locale) throws JSRangeErrorException {
        return "latn";
    }
}
