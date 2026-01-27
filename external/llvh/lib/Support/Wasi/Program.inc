//===- llvm/Support/Unix/Program.cpp -----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Unix specific portion of the Program class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on *all* UNIX variants.
//===----------------------------------------------------------------------===//

#include "Unix.h"
#include "llvh/ADT/StringExtras.h"
#include "llvh/Config/config.h"
#include "llvh/Support/Compiler.h"
#include "llvh/Support/Errc.h"
#include "llvh/Support/FileSystem.h"
#include "llvh/Support/Path.h"
#include "llvh/Support/StringSaver.h"
#include "llvh/Support/raw_ostream.h"
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#ifdef HAVE_POSIX_SPAWN
#include <spawn.h>

#if defined(__APPLE__) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
#define USE_NSGETENVIRON 1
#else
#define USE_NSGETENVIRON 0
#endif

#if !USE_NSGETENVIRON
  extern char **environ;
#else
#include <crt_externs.h> // _NSGetEnviron
#endif
#endif

namespace llvh {

using namespace sys;

ProcessInfo::ProcessInfo() : Pid(0), ReturnCode(0) {}

ErrorOr<std::string> sys::findProgramByName(StringRef Name,
                                            ArrayRef<StringRef> Paths) {
  assert(!Name.empty() && "Must have a name!");
  // Use the given path verbatim if it contains any slashes; this matches
  // the behavior of sh(1) and friends.
  if (Name.find('/') != StringRef::npos)
    return std::string(Name);

  SmallVector<StringRef, 16> EnvironmentPaths;
  if (Paths.empty())
    if (const char *PathEnv = std::getenv("PATH")) {
      SplitString(PathEnv, EnvironmentPaths, ":");
      Paths = EnvironmentPaths;
    }

  for (auto Path : Paths) {
    if (Path.empty())
      continue;

    // Check to see if this first directory contains the executable...
    SmallString<128> FilePath(Path);
    sys::path::append(FilePath, Name);
    if (sys::fs::can_execute(FilePath.c_str()))
      return std::string(FilePath.str()); // Found the executable!
  }
  return errc::no_such_file_or_directory;
}

#ifdef HAVE_POSIX_SPAWN
static bool RedirectIO_PS(const std::string *Path, int FD, std::string *ErrMsg,
                          posix_spawn_file_actions_t *FileActions) {
  if (!Path) // Noop
    return false;
  const char *File;
  if (Path->empty())
    // Redirect empty paths to /dev/null
    File = "/dev/null";
  else
    File = Path->c_str();

  if (int Err = posix_spawn_file_actions_addopen(
          FileActions, FD, File,
          FD == 0 ? O_RDONLY : O_WRONLY | O_CREAT, 0666))
    return MakeErrMsg(ErrMsg, "Cannot dup2", Err);
  return false;
}
#endif

}

static bool Execute(ProcessInfo &PI, StringRef Program,
                    ArrayRef<StringRef> Args, Optional<ArrayRef<StringRef>> Env,
                    ArrayRef<Optional<StringRef>> Redirects,
                    unsigned MemoryLimit, std::string *ErrMsg) {
  if (ErrMsg)
    *ErrMsg = std::string("Execute() is not supported on Wasi");
  return false;
}

namespace llvh {

ProcessInfo sys::Wait(const ProcessInfo &PI, unsigned SecondsToWait,
                      bool WaitUntilTerminates, std::string *ErrMsg) {
  if (ErrMsg)
    *ErrMsg = std::string("Wait() is not supported on Wasi");
  return {};
}

std::error_code sys::ChangeStdinToBinary() {
  // Do nothing, as Unix doesn't differentiate between text and binary.
  return std::error_code();
}

std::error_code sys::ChangeStdoutToBinary() {
  // Do nothing, as Unix doesn't differentiate between text and binary.
  return std::error_code();
}

std::error_code
llvh::sys::writeFileWithEncoding(StringRef FileName, StringRef Contents,
                                 WindowsEncodingMethod Encoding /*unused*/) {
  std::error_code EC;
  llvh::raw_fd_ostream OS(FileName, EC, llvh::sys::fs::OpenFlags::F_Text);

  if (EC)
    return EC;

  OS << Contents;

  if (OS.has_error())
    return make_error_code(errc::io_error);

  return EC;
}

bool llvh::sys::commandLineFitsWithinSystemLimits(StringRef Program,
                                                  ArrayRef<StringRef> Args) {
  static long ArgMax = sysconf(_SC_ARG_MAX);
  // POSIX requires that _POSIX_ARG_MAX is 4096, which is the lowest possible
  // value for ARG_MAX on a POSIX compliant system.
  static long ArgMin = _POSIX_ARG_MAX;

  // This the same baseline used by xargs.
  long EffectiveArgMax = 128 * 1024;

  if (EffectiveArgMax > ArgMax)
    EffectiveArgMax = ArgMax;
  else if (EffectiveArgMax < ArgMin)
    EffectiveArgMax = ArgMin;

  // System says no practical limit.
  if (ArgMax == -1)
    return true;

  // Conservatively account for space required by environment variables.
  long HalfArgMax = EffectiveArgMax / 2;

  size_t ArgLength = Program.size() + 1;
  for (StringRef Arg : Args) {
    // Ensure that we do not exceed the MAX_ARG_STRLEN constant on Linux, which
    // does not have a constant unlike what the man pages would have you
    // believe. Since this limit is pretty high, perform the check
    // unconditionally rather than trying to be aggressive and limiting it to
    // Linux only.
    if (Arg.size() >= (32 * 4096))
      return false;

    ArgLength += Arg.size() + 1;
    if (ArgLength > size_t(HalfArgMax)) {
      return false;
    }
  }

  return true;
}
}
