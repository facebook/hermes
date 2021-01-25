/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/Platform/Unicode/PlatformUnicode.h"

#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_WINGLOB

#include <cstdint>
#include "Windows.Globalization.h"

namespace hermes {
namespace platform_unicode {

NORM_FORM convertToWinNLSNormForm(NormalizationForm normForm) {
  switch (normForm) {
    case NormalizationForm::C:
      return NORM_FORM::NormalizationC;
    case NormalizationForm::D:
      return NORM_FORM::NormalizationD;
    case NormalizationForm::KC:
      return NORM_FORM::NormalizationKC;
    case NormalizationForm::KD:
      return NORM_FORM::NormalizationKD;
    default:
      llvm_unreachable("Unknown normalization form !");
  }
}

bool IsNormalizedString(
    const char16_t *bufferPtr,
    uint32_t bufferLength,
    NormalizationForm normForm) {
  static_assert(
      sizeof(wchar_t) == sizeof(char16_t), "sizeof(wchar_t) == sizeof(UChar)");
  return ::IsNormalizedString(
      convertToWinNLSNormForm(normForm),
      reinterpret_cast<const wchar_t *>(bufferPtr),
      bufferLength);
}

bool GetNormalizedString(
    const char16_t *bufferPtr,
    uint32_t bufferLength,
    NormalizationForm normForm,
    llvh::SmallVectorImpl<char16_t> &output,
    uint32_t &sizeOfNormalizedStringWithoutNullTerminator) {
  // IMPORTANT: Implementation Notes
  // We referred to ChakraCore implementation and the following awesome comment is
  // copied from there.
  // Normalize string estimates
  // the required size of the buffer based on averages and other data. It is
  // very hard to get a precise size from an input string without
  // expanding/contracting it on the buffer. It is estimated that the maximum
  // size the string after an NFC is 6x the input length, and 18x for NFD. This
  // approach isn't very feasible as well. The approach taken is based on the
  // simple example in the MSDN article.
  //  - Loop until the return value is either an error (apart from insufficient
  //  buffer size), or success.
  //  - Each time recreate a temporary buffer based on the last guess.
  //  - When creating the JS string, use the positive return value and copy the
  //  buffer across.
  // Design choice for "guesses" comes from data Windows collected; and in most
  // cases the loop will not iterate more than 2 times.

  assert(
      !IsNormalizedString(bufferPtr, bufferLength, normForm) &&
      "It is unoptimal to call this function for normalized strings. The caller should first call IsNormalizedString.");

  static_assert(
      sizeof(wchar_t) == sizeof(char16_t), "sizeof(wchar_t) == sizeof(UChar)");

  // 1. First query for the initial estimate of the output buffer size.
  int sizeEstimate = NormalizeString(
      convertToWinNLSNormForm(normForm),
      reinterpret_cast<const wchar_t *>(bufferPtr),
      bufferLength,
      nullptr,
      0);

  assert(sizeEstimate > 0);
  if (sizeEstimate <= 0) {
    return false;
  }

  const int MAX_NORMATLIZATION_ITERATIONS = 10;
  for (int i = 0; i < MAX_NORMATLIZATION_ITERATIONS; i++) {
    // Try normalizing again after resizing the output buffer based on the last estimate.
    output.resize(sizeEstimate);
    int newSizeEstimate = NormalizeString(
        convertToWinNLSNormForm(normForm),
        reinterpret_cast<const wchar_t *>(bufferPtr),
        bufferLength,
        reinterpret_cast<wchar_t *>(output.data()),
        sizeEstimate);

    // The normalization succeded if the return value is greter than zero.
    if (newSizeEstimate > 0) {
      output.resize(newSizeEstimate); // Compact the output buffer.
      return true;
    } else {
      // If the return value is less than zero and the error code is set to
      // ERROR_INSUFFICIENT_BUFFER, retry with the new estimate. Fail otherwise.
      DWORD dwError = GetLastError();
      if (dwError != ERROR_INSUFFICIENT_BUFFER) {
        return false;
      }

      newSizeEstimate = -1 * newSizeEstimate;
      if (newSizeEstimate < sizeEstimate) {
        return false;
      }
    }
  }

  return false;
}

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {
  llvh::SmallVector<char16_t, 16> normalizedLeft;
  if (!IsNormalizedString(left.data(), left.size(), NormalizationForm::C)) {
    uint32_t sizeOfNormalizedStringWithoutNullTerminator;
    bool success = GetNormalizedString(
        left.data(),
        left.size(),
        NormalizationForm::C,
        normalizedLeft,
        sizeOfNormalizedStringWithoutNullTerminator);
    if (success)
      left = llvh::ArrayRef<char16_t>(
          normalizedLeft.data(), normalizedLeft.size());
  }

  llvh::SmallVector<char16_t, 16> normalizedRight;
  if (!IsNormalizedString(right.data(), right.size(), NormalizationForm::C)) {
    uint32_t sizeOfNormalizedStringWithoutNullTerminator;
    bool success = GetNormalizedString(
        right.data(),
        right.size(),
        NormalizationForm::C,
        normalizedRight,
        sizeOfNormalizedStringWithoutNullTerminator);
    if (success)
      right = llvh::ArrayRef<char16_t>(
          normalizedRight.data(), normalizedRight.size());
  }

  DWORD comparisonFlags = 0;
  int compareResult = CompareStringEx(
      LOCALE_NAME_USER_DEFAULT,
      comparisonFlags,
      reinterpret_cast<LPWCH>(const_cast<char16_t *>(left.data())),
      left.size(),
      reinterpret_cast<LPWCH>(const_cast<char16_t *>(right.data())),
      right.size(),
      nullptr /* lpReserved */,
      nullptr /*lParam*/,
      0);

  if (compareResult == CSTR_EQUAL) {
    return 0;
  } else if (compareResult == CSTR_LESS_THAN) {
    return -1;
  } else if (compareResult == CSTR_GREATER_THAN) {
    return 1;
  } else {
    llvm_unreachable("CompareStringEx returned unknown value !");
  }
}

namespace {
// Note: These are some helper functions copied from Chakracore source. Ref:
// https://github.com/chakra-core/ChakraCore/blob/master/lib/Common/Common/DateUtilities.cpp
// The day numbers for the months of a leap year.
static const int g_rgday[12] = {
    0,
    31,
    60,
    91,
    121,
    152,
    182,
    213,
    244,
    274,
    305,
    335,
};

// Decomposed date (Year-Month-Date).
struct YMD {
  int year; // year
  int yt; // year type: wkd of Jan 1 (plus 7 if a leap year).
  int mon; // month (0 to 11)
  int mday; // day in month (0 to 30)
  int yday; // day in year (0 to 365)
  int wday; // week day (0 to 6)
  int time; // time of day (in milliseconds: 0 to 86399999)

  void ToSystemTime(SYSTEMTIME *sys) {
    sys->wYear = (WORD)year;
    sys->wMonth = (WORD)(mon + 1);
    sys->wDay = (WORD)(mday + 1);
    int t = time;
    sys->wMilliseconds = (WORD)(t % 1000);
    t /= 1000;
    sys->wSecond = (WORD)(t % 60);
    t /= 60;
    sys->wMinute = (WORD)(t % 60);
    t /= 60;
    sys->wHour = (WORD)t;
  }
};

///------------------------------------------------------------------------------
/// Get the non-negative remainder.
///------------------------------------------------------------------------------
double DblModPos(double dbl, double dblDen) {
  // AssertMsg(dblDen > 0, "value not positive");
  dbl = fmod(dbl, dblDen);
  if (dbl < 0) {
    dbl += dblDen;
  }
  // AssertMsg(dbl >= 0 && dbl < dblDen, "");
  return dbl;
}

double DayFromYear(double year) {
  double day = 365 * (year -= 1970);

  if (day > 0) {
    day += ((int)((year + 1) / 4)) - ((int)((year + 69) / 100)) +
        ((int)((year + 369) / 400));
  } else {
    day += floor((year + 1) / 4) - floor((year + 69) / 100) +
        floor((year + 369) / 400);
  }
  return day;
}

int GetDayMinAndUpdateYear(int day, int &year) {
  int dayMin = (int)DayFromYear(year);
  if (day < dayMin) {
    year--;
    dayMin = (int)DayFromYear(year);
  }
  return dayMin;
}

bool FLeap(int year) {
  return (0 == (year & 3)) && (0 != (year % 100) || 0 == (year % 400));
}

///------------------------------------------------------------------------------
/// Converts the time value relative to Jan 1, 1970 into a YMD.
///
/// The year number y and day number d relative to Jan 1, 1970 satisfy the
/// inequalities:
///    floor((400*d-82)/146097) <= y <= floor((400*d+398)/146097)
/// These inequalities get us within one of the correct answer for the year.
/// We then use DayFromYear to adjust if necessary.
///------------------------------------------------------------------------------
void GetYmdFromTv(double tv, YMD *pymd) {
  //      Assert(pymd);

  int day;
  int dayMin;
  int yday;

  if (tv > 0) {
    day = (int)(tv / 86400000);
    pymd->time = (int)DblModPos(tv, 86400000);
    pymd->wday = (day + 4) % 7;

    pymd->year = 1970 + (int)((400 * (double)day + 398) / 146097);
    dayMin = GetDayMinAndUpdateYear(day, pymd->year);
    pymd->yt = (int)((dayMin + 4) % 7);

  } else {
    day = (int)floor(tv / 86400000);
    pymd->time = (int)DblModPos(tv, 86400000);
    pymd->wday = (int)DblModPos(day + 4, 7);

    pymd->year = 1970 + (int)floor(((400 * (double)day + 398) / 146097));
    dayMin = GetDayMinAndUpdateYear(day, pymd->year);
    pymd->yt = (int)DblModPos(dayMin + 4, 7);
  }
  yday = (int)(day - dayMin);
  //      Assert(yday >= 0 && (yday < 365 || yday == 365 && FLeap(pymd->year)));
  pymd->yday = yday;

  if (FLeap(pymd->year)) {
    pymd->yt += 7;
  } else if (yday >= 59) {
    yday++;
  }

  // Get the month.
  if (yday < 182) {
    if (yday < 60) {
      pymd->mon = 0 + ((yday >= 31) ? 1 : 0);
    } else if (yday < 121) {
      pymd->mon = 2 + ((yday >= 91) ? 1 : 0);
    } else {
      pymd->mon = 4 + ((yday >= 152) ? 1 : 0);
    }
  } else {
    if (yday < 244) {
      pymd->mon = 6 + ((yday >= 213) ? 1 : 0);
    } else if (yday < 305) {
      pymd->mon = 8 + ((yday >= 274) ? 1 : 0);
    } else {
      pymd->mon = 10 + ((yday >= 335) ? 1 : 0);
    }
  }
  //      Assert(pymd->mon >= 0 && pymd->mon < 12);

  pymd->mday = yday - g_rgday[pymd->mon];
}

} // namespace

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {
  YMD pymd;
  GetYmdFromTv(unixtimeMs, &pymd);

  SYSTEMTIME st;
  st.wYear = (WORD)pymd.year;
  st.wMonth = (WORD)pymd.mon + 1;
  st.wDayOfWeek = (WORD)pymd.wday;
  st.wDay = (WORD)pymd.mday + 1;
  st.wHour = (WORD)(pymd.time / 3600000);
  st.wMinute = (WORD)((pymd.time / 60000) % 60);
  st.wSecond = (WORD)((pymd.time / 1000) % 60);
  st.wMilliseconds = (WORD)(pymd.time % 60);

  LCID lcid = GetUserDefaultLCID();

  if (formatDate) {
    DWORD dwFormat = DATE_SHORTDATE;

    if ((PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_ARABIC) ||
        (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_HEBREW)) {
      dwFormat |= DATE_RTLREADING;
    }

    int sizeEstimate = GetDateFormatW(lcid, dwFormat, &st, nullptr, nullptr, 0);
    assert(sizeEstimate > 0);

    llvh::SmallVector<char16_t, 16> dateBuffer;
    dateBuffer.resize(sizeEstimate);
    int success = GetDateFormatW(
        lcid,
        dwFormat,
        &st,
        nullptr,
        reinterpret_cast<LPWSTR>(dateBuffer.data()),
        dateBuffer.size());

    buf.append(dateBuffer.begin(), dateBuffer.end());
  }

  if (formatDate && formatTime) {
    buf.emplace_back(L' ');
  }

  if (formatTime) {
    int sizeEstimate = GetTimeFormatW(lcid, 0, &st, nullptr, nullptr, 0);
    assert(sizeEstimate > 0);

    llvh::SmallVector<char16_t, 16> timeBuffer;
    timeBuffer.resize(sizeEstimate);
    int success = GetTimeFormatW(
        lcid,
        0,
        &st,
        nullptr,
        reinterpret_cast<LPWSTR>(timeBuffer.data()),
        timeBuffer.size());

    buf.append(timeBuffer.begin(), timeBuffer.end());
  }
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    bool useCurrentLocale) {
  if (buf.size() == 0)
    return;
  DWORD dwFlags =
      targetCase == CaseConversion::ToUpper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE;
  static_assert(
      sizeof(wchar_t) == sizeof(char16_t), "sizeof(wchar_t) == sizeof(UChar)");
  int sizeEstimate = LCMapStringEx(
      LOCALE_NAME_USER_DEFAULT,
      dwFlags,
      reinterpret_cast<LPCWSTR>(buf.data()),
      buf.size(),
      nullptr,
      0,
      nullptr /* lpVersionInformation */,
      nullptr /* lpReserved */,
      0 /* sortHandle */);
  if (sizeEstimate > 0) {
    llvh::SmallVector<char16_t, 64> output{};
    output.resize(sizeEstimate);
    int charsWritten = LCMapStringEx(
        LOCALE_NAME_USER_DEFAULT,
        dwFlags,
        reinterpret_cast<LPCWSTR>(buf.data()),
        buf.size(),
        reinterpret_cast<LPWSTR>(output.data()),
        output.size(),
        nullptr /* lpVersionInformation */,
        nullptr /* lpReserved */,
        0 /* sortHandle */);
    assert (charsWritten == 0);
    buf = output;
  }
}

void normalize(llvh::SmallVectorImpl<char16_t> &buf, NormalizationForm form) {
  if (!IsNormalizedString(buf.data(), buf.size(), NormalizationForm::C)) {
    uint32_t sizeOfNormalizedStringWithoutNullTerminator;

    llvh::SmallVector<char16_t, 32> normalized;
    bool success = GetNormalizedString(
        buf.data(),
        buf.size(),
        NormalizationForm::C,
        normalized,
        sizeOfNormalizedStringWithoutNullTerminator);
    assert(success && "GetNormalizedString failed !");

    if (success)
      buf = normalized;
  }
}

} // namespace platform_unicode
} // namespace hermes

#endif // HERMES_PLATFORM_UNICODE_WinGlob
