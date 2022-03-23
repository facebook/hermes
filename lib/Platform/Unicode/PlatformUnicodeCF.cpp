/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"
#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_CF

#include <CoreFoundation/CoreFoundation.h>

#include <ctime>

namespace hermes {
namespace platform_unicode {

namespace {
constexpr double MINUTES_PER_HOUR = 60;
constexpr double SECONDS_PER_MINUTE = 60;
constexpr double MS_PER_SECOND = 1000;
constexpr double MS_PER_MINUTE = MS_PER_SECOND * SECONDS_PER_MINUTE;
constexpr double MS_PER_HOUR = MS_PER_MINUTE * MINUTES_PER_HOUR;

/// Create the locale used for date formatting and collation. \return the
/// locale, transferring ownership to the caller (the "create" rule).
CFLocaleRef createLocale() {
  CFLocaleRef localeRef = nullptr;
  // Look for our special environment variable. This is used for testing only.
  const char *hermesLocale = std::getenv("_HERMES_TEST_LOCALE");
  if (hermesLocale) {
    CFStringRef localeName =
        CFStringCreateWithCString(nullptr, hermesLocale, kCFStringEncodingUTF8);
    localeRef = CFLocaleCreate(nullptr, localeName);
    CFRelease(localeName);
  }
  if (!localeRef)
    localeRef = CFLocaleCopyCurrent();
  return localeRef;
}

CFLocaleRef copyLocale() {
  static CFLocaleRef hermesLocale = createLocale();
  return CFLocaleCreateCopy(nullptr, hermesLocale);
}

/// return the local time zone adjustment in milliseconds.
double localTZA() {
  ::tzset();

  // Get the current time in seconds (might have DST adjustment included).
  time_t currentWithDST = std::time(nullptr);
  if (currentWithDST == static_cast<time_t>(-1)) {
    return 0;
  }

  // Deconstruct the time into localTime.
  std::tm *local = std::localtime(&currentWithDST);
  if (!local) {
    llvm_unreachable("localtime failed in localTZA()");
  }

#ifdef _WINDOWS
  long gmtoff = -_timezone;
#else
  long gmtoff = local->tm_gmtoff;
#endif

  // Use the gmtoff field and subtract an hour if currently in DST.
  return (gmtoff * MS_PER_SECOND) - (local->tm_isdst ? MS_PER_HOUR : 0);
}

} // namespace

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  auto s1 = CFStringCreateWithCharacters(
      nullptr, reinterpret_cast<const UniChar *>(left.data()), left.size());
  auto s2 = CFStringCreateWithCharacters(
      nullptr, reinterpret_cast<const UniChar *>(right.data()), right.size());
  CFLocaleRef locale = copyLocale();

  // Call this function with the localized and nonliteral flags.
  // Localized: Use the provided locale.
  // Nonliteral: account for things like ö being represented as the precomposed
  // \u00F6, or decomposed 'o' followed by diaeresis \u0308. This ensures that
  // canonically equivalent strings compare the same by applying NFD
  // decomposition.
  auto result = CFStringCompareWithOptionsAndLocale(
      s1,
      s2,
      CFRangeMake(0, CFStringGetLength(s1)),
      kCFCompareLocalized | kCFCompareNonliteral,
      locale);

  CFRelease(s1);
  CFRelease(s2);
  CFRelease(locale);

  switch (result) {
    case kCFCompareLessThan:
      return -1;
    case kCFCompareEqualTo:
      return 0;
    case kCFCompareGreaterThan:
      return 1;
  }
}

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  CFDateFormatterStyle dateStyle =
      formatDate ? kCFDateFormatterMediumStyle : kCFDateFormatterNoStyle;
  CFDateFormatterStyle timeStyle =
      formatTime ? kCFDateFormatterMediumStyle : kCFDateFormatterNoStyle;

  CFAbsoluteTime absTime =
      std::floor(unixtimeMs / MS_PER_SECOND) - kCFAbsoluteTimeIntervalSince1970;

  CFLocaleRef localeRef = copyLocale();
  CFTimeZoneRef timezoneRef = CFTimeZoneCreateWithTimeIntervalFromGMT(
      nullptr, localTZA() / MS_PER_SECOND);

  auto formatter =
      CFDateFormatterCreate(nullptr, localeRef, dateStyle, timeStyle);
  CFDateFormatterSetProperty(formatter, kCFDateFormatterTimeZone, timezoneRef);

  auto result =
      CFDateFormatterCreateStringWithAbsoluteTime(nullptr, formatter, absTime);

  auto len = CFStringGetLength(result);
  buf.resize(len);
  CFStringGetCharacters(result, CFRangeMake(0, len), (UniChar *)buf.data());

  CFRelease(result);
  CFRelease(formatter);
  CFRelease(timezoneRef);
  CFRelease(localeRef);
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  // UniChar is 16 bits, so a cast works.
  static_assert(sizeof(UniChar) == sizeof(char16_t), "Unexpected UniChar size");

  CFMutableStringRef cfstr = CFStringCreateMutable(nullptr, 0);
  CFStringAppendCharacters(cfstr, (UniChar *)buf.data(), buf.size());

  auto converter = targetCase == CaseConversion::ToUpper ? CFStringUppercase
                                                         : CFStringLowercase;
  if (useCurrentLocale) {
    CFLocaleRef locale = copyLocale();
    converter(cfstr, locale);
    CFRelease(locale);
  } else {
    converter(cfstr, nullptr);
  }

  // Reuse buf for the output string.
  uint32_t resultLen = CFStringGetLength(cfstr);
  buf.resize(resultLen);
  CFStringGetCharacters(
      cfstr, CFRangeMake(0, resultLen), (UniChar *)buf.data());
  CFRelease(cfstr);
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  // UniChar is 16 bits, so a cast works.
  static_assert(sizeof(UniChar) == sizeof(char16_t), "Unexpected UniChar size");

  CFMutableStringRef cfStr = CFStringCreateMutable(nullptr, 0);
  CFStringAppendCharacters(cfStr, (UniChar *)buf.data(), buf.size());

  CFStringNormalizationForm cfForm;
  switch (form) {
    case NormalizationForm::C:
      cfForm = kCFStringNormalizationFormC;
      break;
    case NormalizationForm::D:
      cfForm = kCFStringNormalizationFormD;
      break;
    case NormalizationForm::KC:
      cfForm = kCFStringNormalizationFormKC;
      break;
    case NormalizationForm::KD:
      cfForm = kCFStringNormalizationFormKD;
      break;
  }

  CFStringNormalize(cfStr, cfForm);

  // Reuse buf for the output string.
  uint32_t resultLen = CFStringGetLength(cfStr);
  buf.resize(resultLen);
  CFStringGetCharacters(
      cfStr, CFRangeMake(0, resultLen), (UniChar *)buf.data());
  CFRelease(cfStr);
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_CF
