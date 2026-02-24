/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * Dynamic ICU provider (v2 vtable).
 *
 * Loads unicode.org ICU DLLs from user-specified paths and populates
 * the hermes_icu_vtable with direct GetProcAddress results using
 * versioned symbol names (e.g., "ucol_open_78").
 *
 * No wrapper functions — each vtable entry is a direct cast of the
 * resolved proc address.
 */

#include "IcuProviderDynamic.h"
#include "hermes/Platform/Intl/hermes_icu.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdio>

namespace hermes {
namespace platform_intl {
namespace {

static hermes_icu_vtable s_dynVtable = {};

/// Resolve a symbol by name from icuin first, then icuuc.
/// If version > 0, appends "_NN" suffix (e.g., "uloc_forLanguageTag_78").
static FARPROC resolveVersioned(
    HMODULE uc,
    HMODULE in,
    const char *name,
    uint32_t ver) {
  char buf[128];
  if (ver > 0)
    snprintf(buf, sizeof(buf), "%s_%u", name, ver);
  else
    snprintf(buf, sizeof(buf), "%s", name);
  // Try i18n DLL first (most Intl functions live there), then common.
  FARPROC p = in ? GetProcAddress(in, buf) : nullptr;
  if (!p)
    p = GetProcAddress(uc, buf);
  return p;
}

/// Resolve a required function — fail if not found.
#define RESOLVE_REQ(uc, in, name, ver)                                 \
  s_dynVtable.name = reinterpret_cast<decltype(s_dynVtable.name)>(     \
      resolveVersioned(uc, in, #name, ver));                            \
  if (!s_dynVtable.name)                                                \
    return -1 /* missing required function */

/// Resolve an optional function — leave nullptr if not found.
#define RESOLVE_OPT(uc, in, name, ver)                                 \
  s_dynVtable.name = reinterpret_cast<decltype(s_dynVtable.name)>(     \
      resolveVersioned(uc, in, #name, ver))

static int32_t loadDynamicIcuImpl(
    const wchar_t *icuCommonDllPath,
    const wchar_t *icuI18nDllPath,
    uint32_t icuVersion) {
  if (!icuCommonDllPath)
    return -1;

  // Load the DLLs. Use LOAD_WITH_ALTERED_SEARCH_PATH so dependency
  // DLLs (like icudt78.dll) are found next to the ICU DLLs.
  HMODULE ucDll =
      LoadLibraryExW(icuCommonDllPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!ucDll)
    return -1;

  HMODULE inDll = nullptr;
  if (icuI18nDllPath) {
    inDll =
        LoadLibraryExW(icuI18nDllPath, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!inDll)
      return -1;
  }

  uint32_t ver = icuVersion;

  // Zero-initialize the vtable.
  memset(&s_dynVtable, 0, sizeof(s_dynVtable));

  // Metadata.
  s_dynVtable.version = HERMES_ICU_VTABLE_VERSION;
  s_dynVtable.icu_version = icuVersion;
  s_dynVtable.provider_name = "custom";
  s_dynVtable.function_count = 0;

  // ---------------------------------------------------------------
  // Required functions — loading fails if any are missing.
  // ---------------------------------------------------------------

  // Locale
  RESOLVE_REQ(ucDll, inDll, uloc_forLanguageTag, ver);
  RESOLVE_REQ(ucDll, inDll, uloc_toLanguageTag, ver);
  RESOLVE_REQ(ucDll, inDll, uloc_canonicalize, ver);
  RESOLVE_REQ(ucDll, inDll, uloc_countAvailable, ver);
  RESOLVE_REQ(ucDll, inDll, uloc_getAvailable, ver);
  RESOLVE_REQ(ucDll, inDll, uloc_getDefault, ver);

  // Collator
  RESOLVE_REQ(ucDll, inDll, ucol_open, ver);
  RESOLVE_REQ(ucDll, inDll, ucol_close, ver);
  RESOLVE_REQ(ucDll, inDll, ucol_strcoll, ver);
  RESOLVE_REQ(ucDll, inDll, ucol_setAttribute, ver);

  // DateFormat
  RESOLVE_REQ(ucDll, inDll, udat_open, ver);
  RESOLVE_REQ(ucDll, inDll, udat_close, ver);
  RESOLVE_REQ(ucDll, inDll, udat_format, ver);
  RESOLVE_REQ(ucDll, inDll, udat_toPattern, ver);
  RESOLVE_REQ(ucDll, inDll, udatpg_open, ver);
  RESOLVE_REQ(ucDll, inDll, udatpg_close, ver);
  RESOLVE_REQ(ucDll, inDll, udatpg_getBestPatternWithOptions, ver);

  // NumberFormat
  RESOLVE_REQ(ucDll, inDll, unum_open, ver);
  RESOLVE_REQ(ucDll, inDll, unum_close, ver);
  RESOLVE_REQ(ucDll, inDll, unum_formatDouble, ver);

  // String utilities
  RESOLVE_REQ(ucDll, inDll, u_strToLower, ver);
  RESOLVE_REQ(ucDll, inDll, u_strToUpper, ver);

  // Calendar/Timezone
  RESOLVE_REQ(ucDll, inDll, ucal_openTimeZones, ver);
  RESOLVE_REQ(ucDll, inDll, ucal_getDefaultTimeZone, ver);

  // Enumeration
  RESOLVE_REQ(ucDll, inDll, uenum_unext, ver);
  RESOLVE_REQ(ucDll, inDll, uenum_close, ver);

  // ---------------------------------------------------------------
  // Optional functions — leave nullptr if not found.
  // ---------------------------------------------------------------

  // Locale (extended)
  RESOLVE_OPT(ucDll, inDll, uloc_getLanguage, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getScript, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getCountry, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getVariant, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getBaseName, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getName, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getParent, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getKeywordValue, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_setKeywordValue, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_openKeywords, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_addLikelySubtags, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_minimizeSubtags, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_openAvailableByType, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getDisplayLanguage, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getDisplayScript, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getDisplayCountry, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_getDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_acceptLanguage, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_toUnicodeLocaleKey, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_toUnicodeLocaleType, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_toLegacyKey, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_toLegacyType, ver);
  RESOLVE_OPT(ucDll, inDll, uloc_isRightToLeft, ver);

  // Collator (extended)
  RESOLVE_OPT(ucDll, inDll, ucol_strcollUTF8, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_setStrength, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getLocaleByType, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_openAvailableLocales, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getKeywordValuesForLocale, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getKeywordValues, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getReorderCodes, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getRules, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_getTailoredSet, ver);
  RESOLVE_OPT(ucDll, inDll, ucol_clone, ver);

  // NumberFormatter v2
  RESOLVE_OPT(ucDll, inDll, unumf_openForSkeletonAndLocale, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_openForSkeletonAndLocaleWithError, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_openResult, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_formatDouble, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_formatInt, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_formatDecimal, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_resultToString, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_resultToDecimalNumber, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_resultAsValue, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_resultGetAllFieldPositions, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_resultNextFieldPosition, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_close, ver);
  RESOLVE_OPT(ucDll, inDll, unumf_closeResult, ver);

  // NumberRangeFormatter
  RESOLVE_OPT(
      ucDll,
      inDll,
      unumrf_openForSkeletonWithCollapseAndIdentityFallback,
      ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_openResult, ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_formatDoubleRange, ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_formatDecimalRange, ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_resultAsValue, ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_close, ver);
  RESOLVE_OPT(ucDll, inDll, unumrf_closeResult, ver);

  // Legacy NumberFormat (extended)
  RESOLVE_OPT(ucDll, inDll, unum_format, ver);
  RESOLVE_OPT(ucDll, inDll, unum_setAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, unum_getAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, unum_setTextAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, unum_formatDoubleForFields, ver);

  // DateFormat (extended)
  RESOLVE_OPT(ucDll, inDll, udat_formatForFields, ver);
  RESOLVE_OPT(ucDll, inDll, udat_formatCalendarForFields, ver);
  RESOLVE_OPT(ucDll, inDll, udat_applyPattern, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getCalendar, ver);
  RESOLVE_OPT(ucDll, inDll, udat_setCalendar, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getNumberFormat, ver);
  RESOLVE_OPT(ucDll, inDll, udat_adoptNumberFormat, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getSymbols, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getLocaleByType, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getBooleanAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, udat_setBooleanAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, udat_setContext, ver);
  RESOLVE_OPT(ucDll, inDll, udat_getContext, ver);
  RESOLVE_OPT(ucDll, inDll, udat_clone, ver);

  // DateTimePatternGenerator (extended)
  RESOLVE_OPT(ucDll, inDll, udatpg_getBestPattern, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_getDefaultHourCycle, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_getSkeleton, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_getBaseSkeleton, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_getPatternForSkeleton, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_getFieldDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, udatpg_clone, ver);

  // DateIntervalFormat
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_open, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_close, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_openResult, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_closeResult, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_formatToResult, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_resultAsValue, ver);
  RESOLVE_OPT(ucDll, inDll, udtitvfmt_format, ver);

  // Calendar/Timezone (extended)
  RESOLVE_OPT(ucDll, inDll, ucal_open, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_close, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_clone, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_get, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_set, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getMillis, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_setMillis, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_setAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getAttribute, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getCanonicalTimeZoneID, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getTimeZoneDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getTimeZoneID, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_setTimeZone, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getTimeZoneIDForWindowsID, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getWindowsTimeZoneID, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getHostTimeZone, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getTimeZoneTransitionDate, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getTimeZoneOffsetFromLocal, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_openTimeZoneIDEnumeration, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_openCountryTimeZones, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getKeywordValuesForLocale, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_countAvailable, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getAvailable, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getType, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getDayOfWeekType, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getWeekendTransition, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_isWeekend, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_inDaylightTime, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getNow, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getFieldDifference, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_setGregorianChange, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_getGregorianChange, ver);
  RESOLVE_OPT(ucDll, inDll, ucal_setDefaultTimeZone, ver);

  // PluralRules
  RESOLVE_OPT(ucDll, inDll, uplrules_open, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_openForType, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_close, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_select, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_selectFormatted, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_selectForRange, ver);
  RESOLVE_OPT(ucDll, inDll, uplrules_getKeywords, ver);

  // ListFormatter
  RESOLVE_OPT(ucDll, inDll, ulistfmt_open, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_openForType, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_close, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_openResult, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_closeResult, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_format, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_formatStringsToResult, ver);
  RESOLVE_OPT(ucDll, inDll, ulistfmt_resultAsValue, ver);

  // BreakIterator
  RESOLVE_OPT(ucDll, inDll, ubrk_open, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_close, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_clone, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_setText, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_setUText, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_current, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_next, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_previous, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_first, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_last, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_preceding, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_following, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_isBoundary, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_getRuleStatus, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_getRuleStatusVec, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_countAvailable, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_getAvailable, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_getLocaleByType, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_openRules, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_getBinaryRules, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_openBinaryRules, ver);
  RESOLVE_OPT(ucDll, inDll, ubrk_refreshUText, ver);

  // RelativeDateTimeFormatter
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_open, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_close, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_openResult, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_closeResult, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_format, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_formatNumeric, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_formatToResult, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_formatNumericToResult, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_resultAsValue, ver);
  RESOLVE_OPT(ucDll, inDll, ureldatefmt_combineDateAndTime, ver);

  // DisplayNames
  RESOLVE_OPT(ucDll, inDll, uldn_open, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_openForContext, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_close, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_getLocale, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_getContext, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_localeDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_languageDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_regionDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_scriptDisplayName, ver);
  RESOLVE_OPT(ucDll, inDll, uldn_keyValueDisplayName, ver);

  // NumberingSystem
  RESOLVE_OPT(ucDll, inDll, unumsys_open, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_openByName, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_close, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_getName, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_getDescription, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_getRadix, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_isAlgorithmic, ver);
  RESOLVE_OPT(ucDll, inDll, unumsys_openAvailableNames, ver);

  // Currency
  RESOLVE_OPT(ucDll, inDll, ucurr_forLocale, ver);
  RESOLVE_OPT(ucDll, inDll, ucurr_getDefaultFractionDigits, ver);
  RESOLVE_OPT(ucDll, inDll, ucurr_getName, ver);
  RESOLVE_OPT(ucDll, inDll, ucurr_openISOCurrencies, ver);

  // FormattedValue / ConstrainedFieldPosition
  RESOLVE_OPT(ucDll, inDll, ucfpos_open, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_close, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_reset, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_constrainCategory, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_constrainField, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_getCategory, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_getField, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_getIndexes, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_getInt64IterationContext, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_setInt64IterationContext, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_matchesField, ver);
  RESOLVE_OPT(ucDll, inDll, ucfpos_setState, ver);
  RESOLVE_OPT(ucDll, inDll, ufmtval_getString, ver);
  RESOLVE_OPT(ucDll, inDll, ufmtval_nextPosition, ver);

  // FieldPositionIterator
  RESOLVE_OPT(ucDll, inDll, ufieldpositer_open, ver);
  RESOLVE_OPT(ucDll, inDll, ufieldpositer_close, ver);
  RESOLVE_OPT(ucDll, inDll, ufieldpositer_next, ver);

  // Enumeration (extended)
  RESOLVE_OPT(ucDll, inDll, uenum_next, ver);
  RESOLVE_OPT(ucDll, inDll, uenum_count, ver);

  // String utilities (extended)
  RESOLVE_OPT(ucDll, inDll, u_strToTitle, ver);
  RESOLVE_OPT(ucDll, inDll, u_strFoldCase, ver);
  RESOLVE_OPT(ucDll, inDll, u_strcmp, ver);
  RESOLVE_OPT(ucDll, inDll, u_getVersion, ver);
  RESOLVE_OPT(ucDll, inDll, u_errorName, ver);
  RESOLVE_OPT(ucDll, inDll, u_init, ver);
  RESOLVE_OPT(ucDll, inDll, u_cleanup, ver);

  // Normalization
  RESOLVE_OPT(ucDll, inDll, unorm2_getInstance, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_getNFCInstance, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_getNFDInstance, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_getNFKCInstance, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_getNFKDInstance, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_normalize, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_normalizeSecondAndAppend, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_isNormalized, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_quickCheck, ver);
  RESOLVE_OPT(ucDll, inDll, unorm2_spanQuickCheckYes, ver);

  // Resource Bundle
  RESOLVE_OPT(ucDll, inDll, ures_open, ver);
  RESOLVE_OPT(ucDll, inDll, ures_close, ver);
  RESOLVE_OPT(ucDll, inDll, ures_getByKey, ver);

  // USet
  RESOLVE_OPT(ucDll, inDll, uset_openEmpty, ver);
  RESOLVE_OPT(ucDll, inDll, uset_close, ver);
  RESOLVE_OPT(ucDll, inDll, uset_contains, ver);
  RESOLVE_OPT(ucDll, inDll, uset_getItem, ver);
  RESOLVE_OPT(ucDll, inDll, uset_getItemCount, ver);

  // canonicalize_locale_tag — left nullptr (memset); dynamic-icu providers
  // fall back to the 3-step C pipeline (uloc_forLanguageTag +
  // uloc_canonicalize + uloc_toLanguageTag).

  // Set the loaded vtable as active.
  hermes_icu_set_vtable(&s_dynVtable);

  // Intentionally leak the DLL handles.
  return 0; // success
}

#undef RESOLVE_REQ
#undef RESOLVE_OPT

} // namespace

int32_t loadDynamicIcu(
    const wchar_t *icuCommonDllPath,
    const wchar_t *icuI18nDllPath,
    uint32_t icuVersion) {
  return loadDynamicIcuImpl(icuCommonDllPath, icuI18nDllPath, icuVersion);
}

} // namespace platform_intl
} // namespace hermes
