/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

public interface IPlatformCollator {

  enum Sensitivity {
    BASE,
    ACCENT,
    CASE,
    VARIANT,
    LOCALE;

    @Override
    public String toString() {
      switch (this) {
        case BASE:
          return "base";
        case ACCENT:
          return "accent";
        case CASE:
          return "case";
        case VARIANT:
          return "variant";
        case LOCALE:
          return ""; // The user should fill in the locale default.
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Usage {
    SORT,
    SEARCH;

    @Override
    public String toString() {
      switch (this) {
        case SORT:
          return "sort";
        case SEARCH:
          return "search";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum CaseFirst {
    UPPER,
    LOWER,
    FALSE;

    @Override
    public String toString() {
      switch (this) {
        case UPPER:
          return "upper";
        case LOWER:
          return "lower";
        case FALSE:
          return "false";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  IPlatformCollator configure(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  Sensitivity getSensitivity();

  IPlatformCollator setSensitivity(Sensitivity sensitivity);

  IPlatformCollator setIgnorePunctuation(boolean ignore);

  IPlatformCollator setNumericAttribute(boolean numeric);

  IPlatformCollator setCaseFirstAttribute(CaseFirst caseFirst);

  int compare(String source, String target);

  String[] getAvailableLocales();
}
