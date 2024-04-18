/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_OUTPUTSTREAM_H
#define HERMES_SUPPORT_OUTPUTSTREAM_H

#include "llvh/Support/raw_ostream.h"

namespace hermes {

/// Manage an output file safely.
class OutputStream {
 public:
  /// Creates an empty object.
  OutputStream() : os_(nullptr) {}
  /// Create an object which initially holds the \p defaultStream.
  OutputStream(llvh::raw_ostream &defaultStream) : os_(&defaultStream) {}

  ~OutputStream() {
    discard();
  }

  /// Replaces the stream with an open stream to a temporary file
  /// named based on \p fileName.  This method will write error
  /// messages, if any, to llvh::errs().  This method can only be
  /// called once on an object.  \return true if the temp file was
  /// created and false otherwise.  If the object is destroyed without
  /// close() being called, the temp file is removed.
  bool open(llvh::Twine fileName, llvh::sys::fs::OpenFlags openFlags) {
    assert(!fdos_ && "OutputStream::open() can be called only once.");

    // Newer versions of llvm have a safe createUniqueFile overload
    // which takes OpenFlags.  Hermes's llvm doesn't, so we have to do
    // it this way, which is a hypothetical race.
    std::error_code EC = llvh::sys::fs::getPotentiallyUniqueFileName(
        fileName + ".%%%%%%", tempName_);
    if (EC) {
      llvh::errs() << "Failed to get temp file for " << fileName << ": "
                   << EC.message() << '\n';
      return false;
    }

    fdos_ = std::make_unique<llvh::raw_fd_ostream>(tempName_, EC, openFlags);
    if (EC) {
      llvh::errs() << "Failed to open file " << tempName_ << ": "
                   << EC.message() << '\n';
      fdos_.reset();
      return false;
    }
    os_ = fdos_.get();
    fileName_ = fileName.str();
    return true;
  }

  /// If a temporary file was created, it is renamed to \p fileName.
  /// If renaming fails, it will be deleted.  This method will write
  /// error messages, if any, to llvh::errs().  \return true if a temp
  /// file was never created or was renamed here; or false otherwise.
  bool close() {
    if (!fdos_) {
      return true;
    }
    fdos_->close();
    fdos_.reset();
    std::error_code EC = llvh::sys::fs::rename(tempName_, fileName_);
    if (EC) {
      llvh::errs() << "Failed to write file " << fileName_ << ": "
                   << EC.message() << '\n';
      llvh::sys::fs::remove(tempName_);
      return false;
    }
    return true;
  }

  /// If a temporary file was created, it is deleted.
  void discard() {
    if (!fdos_) {
      return;
    }

    fdos_->close();
    fdos_.reset();
    llvh::sys::fs::remove(tempName_);
  }

  llvh::raw_ostream &os() {
    assert(os_ && "OutputStream never initialized");
    return *os_;
  }

 private:
  llvh::raw_ostream *os_;
  llvh::SmallString<32> tempName_;
  std::unique_ptr<llvh::raw_fd_ostream> fdos_;
  std::string fileName_;
};

} // namespace hermes

#endif // HERMES_SUPPORT_OUTPUTSTREAM_H
