/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * \file icu_types.h
 * \brief ICU type definitions for the Hermes ICU vtable.
 *
 * Pure C header - no C++ dependencies, no ICU headers required.
 * Provides forward declarations and value types matching the ICU C API
 * so that the vtable header and call sites can use real ICU types
 * without including ICU headers.
 *
 * When real ICU headers are available (detected via U_ICU_VERSION_MAJOR_NUM),
 * all type definitions are skipped to avoid redefinition conflicts.
 * This allows the bundled ICU DLL glue code to include real ICU headers
 * first and then include hermes_icu.h for the vtable struct definition.
 */

#ifndef HERMES_ICU_TYPES_H
#define HERMES_ICU_TYPES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * If real ICU headers are included before this file, skip all type
 * definitions — the real ICU types take precedence. The bundled ICU
 * DLL build includes real ICU headers; all other consumers use our
 * forward declarations below.
 */
#ifndef U_ICU_VERSION_MAJOR_NUM

/* ===================================================================
 * ICU opaque pointer types (forward-declared structs)
 * =================================================================== */

typedef struct UCollator UCollator;
typedef struct UDateFormat UDateFormat;
typedef struct UDateTimePatternGenerator UDateTimePatternGenerator;
typedef struct UNumberFormat UNumberFormat;
typedef struct UNumberFormatter UNumberFormatter;
typedef struct UFormattedNumber UFormattedNumber;
typedef struct UNumberRangeFormatter UNumberRangeFormatter;
typedef struct UFormattedNumberRange UFormattedNumberRange;
typedef struct UPluralRules UPluralRules;
typedef struct UListFormatter UListFormatter;
typedef struct UFormattedList UFormattedList;
typedef struct UBreakIterator UBreakIterator;
typedef struct URelativeDateTimeFormatter URelativeDateTimeFormatter;
typedef struct UFormattedRelativeDateTime UFormattedRelativeDateTime;
typedef struct ULocaleDisplayNames ULocaleDisplayNames;
typedef struct UDateIntervalFormat UDateIntervalFormat;
typedef struct UFormattedDateInterval UFormattedDateInterval;
typedef struct UCalendar UCalendar;
typedef struct UEnumeration UEnumeration;
typedef struct UFieldPositionIterator UFieldPositionIterator;
typedef struct UConstrainedFieldPosition UConstrainedFieldPosition;
typedef struct UFormattedValue UFormattedValue;
typedef struct UNumberingSystem UNumberingSystem;
typedef struct UNormalizer2 UNormalizer2;
typedef struct UResourceBundle UResourceBundle;
typedef struct USet USet;
typedef struct UText UText;

/* ===================================================================
 * ICU value types
 * =================================================================== */

typedef uint16_t UChar;
typedef int32_t UChar32;
typedef double UDate;
typedef int8_t UBool;
typedef uint8_t UVersionInfo[4];

/* ===================================================================
 * ICU error code enum
 *
 * Partial — includes only values checked by Hermes code.
 * Using a real C enum for type safety and debugger readability.
 * C guarantees enum underlying type is int, matching ICU's ABI.
 * =================================================================== */

typedef enum UErrorCode {
  U_USING_FALLBACK_WARNING = -128,
  U_USING_DEFAULT_WARNING = -127,
  U_ZERO_ERROR = 0,
  U_ILLEGAL_ARGUMENT_ERROR = 1,
  U_MISSING_RESOURCE_ERROR = 2,
  U_INVALID_FORMAT_ERROR = 3,
  U_FILE_ACCESS_ERROR = 4,
  U_INTERNAL_PROGRAM_ERROR = 5,
  U_MEMORY_ALLOCATION_ERROR = 7,
  U_BUFFER_OVERFLOW_ERROR = 15,
  U_UNSUPPORTED_ERROR = 16,
} UErrorCode;

#define U_SUCCESS(x) ((x) <= 0)
#define U_FAILURE(x) ((x) > 0)

/* ===================================================================
 * ICU struct types used in vtable function signatures
 * =================================================================== */

/** Parse error details — used by unum_open, udat_open, ubrk_openRules, etc. */
typedef struct UParseError {
  int32_t line;
  int32_t offset;
  UChar preContext[16];
  UChar postContext[16];
} UParseError;

/** Field position — used by unum_formatDouble, udat_format, etc. */
typedef struct UFieldPosition {
  int32_t field;
  int32_t beginIndex;
  int32_t endIndex;
} UFieldPosition;

/* ===================================================================
 * ICU enum types used in vtable function signatures
 *
 * Each is typedef'd to int32_t for ABI compatibility. The actual
 * enum constants are defined where they're used in implementation
 * files, not here. This avoids maintaining copies of every ICU
 * enum constant while preserving ABI compatibility.
 * =================================================================== */

typedef int32_t UColAttribute;
typedef int32_t UColAttributeValue;
typedef int32_t UCollationResult;
typedef int32_t UCollationStrength;
typedef int32_t UDateFormatStyle;
typedef int32_t UDateFormatBooleanAttribute;
typedef int32_t UDateFormatSymbolType;
typedef int32_t UDateTimePatternMatchOptions;
typedef int32_t UDateTimePatternField;
typedef int32_t UDateTimePGDisplayWidth;
typedef int32_t UDateFormatHourCycle;
typedef int32_t UNumberFormatStyle;
typedef int32_t UNumberFormatAttribute;
typedef int32_t UNumberFormatTextAttribute;
typedef int32_t UNumberRangeCollapse;
typedef int32_t UNumberRangeIdentityFallback;
typedef int32_t UCalendarType;
typedef int32_t UCalendarDateFields;
typedef int32_t UCalendarAttribute;
typedef int32_t UCalendarDisplayNameType;
typedef int32_t UCalendarDaysOfWeek;
typedef int32_t UCalendarWeekdayType;
typedef int32_t USystemTimeZoneType;
typedef int32_t UTimeZoneTransitionType;
typedef int32_t UTimeZoneLocalOption;
typedef int32_t UPluralType;
typedef int32_t UListFormatterType;
typedef int32_t UListFormatterWidth;
typedef int32_t UBreakIteratorType;
typedef int32_t UDateRelativeDateTimeFormatterStyle;
typedef int32_t URelativeDateTimeUnit;
typedef int32_t UDialectHandling;
typedef int32_t UDisplayContext;
typedef int32_t UDisplayContextType;
typedef int32_t ULocDataLocaleType;
typedef int32_t UAcceptResult;
typedef int32_t UNormalization2Mode;
typedef int32_t UNormalizationCheckResult;
typedef int32_t UCurrNameStyle;

#endif /* !U_ICU_VERSION_MAJOR_NUM */

#ifdef __cplusplus
}
#endif

#endif /* HERMES_ICU_TYPES_H */
