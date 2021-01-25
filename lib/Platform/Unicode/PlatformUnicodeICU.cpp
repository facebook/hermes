/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

bool IsNormalizedString(const UChar *str, uint32_t length, NORM_FORM form) {
  static_assert(
      sizeof(wchar_t) == sizeof(UChar), "sizeof(wchar_t) == sizeof(UChar)");
  return ::IsNormalizedString(
      NormalizationC, reinterpret_cast<const wchar_t *>(str), length);
}

llvh::SmallVector<char16_t,16> GetNormalizedString(
    const UChar *str,
    uint32_t length,
    NORM_FORM form,
    uint32_t &sizeOfNormalizedStringWithoutNullTerminator) {
  
  // IMPORTANT: Implementation Notes
  // Normalize string estimates the required size of the buffer based on
  // averages and other data. It is very hard to get a precise size from an
  // input string without expanding/contracting it on the buffer. It is
  // estimated that the maximum size the string after an NFC is 6x the input
  // length, and 18x for NFD. This approach isn't very feasible as well. The
  // approach taken is based on the simple example in the MSDN article.
  //  - Loop until the return value is either an error (apart from insufficient
  //  buffer size), or success.
  //  - Each time recreate a temporary buffer based on the last guess.
  //  - When creating the JS string, use the positive return value and copy the
  //  buffer across.
  // Design choice for "guesses" comes from data Windows collected; and in most
  // cases the loop will not iterate more than 2 times.

  /*Assert(
      !UnicodeText::IsNormalizedString(form, this->GetSz(), this->GetLength()));*/

  // Get the first size estimate
  // UnicodeText::ApiError error;

  static_assert(sizeof(wchar_t) == sizeof(UChar), "sizeof(wchar_t) == sizeof(UChar)");
  int sizeEstimate = NormalizeString(
        NormalizationC,
        reinterpret_cast<const wchar_t *>(str),
        length,
        NULL,
        0);

  // LPWSTR strResult = NULL;
  llvh::SmallVector<char16_t, 16> output;
  for (int i = 0; i < 10; i++) {

    output.resize(sizeEstimate);

    sizeEstimate = NormalizeString(
        NormalizationC,
        reinterpret_cast<const wchar_t *>(str),
        length,
        reinterpret_cast<wchar_t *>(output.data()),
        sizeEstimate);

    if (sizeEstimate > 0) {
      output.resize(sizeEstimate);
      return output;
    }

    if (sizeEstimate <= 0) {
      DWORD dwError = GetLastError();
      if (dwError != ERROR_INSUFFICIENT_BUFFER)
        break; // Real error, not buffer error

      // New guess is negative of the return value.
      sizeEstimate = -1 * sizeEstimate;
    }
  }

  //if (sizeEstimate > 0)

  ///*int32 sizeEstimate = NormalizeString(
  //    form, this->GetSz(), this->GetLength() + 1, nullptr, 0, &error);*/

  //// char16 *tmpBuffer = nullptr;
  //// Loop while the size estimate is bigger than 0
  //while (error == UnicodeText::ApiError::InsufficientBuffer) {
  //  tmpBuffer = AnewArray(tempAllocator, char16, sizeEstimate);
  //  sizeEstimate = UnicodeText::NormalizeString(
  //      form,
  //      this->GetSz(),
  //      this->GetLength() + 1,
  //      tmpBuffer,
  //      sizeEstimate,
  //      &error);

  //  // Success, sizeEstimate is the exact size including the null terminator
  //  if (sizeEstimate > 0) {
  //    sizeOfNormalizedStringWithoutNullTerminator = sizeEstimate - 1;
  //    return tmpBuffer;
  //  }

  //  // Anything less than 0, we have an error, flip sizeEstimate now. As both
  //  // times we need to use it, we need positive anyways.
  //  sizeEstimate *= -1;
  //}

  //switch (error) {
  //  case UnicodeText::ApiError::InvalidParameter:
  //    // some invalid parameter, coding error
  //    AssertMsg(
  //        false, "Invalid Parameter- check pointers passed to NormalizeString");
  //    JavascriptError::ThrowRangeError(scriptContext, JSERR_FailedToNormalize);
  //    break;
  //  case UnicodeText::ApiError::InvalidUnicodeText:
  //    // the value returned is the negative index of an invalid unicode
  //    // character
  //    JavascriptError::ThrowRangeErrorVar(
  //        scriptContext, JSERR_InvalidUnicodeCharacter, sizeEstimate);
  //    break;
  //  case UnicodeText::ApiError::NoError:
  //    // The actual size of the output string is zero.
  //    // Theoretically only empty input string should produce this, which is
  //    // handled above, thus the code path should not be hit.
  //    AssertMsg(
  //        false,
  //        "This code path should not be hit, empty string case is handled above. Perhaps a false error (sizeEstimate <= 0; but lastError == 0; ERROR_SUCCESS and NO_ERRROR == 0)");
  //    sizeOfNormalizedStringWithoutNullTerminator = 0;
  //    return nullptr; // scriptContext->GetLibrary()->GetEmptyString();
  //    break;
  //  default:
  //    AssertMsg(false, "Unknown error");
  //    JavascriptError::ThrowRangeError(scriptContext, JSERR_FailedToNormalize);
  //    break;
  //}
}

int localeCompare(
    llvh::ArrayRef<char16_t> left,
    llvh::ArrayRef<char16_t> right) {


    UChar *data1 ;
    int sz1;
    if (!IsNormalizedString(left.data(), left.size(), NormalizationC)) {
        uint32_t sizeOfNormalizedStringWithoutNullTerminator;

        llvh::SmallVector<char16_t, 16> normalized = GetNormalizedString(
            left.data(),
            left.size(),
            NormalizationC,
            sizeOfNormalizedStringWithoutNullTerminator);

        data1 = normalized.data();
        sz1 = normalized.size();
    } else {
      data1 = const_cast<UChar *> (reinterpret_cast<const UChar *>(
          left.data()));
      sz1 = left.size();
    }

    UChar *data2;
    int sz2;
    if (!IsNormalizedString(right.data(), right.size(), NormalizationC)) {
      uint32_t sizeOfNormalizedStringWithoutNullTerminator;

      llvh::SmallVector<char16_t, 16> normalized = GetNormalizedString(
          right.data(),
          right.size(),
          NormalizationC,
          sizeOfNormalizedStringWithoutNullTerminator);

      data2 = normalized.data();
      sz2 = normalized.size();
    } else {
      data2 = const_cast<UChar *> (reinterpret_cast<const UChar *>(right.data()));
      sz2 = right.size();
    }
  
    int compareResult = CompareStringEx(
        L"en-US", 0, reinterpret_cast<LPWCH>(data1), sz1, reinterpret_cast<LPWCH>(data2), sz2, NULL, NULL, 0);
    ;
  
    if (compareResult == CSTR_EQUAL) {
      return 0;
    } else if (compareResult == CSTR_LESS_THAN) {
      return -1;
    } else if (compareResult == CSTR_EQUAL) {
      return 1;
    } else {
      DWORD dwError = GetLastError();
      std::abort();
    }

    // int iSizeEstimated = NormalizeString(NormalizationC, L"", -1, NULL, 0);

    /*

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
  llvm_unreachable("Invalid result from ucol_strcoll"); */
}

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


//int GetDayMinAndUpdateYear(int day, int &year) {
//  int dayMin = (int)DayFromYear(year);
//  if (day < dayMin) {
//    year--;
//    dayMin = (int)DayFromYear(year);
//  }
//  return dayMin;
//}

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

void dateFormat(
    double unixtimeMs,
    bool formatDate,
    bool formatTime,
    llvh::SmallVectorImpl<char16_t> &buf) {

    YMD pymd;
  GetYmdFromTv(unixtimeMs, &pymd);

  SYSTEMTIME st;
  // the caller of this function should ensure that the range of pymd->year is
  // such that the following conversion works.
  st.wYear = (WORD)pymd.year;
  st.wMonth = (WORD)pymd.mon + 1;
  st.wDayOfWeek = (WORD)pymd.wday;
  st.wDay = (WORD)pymd.mday + 1;
  st.wHour = (WORD)(pymd.time / 3600000);
  st.wMinute = (WORD)((pymd.time / 60000) % 60);
  st.wSecond = (WORD)((pymd.time / 1000) % 60);
  st.wMilliseconds = (WORD)(pymd.time % 60);

  llvh::SmallVector<char16_t, 64> destformat{};
  destformat.resize(128);

  llvh::SmallVector<char16_t, 64> deststr{};
  deststr.resize(128);

  LCID lcid = GetUserDefaultLCID();
  DWORD dwFormat = DATE_LONGDATE;
  int c = GetDateFormatW(
      lcid,
      dwFormat,
      &st,
      NULL,
      (wchar_t *)deststr.data(), deststr.size());

  UChar *res = deststr.data();
  buf = deststr;
  

  return;


  //UDateFormatStyle dateStyle = formatDate ? UDAT_MEDIUM : UDAT_NONE;
  //UDateFormatStyle timeStyle = formatTime ? UDAT_MEDIUM : UDAT_NONE;

  //UErrorCode err{U_ZERO_ERROR};

  //llvh::SmallVector<char16_t, 32> tzstr;

  //// We have to use this roundabout method to get the timezone name because
  //// tzname cannot be concurrently accessed. Specifically, we have previously
  //// observed cases where calls to tzset (and functions that call tzset, such as
  //// localtime), may set elements of tzname to null. In this case, any thread
  //// directly reading tzname at the same time may read the null value, causing
  //// a segfault.
  //char tz[50];
  //time_t t;
  //struct tm *timeInfo;
  //timeInfo = localtime(&t);
  //auto numChars = strftime(tz, sizeof(tz), "%Z", timeInfo);
  //(void)numChars;
  //assert(numChars < sizeof(tz) && "Chars written must fit within buffer");

  //tzstr.append(tz, tz + strlen(tz));

  //auto *df = udat_open(
  //    timeStyle,
  //    dateStyle,
  //    uloc_getDefault(),
  //    (const UChar *)tzstr.data(),
  //    tzstr.size(),
  //    nullptr,
  //    0,
  //    &err);
  //if (!df) {
  //  return;
  //}

  //const int INITIAL_SIZE = 128;
  //buf.resize(INITIAL_SIZE);
  //err = U_ZERO_ERROR;
  //int length =
  //    udat_format(df, unixtimeMs, (UChar *)buf.begin(), INITIAL_SIZE, 0, &err);
  //if (length > INITIAL_SIZE) {
  //  buf.resize(length + 1);
  //  err = U_ZERO_ERROR;
  //  udat_format(df, unixtimeMs, (UChar *)buf.begin(), length, 0, &err);
  //  buf.resize(length);
  //} else {
  //  buf.resize(length);
  //}

  //udat_close(df);
}

void convertToCase(
    llvh::SmallVectorImpl<char16_t> &buf,
    CaseConversion targetCase,
    bool useCurrentLocale) {

    DWORD dwFlags =
      targetCase == CaseConversion::ToUpper ? LCMAP_UPPERCASE : LCMAP_LOWERCASE;

    const wchar_t *src = (const wchar_t *)buf.data();
    auto srcLen = buf.size();

    llvh::SmallVector<char16_t, 64> destv{};
    destv.resize(srcLen);

    wchar_t *dest = (wchar_t *)destv.data();
    auto destLen = destv.size();

      static_assert(
      sizeof(wchar_t) == sizeof(UChar), "sizeof(wchar_t) == sizeof(UChar)");
  int sizeEstimate = LCMapStringEx(
      LOCALE_NAME_USER_DEFAULT,
      dwFlags,
          src,
          srcLen,
      dest,
          destLen,
      nullptr,
      nullptr,
      0);

  if (sizeEstimate == 0) {
    DWORD dwError = GetLastError();
    if (dwError != ERROR_INSUFFICIENT_BUFFER)
      std::abort(); // TODO - MEssage the error back.

    if (dwError == ERROR_INSUFFICIENT_BUFFER)
      std::abort(); // TODO - Retry
  }

  buf = destv;

  // LPWSTR strResult = NULL;
  //llvh::SmallVector<char16_t, 16> output;
  //for (int i = 0; i < 10; i++) {
  //  output.resize(sizeEstimate);

  //  sizeEstimate = NormalizeString(
  //      NormalizationC,
  //      reinterpret_cast<const wchar_t *>(str),
  //      length,
  //      reinterpret_cast<wchar_t *>(output.data()),
  //      sizeEstimate);

  //  if (sizeEstimate > 0) {
  //    output.resize(sizeEstimate);
  //    return output;
  //  }

  //  if (sizeEstimate <= 0) {
  //    DWORD dwError = GetLastError();
  //    if (dwError != ERROR_INSUFFICIENT_BUFFER)
  //      break; // Real error, not buffer error

  //    // New guess is negative of the return value.
  //    sizeEstimate = -1 * sizeEstimate;
  //  }
  //}




  // To be able to call the converters, we have to get a pointer.
  // UChar is 16 bits, so a cast works.
  //const UChar *src = (const UChar *)buf.data();
  //auto srcLen = buf.size();

  //auto converter =
  //    targetCase == CaseConversion::ToUpper ? u_strToUpper : u_strToLower;
  //const char *locale = useCurrentLocale ? uloc_getDefault() : "";

  //// First, try to uppercase without changing the length.
  //// This will likely work.
  //llvh::SmallVector<char16_t, 64> dest{};
  //dest.resize(srcLen);
  //UErrorCode err = U_ZERO_ERROR;
  //size_t resultLen =
  //    converter((UChar *)dest.begin(), srcLen, src, srcLen, locale, &err);
  //dest.resize(resultLen);

  //// In the rare case our string was too small, rerun it.
  //if (LLVM_UNLIKELY(resultLen > srcLen)) {
  //  err = U_ZERO_ERROR;
  //  converter((UChar *)dest.begin(), resultLen, src, srcLen, locale, &err);
  //}
  //// Assign to the inout parameter.
  //buf = dest;
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
