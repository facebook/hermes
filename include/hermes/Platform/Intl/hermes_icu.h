/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * \file hermes_icu.h
 * \brief C-compatible ICU vtable v2 for runtime ICU provider selection.
 *
 * Pure C header - no C++ dependencies.
 *
 * v2 design: every function pointer uses the exact ICU C API signature.
 * No custom wrapper types - real ICU types (UCollator*, UErrorCode*, etc.)
 * are used throughout, enabling direct GetProcAddress population for
 * system ICU providers without wrapper functions.
 */

#ifndef HERMES_ICU_H
#define HERMES_ICU_H

#include "icu_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Calling convention (explicit for x86 ABI stability) --- */
#ifndef HERMES_ICU_CDECL
#ifdef _WIN32
#define HERMES_ICU_CDECL __cdecl
#else
#define HERMES_ICU_CDECL
#endif
#endif

/* --- DLL export/import for hermes_icu C API functions ---
 * When building hermes.dll, these default to dllexport.
 * When consuming hermes.dll (linking against hermes.lib), define
 * HERMES_ICU_API as __declspec(dllimport) before including this header.
 * Follows the NAPI_EXTERN pattern from js_native_api.h. */
#ifndef HERMES_ICU_API
#ifdef _WIN32
#define HERMES_ICU_API __declspec(dllexport)
#else
#define HERMES_ICU_API __attribute__((visibility("default")))
#endif
#endif

/**
 * Version of the vtable layout. Providers must match this version.
 * Bumped to 100 to clearly distinguish from v1 (which was version 6).
 */
#define HERMES_ICU_VTABLE_VERSION 100

/**
 * C-compatible struct of ICU function pointers.
 *
 * Design principles (v2):
 *   - Every function pointer uses the exact ICU C API signature
 *   - Error handling via UErrorCode* parameter (standard ICU convention)
 *   - Real ICU opaque types (UCollator*, UDateFormat*, etc.)
 *   - HERMES_ICU_CDECL calling convention on all function pointers
 *   - Never reorder or remove existing pointers — only append
 *   - Bump HERMES_ICU_VTABLE_VERSION when the struct grows
 *   - No custom (non-ICU) function entries
 *   - Entries for future Intl objects are included from the start;
 *     providers populate them when available, callers check for NULL
 */
typedef struct hermes_icu_vtable {
  /* ---------------------------------------------------------------
   * Metadata
   * --------------------------------------------------------------- */

  /** Vtable ABI version — must equal HERMES_ICU_VTABLE_VERSION. */
  uint32_t version;
  /** ICU version (e.g. 78 for ICU 78.2). 0 = unknown (system-managed). */
  uint32_t icu_version;
  /** Provider name (UTF-8, null-terminated).
   *  Suggested: "bundled", "windows", "custom". NULL = unknown. */
  const char *provider_name;
  /** Number of populated function pointers (for forward compat). */
  uint32_t function_count;

  /* ===============================================================
   * 2.3.1 Locale (uloc_*) — 27 functions
   * =============================================================== */

  int32_t(HERMES_ICU_CDECL *uloc_forLanguageTag)(
      const char *langtag,
      char *localeID,
      int32_t localeIDCapacity,
      int32_t *parsedLength,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_toLanguageTag)(
      const char *localeID,
      char *langtag,
      int32_t langtagCapacity,
      UBool strict,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_canonicalize)(
      const char *localeID,
      char *name,
      int32_t nameCapacity,
      UErrorCode *err);

  /* Component extraction */
  int32_t(HERMES_ICU_CDECL *uloc_getLanguage)(
      const char *localeID,
      char *language,
      int32_t languageCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getScript)(
      const char *localeID,
      char *script,
      int32_t scriptCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getCountry)(
      const char *localeID,
      char *country,
      int32_t countryCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getVariant)(
      const char *localeID,
      char *variant,
      int32_t variantCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getBaseName)(
      const char *localeID,
      char *name,
      int32_t nameCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getName)(
      const char *localeID,
      char *name,
      int32_t nameCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getParent)(
      const char *localeID,
      char *parent,
      int32_t parentCapacity,
      UErrorCode *err);

  /* Keyword operations */
  int32_t(HERMES_ICU_CDECL *uloc_getKeywordValue)(
      const char *localeID,
      const char *keywordName,
      char *buffer,
      int32_t bufferCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_setKeywordValue)(
      const char *keywordName,
      const char *keywordValue,
      char *buffer,
      int32_t bufferCapacity,
      UErrorCode *err);
  UEnumeration *(HERMES_ICU_CDECL *uloc_openKeywords)(
      const char *localeID,
      UErrorCode *err);

  /* Likely subtags */
  int32_t(HERMES_ICU_CDECL *uloc_addLikelySubtags)(
      const char *localeID,
      char *maximizedLocaleID,
      int32_t maximizedLocaleIDCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_minimizeSubtags)(
      const char *localeID,
      char *minimizedLocaleID,
      int32_t minimizedLocaleIDCapacity,
      UErrorCode *err);

  /* Available locales */
  int32_t(HERMES_ICU_CDECL *uloc_countAvailable)(void);
  const char *(HERMES_ICU_CDECL *uloc_getAvailable)(int32_t n);
  const char *(HERMES_ICU_CDECL *uloc_getDefault)(void);
  UEnumeration *(HERMES_ICU_CDECL *uloc_openAvailableByType)(
      int32_t type,
      UErrorCode *err);

  /* Display names */
  int32_t(HERMES_ICU_CDECL *uloc_getDisplayLanguage)(
      const char *locale,
      const char *displayLocale,
      UChar *language,
      int32_t languageCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getDisplayScript)(
      const char *locale,
      const char *displayLocale,
      UChar *script,
      int32_t scriptCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getDisplayCountry)(
      const char *locale,
      const char *displayLocale,
      UChar *country,
      int32_t countryCapacity,
      UErrorCode *err);
  int32_t(HERMES_ICU_CDECL *uloc_getDisplayName)(
      const char *localeID,
      const char *inLocaleID,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *err);

  /* Locale matching */
  int32_t(HERMES_ICU_CDECL *uloc_acceptLanguage)(
      char *result,
      int32_t resultAvailable,
      UAcceptResult *outResult,
      const char **acceptList,
      int32_t acceptListCount,
      UEnumeration *availableLocales,
      UErrorCode *err);

  /* Unicode locale key/type conversion */
  const char *(HERMES_ICU_CDECL *uloc_toUnicodeLocaleKey)(const char *keyword);
  const char *(HERMES_ICU_CDECL *uloc_toUnicodeLocaleType)(
      const char *keyword,
      const char *value);
  const char *(HERMES_ICU_CDECL *uloc_toLegacyKey)(const char *keyword);
  const char *(HERMES_ICU_CDECL *uloc_toLegacyType)(
      const char *keyword,
      const char *value);

  /* Script direction */
  UBool(HERMES_ICU_CDECL *uloc_isRightToLeft)(const char *locale);

  /* ===============================================================
   * 2.3.2 Collator (ucol_*) — 15 functions
   * =============================================================== */

  UCollator *(HERMES_ICU_CDECL *ucol_open)(
      const char *loc,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucol_close)(UCollator *coll);
  UCollationResult(HERMES_ICU_CDECL *ucol_strcoll)(
      const UCollator *coll,
      const UChar *source,
      int32_t sourceLength,
      const UChar *target,
      int32_t targetLength);
  UCollationResult(HERMES_ICU_CDECL *ucol_strcollUTF8)(
      const UCollator *coll,
      const char *source,
      int32_t sourceLength,
      const char *target,
      int32_t targetLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucol_setAttribute)(
      UCollator *coll,
      UColAttribute attr,
      UColAttributeValue value,
      UErrorCode *status);
  UColAttributeValue(HERMES_ICU_CDECL *ucol_getAttribute)(
      const UCollator *coll,
      UColAttribute attr,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucol_setStrength)(
      UCollator *coll,
      UCollationStrength strength);
  const char *(HERMES_ICU_CDECL *ucol_getLocaleByType)(
      const UCollator *coll,
      ULocDataLocaleType type,
      UErrorCode *status);
  UEnumeration *(HERMES_ICU_CDECL *ucol_openAvailableLocales)(
      UErrorCode *status);
  UEnumeration *(HERMES_ICU_CDECL *ucol_getKeywordValuesForLocale)(
      const char *key,
      const char *locale,
      UBool commonlyUsed,
      UErrorCode *status);
  UEnumeration *(HERMES_ICU_CDECL *ucol_getKeywordValues)(
      const char *keyword,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucol_getReorderCodes)(
      const UCollator *coll,
      int32_t *dest,
      int32_t destCapacity,
      UErrorCode *status);
  const UChar *(HERMES_ICU_CDECL *ucol_getRules)(
      const UCollator *coll,
      int32_t *length);
  USet *(HERMES_ICU_CDECL *ucol_getTailoredSet)(
      const UCollator *coll,
      UErrorCode *status);
  UCollator *(HERMES_ICU_CDECL *ucol_clone)(
      const UCollator *coll,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.3 NumberFormatter v2 (unumf_*) — 13 functions
   * =============================================================== */

  UNumberFormatter *(HERMES_ICU_CDECL *unumf_openForSkeletonAndLocale)(
      const UChar *skeleton,
      int32_t skeletonLen,
      const char *locale,
      UErrorCode *ec);
  UNumberFormatter *(
      HERMES_ICU_CDECL *unumf_openForSkeletonAndLocaleWithError)(
      const UChar *skeleton,
      int32_t skeletonLen,
      const char *locale,
      UParseError *perror,
      UErrorCode *ec);
  UFormattedNumber *(HERMES_ICU_CDECL *unumf_openResult)(UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumf_formatDouble)(
      const UNumberFormatter *uformatter,
      double value,
      UFormattedNumber *uresult,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumf_formatInt)(
      const UNumberFormatter *uformatter,
      int64_t value,
      UFormattedNumber *uresult,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumf_formatDecimal)(
      const UNumberFormatter *uformatter,
      const char *value,
      int32_t valueLen,
      UFormattedNumber *uresult,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *unumf_resultToString)(
      const UFormattedNumber *uresult,
      UChar *buffer,
      int32_t bufferCapacity,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *unumf_resultToDecimalNumber)(
      const UFormattedNumber *uresult,
      char *dest,
      int32_t destCapacity,
      UErrorCode *ec);
  const UFormattedValue *(HERMES_ICU_CDECL *unumf_resultAsValue)(
      const UFormattedNumber *uresult,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumf_resultGetAllFieldPositions)(
      const UFormattedNumber *uresult,
      UFieldPositionIterator *fpositer,
      UErrorCode *ec);
  UBool(HERMES_ICU_CDECL *unumf_resultNextFieldPosition)(
      const UFormattedNumber *uresult,
      UFieldPosition *ufpos,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumf_close)(UNumberFormatter *uformatter);
  void(HERMES_ICU_CDECL *unumf_closeResult)(UFormattedNumber *uresult);

  /* ===============================================================
   * 2.3.4 NumberRangeFormatter (unumrf_*) — 7 functions
   * =============================================================== */

  UNumberRangeFormatter *(
      HERMES_ICU_CDECL
          *unumrf_openForSkeletonWithCollapseAndIdentityFallback)(
      const UChar *skeleton,
      int32_t skeletonLen,
      UNumberRangeCollapse collapse,
      UNumberRangeIdentityFallback identityFallback,
      const char *locale,
      UParseError *perror,
      UErrorCode *ec);
  UFormattedNumberRange *(HERMES_ICU_CDECL *unumrf_openResult)(
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumrf_formatDoubleRange)(
      const UNumberRangeFormatter *uformatter,
      double first,
      double second,
      UFormattedNumberRange *uresult,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumrf_formatDecimalRange)(
      const UNumberRangeFormatter *uformatter,
      const char *first,
      int32_t firstLen,
      const char *second,
      int32_t secondLen,
      UFormattedNumberRange *uresult,
      UErrorCode *ec);
  const UFormattedValue *(HERMES_ICU_CDECL *unumrf_resultAsValue)(
      const UFormattedNumberRange *uresult,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *unumrf_close)(UNumberRangeFormatter *uformatter);
  void(HERMES_ICU_CDECL *unumrf_closeResult)(
      UFormattedNumberRange *uresult);

  /* ===============================================================
   * 2.3.5 Legacy NumberFormat (unum_*) — 6 functions
   * =============================================================== */

  UNumberFormat *(HERMES_ICU_CDECL *unum_open)(
      UNumberFormatStyle style,
      const UChar *pattern,
      int32_t patternLength,
      const char *locale,
      UParseError *parseErr,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *unum_close)(UNumberFormat *fmt);
  int32_t(HERMES_ICU_CDECL *unum_formatDouble)(
      const UNumberFormat *fmt,
      double number,
      UChar *result,
      int32_t resultLength,
      UFieldPosition *pos,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *unum_format)(
      const UNumberFormat *fmt,
      int32_t number,
      UChar *result,
      int32_t resultLength,
      UFieldPosition *pos,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *unum_setAttribute)(
      UNumberFormat *fmt,
      UNumberFormatAttribute attr,
      int32_t newValue);
  int32_t(HERMES_ICU_CDECL *unum_getAttribute)(
      const UNumberFormat *fmt,
      UNumberFormatAttribute attr);
  void(HERMES_ICU_CDECL *unum_setTextAttribute)(
      UNumberFormat *fmt,
      UNumberFormatTextAttribute tag,
      const UChar *newValue,
      int32_t newValueLength,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *unum_formatDoubleForFields)(
      const UNumberFormat *fmt,
      double number,
      UChar *result,
      int32_t resultLength,
      UFieldPositionIterator *fpositer,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.6 DateFormat (udat_*) — 18 functions
   * =============================================================== */

  UDateFormat *(HERMES_ICU_CDECL *udat_open)(
      UDateFormatStyle timeStyle,
      UDateFormatStyle dateStyle,
      const char *locale,
      const UChar *tzID,
      int32_t tzIDLength,
      const UChar *pattern,
      int32_t patternLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *udat_close)(UDateFormat *format);
  int32_t(HERMES_ICU_CDECL *udat_format)(
      const UDateFormat *format,
      UDate dateToFormat,
      UChar *result,
      int32_t resultLength,
      UFieldPosition *position,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *udat_formatForFields)(
      const UDateFormat *format,
      UDate dateToFormat,
      UChar *result,
      int32_t resultLength,
      UFieldPositionIterator *fpositer,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *udat_formatCalendarForFields)(
      const UDateFormat *format,
      UCalendar *calendar,
      UChar *result,
      int32_t capacity,
      UFieldPositionIterator *fpositer,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *udat_toPattern)(
      const UDateFormat *fmt,
      UBool localized,
      UChar *result,
      int32_t resultLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *udat_applyPattern)(
      UDateFormat *format,
      UBool localized,
      const UChar *pattern,
      int32_t patternLength);
  const UCalendar *(HERMES_ICU_CDECL *udat_getCalendar)(
      const UDateFormat *fmt);
  void(HERMES_ICU_CDECL *udat_setCalendar)(
      UDateFormat *fmt,
      const UCalendar *calendarToSet);
  const UNumberFormat *(HERMES_ICU_CDECL *udat_getNumberFormat)(
      const UDateFormat *fmt);
  void(HERMES_ICU_CDECL *udat_adoptNumberFormat)(
      UDateFormat *fmt,
      UNumberFormat *numberFormatToAdopt);
  int32_t(HERMES_ICU_CDECL *udat_getSymbols)(
      const UDateFormat *fmt,
      UDateFormatSymbolType type,
      int32_t symbolIndex,
      UChar *result,
      int32_t resultLength,
      UErrorCode *status);
  const char *(HERMES_ICU_CDECL *udat_getLocaleByType)(
      const UDateFormat *fmt,
      ULocDataLocaleType type,
      UErrorCode *status);
  UBool(HERMES_ICU_CDECL *udat_getBooleanAttribute)(
      const UDateFormat *fmt,
      UDateFormatBooleanAttribute attr,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *udat_setBooleanAttribute)(
      UDateFormat *fmt,
      UDateFormatBooleanAttribute attr,
      UBool newValue,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *udat_setContext)(
      UDateFormat *fmt,
      UDisplayContext value,
      UErrorCode *status);
  UDisplayContext(HERMES_ICU_CDECL *udat_getContext)(
      const UDateFormat *fmt,
      UDisplayContextType type,
      UErrorCode *status);
  UDateFormat *(HERMES_ICU_CDECL *udat_clone)(
      const UDateFormat *fmt,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.7 DateTimePatternGenerator (udatpg_*) — 10 functions
   * =============================================================== */

  UDateTimePatternGenerator *(HERMES_ICU_CDECL *udatpg_open)(
      const char *locale,
      UErrorCode *pErrorCode);
  void(HERMES_ICU_CDECL *udatpg_close)(UDateTimePatternGenerator *dtpg);
  int32_t(HERMES_ICU_CDECL *udatpg_getBestPattern)(
      UDateTimePatternGenerator *dtpg,
      const UChar *skeleton,
      int32_t length,
      UChar *bestPattern,
      int32_t capacity,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *udatpg_getBestPatternWithOptions)(
      UDateTimePatternGenerator *dtpg,
      const UChar *skeleton,
      int32_t length,
      UDateTimePatternMatchOptions options,
      UChar *bestPattern,
      int32_t capacity,
      UErrorCode *pErrorCode);
  UDateFormatHourCycle(HERMES_ICU_CDECL *udatpg_getDefaultHourCycle)(
      const UDateTimePatternGenerator *dtpg,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *udatpg_getSkeleton)(
      UDateTimePatternGenerator *unusedDtpg,
      const UChar *pattern,
      int32_t length,
      UChar *skeleton,
      int32_t capacity,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *udatpg_getBaseSkeleton)(
      UDateTimePatternGenerator *unusedDtpg,
      const UChar *pattern,
      int32_t length,
      UChar *baseSkeleton,
      int32_t capacity,
      UErrorCode *pErrorCode);
  const UChar *(HERMES_ICU_CDECL *udatpg_getPatternForSkeleton)(
      const UDateTimePatternGenerator *dtpg,
      const UChar *skeleton,
      int32_t skeletonLength,
      int32_t *pLength);
  int32_t(HERMES_ICU_CDECL *udatpg_getFieldDisplayName)(
      const UDateTimePatternGenerator *dtpg,
      UDateTimePatternField field,
      UDateTimePGDisplayWidth width,
      UChar *fieldName,
      int32_t capacity,
      UErrorCode *pErrorCode);
  UDateTimePatternGenerator *(HERMES_ICU_CDECL *udatpg_clone)(
      const UDateTimePatternGenerator *dtpg,
      UErrorCode *pErrorCode);

  /* ===============================================================
   * 2.3.8 DateIntervalFormat (udtitvfmt_*) — 7 functions
   * =============================================================== */

  UDateIntervalFormat *(HERMES_ICU_CDECL *udtitvfmt_open)(
      const char *locale,
      const UChar *skeleton,
      int32_t skeletonLength,
      const UChar *tzID,
      int32_t tzIDLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *udtitvfmt_close)(UDateIntervalFormat *formatter);
  UFormattedDateInterval *(HERMES_ICU_CDECL *udtitvfmt_openResult)(
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *udtitvfmt_closeResult)(
      UFormattedDateInterval *uresult);
  void(HERMES_ICU_CDECL *udtitvfmt_formatToResult)(
      const UDateIntervalFormat *formatter,
      UDate fromDate,
      UDate toDate,
      UFormattedDateInterval *result,
      UErrorCode *status);
  const UFormattedValue *(HERMES_ICU_CDECL *udtitvfmt_resultAsValue)(
      const UFormattedDateInterval *uresult,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *udtitvfmt_format)(
      const UDateIntervalFormat *formatter,
      UDate fromDate,
      UDate toDate,
      UChar *result,
      int32_t resultCapacity,
      UFieldPosition *position,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.9 Calendar/Timezone (ucal_*) — 35 functions
   * =============================================================== */

  UCalendar *(HERMES_ICU_CDECL *ucal_open)(
      const UChar *zoneID,
      int32_t len,
      const char *locale,
      UCalendarType type,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_close)(UCalendar *cal);
  UCalendar *(HERMES_ICU_CDECL *ucal_clone)(
      const UCalendar *cal,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_get)(
      const UCalendar *cal,
      UCalendarDateFields field,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_set)(
      UCalendar *cal,
      UCalendarDateFields field,
      int32_t value);
  UDate(HERMES_ICU_CDECL *ucal_getMillis)(
      const UCalendar *cal,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_setMillis)(
      UCalendar *cal,
      UDate dateTime,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_setAttribute)(
      UCalendar *cal,
      UCalendarAttribute attr,
      int32_t newValue);
  int32_t(HERMES_ICU_CDECL *ucal_getAttribute)(
      const UCalendar *cal,
      UCalendarAttribute attr);

  /* Timezone operations */
  int32_t(HERMES_ICU_CDECL *ucal_getDefaultTimeZone)(
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ucal_getCanonicalTimeZoneID)(
      const UChar *id,
      int32_t len,
      UChar *result,
      int32_t resultCapacity,
      UBool *isSystemID,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ucal_getTimeZoneDisplayName)(
      const UCalendar *cal,
      UCalendarDisplayNameType type,
      const char *locale,
      UChar *result,
      int32_t resultLength,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_getTimeZoneID)(
      const UCalendar *cal,
      UChar *result,
      int32_t resultLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_setTimeZone)(
      UCalendar *cal,
      const UChar *zoneID,
      int32_t len,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_getTimeZoneIDForWindowsID)(
      const UChar *winid,
      int32_t len,
      const char *region,
      UChar *id,
      int32_t idCapacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_getWindowsTimeZoneID)(
      const UChar *id,
      int32_t len,
      UChar *winid,
      int32_t winidCapacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_getHostTimeZone)(
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *ec);
  UBool(HERMES_ICU_CDECL *ucal_getTimeZoneTransitionDate)(
      const UCalendar *cal,
      UTimeZoneTransitionType type,
      UDate *transition,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_getTimeZoneOffsetFromLocal)(
      const UCalendar *cal,
      UTimeZoneLocalOption nonExistingTimeOpt,
      UTimeZoneLocalOption duplicatedTimeOpt,
      int32_t *rawOffset,
      int32_t *dstOffset,
      UErrorCode *status);

  /* Enumerations */
  UEnumeration *(HERMES_ICU_CDECL *ucal_openTimeZones)(UErrorCode *ec);
  UEnumeration *(HERMES_ICU_CDECL *ucal_openTimeZoneIDEnumeration)(
      USystemTimeZoneType zoneType,
      const char *region,
      const int32_t *rawOffset,
      UErrorCode *ec);
  UEnumeration *(HERMES_ICU_CDECL *ucal_openCountryTimeZones)(
      const char *country,
      UErrorCode *ec);
  UEnumeration *(HERMES_ICU_CDECL *ucal_getKeywordValuesForLocale)(
      const char *key,
      const char *locale,
      UBool commonlyUsed,
      UErrorCode *status);

  /* Calendar properties */
  int32_t(HERMES_ICU_CDECL *ucal_countAvailable)(void);
  const char *(HERMES_ICU_CDECL *ucal_getAvailable)(int32_t localeIndex);
  const char *(HERMES_ICU_CDECL *ucal_getType)(
      const UCalendar *cal,
      UErrorCode *status);
  UCalendarWeekdayType(HERMES_ICU_CDECL *ucal_getDayOfWeekType)(
      const UCalendar *cal,
      UCalendarDaysOfWeek dayOfWeek,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ucal_getWeekendTransition)(
      const UCalendar *cal,
      UCalendarDaysOfWeek dayOfWeek,
      UErrorCode *status);
  UBool(HERMES_ICU_CDECL *ucal_isWeekend)(
      const UCalendar *cal,
      UDate date,
      UErrorCode *status);
  UBool(HERMES_ICU_CDECL *ucal_inDaylightTime)(
      const UCalendar *cal,
      UErrorCode *status);
  UDate(HERMES_ICU_CDECL *ucal_getNow)(void);
  int32_t(HERMES_ICU_CDECL *ucal_getFieldDifference)(
      UCalendar *cal,
      UDate target,
      UCalendarDateFields field,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ucal_setGregorianChange)(
      UCalendar *cal,
      UDate date,
      UErrorCode *pErrorCode);
  UDate(HERMES_ICU_CDECL *ucal_getGregorianChange)(
      const UCalendar *cal,
      UErrorCode *pErrorCode);
  void(HERMES_ICU_CDECL *ucal_setDefaultTimeZone)(
      const UChar *zoneID,
      UErrorCode *ec);

  /* ===============================================================
   * 2.3.10 PluralRules (uplrules_*) — 7 functions
   * =============================================================== */

  UPluralRules *(HERMES_ICU_CDECL *uplrules_open)(
      const char *locale,
      UErrorCode *status);
  UPluralRules *(HERMES_ICU_CDECL *uplrules_openForType)(
      const char *locale,
      UPluralType type,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *uplrules_close)(UPluralRules *uplrules);
  int32_t(HERMES_ICU_CDECL *uplrules_select)(
      const UPluralRules *uplrules,
      double number,
      UChar *keyword,
      int32_t capacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *uplrules_selectFormatted)(
      const UPluralRules *uplrules,
      const UFormattedNumber *number,
      UChar *keyword,
      int32_t capacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *uplrules_selectForRange)(
      const UPluralRules *uplrules,
      const UFormattedNumberRange *urange,
      UChar *keyword,
      int32_t capacity,
      UErrorCode *status);
  UEnumeration *(HERMES_ICU_CDECL *uplrules_getKeywords)(
      const UPluralRules *uplrules,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.11 ListFormatter (ulistfmt_*) — 8 functions
   * =============================================================== */

  UListFormatter *(HERMES_ICU_CDECL *ulistfmt_open)(
      const char *locale,
      UErrorCode *status);
  UListFormatter *(HERMES_ICU_CDECL *ulistfmt_openForType)(
      const char *locale,
      UListFormatterType type,
      UListFormatterWidth width,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ulistfmt_close)(UListFormatter *listfmt);
  UFormattedList *(HERMES_ICU_CDECL *ulistfmt_openResult)(UErrorCode *ec);
  void(HERMES_ICU_CDECL *ulistfmt_closeResult)(UFormattedList *uresult);
  int32_t(HERMES_ICU_CDECL *ulistfmt_format)(
      const UListFormatter *listfmt,
      const UChar *const strings[],
      const int32_t *stringLengths,
      int32_t stringCount,
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ulistfmt_formatStringsToResult)(
      const UListFormatter *listfmt,
      const UChar *const strings[],
      const int32_t *stringLengths,
      int32_t stringCount,
      UFormattedList *uresult,
      UErrorCode *status);
  const UFormattedValue *(HERMES_ICU_CDECL *ulistfmt_resultAsValue)(
      const UFormattedList *uresult,
      UErrorCode *ec);

  /* ===============================================================
   * 2.3.12 BreakIterator (ubrk_*) — 22 functions
   * =============================================================== */

  UBreakIterator *(HERMES_ICU_CDECL *ubrk_open)(
      UBreakIteratorType type,
      const char *locale,
      const UChar *text,
      int32_t textLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ubrk_close)(UBreakIterator *bi);
  UBreakIterator *(HERMES_ICU_CDECL *ubrk_clone)(
      const UBreakIterator *bi,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ubrk_setText)(
      UBreakIterator *bi,
      const UChar *text,
      int32_t textLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ubrk_setUText)(
      UBreakIterator *bi,
      UText *text,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ubrk_current)(const UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_next)(UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_previous)(UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_first)(UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_last)(UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_preceding)(
      UBreakIterator *bi,
      int32_t offset);
  int32_t(HERMES_ICU_CDECL *ubrk_following)(
      UBreakIterator *bi,
      int32_t offset);
  UBool(HERMES_ICU_CDECL *ubrk_isBoundary)(
      UBreakIterator *bi,
      int32_t offset);
  int32_t(HERMES_ICU_CDECL *ubrk_getRuleStatus)(UBreakIterator *bi);
  int32_t(HERMES_ICU_CDECL *ubrk_getRuleStatusVec)(
      UBreakIterator *bi,
      int32_t *fillInVec,
      int32_t capacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ubrk_countAvailable)(void);
  const char *(HERMES_ICU_CDECL *ubrk_getAvailable)(int32_t index);
  const char *(HERMES_ICU_CDECL *ubrk_getLocaleByType)(
      const UBreakIterator *bi,
      ULocDataLocaleType type,
      UErrorCode *status);
  UBreakIterator *(HERMES_ICU_CDECL *ubrk_openRules)(
      const UChar *rules,
      int32_t rulesLength,
      const UChar *text,
      int32_t textLength,
      UParseError *parseErr,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ubrk_getBinaryRules)(
      UBreakIterator *bi,
      uint8_t *binaryRules,
      int32_t rulesCapacity,
      UErrorCode *status);
  UBreakIterator *(HERMES_ICU_CDECL *ubrk_openBinaryRules)(
      const uint8_t *binaryRules,
      int32_t rulesLength,
      const UChar *text,
      int32_t textLength,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ubrk_refreshUText)(
      UBreakIterator *bi,
      UText *text,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.13 RelativeDateTimeFormatter (ureldatefmt_*) — 10 functions
   * =============================================================== */

  URelativeDateTimeFormatter *(HERMES_ICU_CDECL *ureldatefmt_open)(
      const char *locale,
      UNumberFormat *nfToAdopt,
      UDateRelativeDateTimeFormatterStyle width,
      UDisplayContext capitalizationContext,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ureldatefmt_close)(
      URelativeDateTimeFormatter *reldatefmt);
  UFormattedRelativeDateTime *(HERMES_ICU_CDECL *ureldatefmt_openResult)(
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ureldatefmt_closeResult)(
      UFormattedRelativeDateTime *ufrdt);
  int32_t(HERMES_ICU_CDECL *ureldatefmt_format)(
      const URelativeDateTimeFormatter *reldatefmt,
      double offset,
      URelativeDateTimeUnit unit,
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *ureldatefmt_formatNumeric)(
      const URelativeDateTimeFormatter *reldatefmt,
      double offset,
      URelativeDateTimeUnit unit,
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ureldatefmt_formatToResult)(
      const URelativeDateTimeFormatter *reldatefmt,
      double offset,
      URelativeDateTimeUnit unit,
      UFormattedRelativeDateTime *result,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ureldatefmt_formatNumericToResult)(
      const URelativeDateTimeFormatter *reldatefmt,
      double offset,
      URelativeDateTimeUnit unit,
      UFormattedRelativeDateTime *result,
      UErrorCode *status);
  const UFormattedValue *(HERMES_ICU_CDECL *ureldatefmt_resultAsValue)(
      const UFormattedRelativeDateTime *ufrdt,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ureldatefmt_combineDateAndTime)(
      const URelativeDateTimeFormatter *reldatefmt,
      const UChar *relativeDateString,
      int32_t relativeDateStringLen,
      const UChar *timeString,
      int32_t timeStringLen,
      UChar *result,
      int32_t resultCapacity,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.14 DisplayNames (uldn_*) — 10 functions
   * =============================================================== */

  ULocaleDisplayNames *(HERMES_ICU_CDECL *uldn_open)(
      const char *locale,
      UDialectHandling dialectHandling,
      UErrorCode *pErrorCode);
  ULocaleDisplayNames *(HERMES_ICU_CDECL *uldn_openForContext)(
      const char *locale,
      UDisplayContext *contexts,
      int32_t length,
      UErrorCode *pErrorCode);
  void(HERMES_ICU_CDECL *uldn_close)(ULocaleDisplayNames *ldn);
  const char *(HERMES_ICU_CDECL *uldn_getLocale)(
      const ULocaleDisplayNames *ldn);
  UDisplayContext(HERMES_ICU_CDECL *uldn_getContext)(
      const ULocaleDisplayNames *ldn,
      UDisplayContextType type,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *uldn_localeDisplayName)(
      const ULocaleDisplayNames *ldn,
      const char *locale,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *uldn_languageDisplayName)(
      const ULocaleDisplayNames *ldn,
      const char *lang,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *uldn_regionDisplayName)(
      const ULocaleDisplayNames *ldn,
      const char *region,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *uldn_scriptDisplayName)(
      const ULocaleDisplayNames *ldn,
      const char *script,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *uldn_keyValueDisplayName)(
      const ULocaleDisplayNames *ldn,
      const char *key,
      const char *value,
      UChar *result,
      int32_t maxResultSize,
      UErrorCode *pErrorCode);

  /* ===============================================================
   * 2.3.15 NumberingSystem (unumsys_*) — 8 functions
   * =============================================================== */

  UNumberingSystem *(HERMES_ICU_CDECL *unumsys_open)(
      const char *locale,
      UErrorCode *status);
  UNumberingSystem *(HERMES_ICU_CDECL *unumsys_openByName)(
      const char *name,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *unumsys_close)(UNumberingSystem *unumsys);
  const char *(HERMES_ICU_CDECL *unumsys_getName)(
      const UNumberingSystem *unumsys);
  int32_t(HERMES_ICU_CDECL *unumsys_getDescription)(
      const UNumberingSystem *unumsys,
      UChar *result,
      int32_t resultLength,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *unumsys_getRadix)(
      const UNumberingSystem *unumsys);
  UBool(HERMES_ICU_CDECL *unumsys_isAlgorithmic)(
      const UNumberingSystem *unumsys);
  UEnumeration *(HERMES_ICU_CDECL *unumsys_openAvailableNames)(
      UErrorCode *status);

  /* ===============================================================
   * 2.3.16 Currency (ucurr_*) — 4 functions
   * =============================================================== */

  int32_t(HERMES_ICU_CDECL *ucurr_forLocale)(
      const char *locale,
      UChar *buff,
      int32_t buffCapacity,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ucurr_getDefaultFractionDigits)(
      const UChar *currency,
      UErrorCode *ec);
  const UChar *(HERMES_ICU_CDECL *ucurr_getName)(
      const UChar *currency,
      const char *locale,
      UCurrNameStyle nameStyle,
      UBool *isChoiceFormat,
      int32_t *len,
      UErrorCode *ec);
  UEnumeration *(HERMES_ICU_CDECL *ucurr_openISOCurrencies)(
      uint32_t currType,
      UErrorCode *ec);

  /* ===============================================================
   * 2.3.17 FormattedValue / ConstrainedFieldPosition — 14 functions
   * =============================================================== */

  UConstrainedFieldPosition *(HERMES_ICU_CDECL *ucfpos_open)(UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_close)(UConstrainedFieldPosition *ucfpos);
  void(HERMES_ICU_CDECL *ucfpos_reset)(
      UConstrainedFieldPosition *ucfpos,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_constrainCategory)(
      UConstrainedFieldPosition *ucfpos,
      int32_t category,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_constrainField)(
      UConstrainedFieldPosition *ucfpos,
      int32_t category,
      int32_t field,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ucfpos_getCategory)(
      const UConstrainedFieldPosition *ucfpos,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *ucfpos_getField)(
      const UConstrainedFieldPosition *ucfpos,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_getIndexes)(
      const UConstrainedFieldPosition *ucfpos,
      int32_t *pStart,
      int32_t *pLimit,
      UErrorCode *ec);
  int64_t(HERMES_ICU_CDECL *ucfpos_getInt64IterationContext)(
      const UConstrainedFieldPosition *ucfpos,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_setInt64IterationContext)(
      UConstrainedFieldPosition *ucfpos,
      int64_t context,
      UErrorCode *ec);
  UBool(HERMES_ICU_CDECL *ucfpos_matchesField)(
      const UConstrainedFieldPosition *ucfpos,
      int32_t category,
      int32_t field,
      UErrorCode *ec);
  void(HERMES_ICU_CDECL *ucfpos_setState)(
      UConstrainedFieldPosition *ucfpos,
      int32_t category,
      int32_t field,
      int32_t start,
      int32_t limit,
      UErrorCode *ec);

  const UChar *(HERMES_ICU_CDECL *ufmtval_getString)(
      const UFormattedValue *ufmtval,
      int32_t *pLength,
      UErrorCode *ec);
  UBool(HERMES_ICU_CDECL *ufmtval_nextPosition)(
      const UFormattedValue *ufmtval,
      UConstrainedFieldPosition *ucfpos,
      UErrorCode *ec);

  /* ===============================================================
   * 2.3.18 Legacy FieldPositionIterator — 3 functions
   * =============================================================== */

  UFieldPositionIterator *(HERMES_ICU_CDECL *ufieldpositer_open)(
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ufieldpositer_close)(
      UFieldPositionIterator *fpositer);
  int32_t(HERMES_ICU_CDECL *ufieldpositer_next)(
      UFieldPositionIterator *fpositer,
      int32_t *beginIndex,
      int32_t *endIndex);

  /* ===============================================================
   * 2.3.19 Enumeration (uenum_*) — 4 functions
   * =============================================================== */

  void(HERMES_ICU_CDECL *uenum_close)(UEnumeration *en);
  const UChar *(HERMES_ICU_CDECL *uenum_unext)(
      UEnumeration *en,
      int32_t *resultLength,
      UErrorCode *status);
  const char *(HERMES_ICU_CDECL *uenum_next)(
      UEnumeration *en,
      int32_t *resultLength,
      UErrorCode *status);
  int32_t(HERMES_ICU_CDECL *uenum_count)(
      UEnumeration *en,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.20 String Utilities — 9 functions
   * =============================================================== */

  int32_t(HERMES_ICU_CDECL *u_strToLower)(
      UChar *dest,
      int32_t destCapacity,
      const UChar *src,
      int32_t srcLength,
      const char *locale,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *u_strToUpper)(
      UChar *dest,
      int32_t destCapacity,
      const UChar *src,
      int32_t srcLength,
      const char *locale,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *u_strToTitle)(
      UChar *dest,
      int32_t destCapacity,
      const UChar *src,
      int32_t srcLength,
      UBreakIterator *titleIter,
      const char *locale,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *u_strFoldCase)(
      UChar *dest,
      int32_t destCapacity,
      const UChar *src,
      int32_t srcLength,
      uint32_t options,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *u_strcmp)(const UChar *s1, const UChar *s2);
  void(HERMES_ICU_CDECL *u_getVersion)(UVersionInfo versionArray);
  const char *(HERMES_ICU_CDECL *u_errorName)(UErrorCode code);
  void(HERMES_ICU_CDECL *u_init)(UErrorCode *status);
  void(HERMES_ICU_CDECL *u_cleanup)(void);

  /* ===============================================================
   * 2.3.21 Normalization (unorm2_*) — 10 functions
   * =============================================================== */

  const UNormalizer2 *(HERMES_ICU_CDECL *unorm2_getInstance)(
      const char *packageName,
      const char *name,
      UNormalization2Mode mode,
      UErrorCode *pErrorCode);
  const UNormalizer2 *(HERMES_ICU_CDECL *unorm2_getNFCInstance)(
      UErrorCode *pErrorCode);
  const UNormalizer2 *(HERMES_ICU_CDECL *unorm2_getNFDInstance)(
      UErrorCode *pErrorCode);
  const UNormalizer2 *(HERMES_ICU_CDECL *unorm2_getNFKCInstance)(
      UErrorCode *pErrorCode);
  const UNormalizer2 *(HERMES_ICU_CDECL *unorm2_getNFKDInstance)(
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *unorm2_normalize)(
      const UNormalizer2 *norm2,
      const UChar *src,
      int32_t length,
      UChar *dest,
      int32_t capacity,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *unorm2_normalizeSecondAndAppend)(
      const UNormalizer2 *norm2,
      UChar *first,
      int32_t firstLength,
      int32_t firstCapacity,
      const UChar *second,
      int32_t secondLength,
      UErrorCode *pErrorCode);
  UBool(HERMES_ICU_CDECL *unorm2_isNormalized)(
      const UNormalizer2 *norm2,
      const UChar *s,
      int32_t length,
      UErrorCode *pErrorCode);
  UNormalizationCheckResult(HERMES_ICU_CDECL *unorm2_quickCheck)(
      const UNormalizer2 *norm2,
      const UChar *s,
      int32_t length,
      UErrorCode *pErrorCode);
  int32_t(HERMES_ICU_CDECL *unorm2_spanQuickCheckYes)(
      const UNormalizer2 *norm2,
      const UChar *s,
      int32_t length,
      UErrorCode *pErrorCode);

  /* ===============================================================
   * 2.3.22 Resource Bundle (ures_*) — 3 functions
   * =============================================================== */

  UResourceBundle *(HERMES_ICU_CDECL *ures_open)(
      const char *packageName,
      const char *locale,
      UErrorCode *status);
  void(HERMES_ICU_CDECL *ures_close)(UResourceBundle *resourceBundle);
  UResourceBundle *(HERMES_ICU_CDECL *ures_getByKey)(
      const UResourceBundle *resourceBundle,
      const char *key,
      UResourceBundle *fillIn,
      UErrorCode *status);

  /* ===============================================================
   * 2.3.23 USet — 5 functions
   * =============================================================== */

  USet *(HERMES_ICU_CDECL *uset_openEmpty)(void);
  void(HERMES_ICU_CDECL *uset_close)(USet *set);
  UBool(HERMES_ICU_CDECL *uset_contains)(const USet *set, UChar32 c);
  int32_t(HERMES_ICU_CDECL *uset_getItem)(
      const USet *set,
      int32_t itemIndex,
      UChar32 *start,
      UChar32 *end,
      UChar *str,
      int32_t strCapacity,
      UErrorCode *ec);
  int32_t(HERMES_ICU_CDECL *uset_getItemCount)(const USet *set);

  /* ===============================================================
   * 2.3.24 Custom shims — functions that require ICU C++ internals
   *
   * ICU's C API uloc_canonicalize() only performs basic locale ID
   * canonicalization (a small static lookup table). Full CLDR alias
   * resolution (languageAlias, scriptAlias, territoryAlias,
   * subdivisionAlias, variantAlias, transformed-extension aliases)
   * requires ICU C++ Locale::canonicalize() which uses AliasReplacer
   * to read CLDR metadata resource bundles. These shims expose that
   * C++ functionality through C function pointers.
   *
   * Providers that only have C API access (system-icu, dynamic)
   * should set these to NULL; the caller will fall back to the
   * 3-step C pipeline (uloc_forLanguageTag + uloc_canonicalize +
   * uloc_toLanguageTag) which handles a subset of aliases.
   * =============================================================== */

  /**
   * Full CLDR canonicalization of a BCP47 language tag.
   *
   * Equivalent to:
   *   Locale loc = Locale::forLanguageTag(langtag, err);
   *   loc.canonicalize(err);
   *   loc.toLanguageTag(result, err);
   *
   * @param langtag       Input BCP47 language tag (UTF-8, null-terminated).
   * @param result        Output buffer for the canonicalized BCP47 tag.
   * @param resultCapacity  Size of the output buffer.
   * @param err           ICU error code (in/out).
   * @return Length of the canonicalized tag (excluding null terminator),
   *         or required capacity if U_BUFFER_OVERFLOW_ERROR.
   *
   * May be NULL — callers must check before use.
   */
  int32_t(HERMES_ICU_CDECL *canonicalize_locale_tag)(
      const char *langtag,
      char *result,
      int32_t resultCapacity,
      UErrorCode *err);
} hermes_icu_vtable;

/**
 * Function pointer type for the hermes-icu.dll entry point.
 * Exported by hermes-icu.dll — returns the vtable struct.
 */
typedef const hermes_icu_vtable *(
    HERMES_ICU_CDECL *hermes_icu_get_vtable_fn)(void);

/**
 * Set a host-provided ICU vtable. Must be called before any Hermes runtime
 * is created. Hermes does NOT take ownership — the host must ensure the
 * vtable remains valid for the lifetime of the process.
 *
 * Pass NULL to reset to the default (auto-detect) behavior.
 */
HERMES_ICU_API void HERMES_ICU_CDECL
hermes_icu_set_vtable(const hermes_icu_vtable *vt);

/**
 * Get the currently active ICU vtable (for inspection/testing).
 * Returns NULL if no provider is active yet (lazy init hasn't happened).
 */
HERMES_ICU_API const hermes_icu_vtable *HERMES_ICU_CDECL
hermes_icu_get_active_vtable(void);

#ifdef _WIN32
/**
 * Load ICU from unicode.org DLL files at the specified paths.
 * Resolves versioned symbol names (e.g. "uloc_forLanguageTag_78").
 * If icu_version is 0, resolves unversioned names (for builds with
 * U_DISABLE_RENAMING=1 or bundled hermes-icu.dll).
 *
 * Sets the loaded vtable as the active provider (calls
 * hermes_icu_set_vtable internally). Must be called before any
 * Hermes runtime is created.
 *
 * @param icu_common_dll_path Full path to icuucNN.dll (required).
 * @param icu_i18n_dll_path   Full path to icuinNN.dll (may be NULL if
 *                            all needed functions are in the common DLL).
 * @param icu_version         ICU major version (e.g. 78), or 0 for
 *                            unversioned names.
 * @return 0 on success, non-zero on failure.
 */
HERMES_ICU_API int32_t HERMES_ICU_CDECL hermes_icu_load_from_path(
    const wchar_t *icu_common_dll_path,
    const wchar_t *icu_i18n_dll_path,
    uint32_t icu_version);
#endif

#ifdef __cplusplus
}
#endif

#endif /* HERMES_ICU_H */
