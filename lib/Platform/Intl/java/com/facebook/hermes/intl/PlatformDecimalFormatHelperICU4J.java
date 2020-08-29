package com.facebook.hermes.intl;

import android.icu.text.CompactDecimalFormat;
import android.icu.text.DateFormat;
import android.icu.text.DecimalFormat;
import android.icu.text.DecimalFormatSymbols;
import android.icu.text.MeasureFormat;
import android.icu.text.NumberFormat;
import android.icu.util.Currency;
import android.icu.util.MeasureUnit;
import android.icu.util.ULocale;

import static android.icu.text.NumberFormat.SCIENTIFICSTYLE;

public class PlatformDecimalFormatHelperICU4J {

    public static DecimalFormat createCurrencyFormat(ULocale locale, String currencyCode, IPlatformNumberFormatter.CurrencyDisplay currencyDisplay, IPlatformNumberFormatter.CurrencySign currencySign) throws JSRangeErrorException {
        DecimalFormat decimalFormat = null;
        switch (currencySign) {
            case accounting:
                decimalFormat = (DecimalFormat) android.icu.text.NumberFormat.getInstance(locale, android.icu.text.NumberFormat.ACCOUNTINGCURRENCYSTYLE);
                break;
            case standard:
                decimalFormat = (DecimalFormat) android.icu.text.NumberFormat.getInstance(locale, NumberFormat.CURRENCYSTYLE);
                break;
            default:
                throw new JSRangeErrorException("Unsupported currency sign !!");
        }

        Currency currency = Currency.getInstance(currencyCode);
        decimalFormat.setCurrency(currency);

        String currencySymbol = currency.getName(locale, Currency.SYMBOL_NAME, null);
        switch (currencyDisplay) {
            case name:
                currencySymbol = currency.getName(locale, Currency.LONG_NAME, null);
                break;
            case code:
                currencySymbol = currencyCode;
                break;
        }

        DecimalFormatSymbols formatSymbols = decimalFormat.getDecimalFormatSymbols();
        formatSymbols.setCurrencySymbol(currencySymbol);

        decimalFormat.setDecimalFormatSymbols(formatSymbols);

        return decimalFormat;

    }

    public static DecimalFormat createPercentFormat(ULocale locale) {
        DecimalFormat decimalFormat = (DecimalFormat) DecimalFormat.getInstance(locale, NumberFormat.PERCENTSTYLE);
        return decimalFormat;
    }

    public static DecimalFormat createStandardFormat(ULocale locale, IPlatformNumberFormatter.Notation notation, IPlatformNumberFormatter.CompactDisplay compactDisplay) {
        DecimalFormat decimalFormat = null;
        switch (notation) {
            case compact:
                decimalFormat = (DecimalFormat) CompactDecimalFormat.getInstance(locale, compactDisplay == IPlatformNumberFormatter.CompactDisplay.SHORT ? CompactDecimalFormat.CompactStyle.SHORT :  CompactDecimalFormat.CompactStyle.LONG);
                break;
            case scientific:
                decimalFormat = (DecimalFormat) DecimalFormat.getInstance(locale, SCIENTIFICSTYLE);
                break;
            case engineering: // TODO :: Not tested.
                decimalFormat = (DecimalFormat) DecimalFormat.getInstance(locale, SCIENTIFICSTYLE);
                decimalFormat.setMaximumIntegerDigits(3);
                decimalFormat.setMinimumIntegerDigits(2);
                break;
            case standard:
            default:
                decimalFormat = (DecimalFormat) DecimalFormat.getNumberInstance(locale);
        }
        return decimalFormat;
    }


    public static MeasureUnit parseUnit(String inUnit) throws JSRangeErrorException {
        String unit = null;
        if(inUnit.contains("-")) {
            unit = inUnit.substring(inUnit.indexOf('-') + 1);
        } else {
            unit = inUnit;
        }

        unit = unit.replaceAll("-", "_");

        switch (unit) {
            // https://github.com/unicode-org/cldr/blob/master/common/validity/unit.xml
            // Replaced "^[a-z]*-(.*)" with "\\\\$0\n            case "$1": return MeasureUnit.$1;"
            // Then find "\..*", Select all occurance from menu and "transform to uppercase" in command palette.
            // acceleration_g_force
            case "g_force": return MeasureUnit.G_FORCE;
            // acceleration_meter_per_square_second
            case "meter_per_square_second": return MeasureUnit.METER_PER_SECOND_SQUARED;
            // angle_arc_minute
            case "arc_minute": return MeasureUnit.ARC_MINUTE;
            // angle_arc_second
            case "arc_second": return MeasureUnit.ARC_SECOND;
            // angle_degree
            case "degree": return MeasureUnit.DEGREE;
            // angle_radian
            case "radian": return MeasureUnit.RADIAN;
            // angle_revolution
            case "revolution": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.REVOLUTION;
                // area_acre
            case "acre": return MeasureUnit.ACRE;
            // area_hectare
            case "hectare": return MeasureUnit.HECTARE;
            // area_square_centimeter
            case "square_centimeter": return MeasureUnit.SQUARE_CENTIMETER;
            // area_square_foot
            case "square_foot": return MeasureUnit.SQUARE_FOOT;
            // area_square_inch
            case "square_inch": return MeasureUnit.SQUARE_INCH;
            // area_square_kilometer
            case "square_kilometer": return MeasureUnit.SQUARE_KILOMETER;
            // area_square_meter
            case "square_meter": return MeasureUnit.SQUARE_METER;
            // area_square_mile
            case "square_mile": return MeasureUnit.SQUARE_MILE;
            // area_square_yard
            case "square_yard": return MeasureUnit.SQUARE_YARD;
            // area_dunam
            case "dunam": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DUNAM;
                // concentr_karat
            case "karat": return MeasureUnit.KARAT;
            // concentr_milligram_per_deciliter
            case "milligram_per_deciliter": throw new JSRangeErrorException("Unsupported measure unit !"); //return MeasureUnit.MILLIGRAM_PER_DECILITER;
                // concentr_millimole_per_liter
            case "millimole_per_liter": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.MILLIMOLE_PER_LITER;
                // concentr_percent
            case "percent": throw new JSRangeErrorException("Unsupported measure unit !");  //MeasureUnit.PERCENT;
                // concentr_permille
            case "permille": throw new JSRangeErrorException("Unsupported measure unit !"); // // return MeasureUnit.PERMILLE;
                // concentr_permyriad
            case "permyriad": throw new JSRangeErrorException("Unsupported measure unit !"); // // return MeasureUnit.PERMYRIAD;
                // concentr_permillion
            case "permillion": throw new JSRangeErrorException("Unsupported measure unit !"); // // return MeasureUnit.PERMILLION;
                // concentr_mole
            case "mole": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.MOLE;
                // concentr_item
            case "item": throw new JSRangeErrorException("Unsupported measure unit !"); //return MeasureUnit.ITEM;
                // concentr_portion
            case "portion": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.PORTION;
                // consumption_liter_per_100_kilometer
            case "liter_per_100_kilometer": throw new JSRangeErrorException("Unsupported measure unit !"); //return MeasureUnit.LITER_PER_100_KILOMETER;
                // consumption_liter_per_kilometer
            case "liter_per_kilometer": return MeasureUnit.LITER_PER_KILOMETER;
            // consumption_mile_per_gallon
            case "mile_per_gallon": return MeasureUnit.MILE_PER_GALLON;
            // consumption_mile_per_gallon_imperial
            case "mile_per_gallon_imperial": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.MILE_PER_GALLON_IMPERIAL;
                // digital_bit
            case "bit": return MeasureUnit.BIT;
            // digital_byte
            case "byte": return MeasureUnit.BYTE;
            // digital_gigabit
            case "gigabit": return MeasureUnit.GIGABIT;
            // digital_gigabyte
            case "gigabyte": return MeasureUnit.GIGABYTE;
            // digital_kilobit
            case "kilobit": return MeasureUnit.KILOBIT;
            // digital_kilobyte
            case "kilobyte": return MeasureUnit.KILOBYTE;
            // digital_megabit
            case "megabit": return MeasureUnit.MEGABIT;
            // digital_megabyte
            case "megabyte": return MeasureUnit.MEGABYTE;
            // digital_petabyte
            case "petabyte": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.PETABYTE;
                // digital_terabit
            case "terabit": return MeasureUnit.TERABIT;
            // digital_terabyte
            case "terabyte": return MeasureUnit.TERABYTE;
            // duration_century
            case "century": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.CENTURY;
                // duration_decade
            case "decade": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.DECADE;
                // duration_day
            case "day": return MeasureUnit.DAY;
            // duration_day_person
            case "day_person": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.DAY_PERSON;
                // duration_hour
            case "hour": return MeasureUnit.HOUR;
            // duration_microsecond
            case "microsecond": return MeasureUnit.MICROSECOND;
            // duration_millisecond
            case "millisecond": return MeasureUnit.MILLISECOND;
            // duration_minute
            case "minute": return MeasureUnit.MINUTE;
            // duration_month
            case "month": return MeasureUnit.MONTH;
            // duration_month_person
            case "month_person": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.MONTH_PERSON;
                // duration_nanosecond
            case "nanosecond": return MeasureUnit.NANOSECOND;
            // duration_second
            case "second": return MeasureUnit.SECOND;
            // duration_week
            case "week": return MeasureUnit.WEEK;
            // duration_week_person
            case "week_person": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.WEEK_PERSON;
                // duration_year
            case "year": return MeasureUnit.YEAR;
            // duration_year_person
            case "year_person": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.YEAR_PERSON;
                // electric_ampere
            case "ampere": return MeasureUnit.AMPERE;
            // electric_milliampere
            case "milliampere": return MeasureUnit.MILLIAMPERE;
            // electric_ohm
            case "ohm": return MeasureUnit.OHM;
            // electric_volt
            case "volt": return MeasureUnit.VOLT;
            // energy_calorie
            case "calorie": return MeasureUnit.CALORIE;
            // energy_foodcalorie
            case "foodcalorie": return MeasureUnit.FOODCALORIE;
            // energy_joule
            case "joule": return MeasureUnit.JOULE;
            // energy_kilocalorie
            case "kilocalorie": return MeasureUnit.KILOCALORIE;
            // energy_kilojoule
            case "kilojoule": return MeasureUnit.KILOJOULE;
            // energy_kilowatt_hour
            case "kilowatt_hour": return MeasureUnit.KILOWATT_HOUR;
            // energy_electronvolt
            case "electronvolt": throw new JSRangeErrorException("Unsupported measure unit !"); //return MeasureUnit.ELECTRONVOLT;
                // energy_therm_us
            case "therm_us": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.THERM_US;
                // energy_british_thermal_unit
            case "british_thermal_unit": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.BRITISH_THERMAL_UNIT;
                // force_pound_force
            case "pound_force": throw new JSRangeErrorException("Unsupported measure unit !");  //return MeasureUnit.POUND_FORCE;
                // force_newton
            case "newton": throw new JSRangeErrorException("Unsupported measure unit !"); //return MeasureUnit.NEWTON;
                // frequency_gigahertz
            case "gigahertz": return MeasureUnit.GIGAHERTZ;
            // frequency_hertz
            case "hertz": return MeasureUnit.HERTZ;
            // frequency_kilohertz
            case "kilohertz": return MeasureUnit.KILOHERTZ;
            // frequency_megahertz
            case "megahertz": return MeasureUnit.MEGAHERTZ;
            // graphics_dot
            case "dot": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DOT;
                // graphics_dot_per_centimeter
            case "dot_per_centimeter": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DOT_PER_CENTIMETER;
                // graphics_dot_per_inch
            case "dot_per_inch": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DOT_PER_INCH;
                // graphics_em
            case "em": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.EM;
                // graphics_megapixel
            case "megapixel": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.MEGAPIXEL;
                // graphics_pixel
            case "pixel": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PIXEL;
                // graphics_pixel_per_centimeter
            case "pixel_per_centimeter": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PIXEL_PER_CENTIMETER;
                // graphics_pixel_per_inch
            case "pixel_per_inch": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PIXEL_PER_INCH;
                // length_100_kilometer
            case "100_kilometer": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.100_KILOMETER;
                // length_astronomical_unit
            case "astronomical_unit": return MeasureUnit.ASTRONOMICAL_UNIT;
            // length_centimeter
            case "centimeter": return MeasureUnit.CENTIMETER;
            // length_decimeter
            case "decimeter": return MeasureUnit.DECIMETER;
            // length_fathom
            case "fathom": return MeasureUnit.FATHOM;
            // length_foot
            case "foot": return MeasureUnit.FOOT;
            // length_furlong
            case "furlong": return MeasureUnit.FURLONG;
            // length_inch
            case "inch": return MeasureUnit.INCH;
            // length_kilometer
            case "kilometer": return MeasureUnit.KILOMETER;
            // length_light_year
            case "light_year": return MeasureUnit.LIGHT_YEAR;
            // length_meter
            case "meter": return MeasureUnit.METER;
            // length_micrometer
            case "micrometer": return MeasureUnit.MICROMETER;
            // length_mile
            case "mile": return MeasureUnit.MILE;
            // length_mile_scandinavian
            case "mile_scandinavian": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.MILE_SCANDINAVIAN;
                // length_millimeter
            case "millimeter": return MeasureUnit.MILLIMETER;
            // length_nanometer
            case "nanometer": return MeasureUnit.NANOMETER;
            // length_nautical_mile
            case "nautical_mile": return MeasureUnit.NAUTICAL_MILE;
            // length_parsec
            case "parsec": return MeasureUnit.PARSEC;
            // length_picometer
            case "picometer": return MeasureUnit.PICOMETER;
            // length_point
            case "point": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.POINT;
                // length_yard
            case "yard": return MeasureUnit.YARD;
            // length_earth_radius
            case "earth_radius": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.EARTH_RADIUS;
                // length_solar_radius
            case "solar_radius": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.SOLAR_RADIUS;
                // light_candela
            case "candela": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.CANDELA;
                // light_lumen
            case "lumen": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.LUMEN;
                // light_lux
            case "lux": return MeasureUnit.LUX;
            // light_solar_luminosity
            case "solar_luminosity": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.SOLAR_LUMINOSITY;
                // mass_carat
            case "carat": return MeasureUnit.CARAT;
            // mass_grain
            case "grain": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.GRAIN;
                // mass_gram
            case "gram": return MeasureUnit.GRAM;
            // mass_kilogram
            case "kilogram": return MeasureUnit.KILOGRAM;
            // mass_metric_ton
            case "metric_ton": return MeasureUnit.METRIC_TON;
            // mass_microgram
            case "microgram": return MeasureUnit.MICROGRAM;
            // mass_milligram
            case "milligram": return MeasureUnit.MILLIGRAM;
            // mass_ounce
            case "ounce": return MeasureUnit.OUNCE;
            // mass_ounce_troy
            case "ounce_troy": return MeasureUnit.OUNCE_TROY;
            // mass_pound
            case "pound": return MeasureUnit.POUND;
            // mass_stone
            case "stone": return MeasureUnit.STONE;
            // mass_ton
            case "ton": return MeasureUnit.TON;
            // mass_dalton
            case "dalton": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DALTON;
                // mass_earth_mass
            case "earth_mass": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.EARTH_MASS;
                // mass_solar_mass
            case "solar_mass": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.SOLAR_MASS;
                // power_gigawatt
            case "gigawatt": return MeasureUnit.GIGAWATT;
            // power_horsepower
            case "horsepower": return MeasureUnit.HORSEPOWER;
            // power_kilowatt
            case "kilowatt": return MeasureUnit.KILOWATT;
            // power_megawatt
            case "megawatt": return MeasureUnit.MEGAWATT;
            // power_milliwatt
            case "milliwatt": return MeasureUnit.MILLIWATT;
            // power_watt
            case "watt": return MeasureUnit.WATT;
            // pressure_atmosphere
            case "atmosphere": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.ATMOSPHERE;
                // pressure_hectopascal
            case "hectopascal": return MeasureUnit.HECTOPASCAL;
            // pressure_inch_ofhg
            case "inch_ofhg": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.INCH_OFHG;
                // pressure_bar
            case "bar": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.BAR;
                // pressure_millibar
            case "millibar": return MeasureUnit.MILLIBAR;
            // pressure_millimeter_ofhg
            case "millimeter_ofhg": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.MILLIMETER_OFHG;
                // pressure_pound_force_per_square_inch
            case "pound_force_per_square_inch": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.POUND_FORCE_PER_SQUARE_INCH;
                // pressure_pascal
            case "pascal": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PASCAL;
                // pressure_kilopascal
            case "kilopascal": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.KILOPASCAL;
                // pressure_megapascal
            case "megapascal": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.MEGAPASCAL;
                // pressure_ofhg
            case "ofhg": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.OFHG;
                // speed_kilometer_per_hour
            case "kilometer_per_hour": return MeasureUnit.KILOMETER_PER_HOUR;
            // speed_knot
            case "knot": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.KNOT;
                // speed_meter_per_second
            case "meter_per_second": return MeasureUnit.METER_PER_SECOND;
            // speed_mile_per_hour
            case "mile_per_hour": return MeasureUnit.MILE_PER_HOUR;
            // temperature_celsius
            case "celsius": return MeasureUnit.CELSIUS;
            // temperature_fahrenheit
            case "fahrenheit": return MeasureUnit.FAHRENHEIT;
            // temperature_generic
            case "generic": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.GENERIC;
                // temperature_kelvin
            case "kelvin": return MeasureUnit.KELVIN;
            // torque_pound_force_foot
            case "pound_force_foot": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.POUND_FORCE_FOOT;

                // torque_newton_meter
            case "newton_meter": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.NEWTON_METER;
                // volume_acre_foot
            case "acre_foot": return MeasureUnit.ACRE_FOOT;
            // volume_bushel
            case "bushel": return MeasureUnit.BUSHEL;
            // volume_centiliter
            case "centiliter": return MeasureUnit.CENTILITER;
            // volume_cubic_centimeter
            case "cubic_centimeter": return MeasureUnit.CUBIC_CENTIMETER;
            // volume_cubic_foot
            case "cubic_foot": return MeasureUnit.CUBIC_FOOT;
            // volume_cubic_inch
            case "cubic_inch": return MeasureUnit.CUBIC_INCH;
            // volume_cubic_kilometer
            case "cubic_kilometer": return MeasureUnit.CUBIC_KILOMETER;
            // volume_cubic_meter
            case "cubic_meter": return MeasureUnit.CUBIC_METER;
            // volume_cubic_mile
            case "cubic_mile": return MeasureUnit.CUBIC_MILE;
            // volume_cubic_yard
            case "cubic_yard": return MeasureUnit.CUBIC_YARD;
            // volume_cup
            case "cup": return MeasureUnit.CUP;
            // volume_cup_metric
            case "cup_metric": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.CUP_METRIC;
                // volume_deciliter
            case "deciliter": return MeasureUnit.DECILITER;

            // volume_dessert_spoon
            case "dessert_spoon": throw new JSRangeErrorException("Unsupported measure unit !"); // return MeasureUnit.DESSERT_SPOON;
                // volume_dessert_spoon_imperial
            case "dessert_spoon_imperial": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DESSERT_SPOON_IMPERIAL;

                // volume_drop
            case "drop": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DROP;

                // volume_dram
            case "dram": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.DRAM;

                // volume_jigger
            case "jigger": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.JIGGER;

                // volume_pinch
            case "pinch": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PINCH;

                // volume_quart_imperial
            case "quart_imperial": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.QUART_IMPERIAL;
                // volume_fluid_ounce
            case "fluid_ounce": return MeasureUnit.FLUID_OUNCE;
            // volume_fluid_ounce_imperial
            case "fluid_ounce_imperial": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.FLUID_OUNCE_IMPERIAL;
                // volume_gallon
            case "gallon": return MeasureUnit.GALLON;
            // volume_gallon_imperial
            case "gallon_imperial": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.GALLON_IMPERIAL;
                // volume_hectoliter
            case "hectoliter": return MeasureUnit.HECTOLITER;
            // volume_liter
            case "liter": return MeasureUnit.LITER;
            // volume_megaliter
            case "megaliter": return MeasureUnit.MEGALITER;
            // volume_milliliter
            case "milliliter": return MeasureUnit.MILLILITER;
            // volume_pint
            case "pint": return MeasureUnit.PINT;
            // volume_pint_metric
            case "pint_metric": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.PINT_METRIC;
                // volume_quart
            case "quart": return MeasureUnit.QUART;
            // volume_tablespoon
            case "tablespoon": return MeasureUnit.TABLESPOON;
            // volume_teaspoon
            case "teaspoon": return MeasureUnit.TEASPOON;
            // volume_barrel
            case "barrel": throw new JSRangeErrorException("Unsupported measure unit !");  // return MeasureUnit.BARREL;
            default: throw new JSRangeErrorException("Unsupported measure unit !");
        }
    }

    public static MeasureFormat createMeasureFormat(ULocale locale, DecimalFormat baseDecimalFormat, String unit, IPlatformNumberFormatter.UnitDisplay unitDisplay) {
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

        return MeasureFormat.getInstance(locale, formatWidth, baseDecimalFormat);
    }


    public static DecimalFormat configureGrouping(ULocale locale, DecimalFormat decimalFormat, boolean mGroupingUsed) {
        decimalFormat.setGroupingUsed(mGroupingUsed);
        return decimalFormat;
    }

    public static DecimalFormat configureMinMaxDigits(ULocale locale, DecimalFormat decimalFormat, int minimumIntegerDigits, int maximumIntegerDigits, int minimumFractionDigits, int maximumFractionDigits) {
        if(minimumIntegerDigits >= 0)
            decimalFormat.setMinimumIntegerDigits(minimumIntegerDigits);

        if(maximumIntegerDigits >= 0)
            decimalFormat.setMaximumIntegerDigits(maximumIntegerDigits);

        if(minimumFractionDigits >= 0)
            decimalFormat.setMinimumFractionDigits(minimumFractionDigits);

        if(maximumFractionDigits >= 0)
            decimalFormat.setMaximumFractionDigits(maximumFractionDigits);

        return decimalFormat;
    }

    public static DecimalFormat configureSignDisplay(ULocale locale, DecimalFormat decimalFormat, IPlatformNumberFormatter.SignDisplay signDisplay) {
        DecimalFormatSymbols symbols = ((DecimalFormat) DecimalFormat.getInstance(locale)).getDecimalFormatSymbols();

        switch (signDisplay) {
            case never:
                decimalFormat.setPositivePrefix("");
                decimalFormat.setPositiveSuffix("");

                decimalFormat.setNegativePrefix("");
                decimalFormat.setNegativeSuffix("");

                break;
            case always:
            case exceptZero:

                if(!decimalFormat.getNegativePrefix().isEmpty())
                    decimalFormat.setPositivePrefix(new String(new char[]{ symbols.getPlusSign()}));

                if(!decimalFormat.getNegativeSuffix().isEmpty())
                    decimalFormat.setPositiveSuffix(new String(new char[]{ symbols.getPlusSign()}));

                // Minus is automatically configured.
                break;
        }

        return decimalFormat;
    }
}
