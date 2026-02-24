/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

/**
 * System ICU provider (v2 vtable).
 *
 * Loads icu.dll from System32 (Windows 10 1903+) and populates the
 * hermes_icu_vtable with direct GetProcAddress results. No wrapper
 * functions — each vtable entry is a direct cast of the resolved proc.
 *
 * Functions not found in the system ICU are left as nullptr. The
 * implementation layer checks for nullptr before calling.
 */

#include "IcuProviderWin.h"
#include "hermes/Platform/Intl/hermes_icu.h"

#include <mutex>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace hermes {
namespace platform_intl {
namespace {

static hermes_icu_vtable s_winVtable = {};
static std::once_flag s_initFlag;
static bool s_loaded = false;

/// Resolve a required function — fail if not found.
#define RESOLVE_REQ(dll, name)                                     \
  s_winVtable.name = reinterpret_cast<decltype(s_winVtable.name)>( \
      GetProcAddress(dll, #name));                                  \
  if (!s_winVtable.name)                                            \
    return false

/// Resolve an optional function — leave nullptr if not found.
#define RESOLVE_OPT(dll, name)                                     \
  s_winVtable.name = reinterpret_cast<decltype(s_winVtable.name)>( \
      GetProcAddress(dll, #name))

static bool loadIcuDll() {
  // Load icu.dll from System32 only (Windows 10 1903+).
  HMODULE dll =
      LoadLibraryExW(L"icu.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
  if (!dll)
    return false;

  // Zero-initialize the vtable (all function pointers start as nullptr).
  memset(&s_winVtable, 0, sizeof(s_winVtable));

  // Metadata.
  s_winVtable.version = HERMES_ICU_VTABLE_VERSION;
  s_winVtable.icu_version = 0; // unknown — system-managed
  s_winVtable.provider_name = "windows";
  s_winVtable.function_count = 0; // set at end

  // ---------------------------------------------------------------
  // Required functions — loading fails if any are missing.
  // These are the core functions needed by the existing Intl objects.
  // ---------------------------------------------------------------

  // Locale
  RESOLVE_REQ(dll, uloc_forLanguageTag);
  RESOLVE_REQ(dll, uloc_toLanguageTag);
  RESOLVE_REQ(dll, uloc_canonicalize);
  RESOLVE_REQ(dll, uloc_countAvailable);
  RESOLVE_REQ(dll, uloc_getAvailable);
  RESOLVE_REQ(dll, uloc_getDefault);

  // Collator
  RESOLVE_REQ(dll, ucol_open);
  RESOLVE_REQ(dll, ucol_close);
  RESOLVE_REQ(dll, ucol_strcoll);
  RESOLVE_REQ(dll, ucol_setAttribute);

  // DateFormat
  RESOLVE_REQ(dll, udat_open);
  RESOLVE_REQ(dll, udat_close);
  RESOLVE_REQ(dll, udat_format);
  RESOLVE_REQ(dll, udat_toPattern);
  RESOLVE_REQ(dll, udatpg_open);
  RESOLVE_REQ(dll, udatpg_close);
  RESOLVE_REQ(dll, udatpg_getBestPatternWithOptions);

  // NumberFormat
  RESOLVE_REQ(dll, unum_open);
  RESOLVE_REQ(dll, unum_close);
  RESOLVE_REQ(dll, unum_formatDouble);

  // String utilities
  RESOLVE_REQ(dll, u_strToLower);
  RESOLVE_REQ(dll, u_strToUpper);

  // Calendar/Timezone
  RESOLVE_REQ(dll, ucal_openTimeZones);
  RESOLVE_REQ(dll, ucal_getDefaultTimeZone);

  // Enumeration
  RESOLVE_REQ(dll, uenum_unext);
  RESOLVE_REQ(dll, uenum_close);

  // ---------------------------------------------------------------
  // Optional functions — leave nullptr if not found.
  // Newer Windows versions or future phases use these.
  // ---------------------------------------------------------------

  // Locale (extended)
  RESOLVE_OPT(dll, uloc_getLanguage);
  RESOLVE_OPT(dll, uloc_getScript);
  RESOLVE_OPT(dll, uloc_getCountry);
  RESOLVE_OPT(dll, uloc_getVariant);
  RESOLVE_OPT(dll, uloc_getBaseName);
  RESOLVE_OPT(dll, uloc_getName);
  RESOLVE_OPT(dll, uloc_getParent);
  RESOLVE_OPT(dll, uloc_getKeywordValue);
  RESOLVE_OPT(dll, uloc_setKeywordValue);
  RESOLVE_OPT(dll, uloc_openKeywords);
  RESOLVE_OPT(dll, uloc_addLikelySubtags);
  RESOLVE_OPT(dll, uloc_minimizeSubtags);
  RESOLVE_OPT(dll, uloc_openAvailableByType);
  RESOLVE_OPT(dll, uloc_getDisplayLanguage);
  RESOLVE_OPT(dll, uloc_getDisplayScript);
  RESOLVE_OPT(dll, uloc_getDisplayCountry);
  RESOLVE_OPT(dll, uloc_getDisplayName);
  RESOLVE_OPT(dll, uloc_acceptLanguage);
  RESOLVE_OPT(dll, uloc_toUnicodeLocaleKey);
  RESOLVE_OPT(dll, uloc_toUnicodeLocaleType);
  RESOLVE_OPT(dll, uloc_toLegacyKey);
  RESOLVE_OPT(dll, uloc_toLegacyType);
  RESOLVE_OPT(dll, uloc_isRightToLeft);

  // Collator (extended)
  RESOLVE_OPT(dll, ucol_strcollUTF8);
  RESOLVE_OPT(dll, ucol_getAttribute);
  RESOLVE_OPT(dll, ucol_setStrength);
  RESOLVE_OPT(dll, ucol_getLocaleByType);
  RESOLVE_OPT(dll, ucol_openAvailableLocales);
  RESOLVE_OPT(dll, ucol_getKeywordValuesForLocale);
  RESOLVE_OPT(dll, ucol_getKeywordValues);
  RESOLVE_OPT(dll, ucol_getReorderCodes);
  RESOLVE_OPT(dll, ucol_getRules);
  RESOLVE_OPT(dll, ucol_getTailoredSet);
  RESOLVE_OPT(dll, ucol_clone);

  // NumberFormatter v2
  RESOLVE_OPT(dll, unumf_openForSkeletonAndLocale);
  RESOLVE_OPT(dll, unumf_openForSkeletonAndLocaleWithError);
  RESOLVE_OPT(dll, unumf_openResult);
  RESOLVE_OPT(dll, unumf_formatDouble);
  RESOLVE_OPT(dll, unumf_formatInt);
  RESOLVE_OPT(dll, unumf_formatDecimal);
  RESOLVE_OPT(dll, unumf_resultToString);
  RESOLVE_OPT(dll, unumf_resultToDecimalNumber);
  RESOLVE_OPT(dll, unumf_resultAsValue);
  RESOLVE_OPT(dll, unumf_resultGetAllFieldPositions);
  RESOLVE_OPT(dll, unumf_resultNextFieldPosition);
  RESOLVE_OPT(dll, unumf_close);
  RESOLVE_OPT(dll, unumf_closeResult);

  // NumberRangeFormatter
  RESOLVE_OPT(dll, unumrf_openForSkeletonWithCollapseAndIdentityFallback);
  RESOLVE_OPT(dll, unumrf_openResult);
  RESOLVE_OPT(dll, unumrf_formatDoubleRange);
  RESOLVE_OPT(dll, unumrf_formatDecimalRange);
  RESOLVE_OPT(dll, unumrf_resultAsValue);
  RESOLVE_OPT(dll, unumrf_close);
  RESOLVE_OPT(dll, unumrf_closeResult);

  // Legacy NumberFormat (extended)
  RESOLVE_OPT(dll, unum_format);
  RESOLVE_OPT(dll, unum_setAttribute);
  RESOLVE_OPT(dll, unum_getAttribute);
  RESOLVE_OPT(dll, unum_setTextAttribute);
  RESOLVE_OPT(dll, unum_formatDoubleForFields);

  // DateFormat (extended)
  RESOLVE_OPT(dll, udat_formatForFields);
  RESOLVE_OPT(dll, udat_formatCalendarForFields);
  RESOLVE_OPT(dll, udat_applyPattern);
  RESOLVE_OPT(dll, udat_getCalendar);
  RESOLVE_OPT(dll, udat_setCalendar);
  RESOLVE_OPT(dll, udat_getNumberFormat);
  RESOLVE_OPT(dll, udat_adoptNumberFormat);
  RESOLVE_OPT(dll, udat_getSymbols);
  RESOLVE_OPT(dll, udat_getLocaleByType);
  RESOLVE_OPT(dll, udat_getBooleanAttribute);
  RESOLVE_OPT(dll, udat_setBooleanAttribute);
  RESOLVE_OPT(dll, udat_setContext);
  RESOLVE_OPT(dll, udat_getContext);
  RESOLVE_OPT(dll, udat_clone);

  // DateTimePatternGenerator (extended)
  RESOLVE_OPT(dll, udatpg_getBestPattern);
  RESOLVE_OPT(dll, udatpg_getDefaultHourCycle);
  RESOLVE_OPT(dll, udatpg_getSkeleton);
  RESOLVE_OPT(dll, udatpg_getBaseSkeleton);
  RESOLVE_OPT(dll, udatpg_getPatternForSkeleton);
  RESOLVE_OPT(dll, udatpg_getFieldDisplayName);
  RESOLVE_OPT(dll, udatpg_clone);

  // DateIntervalFormat
  RESOLVE_OPT(dll, udtitvfmt_open);
  RESOLVE_OPT(dll, udtitvfmt_close);
  RESOLVE_OPT(dll, udtitvfmt_openResult);
  RESOLVE_OPT(dll, udtitvfmt_closeResult);
  RESOLVE_OPT(dll, udtitvfmt_formatToResult);
  RESOLVE_OPT(dll, udtitvfmt_resultAsValue);
  RESOLVE_OPT(dll, udtitvfmt_format);

  // Calendar/Timezone (extended)
  RESOLVE_OPT(dll, ucal_open);
  RESOLVE_OPT(dll, ucal_close);
  RESOLVE_OPT(dll, ucal_clone);
  RESOLVE_OPT(dll, ucal_get);
  RESOLVE_OPT(dll, ucal_set);
  RESOLVE_OPT(dll, ucal_getMillis);
  RESOLVE_OPT(dll, ucal_setMillis);
  RESOLVE_OPT(dll, ucal_setAttribute);
  RESOLVE_OPT(dll, ucal_getAttribute);
  RESOLVE_OPT(dll, ucal_getCanonicalTimeZoneID);
  RESOLVE_OPT(dll, ucal_getTimeZoneDisplayName);
  RESOLVE_OPT(dll, ucal_getTimeZoneID);
  RESOLVE_OPT(dll, ucal_setTimeZone);
  RESOLVE_OPT(dll, ucal_getTimeZoneIDForWindowsID);
  RESOLVE_OPT(dll, ucal_getWindowsTimeZoneID);
  RESOLVE_OPT(dll, ucal_getHostTimeZone);
  RESOLVE_OPT(dll, ucal_getTimeZoneTransitionDate);
  RESOLVE_OPT(dll, ucal_getTimeZoneOffsetFromLocal);
  RESOLVE_OPT(dll, ucal_openTimeZoneIDEnumeration);
  RESOLVE_OPT(dll, ucal_openCountryTimeZones);
  RESOLVE_OPT(dll, ucal_getKeywordValuesForLocale);
  RESOLVE_OPT(dll, ucal_countAvailable);
  RESOLVE_OPT(dll, ucal_getAvailable);
  RESOLVE_OPT(dll, ucal_getType);
  RESOLVE_OPT(dll, ucal_getDayOfWeekType);
  RESOLVE_OPT(dll, ucal_getWeekendTransition);
  RESOLVE_OPT(dll, ucal_isWeekend);
  RESOLVE_OPT(dll, ucal_inDaylightTime);
  RESOLVE_OPT(dll, ucal_getNow);
  RESOLVE_OPT(dll, ucal_getFieldDifference);
  RESOLVE_OPT(dll, ucal_setGregorianChange);
  RESOLVE_OPT(dll, ucal_getGregorianChange);
  RESOLVE_OPT(dll, ucal_setDefaultTimeZone);

  // PluralRules
  RESOLVE_OPT(dll, uplrules_open);
  RESOLVE_OPT(dll, uplrules_openForType);
  RESOLVE_OPT(dll, uplrules_close);
  RESOLVE_OPT(dll, uplrules_select);
  RESOLVE_OPT(dll, uplrules_selectFormatted);
  RESOLVE_OPT(dll, uplrules_selectForRange);
  RESOLVE_OPT(dll, uplrules_getKeywords);

  // ListFormatter
  RESOLVE_OPT(dll, ulistfmt_open);
  RESOLVE_OPT(dll, ulistfmt_openForType);
  RESOLVE_OPT(dll, ulistfmt_close);
  RESOLVE_OPT(dll, ulistfmt_openResult);
  RESOLVE_OPT(dll, ulistfmt_closeResult);
  RESOLVE_OPT(dll, ulistfmt_format);
  RESOLVE_OPT(dll, ulistfmt_formatStringsToResult);
  RESOLVE_OPT(dll, ulistfmt_resultAsValue);

  // BreakIterator
  RESOLVE_OPT(dll, ubrk_open);
  RESOLVE_OPT(dll, ubrk_close);
  RESOLVE_OPT(dll, ubrk_clone);
  RESOLVE_OPT(dll, ubrk_setText);
  RESOLVE_OPT(dll, ubrk_setUText);
  RESOLVE_OPT(dll, ubrk_current);
  RESOLVE_OPT(dll, ubrk_next);
  RESOLVE_OPT(dll, ubrk_previous);
  RESOLVE_OPT(dll, ubrk_first);
  RESOLVE_OPT(dll, ubrk_last);
  RESOLVE_OPT(dll, ubrk_preceding);
  RESOLVE_OPT(dll, ubrk_following);
  RESOLVE_OPT(dll, ubrk_isBoundary);
  RESOLVE_OPT(dll, ubrk_getRuleStatus);
  RESOLVE_OPT(dll, ubrk_getRuleStatusVec);
  RESOLVE_OPT(dll, ubrk_countAvailable);
  RESOLVE_OPT(dll, ubrk_getAvailable);
  RESOLVE_OPT(dll, ubrk_getLocaleByType);
  RESOLVE_OPT(dll, ubrk_openRules);
  RESOLVE_OPT(dll, ubrk_getBinaryRules);
  RESOLVE_OPT(dll, ubrk_openBinaryRules);
  RESOLVE_OPT(dll, ubrk_refreshUText);

  // RelativeDateTimeFormatter
  RESOLVE_OPT(dll, ureldatefmt_open);
  RESOLVE_OPT(dll, ureldatefmt_close);
  RESOLVE_OPT(dll, ureldatefmt_openResult);
  RESOLVE_OPT(dll, ureldatefmt_closeResult);
  RESOLVE_OPT(dll, ureldatefmt_format);
  RESOLVE_OPT(dll, ureldatefmt_formatNumeric);
  RESOLVE_OPT(dll, ureldatefmt_formatToResult);
  RESOLVE_OPT(dll, ureldatefmt_formatNumericToResult);
  RESOLVE_OPT(dll, ureldatefmt_resultAsValue);
  RESOLVE_OPT(dll, ureldatefmt_combineDateAndTime);

  // DisplayNames
  RESOLVE_OPT(dll, uldn_open);
  RESOLVE_OPT(dll, uldn_openForContext);
  RESOLVE_OPT(dll, uldn_close);
  RESOLVE_OPT(dll, uldn_getLocale);
  RESOLVE_OPT(dll, uldn_getContext);
  RESOLVE_OPT(dll, uldn_localeDisplayName);
  RESOLVE_OPT(dll, uldn_languageDisplayName);
  RESOLVE_OPT(dll, uldn_regionDisplayName);
  RESOLVE_OPT(dll, uldn_scriptDisplayName);
  RESOLVE_OPT(dll, uldn_keyValueDisplayName);

  // NumberingSystem
  RESOLVE_OPT(dll, unumsys_open);
  RESOLVE_OPT(dll, unumsys_openByName);
  RESOLVE_OPT(dll, unumsys_close);
  RESOLVE_OPT(dll, unumsys_getName);
  RESOLVE_OPT(dll, unumsys_getDescription);
  RESOLVE_OPT(dll, unumsys_getRadix);
  RESOLVE_OPT(dll, unumsys_isAlgorithmic);
  RESOLVE_OPT(dll, unumsys_openAvailableNames);

  // Currency
  RESOLVE_OPT(dll, ucurr_forLocale);
  RESOLVE_OPT(dll, ucurr_getDefaultFractionDigits);
  RESOLVE_OPT(dll, ucurr_getName);
  RESOLVE_OPT(dll, ucurr_openISOCurrencies);

  // FormattedValue / ConstrainedFieldPosition
  RESOLVE_OPT(dll, ucfpos_open);
  RESOLVE_OPT(dll, ucfpos_close);
  RESOLVE_OPT(dll, ucfpos_reset);
  RESOLVE_OPT(dll, ucfpos_constrainCategory);
  RESOLVE_OPT(dll, ucfpos_constrainField);
  RESOLVE_OPT(dll, ucfpos_getCategory);
  RESOLVE_OPT(dll, ucfpos_getField);
  RESOLVE_OPT(dll, ucfpos_getIndexes);
  RESOLVE_OPT(dll, ucfpos_getInt64IterationContext);
  RESOLVE_OPT(dll, ucfpos_setInt64IterationContext);
  RESOLVE_OPT(dll, ucfpos_matchesField);
  RESOLVE_OPT(dll, ucfpos_setState);
  RESOLVE_OPT(dll, ufmtval_getString);
  RESOLVE_OPT(dll, ufmtval_nextPosition);

  // FieldPositionIterator
  RESOLVE_OPT(dll, ufieldpositer_open);
  RESOLVE_OPT(dll, ufieldpositer_close);
  RESOLVE_OPT(dll, ufieldpositer_next);

  // Enumeration (extended)
  RESOLVE_OPT(dll, uenum_next);
  RESOLVE_OPT(dll, uenum_count);

  // String utilities (extended)
  RESOLVE_OPT(dll, u_strToTitle);
  RESOLVE_OPT(dll, u_strFoldCase);
  RESOLVE_OPT(dll, u_strcmp);
  RESOLVE_OPT(dll, u_getVersion);
  RESOLVE_OPT(dll, u_errorName);
  RESOLVE_OPT(dll, u_init);
  RESOLVE_OPT(dll, u_cleanup);

  // Normalization
  RESOLVE_OPT(dll, unorm2_getInstance);
  RESOLVE_OPT(dll, unorm2_getNFCInstance);
  RESOLVE_OPT(dll, unorm2_getNFDInstance);
  RESOLVE_OPT(dll, unorm2_getNFKCInstance);
  RESOLVE_OPT(dll, unorm2_getNFKDInstance);
  RESOLVE_OPT(dll, unorm2_normalize);
  RESOLVE_OPT(dll, unorm2_normalizeSecondAndAppend);
  RESOLVE_OPT(dll, unorm2_isNormalized);
  RESOLVE_OPT(dll, unorm2_quickCheck);
  RESOLVE_OPT(dll, unorm2_spanQuickCheckYes);

  // Resource Bundle
  RESOLVE_OPT(dll, ures_open);
  RESOLVE_OPT(dll, ures_close);
  RESOLVE_OPT(dll, ures_getByKey);

  // USet
  RESOLVE_OPT(dll, uset_openEmpty);
  RESOLVE_OPT(dll, uset_close);
  RESOLVE_OPT(dll, uset_contains);
  RESOLVE_OPT(dll, uset_getItem);
  RESOLVE_OPT(dll, uset_getItemCount);

  // canonicalize_locale_tag — left nullptr (memset); system-icu providers
  // fall back to the 3-step C pipeline (uloc_forLanguageTag +
  // uloc_canonicalize + uloc_toLanguageTag).

  // Intentionally leak the DLL handle — stays loaded for process lifetime.
  return true;
}

#undef RESOLVE_REQ
#undef RESOLVE_OPT

} // namespace

const hermes_icu_vtable *getWinIcuVtable() {
  std::call_once(s_initFlag, []() { s_loaded = loadIcuDll(); });
  return s_loaded ? &s_winVtable : nullptr;
}

} // namespace platform_intl
} // namespace hermes
