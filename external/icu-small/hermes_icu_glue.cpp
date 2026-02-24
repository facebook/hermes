/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * Glue layer for hermes-icu.dll (v2 vtable).
 *
 * Populates the hermes_icu_vtable with direct function pointers to ICU C
 * API symbols (statically linked into this DLL). No wrapper functions —
 * every vtable entry points directly to the real ICU function.
 *
 * Exports a single function: hermes_icu_get_vtable().
 */

// Include real ICU headers FIRST — they define UErrorCode, UChar, etc.
// icu_types.h (included via hermes_icu.h) detects U_ICU_VERSION_MAJOR_NUM
// and skips its own type definitions to avoid redefinition conflicts.

// ICU common headers
#include "unicode/ubrk.h"
#include "unicode/uclean.h"
#include "unicode/ucurr.h"
#include "unicode/uenum.h"
#include "unicode/uldnames.h"
#include "unicode/uloc.h"
#include "unicode/unorm2.h"
#include "unicode/ures.h"
#include "unicode/uset.h"
#include "unicode/ustring.h"
#include "unicode/utext.h"
#include "unicode/utypes.h"
#include "unicode/uversion.h"

// ICU C++ headers (for shim functions that need C++ internals)
#include "unicode/locid.h"

// ICU i18n headers
#include "unicode/ucal.h"
#include "unicode/ucol.h"
#include "unicode/udat.h"
#include "unicode/udateintervalformat.h"
#include "unicode/udatpg.h"
#include "unicode/ufieldpositer.h"
#include "unicode/uformattedvalue.h"
#include "unicode/ulistformatter.h"
#include "unicode/unum.h"
#include "unicode/unumberformatter.h"
#include "unicode/unumberrangeformatter.h"
#include "unicode/unumsys.h"
#include "unicode/upluralrules.h"
#include "unicode/ureldatefmt.h"

// Hermes vtable header (includes icu_types.h, which is a no-op here
// because real ICU headers were included above).
#include "hermes/Platform/Intl/hermes_icu.h"

// ============================================================================
// Shims for functions whose ICU signatures don't perfectly match the vtable
// ============================================================================

// unum_setTextAttribute takes an extra 'tag' parameter typed as
// UNumberFormatTextAttribute (enum). Our vtable uses int32_t.
// The function pointers are ABI-compatible (enums are int-sized),
// so no shim is needed — the cast in the vtable assignment works.

// ucal_getCanonicalTimeZoneID: ICU uses UBool* for isSystemID.
// Our vtable uses UBool* which is the same type. No shim needed.

// ucol_openAvailableLocales is guarded by #if !UCONFIG_NO_SERVICE in ICU,
// which is disabled in our bundled build (UCONFIG_NO_SERVICE=1).
// Provide a shim that builds an enumeration from ucol_countAvailable /
// ucol_getAvailable (which are always available).
static UEnumeration *HERMES_ICU_CDECL
shim_ucol_openAvailableLocales(UErrorCode *status) {
  if (U_FAILURE(*status))
    return nullptr;
  // uloc_openAvailableByType(ULOC_AVAILABLE_DEFAULT) returns all ICU
  // locales, which is a superset of collation locales. This is the
  // best we can do without the service registry.
  return uloc_openAvailableByType(ULOC_AVAILABLE_DEFAULT, status);
}

// Full CLDR canonicalization of a BCP47 language tag.
// ICU's C API uloc_canonicalize() only uses a small static lookup table.
// The C++ Locale::canonicalize() additionally uses AliasReplacer which reads
// comprehensive CLDR alias data (language, script, territory, subdivision,
// variant, transformed-extension aliases) from resource bundles.
static int32_t HERMES_ICU_CDECL
shim_canonicalize_locale_tag(
    const char *langtag,
    char *result,
    int32_t resultCapacity,
    UErrorCode *err) {
  if (U_FAILURE(*err))
    return 0;

  icu::Locale loc = icu::Locale::forLanguageTag(langtag, *err);
  if (U_FAILURE(*err))
    return 0;

  loc.canonicalize(*err);
  if (U_FAILURE(*err))
    return 0;

  // toLanguageTag writes to a ByteSink. Use a std::string sink.
  std::string tagStr;
  icu::StringByteSink<std::string> sink(&tagStr);
  loc.toLanguageTag(sink, *err);
  if (U_FAILURE(*err))
    return 0;

  int32_t len = static_cast<int32_t>(tagStr.length());
  if (len >= resultCapacity) {
    *err = U_BUFFER_OVERFLOW_ERROR;
    return len + 1;
  }
  memcpy(result, tagStr.c_str(), len + 1);
  return len;
}

// uloc_openAvailableByType: ICU uses ULocAvailableType enum, but our
// vtable uses int32_t for ABI portability. Cast via decltype.
#define CAST_VTABLE(field, fn) \
  reinterpret_cast<decltype(hermes_icu_vtable::field)>(&(fn))

// ============================================================================
// Static vtable instance — all entries are direct function pointers
// ============================================================================

// Count of populated function pointers (all ICU categories).
// This is an approximate count for the function_count metadata field.
#define HERMES_ICU_FUNCTION_COUNT 259

static const hermes_icu_vtable s_vtable = {
    /* version */ HERMES_ICU_VTABLE_VERSION,
    /* icu_version */ U_ICU_VERSION_MAJOR_NUM,
    /* provider_name */ "bundled",
    /* function_count */ HERMES_ICU_FUNCTION_COUNT,

    // === 2.3.1 Locale (uloc_*) — 27 functions ===
    /* uloc_forLanguageTag */ &uloc_forLanguageTag,
    /* uloc_toLanguageTag */ &uloc_toLanguageTag,
    /* uloc_canonicalize */ &uloc_canonicalize,
    /* uloc_getLanguage */ &uloc_getLanguage,
    /* uloc_getScript */ &uloc_getScript,
    /* uloc_getCountry */ &uloc_getCountry,
    /* uloc_getVariant */ &uloc_getVariant,
    /* uloc_getBaseName */ &uloc_getBaseName,
    /* uloc_getName */ &uloc_getName,
    /* uloc_getParent */ &uloc_getParent,
    /* uloc_getKeywordValue */ &uloc_getKeywordValue,
    /* uloc_setKeywordValue */ &uloc_setKeywordValue,
    /* uloc_openKeywords */ &uloc_openKeywords,
    /* uloc_addLikelySubtags */ &uloc_addLikelySubtags,
    /* uloc_minimizeSubtags */ &uloc_minimizeSubtags,
    /* uloc_countAvailable */ &uloc_countAvailable,
    /* uloc_getAvailable */ &uloc_getAvailable,
    /* uloc_getDefault */ &uloc_getDefault,
    /* uloc_openAvailableByType */
    CAST_VTABLE(uloc_openAvailableByType, uloc_openAvailableByType),
    /* uloc_getDisplayLanguage */ &uloc_getDisplayLanguage,
    /* uloc_getDisplayScript */ &uloc_getDisplayScript,
    /* uloc_getDisplayCountry */ &uloc_getDisplayCountry,
    /* uloc_getDisplayName */ &uloc_getDisplayName,
    /* uloc_acceptLanguage */ &uloc_acceptLanguage,
    /* uloc_toUnicodeLocaleKey */ &uloc_toUnicodeLocaleKey,
    /* uloc_toUnicodeLocaleType */ &uloc_toUnicodeLocaleType,
    /* uloc_toLegacyKey */ &uloc_toLegacyKey,
    /* uloc_toLegacyType */ &uloc_toLegacyType,
    /* uloc_isRightToLeft */ &uloc_isRightToLeft,

    // === 2.3.2 Collator (ucol_*) — 15 functions ===
    /* ucol_open */ &ucol_open,
    /* ucol_close */ &ucol_close,
    /* ucol_strcoll */ &ucol_strcoll,
    /* ucol_strcollUTF8 */ &ucol_strcollUTF8,
    /* ucol_setAttribute */ &ucol_setAttribute,
    /* ucol_getAttribute */ &ucol_getAttribute,
    /* ucol_setStrength */ &ucol_setStrength,
    /* ucol_getLocaleByType */ &ucol_getLocaleByType,
    /* ucol_openAvailableLocales */ &shim_ucol_openAvailableLocales,
    /* ucol_getKeywordValuesForLocale */ &ucol_getKeywordValuesForLocale,
    /* ucol_getKeywordValues */ &ucol_getKeywordValues,
    /* ucol_getReorderCodes */ &ucol_getReorderCodes,
    /* ucol_getRules */ &ucol_getRules,
    /* ucol_getTailoredSet */ &ucol_getTailoredSet,
    /* ucol_clone */ &ucol_clone,

    // === 2.3.3 NumberFormatter v2 (unumf_*) — 13 functions ===
    /* unumf_openForSkeletonAndLocale */ &unumf_openForSkeletonAndLocale,
    /* unumf_openForSkeletonAndLocaleWithError */
    &unumf_openForSkeletonAndLocaleWithError,
    /* unumf_openResult */ &unumf_openResult,
    /* unumf_formatDouble */ &unumf_formatDouble,
    /* unumf_formatInt */ &unumf_formatInt,
    /* unumf_formatDecimal */ &unumf_formatDecimal,
    /* unumf_resultToString */ &unumf_resultToString,
    /* unumf_resultToDecimalNumber */ &unumf_resultToDecimalNumber,
    /* unumf_resultAsValue */ &unumf_resultAsValue,
    /* unumf_resultGetAllFieldPositions */ &unumf_resultGetAllFieldPositions,
    /* unumf_resultNextFieldPosition */ &unumf_resultNextFieldPosition,
    /* unumf_close */ &unumf_close,
    /* unumf_closeResult */ &unumf_closeResult,

    // === 2.3.4 NumberRangeFormatter (unumrf_*) — 7 functions ===
    /* unumrf_openForSkeletonWithCollapseAndIdentityFallback */
    &unumrf_openForSkeletonWithCollapseAndIdentityFallback,
    /* unumrf_openResult */ &unumrf_openResult,
    /* unumrf_formatDoubleRange */ &unumrf_formatDoubleRange,
    /* unumrf_formatDecimalRange */ &unumrf_formatDecimalRange,
    /* unumrf_resultAsValue */ &unumrf_resultAsValue,
    /* unumrf_close */ &unumrf_close,
    /* unumrf_closeResult */ &unumrf_closeResult,

    // === 2.3.5 Legacy NumberFormat (unum_*) — 6 functions ===
    /* unum_open */ &unum_open,
    /* unum_close */ &unum_close,
    /* unum_formatDouble */ &unum_formatDouble,
    /* unum_format */ &unum_format,
    /* unum_setAttribute */ &unum_setAttribute,
    /* unum_getAttribute */ &unum_getAttribute,
    /* unum_setTextAttribute */ &unum_setTextAttribute,
    /* unum_formatDoubleForFields */ &unum_formatDoubleForFields,

    // === 2.3.6 DateFormat (udat_*) — 18 functions ===
    /* udat_open */ &udat_open,
    /* udat_close */ &udat_close,
    /* udat_format */ &udat_format,
    /* udat_formatForFields */ &udat_formatForFields,
    /* udat_formatCalendarForFields */ &udat_formatCalendarForFields,
    /* udat_toPattern */ &udat_toPattern,
    /* udat_applyPattern */ &udat_applyPattern,
    /* udat_getCalendar */ &udat_getCalendar,
    /* udat_setCalendar */ &udat_setCalendar,
    /* udat_getNumberFormat */ &udat_getNumberFormat,
    /* udat_adoptNumberFormat */ &udat_adoptNumberFormat,
    /* udat_getSymbols */ &udat_getSymbols,
    /* udat_getLocaleByType */ &udat_getLocaleByType,
    /* udat_getBooleanAttribute */ &udat_getBooleanAttribute,
    /* udat_setBooleanAttribute */ &udat_setBooleanAttribute,
    /* udat_setContext */ &udat_setContext,
    /* udat_getContext */ &udat_getContext,
    /* udat_clone */ &udat_clone,

    // === 2.3.7 DateTimePatternGenerator (udatpg_*) — 10 functions ===
    /* udatpg_open */ &udatpg_open,
    /* udatpg_close */ &udatpg_close,
    /* udatpg_getBestPattern */ &udatpg_getBestPattern,
    /* udatpg_getBestPatternWithOptions */ &udatpg_getBestPatternWithOptions,
    /* udatpg_getDefaultHourCycle */ &udatpg_getDefaultHourCycle,
    /* udatpg_getSkeleton */ &udatpg_getSkeleton,
    /* udatpg_getBaseSkeleton */ &udatpg_getBaseSkeleton,
    /* udatpg_getPatternForSkeleton */ &udatpg_getPatternForSkeleton,
    /* udatpg_getFieldDisplayName */ &udatpg_getFieldDisplayName,
    /* udatpg_clone */ &udatpg_clone,

    // === 2.3.8 DateIntervalFormat (udtitvfmt_*) — 7 functions ===
    /* udtitvfmt_open */ &udtitvfmt_open,
    /* udtitvfmt_close */ &udtitvfmt_close,
    /* udtitvfmt_openResult */ &udtitvfmt_openResult,
    /* udtitvfmt_closeResult */ &udtitvfmt_closeResult,
    /* udtitvfmt_formatToResult */ &udtitvfmt_formatToResult,
    /* udtitvfmt_resultAsValue */ &udtitvfmt_resultAsValue,
    /* udtitvfmt_format */ &udtitvfmt_format,

    // === 2.3.9 Calendar/Timezone (ucal_*) — 35 functions ===
    /* ucal_open */ &ucal_open,
    /* ucal_close */ &ucal_close,
    /* ucal_clone */ &ucal_clone,
    /* ucal_get */ &ucal_get,
    /* ucal_set */ &ucal_set,
    /* ucal_getMillis */ &ucal_getMillis,
    /* ucal_setMillis */ &ucal_setMillis,
    /* ucal_setAttribute */ &ucal_setAttribute,
    /* ucal_getAttribute */ &ucal_getAttribute,
    /* ucal_getDefaultTimeZone */ &ucal_getDefaultTimeZone,
    /* ucal_getCanonicalTimeZoneID */ &ucal_getCanonicalTimeZoneID,
    /* ucal_getTimeZoneDisplayName */ &ucal_getTimeZoneDisplayName,
    /* ucal_getTimeZoneID */ &ucal_getTimeZoneID,
    /* ucal_setTimeZone */ &ucal_setTimeZone,
    /* ucal_getTimeZoneIDForWindowsID */ &ucal_getTimeZoneIDForWindowsID,
    /* ucal_getWindowsTimeZoneID */ &ucal_getWindowsTimeZoneID,
    /* ucal_getHostTimeZone */ &ucal_getHostTimeZone,
    /* ucal_getTimeZoneTransitionDate */ &ucal_getTimeZoneTransitionDate,
    /* ucal_getTimeZoneOffsetFromLocal */ &ucal_getTimeZoneOffsetFromLocal,
    /* ucal_openTimeZones */ &ucal_openTimeZones,
    /* ucal_openTimeZoneIDEnumeration */ &ucal_openTimeZoneIDEnumeration,
    /* ucal_openCountryTimeZones */ &ucal_openCountryTimeZones,
    /* ucal_getKeywordValuesForLocale */ &ucal_getKeywordValuesForLocale,
    /* ucal_countAvailable */ &ucal_countAvailable,
    /* ucal_getAvailable */ &ucal_getAvailable,
    /* ucal_getType */ &ucal_getType,
    /* ucal_getDayOfWeekType */ &ucal_getDayOfWeekType,
    /* ucal_getWeekendTransition */ &ucal_getWeekendTransition,
    /* ucal_isWeekend */ &ucal_isWeekend,
    /* ucal_inDaylightTime */ &ucal_inDaylightTime,
    /* ucal_getNow */ &ucal_getNow,
    /* ucal_getFieldDifference */ &ucal_getFieldDifference,
    /* ucal_setGregorianChange */ &ucal_setGregorianChange,
    /* ucal_getGregorianChange */ &ucal_getGregorianChange,
    /* ucal_setDefaultTimeZone */ &ucal_setDefaultTimeZone,

    // === 2.3.10 PluralRules (uplrules_*) — 7 functions ===
    /* uplrules_open */ &uplrules_open,
    /* uplrules_openForType */ &uplrules_openForType,
    /* uplrules_close */ &uplrules_close,
    /* uplrules_select */ &uplrules_select,
    /* uplrules_selectFormatted */ &uplrules_selectFormatted,
    /* uplrules_selectForRange */ &uplrules_selectForRange,
    /* uplrules_getKeywords */ &uplrules_getKeywords,

    // === 2.3.11 ListFormatter (ulistfmt_*) — 8 functions ===
    /* ulistfmt_open */ &ulistfmt_open,
    /* ulistfmt_openForType */ &ulistfmt_openForType,
    /* ulistfmt_close */ &ulistfmt_close,
    /* ulistfmt_openResult */ &ulistfmt_openResult,
    /* ulistfmt_closeResult */ &ulistfmt_closeResult,
    /* ulistfmt_format */ &ulistfmt_format,
    /* ulistfmt_formatStringsToResult */ &ulistfmt_formatStringsToResult,
    /* ulistfmt_resultAsValue */ &ulistfmt_resultAsValue,

    // === 2.3.12 BreakIterator (ubrk_*) — 22 functions ===
    /* ubrk_open */ &ubrk_open,
    /* ubrk_close */ &ubrk_close,
    /* ubrk_clone */ &ubrk_clone,
    /* ubrk_setText */ &ubrk_setText,
    /* ubrk_setUText */ &ubrk_setUText,
    /* ubrk_current */ &ubrk_current,
    /* ubrk_next */ &ubrk_next,
    /* ubrk_previous */ &ubrk_previous,
    /* ubrk_first */ &ubrk_first,
    /* ubrk_last */ &ubrk_last,
    /* ubrk_preceding */ &ubrk_preceding,
    /* ubrk_following */ &ubrk_following,
    /* ubrk_isBoundary */ &ubrk_isBoundary,
    /* ubrk_getRuleStatus */ &ubrk_getRuleStatus,
    /* ubrk_getRuleStatusVec */ &ubrk_getRuleStatusVec,
    /* ubrk_countAvailable */ &ubrk_countAvailable,
    /* ubrk_getAvailable */ &ubrk_getAvailable,
    /* ubrk_getLocaleByType */ &ubrk_getLocaleByType,
    /* ubrk_openRules */ &ubrk_openRules,
    /* ubrk_getBinaryRules */ &ubrk_getBinaryRules,
    /* ubrk_openBinaryRules */ &ubrk_openBinaryRules,
    /* ubrk_refreshUText */ &ubrk_refreshUText,

    // === 2.3.13 RelativeDateTimeFormatter (ureldatefmt_*) — 10 functions ===
    /* ureldatefmt_open */ &ureldatefmt_open,
    /* ureldatefmt_close */ &ureldatefmt_close,
    /* ureldatefmt_openResult */ &ureldatefmt_openResult,
    /* ureldatefmt_closeResult */ &ureldatefmt_closeResult,
    /* ureldatefmt_format */ &ureldatefmt_format,
    /* ureldatefmt_formatNumeric */ &ureldatefmt_formatNumeric,
    /* ureldatefmt_formatToResult */ &ureldatefmt_formatToResult,
    /* ureldatefmt_formatNumericToResult */ &ureldatefmt_formatNumericToResult,
    /* ureldatefmt_resultAsValue */ &ureldatefmt_resultAsValue,
    /* ureldatefmt_combineDateAndTime */ &ureldatefmt_combineDateAndTime,

    // === 2.3.14 DisplayNames (uldn_*) — 10 functions ===
    /* uldn_open */ &uldn_open,
    /* uldn_openForContext */ &uldn_openForContext,
    /* uldn_close */ &uldn_close,
    /* uldn_getLocale */ &uldn_getLocale,
    /* uldn_getContext */ &uldn_getContext,
    /* uldn_localeDisplayName */ &uldn_localeDisplayName,
    /* uldn_languageDisplayName */ &uldn_languageDisplayName,
    /* uldn_regionDisplayName */ &uldn_regionDisplayName,
    /* uldn_scriptDisplayName */ &uldn_scriptDisplayName,
    /* uldn_keyValueDisplayName */ &uldn_keyValueDisplayName,

    // === 2.3.15 NumberingSystem (unumsys_*) — 8 functions ===
    /* unumsys_open */ &unumsys_open,
    /* unumsys_openByName */ &unumsys_openByName,
    /* unumsys_close */ &unumsys_close,
    /* unumsys_getName */ &unumsys_getName,
    /* unumsys_getDescription */ &unumsys_getDescription,
    /* unumsys_getRadix */ &unumsys_getRadix,
    /* unumsys_isAlgorithmic */ &unumsys_isAlgorithmic,
    /* unumsys_openAvailableNames */ &unumsys_openAvailableNames,

    // === 2.3.16 Currency (ucurr_*) — 4 functions ===
    /* ucurr_forLocale */ &ucurr_forLocale,
    /* ucurr_getDefaultFractionDigits */ &ucurr_getDefaultFractionDigits,
    /* ucurr_getName */ &ucurr_getName,
    /* ucurr_openISOCurrencies */ &ucurr_openISOCurrencies,

    // === 2.3.17 FormattedValue / ConstrainedFieldPosition — 14 functions ===
    /* ucfpos_open */ &ucfpos_open,
    /* ucfpos_close */ &ucfpos_close,
    /* ucfpos_reset */ &ucfpos_reset,
    /* ucfpos_constrainCategory */ &ucfpos_constrainCategory,
    /* ucfpos_constrainField */ &ucfpos_constrainField,
    /* ucfpos_getCategory */ &ucfpos_getCategory,
    /* ucfpos_getField */ &ucfpos_getField,
    /* ucfpos_getIndexes */ &ucfpos_getIndexes,
    /* ucfpos_getInt64IterationContext */ &ucfpos_getInt64IterationContext,
    /* ucfpos_setInt64IterationContext */ &ucfpos_setInt64IterationContext,
    /* ucfpos_matchesField */ &ucfpos_matchesField,
    /* ucfpos_setState */ &ucfpos_setState,
    /* ufmtval_getString */ &ufmtval_getString,
    /* ufmtval_nextPosition */ &ufmtval_nextPosition,

    // === 2.3.18 Legacy FieldPositionIterator — 3 functions ===
    /* ufieldpositer_open */ &ufieldpositer_open,
    /* ufieldpositer_close */ &ufieldpositer_close,
    /* ufieldpositer_next */ &ufieldpositer_next,

    // === 2.3.19 Enumeration (uenum_*) — 4 functions ===
    /* uenum_close */ &uenum_close,
    /* uenum_unext */ &uenum_unext,
    /* uenum_next */ &uenum_next,
    /* uenum_count */ &uenum_count,

    // === 2.3.20 String Utilities — 9 functions ===
    /* u_strToLower */ &u_strToLower,
    /* u_strToUpper */ &u_strToUpper,
    /* u_strToTitle */ &u_strToTitle,
    /* u_strFoldCase */ &u_strFoldCase,
    /* u_strcmp */ &u_strcmp,
    /* u_getVersion */ &u_getVersion,
    /* u_errorName */ &u_errorName,
    /* u_init */ &u_init,
    /* u_cleanup */ &u_cleanup,

    // === 2.3.21 Normalization (unorm2_*) — 10 functions ===
    /* unorm2_getInstance */ &unorm2_getInstance,
    /* unorm2_getNFCInstance */ &unorm2_getNFCInstance,
    /* unorm2_getNFDInstance */ &unorm2_getNFDInstance,
    /* unorm2_getNFKCInstance */ &unorm2_getNFKCInstance,
    /* unorm2_getNFKDInstance */ &unorm2_getNFKDInstance,
    /* unorm2_normalize */ &unorm2_normalize,
    /* unorm2_normalizeSecondAndAppend */ &unorm2_normalizeSecondAndAppend,
    /* unorm2_isNormalized */ &unorm2_isNormalized,
    /* unorm2_quickCheck */ &unorm2_quickCheck,
    /* unorm2_spanQuickCheckYes */ &unorm2_spanQuickCheckYes,

    // === 2.3.22 Resource Bundle (ures_*) — 3 functions ===
    /* ures_open */ &ures_open,
    /* ures_close */ &ures_close,
    /* ures_getByKey */ &ures_getByKey,

    // === 2.3.23 USet — 5 functions ===
    /* uset_openEmpty */ &uset_openEmpty,
    /* uset_close */ &uset_close,
    /* uset_contains */ &uset_contains,
    /* uset_getItem */ &uset_getItem,
    /* uset_getItemCount */ &uset_getItemCount,

    // === 2.3.24 Custom shims ===
    /* canonicalize_locale_tag */ &shim_canonicalize_locale_tag,
};

// ============================================================================
// Single DLL export: hermes_icu_get_vtable
// ============================================================================

extern "C" __declspec(dllexport) const hermes_icu_vtable *HERMES_ICU_CDECL
hermes_icu_get_vtable(void) {
  return &s_vtable;
}
