package com.facebook.hermes.intl;

import android.icu.text.DecimalFormatSymbols;
import android.icu.util.Currency;
import android.icu.util.ULocale;

import java.text.AttributedCharacterIterator;
import java.util.List;
import java.util.Map;

import static com.facebook.hermes.intl.IPlatformNumberFormatter.Style.currency;

public interface IPlatformNumberFormatter {

    // [[style]]
    // The formatting style to use , the default is "decimal".
    // "decimal" for plain number formatting.
    // "currency" for currency formatting.
    // "percent" for percent formatting
    // "unit" for unit formatting
    enum Style {
        decimal, percent, currency, unit
    }

    // [[Notation ]]
    // The formatting that should be displayed for the number, the defaults is "standard"
    //
    //    "standard" plain number formatting
    //    "scientific" return the order-of-magnitude for formatted number.
    //    "engineering" return the exponent of ten when divisible by three
    //    "compact" string representing exponent, defaults is using the "short" form.
    enum Notation {
        standard,
        scientific,
        engineering,
        compact
    }

    // [[compactDisplay]]
    // Only used when notation is "compact". Takes either "short" (default) or "long".
    enum CompactDisplay {
        SHORT,
        LONG
    }

    enum SignDisplay {
        auto,
        always,
        never,
        exceptZero
    }

    enum UnitDisplay {
        SHORT,
        narrow,
        LONG
    }


    // How to display the currency in currency formatting. Possible values are:
    // "symbol" to use a localized currency symbol such as €, this is the default value,
    // "narrowSymbol" to use a narrow format symbol ("$100" rather than "US$100"),
    // "code" to use the ISO currency code,
    // "name" to use a localized currency name such as "dollar",
    enum CurrencyDisplay {
        symbol,
        narrowSymbol,
        code,
        name,
    }

    // In many locales, accounting format means to wrap the number with parentheses instead of appending a minus sign.
    // You can enable this formatting by setting the currencySign option to "accounting". The default value is "standard".
    enum CurrencySign {
        standard,
        accounting
    }

    enum RoundingType {
        significantDigits,
        fractionDigits,
        compactRounding
    }


    IPlatformNumberFormatter configureCurrency(String currencyCode, CurrencyDisplay currencyDisplay) throws JSRangeErrorException;
    IPlatformNumberFormatter configureGrouping(boolean mGroupingUsed);
    IPlatformNumberFormatter configureMinIntergerDigits(int minimumIntegerDigits);
    IPlatformNumberFormatter configureSignificantDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumSignificantDigits, int maximumSignificantDigits) throws JSRangeErrorException ;
    IPlatformNumberFormatter configureFractinDigits(IPlatformNumberFormatter.RoundingType roundingType, int minimumFractionDigits, int maximumFractionDigits);
    IPlatformNumberFormatter configureSignDisplay(IPlatformNumberFormatter.SignDisplay signDisplay);
    IPlatformNumberFormatter configureUnits(String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) throws JSRangeErrorException;

    String format(double n) throws JSRangeErrorException;
    AttributedCharacterIterator formatToParts(double n) throws JSRangeErrorException;
}

