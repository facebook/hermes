package com.facebook.hermes.intl;

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


    String format(double number);
}

