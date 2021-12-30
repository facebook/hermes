/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import static com.facebook.hermes.intl.IPlatformCollator.Sensitivity.*;
import static com.facebook.hermes.intl.IPlatformCollator.Sensitivity.BASE;

import android.icu.text.Collator;
import android.icu.text.RuleBasedCollator;
import android.os.Build;
import androidx.annotation.RequiresApi;
import java.util.ArrayList;

public class PlatformCollatorICU implements IPlatformCollator {

  private android.icu.text.RuleBasedCollator mCollator = null;

  PlatformCollatorICU() {}

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public IPlatformCollator configure(ILocaleObject<?> localeObject) throws JSRangeErrorException {
    LocaleObjectICU mLocale = (LocaleObjectICU) localeObject;

    mCollator = (RuleBasedCollator) RuleBasedCollator.getInstance(mLocale.getLocale());

    // Normalization is always on by the spec. We don't know whether the text is already normalized,
    // hence we can't optimize as of now.
    mCollator.setDecomposition(android.icu.text.Collator.CANONICAL_DECOMPOSITION);

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public int compare(String source, String target) {
    return mCollator.compare(source, target);
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public Sensitivity getSensitivity() {
    if (mCollator == null) {
      return LOCALE; // TODO: Ad-hoc default
    }

    int strength = mCollator.getStrength();
    if (strength == android.icu.text.Collator.PRIMARY) {
      if (mCollator.isCaseLevel()) return CASE;
      else return BASE;
    }

    if (strength == Collator.SECONDARY) return ACCENT;

    return VARIANT;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public IPlatformCollator setSensitivity(IPlatformCollator.Sensitivity sensitivity) {
    switch (sensitivity) {
      case BASE:
        mCollator.setStrength(android.icu.text.Collator.PRIMARY);
        break;
      case ACCENT:
        mCollator.setStrength(android.icu.text.Collator.SECONDARY);
        break;
      case CASE:
        mCollator.setStrength(android.icu.text.Collator.PRIMARY);
        mCollator.setCaseLevel(true);
        break;
      case VARIANT:
        mCollator.setStrength(android.icu.text.Collator.TERTIARY);
        break;
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public IPlatformCollator setIgnorePunctuation(boolean ignore) {
    if (ignore) {
      // Read for an explanation:
      // http://userguide.icu-project.org/collation/customization/ignorepunct
      mCollator.setAlternateHandlingShifted(true);
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public IPlatformCollator setNumericAttribute(boolean numeric) {
    if (numeric) {
      mCollator.setNumericCollation(JSObjects.getJavaBoolean(true));
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public IPlatformCollator setCaseFirstAttribute(CaseFirst caseFirst) {
    switch (caseFirst) {
      case UPPER:
        mCollator.setUpperCaseFirst(true);
        break;

      case LOWER:
        mCollator.setLowerCaseFirst(true);
        break;

      case FALSE:
      default:
        mCollator.setCaseFirstDefault();
        break;
    }

    return this;
  }

  @RequiresApi(api = Build.VERSION_CODES.N)
  @Override
  public String[] getAvailableLocales() {
    ArrayList<String> availableLocaleIds = new ArrayList<>();

    // [[Comment copied here from NumberFormat]]
    // NumberFormat.getAvailableLocales() returns a shorter list compared to
    // ULocale.getAvailableLocales.
    // For e.g. "zh-TW" is missing in the list returned by NumberFormat.getAvailableLocales() in my
    // emulator.
    // But, NumberFormatter is able to format specific to "zh-TW" .. for instance "NaN" is expected
    // to be formatted as "非數值" in "zh-TW" by as "NaN" in "zh"
    // In short, NumberFormat.getAvailableLocales() doesn't contain all the locales as the
    // NumberFormat can format. Hence, using ULocale.getAvailableLocales()
    //
    // java.util.Locale[] availableLocales = android.icu.text.Collator.getAvailableLocales();
    android.icu.util.ULocale[] availableLocales = android.icu.util.ULocale.getAvailableLocales();

    for (android.icu.util.ULocale locale : availableLocales) {
      availableLocaleIds.add(locale.toLanguageTag()); // TODO:: Not available on platforms <= 20
    }

    String[] availableLocaleIdsArray = new String[availableLocaleIds.size()];
    return availableLocaleIds.toArray(availableLocaleIdsArray);
  }
}
