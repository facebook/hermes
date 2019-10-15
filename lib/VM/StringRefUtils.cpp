/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringRefUtils.h"
#include <clocale>

#include "hermes/Platform/Unicode/PlatformUnicode.h"
#if HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU
#include "hermes/Platform/Unicode/icu.h"
#endif
#include "hermes/Support/UTF8.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {
namespace vm {

using llvm::ConversionResult;
using llvm::UTF16;
using llvm::UTF8;

UTF16Ref createUTF16Ref(const char16_t *str) {
  return UTF16Ref(str, utf16_traits::length(str));
}

ASCIIRef createASCIIRef(const char *str) {
  return ASCIIRef(str, ascii_traits::length(str));
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, ASCIIRef asciiRef) {
  return OS << llvm::StringRef(asciiRef.data(), asciiRef.size());
}

#if HERMES_PLATFORM_UNICODE != HERMES_PLATFORM_UNICODE_ICU
/// Print the given UTF-16. We just assume UTF-8 output to avoid the increased
/// binary size of supporting multiple encodings.
llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, UTF16Ref u16ref) {
  // Note this assumes that the desired output encoding is UTF-8, which may
  // not be a valid assumption if outputting to a tty.
  std::string narrowStr;
  convertUTF16ToUTF8WithReplacements(narrowStr, u16ref);
  return OS << narrowStr;
}
#else // HERMES_PLATFORM_UNICODE == HERMES_PLATFORM_UNICODE_ICU
namespace {
/// Class to obtain and serve user's preferred output encoding.
class OutputEncoding {
  /// User's preferred output encoding.
  llvm::SmallString<16> preferredEncoding_;

 public:
  /// Read in user's preferred encoding and store it for future use.
  OutputEncoding() {
#ifdef _WINDOWS
    // On Windows, the encoding section of the value returned by
    // setlocale(LC_ALL, "") is the system locale for non-Unicode programs.
    // On an en-us installation, for example, this returns 1252,
    // which represents ISO-8859-1. That is a non-sensical choice.
    // As a result, we force encoding to UTF-8.
    char *locale = const_cast<char *>(".UTF-8");
#else
    // Set our locale to user's preferred locale by passing "" to setlocale.
    // When determining user's locale, the following environment
    // variables are considered: LC_ALL, LC_CTYPE, and LANG.
    // Even if user's locale contains an extension to the locale name
    // using the '@' sign (e.g. de_DE.iso885915@euro),
    // that extension is stripped by setlocale.
    char *locale = std::setlocale(LC_ALL, "");

    // If setlocale fails setting user's preferences, it returns null.
    if (!locale) {
      // Let's just get the default. It will likely be 'C'.
      locale = std::setlocale(LC_ALL, nullptr);
    }
#endif

    // If we were able to come up with an encoding,
    // make sure to convert it to the format ICU expects.
    if (locale && strlen(locale) > 0) {
      // At this point we likely have a string like en_US.UTF-8,
      // unless it is "C" or "POSIX".
      // Let's trim the string if we can and get the second part.
      llvm::StringRef localeStrRef(locale);
      size_t dotLoc = localeStrRef.find('.');
      if (dotLoc != llvm::StringRef::npos) {
        // This may return an empty string if targetEncoding's last character is
        // the only '.' in the string.
        localeStrRef = localeStrRef.substr(dotLoc + 1);
      }

      preferredEncoding_ = localeStrRef;
      // StringRef is not guaranteed to null-terminate. Add '\0' explicitly.
      preferredEncoding_ += '\0';
    }
  }

  /// \returns user's preferred encoding using user's locale.
  /// May return empty string if user's preferred encoding cannot be determined.
  llvm::StringRef getPreferredEncoding() const {
    return preferredEncoding_.str();
  }
};

/// \returns the default OutputEncoding object.
const OutputEncoding &getDefaultOutputEncodingObject() {
  // Use a static object to make sure we create only one such object.
  static const OutputEncoding encoding;
  return encoding;
}

/// \returns the default output encoding.
inline llvm::StringRef getDefaultOutputEncoding() {
  return getDefaultOutputEncodingObject().getPreferredEncoding();
}
} // anonymous namespace

/// Create a converter from internal encoding to user's preferred encoding.
/// Callers of this function is responsible for closing the converter
/// with a call to freeEncodingConverter.
/// \returns the converter if opening the converter succeeds; null otherwise.
static inline UConverter *createEncodingConverter(UErrorCode *converterStatus) {
  // Get the target encoding from user's environment.
  llvm::StringRef targetEncoding = getDefaultOutputEncoding();

  // Create a converter from UTF-16 to user's encoding.
  // Note that targetEncoding may be empty or it may not make sense as it
  // depends on user's environment variables. If ucnv_open fails, we will try
  // again below to get the default converter.
  *converterStatus = U_ZERO_ERROR;
  UConverter *converter = nullptr;
  if (!targetEncoding.empty()) {
    converter = ucnv_open(targetEncoding.data(), converterStatus);
  }

  // If we cannot get a converter, fall back to the default converter.
  // Note that when we test with LIT, environment variables are not preserved.
  // Therefore, we usually use the default converter while running tests.
  if (LLVM_UNLIKELY(U_FAILURE(*converterStatus) || converter == nullptr)) {
    // Let's get the default converter.
    // If we fail to get the default converter, nothing else to do.
    *converterStatus = U_ZERO_ERROR;
    converter = ucnv_open(nullptr, converterStatus);
  }
  return converter;
}

/// Closes a given converter opened by a call to createEncodingConverter.
/// Passing null to this function is a valid operation.
static inline void freeEncodingConverter(UConverter *converter) {
  if (converter) {
    ucnv_close(converter);
  }
}

/// Print given UTF-16 after converting to user's preferred encoding.
llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, UTF16Ref u16ref) {
  // Get the converter to print in user's preferred encoding.
  UErrorCode converterStatus;
  UConverter *converter = createEncodingConverter(&converterStatus);

  // Check if we did get a converter.
  if (converter == nullptr) {
    OS << u_errorName(converterStatus) << "\n";
    return OS;
  }

  // Create a buffer to hold the output string.
  char buf[128];
  // Set number of available bytes.
  size_t availOut = sizeof(buf) / sizeof(char);
  char *targetEnd = buf + availOut;

  const UChar *sourceStart = (const UChar *)u16ref.begin();
  const UChar *sourceEnd = (const UChar *)u16ref.end();

  // We have a limited buffer; we may have to loop to print everything.
  do {
    // Set up target pointers.
    char *targetStart = buf;
    // Reset the converter status so it won't get confused.
    converterStatus = U_ZERO_ERROR;
    // If we are at the end of the source buffer, force flush.
    UBool flush = (sourceStart == sourceEnd);

    // Actual conversion done by the ICU library.
    ucnv_fromUnicode(
        converter,
        &targetStart,
        targetEnd,
        &sourceStart,
        sourceEnd,
        NULL,
        flush,
        &converterStatus);

    // If we had any issues other than a buffer overflow, report!
    if (U_FAILURE(converterStatus) &&
        (converterStatus != U_BUFFER_OVERFLOW_ERROR)) {
      OS << u_errorName(converterStatus) << "\n";

      // Close the converter before returning.
      freeEncodingConverter(converter);
      return OS;
    }

    // Actual print operation of the buffer.
    OS << llvm::StringRef(buf, targetStart - buf);
  } while (converterStatus == U_BUFFER_OVERFLOW_ERROR);

  // Close the converter now that we are done with it.
  freeEncodingConverter(converter);

  return OS;
}
#endif // OUTPUT_IN_USER_ENCODING

} // namespace vm
} // namespace hermes
