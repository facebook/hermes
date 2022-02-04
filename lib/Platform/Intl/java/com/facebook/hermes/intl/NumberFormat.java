/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.CURRENCY;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.PERCENT;
import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.UNIT;

import android.os.Build;
import com.facebook.proguard.annotations.DoNotStrip;
import java.text.AttributedCharacterIterator;
import java.text.CharacterIterator;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android Intl.NumberFormat implementation. The
 * interaction with the Hermes JaveScript internals are implemented in C++ and should not generally
 * need to be changed. Implementers' notes here will describe what parts of the ECMA 402 spec remain
 * to be implemented.
 *
 * <p>Also see the implementer' notes on DateTimeFormat.java.
 */
@DoNotStrip
public class NumberFormat {
  // options are localeMatcher:string, numberingSystem:string, notation:string,
  // compactDisplay:string, useGrouping:string, signDisplay:string
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializenumberformat
  // Also see the implementer notes on DateTimeFormat#DateTimeFormat()

  private IPlatformNumberFormatter.Style mResolvedStyle;

  private String mResolvedCurrency = null;
  private IPlatformNumberFormatter.CurrencyDisplay mResolvedCurrencyDisplay =
      IPlatformNumberFormatter.CurrencyDisplay.SYMBOL;
  private IPlatformNumberFormatter.CurrencySign mResolvedCurrencySign =
      IPlatformNumberFormatter.CurrencySign.STANDARD;

  private String mResolvedUnit = null;
  private IPlatformNumberFormatter.UnitDisplay mResolvedUnitDisplay;

  private boolean mGroupingUsed = true;

  private int mResolvedMinimumIntegerDigits = -1,
      mResolvedMinimumFractionDigits = -1,
      mResolvedMaximumFractionDigits = -1;
  private int mResolvedMinimumSignificantDigits = -1, mResolvedMaximumSignificantDigits = -1;

  private IPlatformNumberFormatter.RoundingType mRoundingType;

  private IPlatformNumberFormatter.SignDisplay mResolvedSignDisplay =
      IPlatformNumberFormatter.SignDisplay.AUTO;

  private IPlatformNumberFormatter mPlatformNumberFormatter;

  private boolean mUseDefaultNumberSystem;
  private String mResolvedNumberingSystem = null;

  private IPlatformNumberFormatter.Notation mResolvedNotation = null;
  private IPlatformNumberFormatter.CompactDisplay mResolvedCompactDisplay;

  private ILocaleObject<?> mResolvedLocaleObject = null;

  // This is a hacky way to avoid the extensions that we add from being shown in "resolvedOptions"
  // ..
  private ILocaleObject<?> mResolvedLocaleObjectForResolvedOptions = null;

  // This list should be kept alphabetically ordered so that we can binary search in it.
  private static String[] s_sanctionedSimpleUnitIdentifiers = {
    "acre",
    "bit",
    "byte",
    "celsius",
    "centimeter",
    "day",
    "degree",
    "fahrenheit",
    "fluid-ounce",
    "foot",
    "gallon",
    "gigabit",
    "gigabyte",
    "gram",
    "hectare",
    "hour",
    "inch",
    "kilobit",
    "kilobyte",
    "kilogram",
    "kilometer",
    "liter",
    "megabit",
    "megabyte",
    "meter",
    "mile",
    "mile-scandinavian",
    "milliliter",
    "millimeter",
    "millisecond",
    "minute",
    "month",
    "ounce",
    "percent",
    "petabyte",
    "pound",
    "second",
    "stone",
    "terabit",
    "terabyte",
    "week",
    "yard",
    "year"
  };

  private boolean isSanctionedSimpleUnitIdentifier(String unitIdentifier) {
    return java.util.Arrays.binarySearch(s_sanctionedSimpleUnitIdentifiers, unitIdentifier) >= 0;
  }

  private boolean isWellFormedUnitIdentifier(String unitIdentifier) {
    // 1.
    if (isSanctionedSimpleUnitIdentifier(unitIdentifier)) return true;

    String per = "-per-";
    int indexOfPer = unitIdentifier.indexOf(per);

    // 2
    if (indexOfPer < 0) return false;

    if (unitIdentifier.indexOf(per, indexOfPer + 1) >= 0) return false;

    // 3, 4
    String numerator = unitIdentifier.substring(0, indexOfPer);
    if (!isSanctionedSimpleUnitIdentifier(numerator)) return false;

    // 5, 6
    String denominator = unitIdentifier.substring(indexOfPer + per.length());
    if (!isSanctionedSimpleUnitIdentifier(denominator)) return false;

    // 7
    return true;
  }

  private String normalizeCurrencyCode(String currencyCode) {
    // https://tc39.es/ecma402/#sec-case-sensitivity-and-case-mapping
    // Note that we should convert only upper case translation in ASCII range.
    StringBuilder normalized = new StringBuilder(currencyCode.length());
    int offset = 'a' - 'A';
    for (int idx = 0; idx < currencyCode.length(); idx++) {
      char c = currencyCode.charAt(idx);
      if (c >= 'a' && c <= 'z') {
        normalized.append((char) (c - offset));
      } else {
        normalized.append(c);
      }
    }

    return normalized.toString();
  }

  // https://tc39.es/ecma402/#sec-iswellformedcurrencycode
  private boolean isWellFormedCurrencyCode(String currencyCode) {
    String normalized = normalizeCurrencyCode(currencyCode);
    return normalized.matches("^[A-Z][A-Z][A-Z]$");
  }

  // https://tc39.es/ecma402/#sec-setnumberformatunitoptions
  private void setNumberFormatUnitOptions(Map<String, Object> options)
      throws JSRangeErrorException {
    // 3,4
    // TODO :: Make it more robust.
    Object style =
        OptionHelpers.GetOption(
            options,
            "style",
            OptionHelpers.OptionType.STRING,
            new String[] {"decimal", "percent", "currency", "unit"},
            "decimal");
    mResolvedStyle =
        OptionHelpers.searchEnum(
            IPlatformNumberFormatter.Style.class, JSObjects.getJavaString(style));

    // 5
    Object currency =
        OptionHelpers.GetOption(
            options,
            "currency",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    if (JSObjects.isUndefined(currency)) {
      if (mResolvedStyle == CURRENCY) {
        // Note: This should be TypeError by spec. But. currently we don't allow throwing TypeError
        // from Java code.
        // This check and throwing TypeError is already coded in native code ahead of the flow
        // reaching here.
        throw new JSRangeErrorException("Expected currency style !");
      }
    } else {
      if (!isWellFormedCurrencyCode(JSObjects.getJavaString(currency)))
        throw new JSRangeErrorException("Malformed currency code !");
    }

    // 6a is done in C++ code.
    // 6b
    Object currencyDisplay =
        OptionHelpers.GetOption(
            options,
            "currencyDisplay",
            OptionHelpers.OptionType.STRING,
            new String[] {"symbol", "narrowSymbol", "code", "name"},
            "symbol");
    Object currencySign =
        OptionHelpers.GetOption(
            options,
            "currencySign",
            OptionHelpers.OptionType.STRING,
            new String[] {"accounting", "standard"},
            "standard");

    Object unit =
        OptionHelpers.GetOption(
            options,
            "unit",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    if (JSObjects.isUndefined(unit)) {
      // Note: This should be TypeError by spec. But. currently we don't allow throwing TypeError
      // from Java code.
      // This check and throwing TypeError is already coded in native code ahead of the flow
      // reaching here.
      if (mResolvedStyle == UNIT) {
        throw new JSRangeErrorException("Expected unit !");
      }
    } else {
      if (!isWellFormedUnitIdentifier(JSObjects.getJavaString(unit)))
        throw new JSRangeErrorException("Malformed unit identifier !");
    }

    // 11 is done in C++ code.
    // 12
    Object unitDisplay =
        OptionHelpers.GetOption(
            options,
            "unitDisplay",
            OptionHelpers.OptionType.STRING,
            new String[] {"long", "short", "narrow"},
            "short");

    if (mResolvedStyle == CURRENCY) {
      mResolvedCurrency = normalizeCurrencyCode(JSObjects.getJavaString(currency));
      mResolvedCurrencyDisplay =
          OptionHelpers.searchEnum(
              IPlatformNumberFormatter.CurrencyDisplay.class,
              JSObjects.getJavaString(currencyDisplay));
      mResolvedCurrencySign =
          OptionHelpers.searchEnum(
              IPlatformNumberFormatter.CurrencySign.class, JSObjects.getJavaString(currencySign));
    } else if (mResolvedStyle == IPlatformNumberFormatter.Style.UNIT) {
      mResolvedUnit = JSObjects.getJavaString(unit);
      mResolvedUnitDisplay =
          OptionHelpers.searchEnum(
              IPlatformNumberFormatter.UnitDisplay.class, JSObjects.getJavaString(unitDisplay));
    }
  }

  // https://tc39.es/ecma402/#sec-setnfdigitoptions
  private void setNumberFormatDigitOptions(
      Map<String, Object> options, Object mnfdDefault, Object mxfdDefault)
      throws JSRangeErrorException {

    Object mnid =
        OptionHelpers.GetNumberOption(
            options,
            "minimumIntegerDigits",
            JSObjects.newNumber(1),
            JSObjects.newNumber(21),
            JSObjects.newNumber(1));

    Object mnfd = JSObjects.Get(options, "minimumFractionDigits");
    Object mxfd = JSObjects.Get(options, "maximumFractionDigits");

    Object mnsd = JSObjects.Get(options, "minimumSignificantDigits");
    Object mxsd = JSObjects.Get(options, "maximumSignificantDigits");

    mResolvedMinimumIntegerDigits = (int) Math.floor(JSObjects.getJavaDouble(mnid));

    if (!JSObjects.isUndefined(mnsd) || !JSObjects.isUndefined(mxsd)) {

      mRoundingType = IPlatformNumberFormatter.RoundingType.SIGNIFICANT_DIGITS;

      mnsd =
          OptionHelpers.DefaultNumberOption(
              mnsd, JSObjects.newNumber(1), JSObjects.newNumber(21), JSObjects.newNumber(1));
      mxsd =
          OptionHelpers.DefaultNumberOption(
              mxsd, mnsd, JSObjects.newNumber(21), JSObjects.newNumber(21));

      mResolvedMinimumSignificantDigits = (int) Math.floor(JSObjects.getJavaDouble(mnsd));
      mResolvedMaximumSignificantDigits = (int) Math.floor(JSObjects.getJavaDouble(mxsd));

    } else if (!JSObjects.isUndefined(mnfd) || !JSObjects.isUndefined(mxfd)) {

      mRoundingType = IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS;

      mnfd =
          OptionHelpers.DefaultNumberOption(
              mnfd, JSObjects.newNumber(0), JSObjects.newNumber(20), mnfdDefault);
      Object mxfdActualDefault =
          JSObjects.newNumber(
              Math.max(JSObjects.getJavaDouble(mnfd), JSObjects.getJavaDouble(mxfdDefault)));

      mxfd =
          OptionHelpers.DefaultNumberOption(mxfd, mnfd, JSObjects.newNumber(20), mxfdActualDefault);

      mResolvedMinimumFractionDigits = (int) Math.floor(JSObjects.getJavaDouble(mnfd));
      mResolvedMaximumFractionDigits = (int) Math.floor(JSObjects.getJavaDouble(mxfd));

    } else if (mResolvedNotation == IPlatformNumberFormatter.Notation.COMPACT) {
      mRoundingType = IPlatformNumberFormatter.RoundingType.COMPACT_ROUNDING;
    } else if (mResolvedNotation == IPlatformNumberFormatter.Notation.ENGINEERING) {
      // The default setting for engineering notation.
      // This is not based on the spec, but is required by our implementation of engineering
      // notation.
      // From
      // https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1DecimalFormat.html
      // If areSignificantDigitsUsed() returns false, then the minimum number of significant digits
      // shown is one,
      // and the maximum number of significant digits shown is the sum of the minimum integer and
      // maximum fraction digits,
      // and is unaffected by the maximum integer digits.
      //
      // In short, the minimum integer will be set to 1 and hence to achieve maximum default
      // fraction digits of "3" (as in spec), we should set the maximum fraction digits to "5"
      mRoundingType = IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS;
      mResolvedMaximumFractionDigits = 5;
    } else {
      mRoundingType = IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS;
      mResolvedMinimumFractionDigits = (int) Math.floor(JSObjects.getJavaDouble(mnfdDefault));
      mResolvedMaximumFractionDigits = (int) Math.floor(JSObjects.getJavaDouble(mxfdDefault));
    }
  }

  private boolean isLocaleIdType(String token) {
    return IntlTextUtils.isUnicodeExtensionKeyTypeItem(token, 0, token.length() - 1);
  }

  private void initializeNumberFormat(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {

    Object opt = JSObjects.newObject();

    Object matcher =
        OptionHelpers.GetOption(
            options,
            Constants.LOCALEMATCHER,
            OptionHelpers.OptionType.STRING,
            Constants.LOCALEMATCHER_POSSIBLE_VALUES,
            Constants.LOCALEMATCHER_BESTFIT);
    JSObjects.Put(opt, "localeMatcher", matcher);

    Object numberingSystem =
        OptionHelpers.GetOption(
            options,
            "numberingSystem",
            OptionHelpers.OptionType.STRING,
            JSObjects.Undefined(),
            JSObjects.Undefined());
    if (!JSObjects.isUndefined(numberingSystem)) {
      if (!isLocaleIdType(JSObjects.getJavaString(numberingSystem)))
        throw new JSRangeErrorException("Invalid numbering system !");
    }
    JSObjects.Put(opt, "nu", numberingSystem);

    // https://tc39.es/ecma402/#sec-intl.numberformat-internal-slots
    // Note:: "cu" won't be accepted.
    List<String> relevantExtensionKeys = Collections.singletonList("nu");

    HashMap<String, Object> r = LocaleResolver.resolveLocale(locales, opt, relevantExtensionKeys);

    mResolvedLocaleObject = (ILocaleObject<?>) JSObjects.getJavaMap(r).get("locale");
    mResolvedLocaleObjectForResolvedOptions = mResolvedLocaleObject.cloneObject();

    Object numeringSystemResolved = JSObjects.Get(r, "nu");
    if (!JSObjects.isNull(numeringSystemResolved)) {
      mUseDefaultNumberSystem = false;
      mResolvedNumberingSystem = JSObjects.getJavaString(numeringSystemResolved);
    } else {
      mUseDefaultNumberSystem = true;
      mResolvedNumberingSystem =
          mPlatformNumberFormatter.getDefaultNumberingSystem(mResolvedLocaleObject);
    }

    // 5,6
    setNumberFormatUnitOptions(options);

    // 17, 18
    Object mnfdDefault;
    Object mxfdDefault;
    if (mResolvedStyle == CURRENCY) {

      int cDigits;
      // TODO
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
        cDigits = PlatformNumberFormatterICU.getCurrencyDigits(mResolvedCurrency);
      } else {
        cDigits = PlatformNumberFormatterAndroid.getCurrencyDigits(mResolvedCurrency);
      }

      mnfdDefault = JSObjects.newNumber(cDigits);
      mxfdDefault = JSObjects.newNumber(cDigits);
    } else {
      mnfdDefault = JSObjects.newNumber(0);

      if (mResolvedStyle == PERCENT) mxfdDefault = JSObjects.newNumber(0);
      else mxfdDefault = JSObjects.newNumber(3);
    }

    // 19, 20
    Object notation =
        OptionHelpers.GetOption(
            options,
            "notation",
            OptionHelpers.OptionType.STRING,
            new String[] {"standard", "scientific", "engineering", "compact"},
            "standard");
    mResolvedNotation =
        OptionHelpers.searchEnum(
            IPlatformNumberFormatter.Notation.class, JSObjects.getJavaString(notation));

    // 21
    setNumberFormatDigitOptions(options, mnfdDefault, mxfdDefault);

    // 22, 23
    Object compactDisplay =
        OptionHelpers.GetOption(
            options,
            "compactDisplay",
            OptionHelpers.OptionType.STRING,
            new String[] {"short", "long"},
            "short");
    if (mResolvedNotation == IPlatformNumberFormatter.Notation.COMPACT) {
      mResolvedCompactDisplay =
          OptionHelpers.searchEnum(
              IPlatformNumberFormatter.CompactDisplay.class,
              JSObjects.getJavaString(compactDisplay));
    }

    Object groupingUsed =
        OptionHelpers.GetOption(
            options,
            "useGrouping",
            OptionHelpers.OptionType.BOOLEAN,
            JSObjects.Undefined(),
            JSObjects.newBoolean(true));
    mGroupingUsed = JSObjects.getJavaBoolean(groupingUsed);

    Object signDisplay =
        OptionHelpers.GetOption(
            options,
            "signDisplay",
            OptionHelpers.OptionType.STRING,
            new String[] {"auto", "never", "always", "exceptZero"},
            "auto");
    mResolvedSignDisplay =
        OptionHelpers.searchEnum(
            IPlatformNumberFormatter.SignDisplay.class, JSObjects.getJavaString(signDisplay));
  }

  @DoNotStrip
  public NumberFormat(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N)
      mPlatformNumberFormatter = new PlatformNumberFormatterICU();
    else mPlatformNumberFormatter = new PlatformNumberFormatterAndroid();

    initializeNumberFormat(locales, options);

    mPlatformNumberFormatter
        .configure(
            mResolvedLocaleObject,
            mUseDefaultNumberSystem ? "" : mResolvedNumberingSystem,
            mResolvedStyle,
            mResolvedCurrencySign,
            mResolvedNotation,
            mResolvedCompactDisplay)
        .setCurrency(mResolvedCurrency, mResolvedCurrencyDisplay)
        .setGrouping(mGroupingUsed)
        .setMinIntergerDigits(mResolvedMinimumIntegerDigits)
        .setSignificantDigits(
            mRoundingType, mResolvedMinimumSignificantDigits, mResolvedMaximumSignificantDigits)
        .setFractionDigits(
            mRoundingType, mResolvedMinimumFractionDigits, mResolvedMaximumFractionDigits)
        .setSignDisplay(mResolvedSignDisplay)
        .setUnits(mResolvedUnit, mResolvedUnitDisplay);
  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.supportedlocalesof
  //
  // The notes on DateTimeFormat#DateTimeFormat() for Locales and
  // Options also apply here.
  @DoNotStrip
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options)
      throws JSRangeErrorException {
    String matcher =
        JSObjects.getJavaString(
            OptionHelpers.GetOption(
                options,
                Constants.LOCALEMATCHER,
                OptionHelpers.OptionType.STRING,
                Constants.LOCALEMATCHER_POSSIBLE_VALUES,
                Constants.LOCALEMATCHER_BESTFIT));
    String[] localeArray = new String[locales.size()];
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N && matcher.equals("best fit")) {
      return Arrays.asList(LocaleMatcher.bestFitSupportedLocales(locales.toArray(localeArray)));
    } else {
      return Arrays.asList(LocaleMatcher.lookupSupportedLocales(locales.toArray(localeArray)));
    }
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.prototype.resolvedoptions
  //
  // Also see the implementer notes on DateTimeFormat#resolvedOptions()
  @DoNotStrip
  public Map<String, Object> resolvedOptions() throws JSRangeErrorException {

    HashMap<String, Object> finalResolvedOptions = new LinkedHashMap<>();

    finalResolvedOptions.put(
        Constants.LOCALE, mResolvedLocaleObjectForResolvedOptions.toCanonicalTag());
    finalResolvedOptions.put("numberingSystem", mResolvedNumberingSystem);

    finalResolvedOptions.put("style", mResolvedStyle.toString());

    if (mResolvedStyle == CURRENCY) {
      finalResolvedOptions.put("currency", mResolvedCurrency);
      finalResolvedOptions.put("currencyDisplay", mResolvedCurrencyDisplay.toString());
      finalResolvedOptions.put("currencySign", mResolvedCurrencySign.toString());
    } else if (mResolvedStyle == UNIT) {
      finalResolvedOptions.put("unit", mResolvedUnit);
      finalResolvedOptions.put("unitDisplay", mResolvedUnitDisplay.toString());
    }

    if (mResolvedMinimumIntegerDigits != -1)
      finalResolvedOptions.put("minimumIntegerDigits", mResolvedMinimumIntegerDigits);

    if (mRoundingType == IPlatformNumberFormatter.RoundingType.SIGNIFICANT_DIGITS) {
      if (mResolvedMaximumSignificantDigits != -1)
        finalResolvedOptions.put("minimumSignificantDigits", mResolvedMaximumSignificantDigits);

      if (mResolvedMinimumSignificantDigits != -1)
        finalResolvedOptions.put("maximumSignificantDigits", mResolvedMinimumSignificantDigits);

    } else if (mRoundingType == IPlatformNumberFormatter.RoundingType.FRACTION_DIGITS) {

      if (mResolvedMinimumFractionDigits != -1)
        finalResolvedOptions.put("minimumFractionDigits", mResolvedMinimumFractionDigits);

      if (mResolvedMaximumFractionDigits != -1)
        finalResolvedOptions.put("maximumFractionDigits", mResolvedMaximumFractionDigits);
    }

    finalResolvedOptions.put("useGrouping", mGroupingUsed);

    finalResolvedOptions.put("notation", mResolvedNotation.toString());
    if (mResolvedNotation == IPlatformNumberFormatter.Notation.COMPACT) {
      finalResolvedOptions.put("compactDisplay", mResolvedCompactDisplay.toString());
    }

    finalResolvedOptions.put("signDisplay", mResolvedSignDisplay.toString());

    return finalResolvedOptions;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumber
  @DoNotStrip
  public String format(double n) throws JSRangeErrorException {
    return mPlatformNumberFormatter.format(n);
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumbertoparts
  @DoNotStrip
  public List<Map<String, String>> formatToParts(double n) throws JSRangeErrorException {
    ArrayList<Map<String, String>> parts = new ArrayList<>();

    AttributedCharacterIterator iterator = mPlatformNumberFormatter.formatToParts(n);
    StringBuilder sb = new StringBuilder();
    for (char ch = iterator.first(); ch != CharacterIterator.DONE; ch = iterator.next()) {
      sb.append(ch);
      if (iterator.getIndex() + 1 == iterator.getRunLimit()) {
        Iterator<AttributedCharacterIterator.Attribute> keyIterator =
            iterator.getAttributes().keySet().iterator();
        String key;

        if (keyIterator.hasNext()) {
          key = mPlatformNumberFormatter.fieldToString(keyIterator.next(), n);
        } else {
          key = "literal";
        }
        String value = sb.toString();
        sb.setLength(0);

        HashMap<String, String> part = new HashMap<>();
        part.put("type", key);
        part.put("value", value);
        parts.add(part);
      }
    }

    return parts;
  }
}
