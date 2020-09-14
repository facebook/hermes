//===- WindowsSupport.h - Common Windows Include File -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines things specific to Windows implementations.  In addition to
// providing some helpers for working with win32 APIs, this header wraps
// <windows.h> with some portability macros.  Always include WindowsSupport.h
// instead of including <windows.h> directly.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic Win32 code that
//===          is guaranteed to work on *all* Win32 variants.
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_WINDOWSSUPPORT_H
#define LLVM_SUPPORT_WINDOWSSUPPORT_H

// mingw-w64 tends to define it as 0x0502 in its headers.
#undef _WIN32_WINNT
#undef _WIN32_IE

// Require at least Windows 7 API.
#define _WIN32_WINNT 0x0601
#define _WIN32_IE    0x0800 // MinGW at it again. FIXME: verify if still needed.
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "llvh/ADT/SmallVector.h"
#include "llvh/ADT/StringExtras.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/ADT/Twine.h"
#include "llvh/Config/config.h" // Get build system configuration settings
#include "llvh/Support/Chrono.h"
#include "llvh/Support/Compiler.h"
#include <cassert>
#include <string>
#include <system_error>
#include <windows.h>

// Must be included after windows.h
#include <wincrypt.h>

/// Determines if the program is running on Windows 8 or newer. This
/// reimplements one of the helpers in the Windows 8.1 SDK, which are intended
/// to supercede raw calls to GetVersionEx. Old SDKs, Cygwin, and MinGW don't
/// yet have VersionHelpers.h, so we have our own helper.
inline bool RunningWindows8OrGreater() {
  // Windows 8 is version 6.2, service pack 0.
  OSVERSIONINFOEXW osvi = {};
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  osvi.dwMajorVersion = 6;
  osvi.dwMinorVersion = 2;
  osvi.wServicePackMajor = 0;

  DWORDLONG Mask = 0;
  Mask = VerSetConditionMask(Mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
  Mask = VerSetConditionMask(Mask, VER_MINORVERSION, VER_GREATER_EQUAL);
  Mask = VerSetConditionMask(Mask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

  return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION |
                                       VER_SERVICEPACKMAJOR,
                            Mask) != FALSE;
}

inline bool MakeErrMsg(std::string *ErrMsg, const std::string &prefix) {
  if (!ErrMsg)
    return true;
  char *buffer = NULL;
  DWORD LastError = GetLastError();
  DWORD R = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_MAX_WIDTH_MASK,
                           NULL, LastError, 0, (LPSTR)&buffer, 1, NULL);
  if (R)
    *ErrMsg = prefix + ": " + buffer;
  else
    *ErrMsg = prefix + ": Unknown error";
  *ErrMsg += " (0x" + llvh::utohexstr(LastError) + ")";

  LocalFree(buffer);
  return R != 0;
}

template <typename HandleTraits>
class ScopedHandle {
  typedef typename HandleTraits::handle_type handle_type;
  handle_type Handle;

  ScopedHandle(const ScopedHandle &other); // = delete;
  void operator=(const ScopedHandle &other); // = delete;
public:
  ScopedHandle()
    : Handle(HandleTraits::GetInvalid()) {}

  explicit ScopedHandle(handle_type h)
    : Handle(h) {}

  ~ScopedHandle() {
    if (HandleTraits::IsValid(Handle))
      HandleTraits::Close(Handle);
  }

  handle_type take() {
    handle_type t = Handle;
    Handle = HandleTraits::GetInvalid();
    return t;
  }

  ScopedHandle &operator=(handle_type h) {
    if (HandleTraits::IsValid(Handle))
      HandleTraits::Close(Handle);
    Handle = h;
    return *this;
  }

  // True if Handle is valid.
  explicit operator bool() const {
    return HandleTraits::IsValid(Handle) ? true : false;
  }

  operator handle_type() const {
    return Handle;
  }
};

struct CommonHandleTraits {
  typedef HANDLE handle_type;

  static handle_type GetInvalid() {
    return INVALID_HANDLE_VALUE;
  }

  static void Close(handle_type h) {
    ::CloseHandle(h);
  }

  static bool IsValid(handle_type h) {
    return h != GetInvalid();
  }
};

struct JobHandleTraits : CommonHandleTraits {
  static handle_type GetInvalid() {
    return NULL;
  }
};

struct CryptContextTraits : CommonHandleTraits {
  typedef HCRYPTPROV handle_type;

  static handle_type GetInvalid() {
    return 0;
  }

  static void Close(handle_type h) {
    ::CryptReleaseContext(h, 0);
  }

  static bool IsValid(handle_type h) {
    return h != GetInvalid();
  }
};

struct RegTraits : CommonHandleTraits {
  typedef HKEY handle_type;

  static handle_type GetInvalid() {
    return NULL;
  }

  static void Close(handle_type h) {
    ::RegCloseKey(h);
  }

  static bool IsValid(handle_type h) {
    return h != GetInvalid();
  }
};

struct FindHandleTraits : CommonHandleTraits {
  static void Close(handle_type h) {
    ::FindClose(h);
  }
};

struct FileHandleTraits : CommonHandleTraits {};

typedef ScopedHandle<CommonHandleTraits> ScopedCommonHandle;
typedef ScopedHandle<FileHandleTraits>   ScopedFileHandle;
typedef ScopedHandle<CryptContextTraits> ScopedCryptContext;
typedef ScopedHandle<RegTraits>          ScopedRegHandle;
typedef ScopedHandle<FindHandleTraits>   ScopedFindHandle;
typedef ScopedHandle<JobHandleTraits>    ScopedJobHandle;

namespace llvh {
template <class T>
class SmallVectorImpl;

template <class T>
typename SmallVectorImpl<T>::const_pointer
c_str(SmallVectorImpl<T> &str) {
  str.push_back(0);
  str.pop_back();
  return str.data();
}

namespace sys {

inline std::chrono::nanoseconds toDuration(FILETIME Time) {
  ULARGE_INTEGER TimeInteger;
  TimeInteger.LowPart = Time.dwLowDateTime;
  TimeInteger.HighPart = Time.dwHighDateTime;

  // FILETIME's are # of 100 nanosecond ticks (1/10th of a microsecond)
  return std::chrono::nanoseconds(100 * TimeInteger.QuadPart);
}

inline TimePoint<> toTimePoint(FILETIME Time) {
  ULARGE_INTEGER TimeInteger;
  TimeInteger.LowPart = Time.dwLowDateTime;
  TimeInteger.HighPart = Time.dwHighDateTime;

  // Adjust for different epoch
  TimeInteger.QuadPart -= 11644473600ll * 10000000;

  // FILETIME's are # of 100 nanosecond ticks (1/10th of a microsecond)
  return TimePoint<>(std::chrono::nanoseconds(100 * TimeInteger.QuadPart));
}

inline FILETIME toFILETIME(TimePoint<> TP) {
  ULARGE_INTEGER TimeInteger;
  TimeInteger.QuadPart = TP.time_since_epoch().count() / 100;
  TimeInteger.QuadPart += 11644473600ll * 10000000;

  FILETIME Time;
  Time.dwLowDateTime = TimeInteger.LowPart;
  Time.dwHighDateTime = TimeInteger.HighPart;
  return Time;
}

namespace windows {
// Returns command line arguments. Unlike arguments given to main(),
// this function guarantees that the returned arguments are encoded in
// UTF-8 regardless of the current code page setting.
std::error_code GetCommandLineArguments(SmallVectorImpl<const char *> &Args,
                                        BumpPtrAllocator &Alloc);
} // end namespace windows
} // end namespace sys
} // end namespace llvh.

#endif
