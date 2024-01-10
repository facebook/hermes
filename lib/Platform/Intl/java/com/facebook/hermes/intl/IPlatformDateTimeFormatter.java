/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.facebook.hermes.intl;

import android.os.Build;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import java.text.AttributedCharacterIterator;
import java.util.Map;
import java.util.Set;

public interface IPlatformDateTimeFormatter {
  enum FormatMatcher {
    BESTFIT,
    BASIC;

    @Override
    public String toString() {
      switch (this) {
        case BESTFIT:
          return "best fit";
        case BASIC:
          return "basic";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum HourCycle {
    H11("h11", 'K'),
    H12("h12", 'h'),
    H23("h23", 'H'),
    H24("h24", 'k'),
    UNDEFINED("", '\0');

    final private String value;
    final private char skeleton;

    HourCycle(String value, char skeleton) {
      this.value = value;
      this.skeleton = skeleton;
    }

    public char getSkeletonSymbol() {
     return this.skeleton;
    }

    @NonNull
    @Override
    public String toString() {
      return this.value;
    }
  }

  enum WeekDay {
    LONG,
    SHORT,
    NARROW,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case LONG:
          return "long";
        case SHORT:
          return "short";
        case NARROW:
          return "narrow";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case LONG:
          return "EEEE";
        case SHORT:
          return "EEE";
        case NARROW:
          return "EEEEE";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Era {
    LONG,
    SHORT,
    NARROW,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case LONG:
          return "long";
        case SHORT:
          return "short";
        case NARROW:
          return "narrow";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case LONG:
          return "GGGG";
        case SHORT:
          return "GGG";
        case NARROW:
          return "G5";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Year {
    NUMERIC,
    DIGIT2,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case NUMERIC:
          return "yyyy";
        case DIGIT2:
          return "yy";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Month {
    NUMERIC,
    DIGIT2,
    LONG,
    SHORT,
    NARROW,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case LONG:
          return "long";
        case SHORT:
          return "short";
        case NARROW:
          return "narrow";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case NUMERIC:
          return "M";
        case DIGIT2:
          return "MM";
        case LONG:
          return "MMMM";
        case SHORT:
          return "MMM";
        case NARROW:
          return "MMMMM";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Day {
    NUMERIC,
    DIGIT2,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case NUMERIC:
          return "d";
        case DIGIT2:
          return "dd";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Hour {
    NUMERIC,
    DIGIT2,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol(HourCycle hourCycle) {
      char hc = hourCycle.getSkeletonSymbol();

      if (hc == '\0') return "";

      switch (this) {
        case NUMERIC:
          return String.format("%s", hc);
        case DIGIT2:
          return String.format("%s%s", hc, hc);
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }


  }

  enum Minute {
    NUMERIC,
    DIGIT2,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case NUMERIC:
          return "m";
        case DIGIT2:
          return "mm";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum Second {
    NUMERIC,
    DIGIT2,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NUMERIC:
          return "numeric";
        case DIGIT2:
          return "2-digit";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case NUMERIC:
          return "s";
        case DIGIT2:
          return "ss";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum TimeZoneName {
    LONG,
    LONGOFFSET,
    LONGGENERIC,
    SHORT,
    SHORTOFFSET,
    SHORTGENERIC,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case LONG:
          return "long";
        case LONGOFFSET:
          return "longOffset";
        case LONGGENERIC:
          return "longGeneric";
        case SHORT:
          return "short";
        case SHORTOFFSET:
          return "shortOffset";
        case SHORTGENERIC:
          return "shortGeneric";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbol() {
      switch (this) {
        case LONG:
          return "zzzz";
        case LONGOFFSET:
          return "OOOO";
        case LONGGENERIC:
          return "vvvv";
        case SHORT:
          return "z";
        case SHORTOFFSET:
          return "O";
        case SHORTGENERIC:
          return "v";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum DayPeriod {
    NARROW,
    SHORT,
    LONG,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case NARROW:
          return "narrow";
        case SHORT:
          return "short";
        case LONG:
          return "long";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    // Day period is only supported on API 28+
    @RequiresApi(api = Build.VERSION_CODES.P)
    public String getSkeletonSymbol() {
      switch (this) {
        case NARROW:
          return "BBBBB";
        case SHORT:
          return "B";
        case LONG:
          return "BBBB";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }

    public String getSkeletonSymbolFallback() {
      switch (this) {
        case NARROW:
          return "aaaaa";
        case SHORT:
          return "a";
        case LONG:
          return "aaaa";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum DateStyle {
    FULL,
    LONG,
    MEDIUM,
    SHORT,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case FULL:
          return "full";
        case LONG:
          return "long";
        case MEDIUM:
          return "medium";
        case SHORT:
          return "short";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  enum TimeStyle {
    FULL,
    LONG,
    MEDIUM,
    SHORT,
    UNDEFINED;

    @Override
    public String toString() {
      switch (this) {
        case FULL:
          return "full";
        case LONG:
          return "long";
        case MEDIUM:
          return "medium";
        case SHORT:
          return "short";
        case UNDEFINED:
          return "";
        default:
          throw new IllegalArgumentException();
      }
    }
  }

  void configure(
      ILocaleObject<?> resolvedLocaleObject,
      String calendar,
      String numberingSystem,
      FormatMatcher formatMatcher,
      WeekDay weekDay,
      Era era,
      Year year,
      Month month,
      Day day,
      Hour hour,
      Minute minute,
      Second second,
      TimeZoneName timeZoneName,
      HourCycle hourCycle,
      Object timeZone,
      DateStyle dateStyle,
      TimeStyle timeStyle,
      Object hour12,
      DayPeriod dayPeriod,
      @Nullable Integer fractionalSecondDigits)
      throws JSRangeErrorException;

  String format(double n) throws JSRangeErrorException;

  String formatRange(double from, double to) throws JSRangeErrorException;

  String fieldAttrsToSourceString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs);

  String fieldAttrsToTypeString(Set<Map.Entry<AttributedCharacterIterator.Attribute, Object>> attrs, String fieldValue);

  AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException;

  AttributedCharacterIterator formatRangeToParts(double from, double to) throws JSRangeErrorException;

  String getDefaultCalendarName(ILocaleObject<?> mResolvedLocaleObject)
      throws JSRangeErrorException;

  HourCycle getHourCycle12(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  HourCycle getHourCycle24(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  HourCycle getHourCycle(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  String getDefaultTimeZone(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  String getDefaultNumberingSystem(ILocaleObject<?> localeObject) throws JSRangeErrorException;

  String[] getAvailableLocales();
}
