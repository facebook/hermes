/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU

#include "hermes/Platform/Unicode/icu.h"

#include <time.h>

namespace hermes {
namespace platform_unicode {

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  UErrorCode err{U_ZERO_ERROR};
  UCollator *coll = ucol_open(uloc_getDefault(), &err);
  if (U_FAILURE(err)) {
    // Failover to root locale if we're unable to open in default locale.
    err = U_ZERO_ERROR;
    coll = ucol_open("", &err);
  }
  assert(U_SUCCESS(err) && "failed to open collator");

  // Normalization mode allows for strings that can be represented
  // in two different ways to compare as equal.
  ucol_setAttribute(coll, UCOL_NORMALIZATION_MODE, UCOL_ON, &err);
  assert(U_SUCCESS(err) && "failed to set collator attribute");

  auto result = ucol_strcoll(
      coll,
      (const UChar *)left.data(),
      left.size(),
      (const UChar *)right.data(),
      right.size());

  ucol_close(coll);

  switch (result) {
    case UCOL_LESS:
      return -1;
    case UCOL_EQUAL:
      return 0;
    case UCOL_GREATER:
      return 1;
  }
  llvm_unreachable("Invalid result from ucol_strcoll");
}

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  UDateFormatStyle dateStyle = formatDate ? UDAT_MEDIUM : UDAT_NONE;
  UDateFormatStyle timeStyle = formatTime ? UDAT_MEDIUM : UDAT_NONE;

  UErrorCode err{U_ZERO_ERROR};

  llvh::SmallVector<char16_t, 32> tzstr;

  // We have to use this roundabout method to get the timezone name because
  // tzname cannot be concurrently accessed. Specifically, we have previously
  // observed cases where calls to tzset (and functions that call tzset, such as
  // localtime), may set elements of tzname to null. In this case, any thread
  // directly reading tzname at the same time may read the null value, causing
  // a segfault.
  char tz[50];
  time_t t = time(nullptr);
  struct tm timeInfo;
#ifdef _WINDOWS
  localtime_s(&timeInfo, &t);
#else
  localtime_r(&t, &timeInfo);
#endif
  auto numChars = strftime(tz, sizeof(tz), "%Z", &timeInfo);
  (void)numChars;
  assert(numChars < sizeof(tz) && "Chars written must fit within buffer");

  tzstr.append(tz, tz + strlen(tz));

  auto *df = udat_open(
      timeStyle,
      dateStyle,
      uloc_getDefault(),
      (const UChar *)tzstr.data(),
      tzstr.size(),
      nullptr,
      0,
      &err);
  if (!df) {
    return;
  }

  const int INITIAL_SIZE = 128;
  buf.resize(INITIAL_SIZE);
  err = U_ZERO_ERROR;
  int length =
      udat_format(df, unixtimeMs, (UChar *)buf.begin(), INITIAL_SIZE, 0, &err);
  if (length > INITIAL_SIZE) {
    buf.resize(length + 1);
    err = U_ZERO_ERROR;
    udat_format(df, unixtimeMs, (UChar *)buf.begin(), length, 0, &err);
    buf.resize(length);
  } else {
    buf.resize(length);
  }

  udat_close(df);
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  // To be able to call the converters, we have to get a pointer.
  // UChar is 16 bits, so a cast works.
  const UChar *src = (const UChar *)buf.data();
  auto srcLen = buf.size();

  auto converter =
      targetCase == CaseConversion::ToUpper ? u_strToUpper : u_strToLower;
  const char *locale = useCurrentLocale ? uloc_getDefault() : "";

  // First, try to uppercase without changing the length.
  // This will likely work.
  llvh::SmallVector<char16_t, 64> dest{};
  dest.resize(srcLen);
  UErrorCode err = U_ZERO_ERROR;
  size_t resultLen =
      converter((UChar *)dest.begin(), srcLen, src, srcLen, locale, &err);
  dest.resize(resultLen);

  // In the rare case our string was too small, rerun it.
  if (LLVM_UNLIKELY(resultLen > srcLen)) {
    err = U_ZERO_ERROR;
    converter((UChar *)dest.begin(), resultLen, src, srcLen, locale, &err);
  }
  // Assign to the inout parameter.
  buf = dest;
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  // To be able to call the converters, we have to get a pointer.
  // UChar is 16 bits, so a cast works.
  const UChar *src = (const UChar *)buf.data();
  auto srcLen = buf.size();
  UErrorCode err = U_ZERO_ERROR;

  const UNormalizer2 *norm = nullptr;
  switch (form) {
    case NormalizationForm::C:
      norm = unorm2_getNFCInstance(&err);
      break;
    case NormalizationForm::D:
      norm = unorm2_getNFDInstance(&err);
      break;
    case NormalizationForm::KC:
      norm = unorm2_getNFKCInstance(&err);
      break;
    case NormalizationForm::KD:
      norm = unorm2_getNFKDInstance(&err);
      break;
  }
  assert(U_SUCCESS(err) && norm && "Failed to get Normalizer instance");

  // First, try to normalize without changing the length.
  // This will likely work; note that this is an optimization for the
  // non-enlarging case.
  llvh::SmallVector<char16_t, 64> dest{};
  dest.resize(srcLen);
  err = U_ZERO_ERROR;
  size_t resultLen =
      unorm2_normalize(norm, src, srcLen, (UChar *)dest.begin(), srcLen, &err);
  dest.resize(resultLen);

  // In the rare case our string was too small, rerun it.
  if (LLVM_UNLIKELY(resultLen > srcLen)) {
    err = U_ZERO_ERROR;
    unorm2_normalize(norm, src, srcLen, (UChar *)dest.begin(), resultLen, &err);
  }

  // Assign to the inout parameter.
  buf = dest;
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_ICU
