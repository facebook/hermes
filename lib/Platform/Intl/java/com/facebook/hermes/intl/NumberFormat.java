/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.icu.text.DecimalFormat;
import android.icu.text.DecimalFormatSymbols;
import android.icu.text.MeasureFormat;
import android.icu.util.Measure;
import android.icu.util.MeasureUnit;
import android.icu.util.ULocale;

import org.w3c.dom.Text;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents the Java part of the Android
 * Intl.NumberFormat implementation.  The interaction with the Hermes
 * JaveScript internals are implemented in C++ and should not
 * generally need to be changed.  Implementers' notes here will
 * describe what parts of the ECMA 402 spec remain to be implemented.
 *
 * Also see the implementer' notes on DateTimeFormat.java.
 */
public class NumberFormat {
  // options are localeMatcher:string, numberingSystem:string, notation:string,
  // compactDisplay:string, useGrouping:string, signDisplay:string
  //
  // Implementer note: The ctor corresponds roughly to
  // https://tc39.es/ecma402/#sec-initializenumberformat
  // Also see the implementer notes on DateTimeFormat#DateTimeFormat()

  private IPlatformNumberFormatter.Style mResolvedStyle = null;

  private String mResolvedCurrency = null;
  private IPlatformNumberFormatter.CurrencyDisplay mResolvedCurrencyDisplay = IPlatformNumberFormatter.CurrencyDisplay.symbol;
  private IPlatformNumberFormatter.CurrencySign mResolvedCurrencySign = IPlatformNumberFormatter.CurrencySign.standard;

  private MeasureUnit mResolveMeasureUnitPlatform;
  private String mResolvedUnit = null;
  private IPlatformNumberFormatter.UnitDisplay mResolvedUnitDisplay;

  private boolean mGroupingUsed = true;

  private int mMinimumIntegerDigits = -1, mMaximumIntegerDigits = -1, mMinimumFractionDigits = -1, mMaximumFractionDigits = -1;

  private IPlatformNumberFormatter.SignDisplay mSignDisplay = IPlatformNumberFormatter.SignDisplay.auto;

  private IPlatformNumberFormatter mPlatformNumberFormatter = null;

  private String mNumberingSystem = null;

  private IPlatformNumberFormatter.Notation mNotation = null;
  private IPlatformNumberFormatter.CompactDisplay mCompactDisplay;

  private DecimalFormat mDecimalFormat = null;
  private MeasureFormat mMeasureFormat = null;

  private String resolveStringOption(Map<String, Object> options, String key, String[] possibleValues, String defaultValue) throws JSRangeErrorException {
    if (options.containsKey(key)) {
      String optionValue = (String) options.get(key);
      if (possibleValues.length == 0 ||  TextUtils.containsString(possibleValues, optionValue)) {
        return optionValue;
      } else {
        throw new JSRangeErrorException(String.format("Invalid value '%s' for option %s", optionValue, key));
      }
    } else {
      return defaultValue;
    }
  }

  private boolean resolveBooleanOption(Map<String, Object> options, String key, boolean defaultValue) throws JSRangeErrorException {
    if (options.containsKey(key)) {
      // Note:: Our JSI interop layer ensures that this object is indeed a boolean
      return (boolean) options.get(key);
    } else {
      return defaultValue;
    }
  }

  private int resolveIntegerOption(Map<String, Object> options, String key, int defaultValue) throws JSRangeErrorException {
    if (options.containsKey(key)) {
      return ((Double)options.get(key)).intValue();
    } else {
      return defaultValue;
    }
  }


  public static <T extends Enum<?>> T searchEnum(Class<T> enumeration,
                                                 String search) {
    for (T each : enumeration.getEnumConstants()) {
      if (each.name().compareToIgnoreCase(search) == 0) {
        return each;
      }
    }
    return null;
  }

  public NumberFormat(List<String> locales, Map<String, Object> options) throws JSRangeErrorException {
    PlatformCollator.LocaleResolutionResult localeResolutionResult = PlatformNumberFormatter.resolveLocales(locales, "best fit");

    // TODO :: Make is more robust.
    mResolvedStyle = Enum.valueOf(IPlatformNumberFormatter.Style.class, resolveStringOption(options, "style", new String[]{"decimal", "currency", "percent", "unit"}, "decimal"));

    mGroupingUsed = resolveBooleanOption(options, "useGrouping", true);

    mMinimumIntegerDigits = resolveIntegerOption(options, "minimumIntegerDigits", -1);
    mMaximumIntegerDigits = resolveIntegerOption(options, "maximumIntegerDigits", -1);
    mMinimumFractionDigits = resolveIntegerOption(options, "minimumFractionDigits", -1);
    mMaximumFractionDigits = resolveIntegerOption(options, "maximumFractionDigits", -1);

    mSignDisplay = Enum.valueOf(IPlatformNumberFormatter.SignDisplay.class, resolveStringOption(options, "signDisplay", new String[]{"auto", "never", "always", "exceptZero"}, "auto"));

    if(options.containsKey("numberingSystem")) {
      mNumberingSystem = resolveStringOption(options, "numberingSystem", new String[]{}, "");

      ArrayList<String> numberingSystemList = new ArrayList<>();
      numberingSystemList.add(mNumberingSystem);
      localeResolutionResult.resolvedLocale.setUnicodeExtensions("nu", numberingSystemList);
    }

    mNotation = Enum.valueOf(IPlatformNumberFormatter.Notation.class, resolveStringOption(options, "notation", new String[]{"standard", "scientific", "engineering", "compact"}, "standard"));
    if(mNotation != IPlatformNumberFormatter.Notation.standard) {
      mCompactDisplay = searchEnum(IPlatformNumberFormatter.CompactDisplay.class, resolveStringOption(options, "compactDisplay", new String[]{"short", "long"}, "short"));
    }

    switch (mResolvedStyle) {
      case currency:
        mResolvedCurrency = resolveStringOption(options, "currency", new String[]{}, "");
        mResolvedCurrencyDisplay = Enum.valueOf(IPlatformNumberFormatter.CurrencyDisplay.class, resolveStringOption(options, "currencyDisplay", new String[]{"symbol", "narrowSymbol", "code", "name"}, "symbol"));
        mResolvedCurrencySign = Enum.valueOf(IPlatformNumberFormatter.CurrencySign.class, resolveStringOption(options, "currencySign", new String[]{"accounting", "standard"}, "standard"));

        mDecimalFormat = PlatformDecimalFormatHelperICU4J.createCurrencyFormat((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mResolvedCurrency, mResolvedCurrencyDisplay, mResolvedCurrencySign);
        // mPlatformNumberFormatter = PlatformNumberFormatter.createCurrencyFormatter(localeResolutionResult.resolvedLocale, mResolvedCurrency, mResolvedCurrencyDisplay, mResolvedCurrencySign, mSignDisplay, mGroupingUsed, mMinimumIntegerDigits, mMaximumIntegerDigits, mMinimumFractionDigits, mMaximumFractionDigits, mNumberingSystem);
        break;

      case unit:
        mResolvedUnit = resolveStringOption(options, "unit", new String[]{}, "");
        String unitDisplayStr = resolveStringOption(options, "unitDisplay", new String[]{"long", "short", "narrow"}, "short");
        mResolvedUnitDisplay = searchEnum(IPlatformNumberFormatter.UnitDisplay.class, unitDisplayStr);
        mResolveMeasureUnitPlatform = PlatformDecimalFormatHelperICU4J.parseUnit(mResolvedUnit);
        //mPlatformNumberFormatter = PlatformNumberFormatter.createUnitFormatter(localeResolutionResult.resolvedLocale, mResolvedUnit, mResolvedUnitDisplay, mSignDisplay, mGroupingUsed, mMinimumIntegerDigits, mMaximumIntegerDigits, mMinimumFractionDigits, mMaximumFractionDigits, mNumberingSystem);

        mDecimalFormat = PlatformDecimalFormatHelperICU4J.createStandardFormat((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mNotation, mCompactDisplay);
        mMeasureFormat = PlatformDecimalFormatHelperICU4J.createMeasureFormat((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mDecimalFormat, mResolvedUnit, mResolvedUnitDisplay);
        break;

      case percent:
        mDecimalFormat = PlatformDecimalFormatHelperICU4J.createPercentFormat((ULocale) localeResolutionResult.resolvedLocale.getLocale());
        // mPlatformNumberFormatter = PlatformNumberFormatter.createPercentFormatter(localeResolutionResult.resolvedLocale, mGroupingUsed, mSignDisplay, mMinimumIntegerDigits, mMaximumIntegerDigits, mMinimumFractionDigits, mMaximumFractionDigits, mNumberingSystem);
        break;

      default:
        mDecimalFormat = PlatformDecimalFormatHelperICU4J.createStandardFormat((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mNotation, mCompactDisplay);
        // mPlatformNumberFormatter = PlatformNumberFormatter.createDecimalFormatter(localeResolutionResult.resolvedLocale, mGroupingUsed, mSignDisplay, mMinimumIntegerDigits, mMaximumIntegerDigits, mMinimumFractionDigits, mMaximumFractionDigits, mNumberingSystem, mNotation, mCompactDisplay);
        break;
    }

    PlatformDecimalFormatHelperICU4J.configureGrouping((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mDecimalFormat, mGroupingUsed);
    PlatformDecimalFormatHelperICU4J.configureMinMaxDigits((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mDecimalFormat, mMinimumIntegerDigits, mMaximumIntegerDigits, mMinimumFractionDigits, mMaximumFractionDigits);
    PlatformDecimalFormatHelperICU4J.configureSignDisplay((ULocale) localeResolutionResult.resolvedLocale.getLocale(), mDecimalFormat, mSignDisplay);
  }

  // options are localeMatcher:string
  //
  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.supportedlocalesof
  //
  // The notes on DateTimeFormat#DateTimeFormat() for Locales and
  // Options also apply here.
  public static List<String> supportedLocalesOf(List<String> locales, Map<String, Object> options) {
    return locales;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-intl.numberformat.prototype.resolvedoptions
  //
  // Also see the implementer notes on DateTimeFormat#resolvedOptions()
  public Map<String, Object> resolvedOptions() {
    return new HashMap<String, Object>();
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumber
  public String format(double n) {
    String result = null;
    if(mResolvedStyle == IPlatformNumberFormatter.Style.unit)
      result = mMeasureFormat.format(new Measure(n, mResolveMeasureUnitPlatform));
    else
      result = mDecimalFormat.format(n);

    return result;
  }

  // Implementer note: This method corresponds roughly to
  // https://tc39.es/ecma402/#sec-formatnumbertoparts
  public List<Map<String, String>> formatToParts(double n) throws JSRangeErrorException {
    final String formattedNumber = format(n);

    final String fullPatten = mDecimalFormat.toPattern();
    final DecimalFormatSymbols symbols = mDecimalFormat.getDecimalFormatSymbols();

    List<Map<String, String>> parts = new ArrayList<>();
    boolean decimalSeparatorSeen = false;

    int formattedNumberCursor = 0;
    while (formattedNumberCursor < formattedNumber.length()) {
      final char formattedChar = formattedNumber.charAt(formattedNumberCursor);

      if (formattedChar == symbols.getPerMill()) {
        parts.add(new HashMap<String, String>() {{ put("permill", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      } else if (formattedChar == symbols.getPercent()) {
        parts.add(new HashMap<String, String>() {{ put("percent", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      } else if (formattedChar == symbols.getDecimalSeparator() || formattedChar == symbols.getMonetaryDecimalSeparator()) {
        parts.add(new HashMap<String, String>() {{ put("decimal", String.valueOf(formattedChar)); }});
        decimalSeparatorSeen = true;
        formattedNumberCursor++;
      }  else if (formattedChar == symbols.getGroupingSeparator() || formattedChar == symbols.getMonetaryGroupingSeparator()) {
        parts.add(new HashMap<String, String>() {{ put("group", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      } else if (formattedChar == symbols.getMinusSign()) {
        parts.add(new HashMap<String, String>() {{ put("minusSign", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      } else if (formattedChar == symbols.getPlusSign()) {
        parts.add(new HashMap<String, String>() {{ put("plusSign", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      } else if (Character.isDigit(formattedChar)) {
        // Find the next number block.
        final StringBuffer numberBlockBuffer = new StringBuffer();
        char formattedCurrentChar = formattedNumber.charAt(formattedNumberCursor);
        while (formattedNumberCursor < formattedNumber.length() && Character.isDigit(formattedCurrentChar)) {
          numberBlockBuffer.append(formattedCurrentChar);
          formattedNumberCursor++;
          formattedCurrentChar = formattedNumber.charAt(formattedNumberCursor);
        }

        if (decimalSeparatorSeen) {
          parts.add(new HashMap<String, String>() {{ put("fraction", numberBlockBuffer.toString()); }});
        } else {
          parts.add(new HashMap<String, String>() {{ put("integer", numberBlockBuffer.toString()); }});
        }

      } else if (formattedNumber.substring(formattedNumberCursor, symbols.getNaN().length()).equals(symbols.getNaN())) {
        parts.add(new HashMap<String, String>() {{ put("nan", symbols.getNaN()); }});
        formattedNumberCursor += symbols.getNaN().length();
      } else if (formattedNumber.substring(formattedNumberCursor, symbols.getInfinity().length()).equals(symbols.getInfinity())) {
        parts.add(new HashMap<String, String>() {{ put("infinity", symbols.getInfinity()); }});
        formattedNumberCursor += symbols.getInfinity().length();
      } else if (formattedNumber.substring(formattedNumberCursor, symbols.getExponentSeparator().length()).equals(symbols.getExponentSeparator())) {
        parts.add(new HashMap<String, String>() {{ put("exponentSeparator", symbols.getExponentSeparator()); }});
        formattedNumberCursor += symbols.getExponentSeparator().length();
      } else if (formattedNumber.substring(formattedNumberCursor, symbols.getCurrencySymbol().length()).equals(symbols.getCurrencySymbol())) {
        parts.add(new HashMap<String, String>() {{ put("currency", symbols.getExponentSeparator()); }});
        formattedNumberCursor += symbols.getCurrencySymbol().length();
      }
      else {

        // TODO :: Support units.

        parts.add(new HashMap<String, String>() {{ put("literal", String.valueOf(formattedChar)); }});
        formattedNumberCursor++;
      }
    }

    return parts;
  }
}

