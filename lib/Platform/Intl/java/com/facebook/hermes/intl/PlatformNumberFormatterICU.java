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
import java.util.List;
import java.util.Map;

import static android.icu.text.NumberFormat.SCIENTIFICSTYLE;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.currency;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.unit;

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
    }

    public DecimalFormat getDecimalFormat() {
        return mDecimalFormat;
    }

    @Override
    public PlatformNumberFormatterICU configureCurrency(String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay) throws JSRangeErrorException {
        if (mStyle == currency) {

            Currency currency = Currency.getInstance(currencyCode);
            mDecimalFormat.setCurrency(currency);

            String currencySymbol;
            switch (currencyDisplay) {
                case name:
                    currencySymbol = currency.getName((ULocale) mLocaleObject.getLocale(), Currency.LONG_NAME, null);
                    break;
                case code:
                    currencySymbol = currencyCode;
                    break;
                case symbol:
                case narrowSymbol:
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
        if (roundingType == IPlatformNumberFormatter.RoundingType.significantDigits) {
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
        if (roundingType == IPlatformNumberFormatter.RoundingType.fractionDigits) {
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
            case never:
                mDecimalFormat.setPositivePrefix("");
                mDecimalFormat.setPositiveSuffix("");

                mDecimalFormat.setNegativePrefix("");
                mDecimalFormat.setNegativeSuffix("");

                break;
            case always:
            case exceptZero:

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
        if (mStyle == Style.unit) {
            MeasureFormat.FormatWidth formatWidth;
            switch (unitDisplay) {
                case LONG:
                    formatWidth = MeasureFormat.FormatWidth.WIDE;
                    break;
                case narrow:
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
        }
        else
            result = mFinalFormat.format(n);

        return result;
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

    public static PlatformNumberFormatterICU createDecimalFormat(ILocaleObject localeObject, IPlatformNumberFormatter.Style style,
                                                                 IPlatformNumberFormatter.CurrencySign currencySign,
                                                                 IPlatformNumberFormatter.Notation notation,
                                                                 IPlatformNumberFormatter.CompactDisplay compactDisplay) throws JSRangeErrorException {
        PlatformNumberFormatterICU platformNumberFormatterICU;
        switch (style) {
            case currency:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatForCurrency(localeObject, currencySign, style);
                break;
            case percent:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatForPercent(localeObject, style);
                break;
            case decimal:
            case unit:
                platformNumberFormatterICU = PlatformNumberFormatterICU.createDecimalFormatGerneric(localeObject, notation, compactDisplay, style);
                break;
            default:
                throw new JSRangeErrorException("Unrecognized formatting style requested.");
        }

        platformNumberFormatterICU.getDecimalFormat().setRoundingMode(BigDecimal.ROUND_HALF_UP);
        return platformNumberFormatterICU;
    }


    public static PlatformNumberFormatterICU createDecimalFormatForCurrency(ILocaleObject localeObject, IPlatformNumberFormatter.CurrencySign currencySign, IPlatformNumberFormatter.Style style) throws JSRangeErrorException {
        switch (currencySign) {
            case accounting:
                return new PlatformNumberFormatterICU((DecimalFormat) android.icu.text.NumberFormat.getInstance((ULocale) localeObject.getLocale(), android.icu.text.NumberFormat.ACCOUNTINGCURRENCYSTYLE),
                        localeObject, style);
            case standard:
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
            case compact:
                return new PlatformNumberFormatterICU((DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(),
                        compactDisplay == IPlatformNumberFormatter.CompactDisplay.SHORT ? CompactDecimalFormat.CompactStyle.SHORT : CompactDecimalFormat.CompactStyle.LONG),
                        localeObject, style);

            case scientific:
                return new PlatformNumberFormatterICU((DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(), SCIENTIFICSTYLE), // TODO :: Why CopactDecimalFormat
                        localeObject, style);

            case engineering:
                DecimalFormat decimalFormat = (DecimalFormat) CompactDecimalFormat.getInstance((ULocale) localeObject.getLocale(), SCIENTIFICSTYLE);

                // Must read the scientific style properties to understand this: https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
                decimalFormat.setMaximumIntegerDigits(3);

                return new PlatformNumberFormatterICU(decimalFormat, localeObject, style);
            case standard:
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
