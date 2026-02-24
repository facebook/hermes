/*
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */

#ifndef HERMES_PLATFORM_INTL_WIN_GLOB_H
#define HERMES_PLATFORM_INTL_WIN_GLOB_H

/**
 * WinGlob Intl fallback - Intl support using Windows NLS APIs.
 *
 * Provides functional Intl when no ICU is available, using APIs
 * available since Windows Vista/7:
 *   - CompareStringEx for collation
 *   - GetDateFormatEx / GetTimeFormatEx for date formatting
 *   - GetNumberFormatEx / GetCurrencyFormatEx for number formatting
 *   - LCMapStringEx for case conversion
 *   - ResolveLocaleName / EnumSystemLocalesEx for locale resolution
 *
 * Not fully ECMA-402 compliant but functional for common usage.
 * Limitations vs ICU:
 *   - No Unicode extension key support (-u-co-, -u-nu-)
 *   - formatToParts returns single literal part (no field decomposition)
 *   - Collator caseFirst has no effect (CompareStringEx limitation)
 *   - NumberFormat significant digits not supported
 *   - DateTimeFormat uses system timezone only
 */

#include "hermes/Platform/Intl/PlatformIntl.h"

#include <cstdint>

namespace hermes {
namespace platform_intl {
namespace winglob {

// --- Locale infrastructure (public, used by PlatformIntlWin.cpp) ---

/// Get all locales available on this Windows installation.
/// Cached on first call.
const std::vector<std::u16string> &getAvailableLocales();

/// Get the system default locale as BCP47 string.
std::u16string getDefaultLocale();

// --- Top-level functions ---

vm::CallResult<std::vector<std::u16string>> getCanonicalLocales(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales);

vm::CallResult<std::u16string> toLocaleLowerCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str);

vm::CallResult<std::u16string> toLocaleUpperCase(
    vm::Runtime &runtime,
    const std::vector<std::u16string> &locales,
    const std::u16string &str);

// --- Collator ---

class CollatorWinGlob : public Collator {
 public:
  CollatorWinGlob() = default;
  ~CollatorWinGlob() override;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;
  double compare(
      const std::u16string &x,
      const std::u16string &y) noexcept;

 private:
  std::u16string locale_;
  std::u16string usage_;
  std::u16string sensitivity_;
  bool ignorePunctuation_ = false;
  bool numeric_ = false;
  std::u16string caseFirst_;
  uint32_t compareFlags_ = 0;
};

// --- NumberFormat ---

class NumberFormatWinGlob : public NumberFormat {
 public:
  NumberFormatWinGlob() = default;
  ~NumberFormatWinGlob() override;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &options) noexcept;

  Options resolvedOptions() noexcept;
  std::u16string format(double number) noexcept;
  std::vector<Part> formatToParts(double number) noexcept;

 private:
  std::u16string formatDecimal(double number) noexcept;
  std::u16string formatCurrency(double number) noexcept;

  std::u16string locale_;
  std::u16string style_;
  std::u16string currency_;
  std::u16string currencyDisplay_;
  std::u16string unit_;
  std::u16string unitDisplay_;
  std::u16string notation_;
  int minimumIntegerDigits_ = 1;
  int minimumFractionDigits_ = 0;
  int maximumFractionDigits_ = 3;
  bool useGrouping_ = true;
  // Cached locale info.
  std::u16string decimalSep_;
  std::u16string thousandsSep_;
  std::u16string currencySymbol_;
};

// --- DateTimeFormat ---

class DateTimeFormatWinGlob : public DateTimeFormat {
 public:
  DateTimeFormatWinGlob() = default;
  ~DateTimeFormatWinGlob() override;

  vm::ExecutionStatus initialize(
      vm::Runtime &runtime,
      const std::vector<std::u16string> &locales,
      const Options &inputOptions) noexcept;

  Options resolvedOptions() noexcept;
  std::u16string format(double jsTimeValue) noexcept;
  std::vector<Part> formatToParts(double jsTimeValue) noexcept;

 private:
  std::u16string locale_;
  std::u16string dateStyle_;
  std::u16string timeStyle_;
  std::u16string weekday_;
  std::u16string era_;
  std::u16string year_;
  std::u16string month_;
  std::u16string day_;
  std::u16string hour_;
  std::u16string minute_;
  std::u16string second_;
  std::u16string hourCycle_;
  std::u16string timeZoneName_;
  // Computed format strings for GetDateFormatEx/GetTimeFormatEx.
  std::u16string dateFormatStr_;
  std::u16string timeFormatStr_;
  uint32_t dateFlags_ = 0;
  uint32_t timeFlags_ = 0;
  bool formatDate_ = false;
  bool formatTime_ = false;
};

} // namespace winglob
} // namespace platform_intl
} // namespace hermes

#endif // HERMES_PLATFORM_INTL_WIN_GLOB_H
