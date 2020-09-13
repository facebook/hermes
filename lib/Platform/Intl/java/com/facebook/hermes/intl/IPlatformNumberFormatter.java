package com.facebook.hermes.intl;

import android.icu.text.DecimalFormat;
import android.icu.util.ULocale;

import java.text.AttributedCharacterIterator;

public interface IPlatformNumberFormatter {

    // [[style]]
    // The formatting style to use , the default is "decimal".
    // "decimal" for plain number formatting.
    // "currency" for currency formatting.
    // "percent" for percent formatting
    // "unit" for unit formatting
    enum Style {
        DECIMAL,
        PERCENT,
        CURRENCY,
        UNIT;

        @Override
        public String toString() {
            switch (this) {
                case DECIMAL:
                    return "decimal";
                case PERCENT:
                    return "percent";
                case CURRENCY:
                    return "currency";
                case UNIT:
                    return "unit";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    // [[Notation ]]
    // The formatting that should be displayed for the number, the defaults is "standard"
    //
    //    "standard" plain number formatting
    //    "scientific" return the order-of-magnitude for formatted number.
    //    "engineering" return the exponent of ten when divisible by three
    //    "compact" string representing exponent, defaults is using the "short" form.
    enum Notation {
        STANDARD,
        SCIENTIFIC,
        ENGINEERING,
        COMPACT;

        @Override
        public String toString() {
            switch (this) {
                case STANDARD:
                    return "standard";
                case SCIENTIFIC:
                    return "scientific";
                case ENGINEERING:
                    return "engineering";
                case COMPACT:
                    return "compact";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    // [[compactDisplay]]
    // Only used when notation is "compact". Takes either "short" (default) or "long".
    enum CompactDisplay {
        SHORT,
        LONG;

        @Override
        public String toString() {
            switch (this) {
                case SHORT:
                    return "short";
                case LONG:
                    return "long";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum SignDisplay {
        AUTO,
        ALWAYS,
        NEVER,
        EXCEPTZERO;

        @Override
        public String toString() {
            switch (this) {
                case AUTO:
                    return "auto";
                case ALWAYS:
                    return "always";
                case NEVER:
                    return "never";
                case EXCEPTZERO:
                    return "exceptZero";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum UnitDisplay {
        SHORT,
        NARROW,
        LONG;

        @Override
        public String toString() {
            switch (this) {
                case SHORT:
                    return "short";
                case NARROW:
                    return "narrow";
                case LONG:
                    return "long";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    // How to display the currency in currency formatting. Possible values are:
    // "symbol" to use a localized currency symbol such as €, this is the default value,
    // "narrowSymbol" to use a narrow format symbol ("$100" rather than "US$100"),
    // "code" to use the ISO currency code,
    // "name" to use a localized currency name such as "dollar",
    enum CurrencyDisplay {
        SYMBOL,
        NARROWSYMBOL,
        CODE,
        NAME;

        @Override
        public String toString() {
            switch (this) {
                case SYMBOL:
                    return "symbol";
                case NARROWSYMBOL:
                    return "narrowSymbol";
                case CODE:
                    return "code";
                case NAME:
                    return "name";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    // In many locales, accounting format means to wrap the number with parentheses instead of appending a minus sign.
    // You can enable this formatting by setting the currencySign option to "accounting". The default value is "standard".
    enum CurrencySign {
        STANDARD,
        ACCOUNTING;

        @Override
        public String toString() {
            switch (this) {
                case ACCOUNTING:
                    return "accounting";
                case STANDARD:
                    return "standard";
                default:
                    throw new IllegalArgumentException();
            }
        }
    }

    enum RoundingType {
        SIGNIFICANT_DIGITS,
        FRACTION_DIGITS,
        COMPACT_ROUNDING
    }


    IPlatformNumberFormatter configureDecimalFormat(ILocaleObject localeObject, IPlatformNumberFormatter.Style style,
                                                                 IPlatformNumberFormatter.CurrencySign currencySign,
                                                                 IPlatformNumberFormatter.Notation notation,
                                                                 IPlatformNumberFormatter.CompactDisplay compactDisplay) throws JSRangeErrorException;

    IPlatformNumberFormatter configureCurrency(String currencyCode, CurrencyDisplay currencyDisplay) throws JSRangeErrorException;

    IPlatformNumberFormatter configureGrouping(boolean mGroupingUsed);

    IPlatformNumberFormatter configureMinIntergerDigits(int minimumIntegerDigits);

    IPlatformNumberFormatter configureSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException;

    IPlatformNumberFormatter configureFractinDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits);

    IPlatformNumberFormatter configureSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay);

    IPlatformNumberFormatter configureUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException;


    String getDefaultNumberingSystem(ILocaleObject localeObject) throws JSRangeErrorException;

    String format(double n) throws JSRangeErrorException;

    String fieldToString(AttributedCharacterIterator.Attribute attribute, double x);

    AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException;

    String[] getAvailableLocales();
}

