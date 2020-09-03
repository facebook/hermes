package com.facebook.hermes.intl;

import android.icu.text.CompactDecimalFormat;
import android.icu.text.DecimalFormat;
import android.icu.text.DecimalFormatSymbols;
import android.icu.text.MeasureFormat;
import android.icu.text.NumberingSystem;
import android.icu.util.Currency;
import android.icu.util.Measure;
import android.icu.util.MeasureUnit;
import android.icu.util.ULocale;

import java.math.BigDecimal;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;

import static android.icu.text.NumberFormat.SCIENTIFICSTYLE;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;

// An implementation of NumberFormat services using ICU4J APIs available in Android from API 24 onwards.
// We could have a much richer implementation on newer API versions with // Note :: https://developer.android.com/reference/android/icu/number/NumberFormatter which was introduced in API 30.
//
public class PlatformNumberFormatterICU implements IPlatformNumberFormatter {

    private java.text.Format mFinalFormat;
    private DecimalFormat mDecimalFormat;
    private ILocaleObject mLocaleObject;
    private IPlatformNumberFormatter.Style mStyle;

    private MeasureUnit mMeasureUnit = null;
    private String mUnitId = null;

    private PlatformNumberFormatterICU(DecimalFormat decimalFormat, ILocaleObject localeObject, IPlatformNumberFormatter.Style style) {
        mDecimalFormat = decimalFormat;
        mFinalFormat = decimalFormat;
        mLocaleObject = localeObject;
        mStyle = style;

        mDecimalFormat.setRoundingMode(BigDecimal.ROUND_HALF_UP);
    }

    @Override
    public PlatformNumberFormatterICU configureCurrency(String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay) throws JSRangeErrorException {
        if (mStyle == CURRENCY) {

            Currency currency = Currency.getInstance(currencyCode);
            mDecimalFormat.setCurrency(currency);

            String currencySymbol;
            switch (currencyDisplay) {
                case NAME:
                    currencySymbol = currency.getName((ULocale) mLocaleObject.getLocale(), Currency.LONG_NAME, null);
                    break;
                case CODE:
                    currencySymbol = currencyCode;
                    break;
                case SYMBOL:
                case NARROWSYMBOL:
                default:
                    currencySymbol = currency.getName((ULocale) mLocaleObject.getLocale(), Currency.SYMBOL_NAME, null);
                    break;
            }

            mDecimalFormat.getDecimalFormatSymbols().setCurrencySymbol(currencySymbol);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU configureGrouping(boolean mGroupingUsed) {
        mDecimalFormat.setGroupingUsed(mGroupingUsed);
        return this;
    }

    @Override
    public PlatformNumberFormatterICU configureMinIntergerDigits(int minimumIntegerDigits) {
        if (minimumIntegerDigits != -1)
            mDecimalFormat.setMinimumIntegerDigits(minimumIntegerDigits);

        return this;
    }

    @Override
    public PlatformNumberFormatterICU configureSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException {
        if (roundingType == IPlatformNumberFormatter.RoundingType.SIGNIFICANT_DIGITS) {
            if (minimumSignificantDigits >= 0)
                mDecimalFormat.setMinimumSignificantDigits(minimumSignificantDigits);

            if (maximumSignificantDigits >= 0) {
                if (maximumSignificantDigits < mDecimalFormat.getMinimumSignificantDigits())
                    throw new JSRangeErrorException("maximumSignificantDigits should be at least equal to minimumSignificantDigits");
                mDecimalFormat.setMaximumSignificantDigits(maximumSignificantDigits);
            }

            mDecimalFormat.setSignificantDigitsUsed(true);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU configureFractinDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits) {
        if (roundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {
            if (minimumFractionDigits >= 0)
                mDecimalFormat.setMinimumFractionDigits(minimumFractionDigits);

            if (maximumFractionDigits >= 0)
                mDecimalFormat.setMaximumFractionDigits(maximumFractionDigits);

            mDecimalFormat.setSignificantDigitsUsed(false);
        }

        return this;
    }

    @Override
    public PlatformNumberFormatterICU configureSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay) {
        DecimalFormatSymbols symbols = mDecimalFormat.getDecimalFormatSymbols();

        switch (signDisplay) {
            case NEVER:
                mDecimalFormat.setPositivePrefix("");
                mDecimalFormat.setPositiveSuffix("");

                mDecimalFormat.setNegativePrefix("");
                mDecimalFormat.setNegativeSuffix("");

                break;
            case ALWAYS:
            case EXCEPTZERO:

                if (!mDecimalFormat.getNegativePrefix().isEmpty())
                    mDecimalFormat.setPositivePrefix(new String(new char[]{symbols.getPlusSign()}));

                if (!mDecimalFormat.getNegativeSuffix().isEmpty())
                    mDecimalFormat.setPositiveSuffix(new String(new char[]{symbols.getPlusSign()}));

                break;
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
    public PlatformNumberFormatterICU configureUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException {
        if (mStyle == Style.UNIT) {
            MeasureFormat.FormatWidth formatWidth;
            switch (unitDisplay) {
                case LONG:
                    formatWidth = MeasureFormat.FormatWidth.WIDE;
                    break;
                case NARROW:
                    formatWidth = MeasureFormat.FormatWidth.NARROW;
                    break;
                case SHORT:
                default:
                    formatWidth = MeasureFormat.FormatWidth.SHORT;
                    break;
            }

            mUnitId = unit;

            MeasureFormat measureFormat = MeasureFormat.getInstance((ULocale) mLocaleObject.getLocale(), formatWidth, mDecimalFormat);
            mFinalFormat = measureFormat;
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
        if (attribute == DecimalFormat.Field.SIGN) {
            if (Double.compare(x, +0) >= 0) {
                return "plusSign";
            }
            return "minusSign";
        }
        if (attribute == DecimalFormat.Field.INTEGER) {
            if (Double.isNaN(x)) {
                return "nan";
            }
            if (Double.isInfinite(x)) {
                return "infinity";
            }
            return "integer";
        }
        if (attribute == DecimalFormat.Field.FRACTION) {
            return "fraction";
        }
        if (attribute == DecimalFormat.Field.EXPONENT) {
            return "exponentInteger";
        }
        if (attribute == DecimalFormat.Field.EXPONENT_SIGN) {
            return "exponentMinusSign";
        }
        if (attribute == DecimalFormat.Field.EXPONENT_SYMBOL) {
            return "exponentSeparator";
        }
        if (attribute == DecimalFormat.Field.DECIMAL_SEPARATOR) {
            return "decimal";
        }
        if (attribute == DecimalFormat.Field.GROUPING_SEPARATOR) {
            return "group";
        }
        if (attribute == DecimalFormat.Field.PERCENT) {
            return "percentSign";
        }
        if (attribute == DecimalFormat.Field.PERMILLE) {
            return "permilleSign";
        }
        if (attribute == DecimalFormat.Field.CURRENCY) {
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

    public static PlatformNumberFormatterICU createDecimalFormat(ILocaleObject localeObject, IPlatformNumberFormatter.Style style,
                                                                 IPlatformNumberFormatter.CurrencySign currencySign,
                                                                 IPlatformNumberFormatter.Notation notation,
                                                                 IPlatformNumberFormatter.CompactDisplay compactDisplay) throws JSRangeErrorException {
        PlatformNumberFormatterICU platformNumberFormatterICU;
        switch (style) {
            case CURRENCY:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatForCurrency(localeObject, currencySign, style);
                break;
            case PERCENT:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatForPercent(localeObject, style);
                break;
            case DECIMAL:
            case UNIT:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatGerneric(localeObject, notation, compactDisplay, style);
                break;
            default:
                throw new JSRangeErrorException("Unrecognized formatting style requested.");
        }

        return platformNumberFormatterICU;
    }


    public static PlatformNumberFormatterICU createDecimalFormatForCurrency(ILocaleObject localeObject, IPlatformNumberFormatter.CurrencySign currencySign, IPlatformNumberFormatter.Style style) throws JSRangeErrorException {
        switch (currencySign) {
            case ACCOUNTING:
                return new PlatformNumberFormatterICU((DecimalFormat) android.icu.text.NumberFormat.getInstance((ULocale) localeObject.getLocale(), android.icu.text.NumberFormat.ACCOUNTINGCURRENCYSTYLE),
                        localeObject, style);
            case STANDARD:
                return new PlatformNumberFormatterICU((DecimalFormat) android.icu.text.NumberFormat.getInstance((ULocale) localeObject.getLocale(), android.icu.text.NumberFormat.CURRENCYSTYLE),
                        localeObject, style);
            default:
                throw new JSRangeErrorException("Unsupported currency sign !!");
        }
    }

    public static PlatformNumberFormatterICU createDecimalFormatForPercent(ILocaleObject localeObject, IPlatformNumberFormatter.Style style) throws JSRangeErrorException {
        return new PlatformNumberFormatterICU((DecimalFormat) DecimalFormat.getInstance((ULocale) localeObject.getLocale(), android.icu.text.NumberFormat.PERCENTSTYLE),
                localeObject, style);
    }

    public static PlatformNumberFormatterICU createDecimalFormatGerneric(ILocaleObject localeObject, IPlatformNumberFormatter.Notation notation, IPlatformNumberFormatter.CompactDisplay compactDisplay, IPlatformNumberFormatter.Style style) throws JSRangeErrorException {
        switch (notation) {
            case COMPACT:
                return new PlatformNumberFormatterICU((DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(),
                        compactDisplay == IPlatformNumberFormatter.CompactDisplay.SHORT ? CompactDecimalFormat.CompactStyle.SHORT : CompactDecimalFormat.CompactStyle.LONG),
                        localeObject, style);

            case SCIENTIFIC:
                return new PlatformNumberFormatterICU((DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(), SCIENTIFICSTYLE), // TODO :: Why CopactDecimalFormat
                        localeObject, style);

            case ENGINEERING:
                DecimalFormat decimalFormat = (DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(), SCIENTIFICSTYLE);

                // Must read the scientific style properties to understand this: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
                decimalFormat.setMaximumIntegerDigits(3);

                return new PlatformNumberFormatterICU(decimalFormat, localeObject, style);
            case STANDARD:
            default:
                return new PlatformNumberFormatterICU((DecimalFormat) DecimalFormat.getNumberInstance((ULocale) localeObject.getLocale()),
                        localeObject, style);
        }
    }

    public static String configureNumberingSystem(String inNumberingSystem, ILocaleObject locale) throws JSRangeErrorException {
        if (!inNumberingSystem.isEmpty()) {
            try {
                NumberingSystem numberingSystemObject = NumberingSystem.getInstanceByName(inNumberingSystem);
            } catch (RuntimeException ex) {
                throw new JSRangeErrorException("Invalid numbering system: " + inNumberingSystem);
            }

            ArrayList<String> numberingSystemList = new ArrayList<>();
            numberingSystemList.add(inNumberingSystem);
            locale.setUnicodeExtensions("nu", numberingSystemList);

            return inNumberingSystem;
        } else {
            return NumberingSystem.getInstance((ULocale) locale.getLocale()).getName();
        }
    }
}
