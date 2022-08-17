/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.DECIMAL;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.UNIT;

import android.icu.text.CompactDecimalFormat;
import android.icu.text.MeasureFormat;
import android.icu.text.NumberFormat;
import android.icu.text.NumberingSystem;
import android.icu.util.Currency;
import android.icu.util.Measure;
import android.icu.util.MeasureUnit;
import android.icu.util.ULocale;
import android.os.Build;
import androidx.annotation.RequiresApi;
import java.math.BigDecimal;
import java.text.AttributedCharacterIterator;
import java.util.ArrayList;

// An implementation of NumberFormat services using ICU4J APIs available in Android from API 24
// onwards.
// We could have a much richer implementation on newer API versions with // Note ::
// https://developer.android.com/reference/android/icu/number/NumberFormatter which was introduced
// in API 30.
//
// The implementation has at least the following deficiencies,
// 1. SignDisplay attribute implementation is partly hacky, and doesn't work in many cases.
// 2. formatToParts does report the whole formatted string as a "literal" for "unit" styling.
public class PlatformNumberFormatterICU implements IPlatformNumberFormatter {
  private java.text.Format mFinalFormat;
  private NumberFormat mNumberFormat;
  private LocaleObjectICU mLocaleObject;
  private IPlatformNumberFormatter.Style mStyle;

  private MeasureUnit mMeasureUnit;

  PlatformNumberFormatterICU() {}

  @RequiresApi(api = Build.VERSION_CODES.N)
  private void initialize(
      NumberFormat numberFormat,
      ILocaleObject<?> localeObject,
      IPlatformNumberFormatter.Style style) {
    mNumberFormat = numberFormat;
    mFinalFormat = numberFormat;
    mLocaleObject = (LocaleObjectICU) localeObject;
    mStyle = style;

    mNumberFormat.setRoundingMode(BigDecimal.ROUND_HALF_UP);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setCurrency(
      String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay)
      throws JSRangeErrorException {
    if (mStyle == CURRENCY) {

      Currency currency = Currency.getInstance(currencyCode);
      mNumberFormat.setCurrency(currency);

      String currencySymbol;
      if (currencyDisplay == CurrencyDisplay.CODE) currencySymbol = currencyCode;
      else
        currencySymbol =
            currency.getName(mLocaleObject.getLocale(), currencyDisplay.getNameStyle(), null);

      if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
        android.icu.text.DecimalFormat decimalFormat =
            (android.icu.text.DecimalFormat) mNumberFormat;
        android.icu.text.DecimalFormatSymbols symbols = decimalFormat.getDecimalFormatSymbols();
        symbols.setCurrencySymbol(currencySymbol);
        decimalFormat.setDecimalFormatSymbols(symbols);
      }
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setGrouping(boolean mGroupingUsed) {
    mNumberFormat.setGroupingUsed(mGroupingUsed);
    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setMinIntergerDigits(int minimumIntegerDigits) {
    if (minimumIntegerDigits != -1) mNumberFormat.setMinimumIntegerDigits(minimumIntegerDigits);

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setSignificantDigits(
      IPlatformNumberFormatter.RoundingType roundingType,
      int minimumSignificantDigits,
      int maximumSignificantDigits)
      throws JSRangeErrorException {
    if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
      if (roundingType == IPlatformNumberFormatter.RoundingType.SIGNIFICANT_DIGITS) {
        android.icu.text.DecimalFormat decimalFormat =
            (android.icu.text.DecimalFormat) mNumberFormat;
        if (minimumSignificantDigits >= 0)
          decimalFormat.setMinimumSignificantDigits(minimumSignificantDigits);

        if (maximumSignificantDigits >= 0) {
          if (maximumSignificantDigits < decimalFormat.getMinimumSignificantDigits())
            throw new JSRangeErrorException(
                "maximumSignificantDigits should be at least equal to minimumSignificantDigits");
          decimalFormat.setMaximumSignificantDigits(maximumSignificantDigits);
        }

        decimalFormat.setSignificantDigitsUsed(true);
      }
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setFractionDigits(
      IPlatformNumberFormatter.RoundingType roundingType,
      int minimumFractionDigits,
      int maximumFractionDigits) {
    if (roundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {
      if (minimumFractionDigits >= 0) mNumberFormat.setMinimumFractionDigits(minimumFractionDigits);

      if (maximumFractionDigits >= 0) mNumberFormat.setMaximumFractionDigits(maximumFractionDigits);

      if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
        ((android.icu.text.DecimalFormat) mNumberFormat).setSignificantDigitsUsed(false);
      }
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setSignDisplay(
      IPlatformNumberFormatter.SignDisplay signDisplay) {
    if (mNumberFormat instanceof android.icu.text.DecimalFormat) {
      android.icu.text.DecimalFormat decimalFormat = (android.icu.text.DecimalFormat) mNumberFormat;
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
            decimalFormat.setPositivePrefix(new String(new char[] {symbols.getPlusSign()}));

          if (!decimalFormat.getNegativeSuffix().isEmpty())
            decimalFormat.setPositiveSuffix(new String(new char[] {symbols.getPlusSign()}));

          break;
      }
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  private static MeasureUnit parseUnit(String inUnit) throws JSRangeErrorException {

    // http://unicode.org/reports/tr35/tr35-general.html#Unit_Identifiers
    // Only core identifiers will be input
    for (MeasureUnit unit : MeasureUnit.getAvailable()) {

      if (unit.getSubtype().equals(inUnit)
          || unit.getSubtype().equals(unit.getType() + "-" + inUnit)) return unit;
    }

    throw new JSRangeErrorException("Unknown unit: " + inUnit);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU setUnits(
      String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException {
    if (mStyle == Style.UNIT) {
      mMeasureUnit = parseUnit(unit);
      mFinalFormat =
          MeasureFormat.getInstance(
              mLocaleObject.getLocale(), unitDisplay.getFormatWidth(), mNumberFormat);
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String format(double n) {
    String result;
    try {
      if (mFinalFormat instanceof MeasureFormat && mMeasureUnit != null) {
        result = mFinalFormat.format(new Measure(n, mMeasureUnit));
      } else result = mFinalFormat.format(n);
    } catch (NumberFormatException ex) {
      // For older API versions (Tested on API 26), java.lang.NumberFormatException is thrown when
      // formatting special numbers values such as Infinity, Nan etc.
      // on special locales such as "ja-u-nu-jpanfin". One observation is that the formatter employs
      // "android.icu.RuleBasedNumberFormat" implementation (unlike "android.icu.DecimalFormat" for
      // relatively simpler locales.)
      // We are trying our best to avoid a crash here.
      try {
        return NumberFormat.getInstance(ULocale.getDefault()).format(n);
      } catch (RuntimeException ex2) {
        return NumberFormat.getInstance(ULocale.forLanguageTag("en")).format(n);
      }
    }

    return result;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
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

    // TODO:: There must be a better way to do this.
    if (attribute.toString().equals("android.icu.text.NumberFormat$Field(compact)"))
      return "compact";

    // Report unsupported/unexpected number fields as literal.
    return "literal";
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public AttributedCharacterIterator formatToParts(double n) {
    AttributedCharacterIterator iterator;
    try {
      if (mFinalFormat instanceof MeasureFormat && mMeasureUnit != null) {
        iterator = mFinalFormat.formatToCharacterIterator(new Measure(n, mMeasureUnit));
      } else {
        iterator = mFinalFormat.formatToCharacterIterator(n);
      }
    } catch (NumberFormatException ex) {
      // For older API versions (Tested on API 26), java.lang.NumberFormatException is thrown when
      // formatting special numbers values such as Infinity, Nan etc.
      // on special locales such as "ja-u-nu-jpanfin". One observation is that the formatter employs
      // "android.icu.RuleBasedNumberFormat" implementation (unlike "android.icu.DecimalFormat" for
      // relatively simpler locales.)
      // We are trying our best to avoid a crash here.
      try {
        return NumberFormat.getInstance(ULocale.getDefault()).formatToCharacterIterator(n);
      } catch (RuntimeException ex2) {
        return NumberFormat.getInstance(ULocale.forLanguageTag("en")).formatToCharacterIterator(n);
      }
    } catch (Exception ex) {
      // Scarily, DecimalFormat.formatToCharacterIterator throws NullPointerEsception when parsing
      // 0.0.
      return NumberFormat.getInstance(ULocale.forLanguageTag("en")).formatToCharacterIterator(n);
    }

    return iterator;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  public static int getCurrencyDigits(String currencyCode) throws JSRangeErrorException {
    try {
      return Currency.getInstance(currencyCode).getDefaultFractionDigits();
    } catch (IllegalArgumentException ex) {
      throw new JSRangeErrorException("Invalid currency code !");
    }
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public PlatformNumberFormatterICU configure(
      ILocaleObject<?> localeObject,
      String numberingSystem,
      IPlatformNumberFormatter.Style style,
      IPlatformNumberFormatter.CurrencySign currencySign,
      IPlatformNumberFormatter.Notation notation,
      IPlatformNumberFormatter.CompactDisplay compactDisplay)
      throws JSRangeErrorException {
    if (!numberingSystem.isEmpty()) {

      NumberingSystem numberingSystemObject;
      try {
        numberingSystemObject =
            NumberingSystem.getInstanceByName(JSObjects.getJavaString(numberingSystem));
      } catch (RuntimeException ex) {
        throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);
      }

      if (numberingSystemObject == null)
        throw new JSRangeErrorException("Invalid numbering system: " + numberingSystem);

      ArrayList<String> numberingSystemList = new ArrayList<>();
      numberingSystemList.add(JSObjects.getJavaString(numberingSystem));

      localeObject.setUnicodeExtensions("nu", numberingSystemList);
    }

    if (notation == Notation.COMPACT
        && (style == DECIMAL
            || style
                == UNIT)) { // TODO :: Note sure whether the compact notation makes sense for other
      // styles ..
      CompactDecimalFormat.CompactStyle compactStyle =
          compactDisplay == IPlatformNumberFormatter.CompactDisplay.SHORT
              ? android.icu.text.CompactDecimalFormat.CompactStyle.SHORT
              : android.icu.text.CompactDecimalFormat.CompactStyle.LONG;

      NumberFormat numberFormat =
          android.icu.text.CompactDecimalFormat.getInstance(
              (ULocale) localeObject.getLocale(), compactStyle);
      initialize(numberFormat, localeObject, style);
    } else {
      int numberFormatStyle = style.getInitialNumberFormatStyle(notation, currencySign);
      NumberFormat numberFormat =
          NumberFormat.getInstance((ULocale) localeObject.getLocale(), numberFormatStyle);

      if (notation == Notation.ENGINEERING) {
        // Must read the scientific style properties to understand this:
        // https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
        numberFormat.setMaximumIntegerDigits(3);
      }

      initialize(numberFormat, localeObject, style);
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String[] getAvailableLocales() {
    ArrayList<String> availableLocaleIds = new ArrayList<>();

    // NumberFormat.getAvailableLocales() returns a shorter list compared to
    // ULocale.getAvailableLocales.
    // For e.g. "zh-TW" is missing in the list returned by NumberFormat.getAvailableLocales() in my
    // emulator.
    // But, NumberFormatter is able to format specific to "zh-TW" .. for instance "NaN" is expected
    // to be formatted as "非數值" in "zh-TW" by as "NaN" in "zh"
    // In short, NumberFormat.getAvailableLocales() doesn't contain all the locales as the
    // NumberFormat can format. Hence, using ULocale.getAvailableLocales()
    //
    // java.util.Locale[] availableLocales = NumberFormat.getAvailableLocales();
    android.icu.util.ULocale[] availableLocales = ULocale.getAvailableLocales();

    for (android.icu.util.ULocale locale : availableLocales) {
      availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
    }

    String[] availableLocaleIdsArray = new String[availableLocaleIds.size()];
    return availableLocaleIds.toArray(availableLocaleIdsArray);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String getDefaultNumberingSystem(ILocaleObject<?> localeObject)
      throws JSRangeErrorException {
    return NumberingSystem.getInstance((ULocale) localeObject.getLocale()).getName();
  }
}
