//===-- llvm/Support/TarWriter.h - Tar archive file creator -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_TAR_WRITER_H
#define LLVM_SUPPORT_TAR_WRITER_H

#include "llvh/ADT/StringRef.h"
#include "llvh/ADT/StringSet.h"
#include "llvh/Support/Error.h"
#include "llvh/Support/raw_ostream.h"

namespace llvh {
class TarWriter {
public:
  static Expected<std::unique_ptr<TarWriter>> create(StringRef OutputPath,
                                                     StringRef BaseDir);

  void append(StringRef Path, StringRef Data);

private:
  TarWriter(int FD, StringRef BaseDir);
  raw_fd_ostream OS;
  std::string BaseDir;
  StringSet<> Files;
};
}

#endif
