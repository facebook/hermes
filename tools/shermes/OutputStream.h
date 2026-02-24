#ifndef SHERMES_OUTPUTSTREAM_H
#define SHERMES_OUTPUTSTREAM_H

#include "llvh/Support/FileSystem.h"
#include "llvh/Support/Signals.h"
#include "llvh/Support/raw_ostream.h"

/// Manage an output file safely by writing to a temporary file and moving it
/// on success.
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
  ///
  /// If however the fileName is "-", STDOUT is used directly, without going
  /// through a temporary file. That is an acceptable strategy when debugging
  /// or when piping, since an error would terminate the entire pipeline.
  bool open(llvh::StringRef fileName, llvh::sys::fs::OpenFlags openFlags) {
    assert(!fdos_ && "OutputStream::open() can be called only once.");

    if (fileName == "-") {
      os_ = &llvh::outs();
      return true;
    }

    // Newer versions of llvm have a safe createUniqueFile overload
    // which takes OpenFlags.  Hermes's llvm doesn't, so we have to do
    // it this way, which is a hypothetical race.
    std::error_code EC = llvh::sys::fs::getPotentiallyUniqueFileName(
        llvh::Twine(fileName) + ".%%%%%%", tempName_);
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
    llvh::sys::RemoveFileOnSignal(tempName_);
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
    llvh::sys::DontRemoveFileOnSignal(tempName_);
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
    llvh::sys::DontRemoveFileOnSignal(tempName_);
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

#endif // SHERMES_OUTPUTSTREAM_H
