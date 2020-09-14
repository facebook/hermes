package com.facebook.hermes.intl;

import android.icu.text.CompactDecimalFormat;
import android.icu.text.MeasureFormat;
import android.icu.text.NumberFormat;
import android.icu.text.NumberingSystem;
import android.icu.util.Currency;
import android.icu.util.Measure;
import android.icu.util.MeasureUnit;
import android.icu.util.ULocale;

import java.math.BigDecimal;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.DECIMAL;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.UNIT;

// An implementation of NumberFormat services using ICU4J APIs available in Android from API 24 onwards.
// We could have a much richer implementation on newer API versions with // Note :: https://developer.android.com/reference/android/icu/number/NumberFormatter which was introduced in API 30.
//
// The implementation has at least the following deficiencies,
// 1. SignDisplay attribute implementation is partly hacky, and doesn't work in many cases.
// 2. formatToParts does report the whole formatted string as a "literal" for "unit" styling.
public class PlatformNumberFormatterICU implements IPlatformNumberFormatter {
    private java.text.Format mFinalFormat;
    private NumberFormat mNumberFormat;
    private LocaleObjectICU mLocaleObject;
    private IPlatformNumberFormatter.Style mStyle;

    private MeasureUnit mMeasureUnit = null;
    private String mUnitId = null;

    PlatformNumberFormatterICU() {}

    private void initialize(NumberFormat numberFormat, ILocaleObject localeObject, IPlatformNumberFormatter.Style style) {
        mNumberFormat = numberFormat;
        mFinalFormat = numberFormat;
        mLocaleObject = (LocaleObjectICU) localeObject;
        mStyle = style;

        mNumberFormat.setRoundingMode(BigDecimal.ROUND_HALF_UP);
    }

    @Override
    public PlatformNumberFormatterICU setCurrency(String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay) throws JSRangeErrorException {
        if (mStyle == CURRENCY) {

            Currency currency = Currency.getInstance(currencyCode);
            mNumberFormat.setCurrency(currency);

            String currencySymbol;
            if(currencyDisplay == CurrencyDisplay.CODE)
                currencySymbol = currencyCode;
            else
                currencySymbol = currency.getName((ULocale) mLocaleObject.getLocale(), currencyDisplay.getNameStyle(), null);

            if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
                android.icu.text.DecimalFormat decimalFormat = (android.icu.text.DecimalFormat) mNumberFormat;
                android.icu.text.DecimalFormatSymbols symbols = decimalFormat.getDecimalFormatSymbols();
                symbols.setCurrencySymbol(currencySymbol);
                decimalFormat.setDecimalFormatSymbols(symbols);
            }
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU setGrouping(boolean mGroupingUsed) {
        mNumberFormat.setGroupingUsed(mGroupingUsed);
        return this;
    }

    @Override
    public PlatformNumberFormatterICU setMinIntergerDigits(int minimumIntegerDigits) {
        if (minimumIntegerDigits != -1)
            mNumberFormat.setMinimumIntegerDigits(minimumIntegerDigits);

        return this;
    }

    @Override
    public PlatformNumberFormatterICU setSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException {
        if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
            if (roundingType == IPlatformNumberFormatter.RoundingType.SIGNIFICANT_DIGITS) {
                android.icu.text.DecimalFormat decimalFormat = (android.icu.text.DecimalFormat)mNumberFormat;
                if (minimumSignificantDigits >= 0)
                    decimalFormat.setMinimumSignificantDigits(minimumSignificantDigits);

                if (maximumSignificantDigits >= 0) {
                    if (maximumSignificantDigits < decimalFormat.getMinimumSignificantDigits())
                        throw new JSRangeErrorException("maximumSignificantDigits should be at least equal to minimumSignificantDigits");
                    decimalFormat.setMaximumSignificantDigits(maximumSignificantDigits);
                }

                decimalFormat.setSignificantDigitsUsed(true);
            }
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU setFractionDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits) {
        if (roundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {
            if (minimumFractionDigits >= 0)
                mNumberFormat.setMinimumFractionDigits(minimumFractionDigits);

            if (maximumFractionDigits >= 0)
                mNumberFormat.setMaximumFractionDigits(maximumFractionDigits);

            if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
                ((android.icu.text.DecimalFormat)mNumberFormat).setSignificantDigitsUsed(false);
            }
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU setSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay) {
        if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
            android.icu.text.DecimalFormat decimalFormat = (android.icu.text.DecimalFormat)mNumberFormat;
            android.icu.text.DecimalFormatSymbols symbols = decimalFormat.getDecimalFormatSymbols();

            switch (signDisplay) {
                case NEVER:
                    decimalFormat.setPositivePrefix("");
                    decimalFormat.setPositiveSuffix("");

                    decimalFormat.setNegativePrefix("");
                    decimalFormat.setNegativeSuffix("");

                    break;
                case ALWAYS:
                case EXCEPTZERO:

                    if (!decimalFormat.getNegativePrefix().isEmpty())
                        decimalFormat.setPositivePrefix(new String(new char[]{symbols.getPlusSign()}));

                    if (!decimalFormat.getNegativeSuffix().isEmpty())
                        decimalFormat.setPositiveSuffix(new String(new char[]{symbols.getPlusSign()}));

                    break;
            }
        }

        return this;
    }

    private static MeasureUnit parseUnit(String inUnit) throws JSRangeErrorException {

        // http://unicode.org/reports/tr35/tr35-general.html#Unit_Identifiers
        // Only core identifiers will be input
        for (MeasureUnit unit : MeasureUnit.getAvailable()) {

            if (unit.getSubtype().equals(inUnit) || unit.getSubtype().equals(unit.getType() + "-" + inUnit))
                return unit;
        }

        throw new JSRangeErrorException("Unknown unit: " + inUnit);
    }

    @Override
    public PlatformNumberFormatterICU setUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException {
        if (mStyle == Style.UNIT) {
            mUnitId = unit;
            mFinalFormat = MeasureFormat.getInstance((ULocale) mLocaleObject.getLocale(), unitDisplay.getFormatWidth(), mNumberFormat);
        }

        return this;
    }

    @Override
    public String format(double n) throws JSRangeErrorException {
        String result;
        if (mFinalFormat instanceof MeasureFormat && mUnitId != null) {

            if (mMeasureUnit == null)
                mMeasureUnit = parseUnit(mUnitId);

            result = mFinalFormat.format(new Measure(n, mMeasureUnit));
        } else
            result = mFinalFormat.format(n);

        return result;
    }

    @Override
    public String fieldToString(AttributedCharacterIterator.Attribute attribute, double x) {
        if (attribute == NumberFormat.Field.SIGN) {
            if (Double.compare(x, +0) >= 0) {
                return "plusSign";
            }
            return "minusSign";
        }
        if (attribute == NumberFormat.Field.INTEGER) {
            if (Double.isNaN(x)) {
                return "nan";
            }
            if (Double.isInfinite(x)) {
                return "infinity";
            }
            return "integer";
        }
        if (attribute == NumberFormat.Field.FRACTION) {
            return "fraction";
        }
        if (attribute == NumberFormat.Field.EXPONENT) {
            return "exponentInteger";
        }
        if (attribute == NumberFormat.Field.EXPONENT_SIGN) {
            return "exponentMinusSign";
        }
        if (attribute == NumberFormat.Field.EXPONENT_SYMBOL) {
            return "exponentSeparator";
        }
        if (attribute == NumberFormat.Field.DECIMAL_SEPARATOR) {
            return "decimal";
        }
        if (attribute == NumberFormat.Field.GROUPING_SEPARATOR) {
            return "group";
        }
        if (attribute == NumberFormat.Field.PERCENT) {
            return "percentSign";
        }
        if (attribute == NumberFormat.Field.PERMILLE) {
            return "permilleSign";
        }
        if (attribute == NumberFormat.Field.CURRENCY) {
            return "currency";
        }
        // Report unsupported/unexpected number fields as literal.
        return "literal";
    }

    @Override
    public AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException {
        AttributedCharacterIterator iterator;
        if (mFinalFormat instanceof MeasureFormat && mUnitId != null) {

            if (mMeasureUnit == null)
                mMeasureUnit = parseUnit(mUnitId);

            iterator = mFinalFormat.formatToCharacterIterator(new Measure(n, mMeasureUnit));
        } else {
            iterator = mFinalFormat.formatToCharacterIterator(n);
        }

        return iterator;
    }

    public static int getCurrencyDigits(String currencyCode) throws JSRangeErrorException {
        try {
            return Currency.getInstance(currencyCode).getDefaultFractionDigits();
        } catch (IllegalArgumentException ex) {
            throw new JSRangeErrorException("Invalid currency code !");
        }
    }

    @Override
    public PlatformNumberFormatterICU configure(ILocaleObject localeObject, String numberingSystem, IPlatformNumberFormatter.Style style,
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

        if(notation == Notation.COMPACT && (style == DECIMAL || style == UNIT )) { // TODO :: Note sure whether the compact notation makes sense for other styles ..
            CompactDecimalFormat.CompactStyle compactStyle = compactDisplay == IPlatformNumberFormatter.CompactDisplay.SHORT
                    ? android.icu.text.CompactDecimalFormat.CompactStyle.SHORT
                    : android.icu.text.CompactDecimalFormat.CompactStyle.LONG;

            NumberFormat numberFormat = android.icu.text.CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(), compactStyle);
            initialize(numberFormat, localeObject, style);
        } else {
            int numberFormatStyle = style.getInitialNumberFormatStyle(notation, currencySign);
            NumberFormat numberFormat = NumberFormat.getInstance((ULocale) localeObject.getLocale(), numberFormatStyle);

            if(notation == Notation.ENGINEERING) {
                // Must read the scientific style properties to understand this: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
                numberFormat.setMaximumIntegerDigits(3);
            }

            initialize(numberFormat, localeObject, style);
        }

        return this;
    }

    @Override
    public String[] getAvailableLocales() {
        ArrayList<String> availableLocaleIds = new ArrayList<>();
        java.util.Locale[] availableLocales = NumberFormat.getAvailableLocales();
        for(java.util.Locale locale: availableLocales) {
            availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
        }

        return availableLocaleIds.toArray(new String[availableLocaleIds.size()]);
    }

    @Override
    public String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException {
        return NumberingSystem.getInstance((ULocale) localeObject.getLocale()).getName();
    }
}
