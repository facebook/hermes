/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#ifndef HERMES_SUPPORT_SOURCEERRORMANAGER_H
#define HERMES_SUPPORT_SOURCEERRORMANAGER_H

#include "hermes/Support/Warning.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Support/SourceMgr.h"

#include <cassert>
#include <string>

namespace hermes {

using llvm::DenseSet;
using llvm::SMLoc;
using llvm::SMRange;
using llvm::Twine;

/// Forward declaration of the private implementation of the source location
/// cache.
class SourceLocationCache;

/// Options for outputting errors
struct SourceErrorOutputOptions {
  /// Determine whether errors should be colorized
  bool showColors = true;

  /// Soft limit on how wide errors should be
  /// The system will attempt to constrain errors to this width (but it is not
  /// guaranteed).
  size_t preferredMaxErrorWidth = UnlimitedWidth;

  /// Convenience value indicating unlimited width
  static constexpr size_t UnlimitedWidth = std::numeric_limits<size_t>::max();

  /// Width of a tab
  static constexpr size_t TabStop = 8;

  /// When showing an error for a range of text, the minimum amount of context
  /// (as a count of source code characters) around that range
  static constexpr size_t MinimumSourceContext = 16;
};

/// A facade around llvm::SourceMgr which simplifies error output and counts the
/// errors.
class SourceErrorManager {
 public:
  enum DiagKind {
    DK_Error = llvm::SourceMgr::DK_Error,
    DK_Warning = llvm::SourceMgr::DK_Warning,
    DK_Note = llvm::SourceMgr::DK_Note,
  };

  /// Encapsulate a buffer id, line number and column number, which uniquely
  /// identifies a location in the source. This is the "decoded" form of SMLoc.
  class SourceCoords {
   public:
    unsigned bufId = 0;
    unsigned line = 0;
    unsigned col = 0;

    SourceCoords() = default;
    SourceCoords(unsigned bufId, unsigned line, unsigned col)
        : bufId(bufId), line(line), col(col) {}

    bool isValid() const {
      return bufId != 0;
    }

    bool isSameSourceLineAs(const SourceCoords &c) const {
      return isValid() && bufId == c.bufId && line == c.line;
    }

    bool less(const SourceCoords &o) const {
      if (bufId != o.bufId)
        return bufId < o.bufId;
      if (line != o.line)
        return line < o.line;
      return col < o.col;
    }
  };

  struct ICoordTranslator {
    virtual ~ICoordTranslator() = 0;
    virtual void translate(SourceCoords &coords) = 0;
  };

  using DiagHandlerTy = llvm::SourceMgr::DiagHandlerTy;

 private:
  llvm::SourceMgr sm_{};
  SourceErrorOutputOptions outputOptions_;
  std::shared_ptr<ICoordTranslator> translator_{};

  static constexpr unsigned kMessageCountSize = 4;

  unsigned messageCount_[kMessageCountSize]{0, 0, 0, 0};

  /// Supress errors after this has been reached
  unsigned errorLimit_ = UINT_MAX;

  /// Set to true once the error limit has been reached.
  bool errorLimitReached_ = false;

  std::shared_ptr<SourceLocationCache> cache_;

  /// Mapping from warnings to true if enabled, false if disabled.
  /// All warnings default to enabled, and the numerical value of the Warning
  /// enum is the index in this vector.
  llvm::SmallBitVector warningStatuses_;

  /// If set, warnings are treated as errors.
  bool warningsAreErrors_{false};

  /// If set, all messages are ignored.
  bool suppressMessages_{false};

  /// Map of bufId to source mapping URLs.
  /// If an entry doesn't exist, then there is no source mapping URL.
  llvm::DenseMap<uint32_t, std::string> sourceMappingUrls_{};

  /// Map of bufId to user-specified source URLs.
  /// If an entry doesn't exist, then there is no user-specified source URL.
  llvm::DenseMap<uint32_t, std::string> sourceUrls_{};

  /// If larger than zero, messages are buffered and not immediately displayed.
  /// They will be displayed once the counter falls back to zero.
  unsigned bufferingEnabled_{0};

  /// An instance of a buffered message, which will be printed later.
  struct BufferedMessage {
    DiagKind dk;
    SMLoc loc;
    SMRange sm;
    std::string msg;
    SourceCoords coords;

    BufferedMessage(
        DiagKind dk,
        SMLoc loc,
        SMRange sm,
        std::string &&msg,
        const SourceCoords &coords)
        : dk(dk), loc(loc), sm(sm), msg(std::move(msg)), coords(coords) {}
  };

  /// All buffered messages. This is empty if \c bufferingEnabled_ is zero.
  std::vector<BufferedMessage> bufferedMessages_{};

  /// Diagnostic printer appropriate for setting via SourceMgr.setDiagHandler
  static void printDiagnostic(const llvm::SMDiagnostic &, void *ctx);

  friend class SaveAndSuppressMessages;
  friend class SaveAndBufferMessages;

  bool isWarningEnabled(Warning warning) {
    return warningStatuses_.test((unsigned)warning);
  }

 public:
  SourceErrorManager();

  /// Increment the "buffering enabled" counter. If the counter is larger than
  /// zero, buffering is enabled. In that mode messages are not printed
  /// immediately but are remembered instead. Once the buffer counter is
  /// decreased back to zero, all buffered messages are sorted by their source
  /// coordinates and printed.
  void enableBuffering();

  /// Decrement the "buffering enabled" counter. Once the buffer counter is
  /// decreased back to zero, all buffered messages are sorted by their source
  /// coordinates and printed.
  void disableBuffering();

  /// Set the error limit - the maximum number of errors after which to abort
  /// compilation. Zero indicates no limit.
  void setErrorLimit(unsigned errorLimit) {
    errorLimit_ = errorLimit == 0 ? UINT_MAX : errorLimit;
  }
  /// \return the error limit, or 0 if no limit is set.
  unsigned int getErrorLimit() const {
    return errorLimit_ == UINT_MAX ? 0 : errorLimit_;
  }

  /// \return true if the error limit has been reached.
  bool isErrorLimitReached() const {
    return errorLimitReached_;
  }

  /// Clear the "error limit reached" flag and the error message count.
  void clearErrorLimitReached() {
    messageCount_[DK_Error] = 0;
    errorLimitReached_ = false;
  }

  bool getWarningsAreErrors() const {
    return warningsAreErrors_;
  }

  void setWarningsAreErrors(bool warningsAreErrors) {
    warningsAreErrors_ = warningsAreErrors;
  }

  void disableAllWarnings() {
    warningStatuses_.reset();
  }

  void setWarningStatus(Warning warning, bool enabled) {
    enabled ? warningStatuses_.set((unsigned)warning)
            : warningStatuses_.reset((unsigned)warning);
  }

  void setOutputOptions(SourceErrorOutputOptions opts) {
    outputOptions_ = opts;
  }

  /// Specify a diagnostic handler to be invoked every time PrintMessage is
  /// called. \p ctx is passed into the handler when it is invoked.
  void setDiagHandler(DiagHandlerTy DH, void *ctx = nullptr) {
    sm_.setDiagHandler(DH, ctx);
  }

  /// \return the current diag handler.
  DiagHandlerTy getDiagHandler() const {
    return sm_.getDiagHandler();
  }

  /// \return the current diag handler context.
  void *getDiagContext() const {
    return sm_.getDiagContext();
  }

  const std::shared_ptr<ICoordTranslator> &getTranslator() const {
    return translator_;
  }
  void setTranslator(const std::shared_ptr<ICoordTranslator> &translator) {
    translator_ = translator;
  }

  /// Add a new source buffer to this source manager. This takes ownership of
  /// the memory buffer.
  /// \return the ID of the newly added buffer.
  uint32_t addNewSourceBuffer(std::unique_ptr<llvm::MemoryBuffer> f) {
    return sm_.AddNewSourceBuffer(std::move(f), SMLoc{});
  }

  /// Add a source buffer which maps to a file, but doesn't actually contain any
  /// source.
  /// \param bufferName the LLVM buffer name associated with the buffer. In
  ///     practice it usually contains a file path.
  uint32_t addNewVirtualSourceBuffer(llvm::StringRef bufferName);

  const llvm::MemoryBuffer *getSourceBuffer(uint32_t bufId) const {
    return sm_.getMemoryBuffer(bufId);
  }

  /// Set the source mapping URL for the buffer \p bufId.
  /// If one was already set, overwrite it.
  void setSourceMappingUrl(uint32_t bufId, llvm::StringRef url) {
    sourceMappingUrls_[bufId] = url;
  }

  /// Get the source mapping URL for file \p bufId.
  /// \return URL if it exists, else return empty string.
  llvm::StringRef getSourceMappingUrl(uint32_t bufId) const {
    const auto it = sourceMappingUrls_.find(bufId);
    if (it == sourceMappingUrls_.end()) {
      return "";
    }
    return it->second;
  }

  /// Set the user-specified source URL for the buffer \p bufId.
  /// If one was already set, overwrite it.
  void setSourceUrl(uint32_t bufId, llvm::StringRef url) {
    sourceUrls_[bufId] = url;
  }

  /// Find the bufferId, line and column of the specified location \p loc.
  /// \return true on success, false if could not be found, in which case
  ///     result.isValid() would also return false.
  bool findBufferLineAndLoc(SMLoc loc, SourceCoords &result);

  /// Find the bufferId, line and column of the specified location \p loc.
  /// Optionally perform source coordinate translation depending on
  /// \p translate.
  /// \return true on success, false if could not be found, in which case
  ///     result.isValid() would also return false.
  bool findBufferLineAndLoc(SMLoc loc, SourceCoords &result, bool translate);

  /// Given a \p loc, return the buffer that the location is in.
  /// Returns nullptr if the buffer is not found.
  const llvm::MemoryBuffer *findBufferForLoc(SMLoc loc) const;

  /// Find the SMLoc corresponding to the supplied source coordinates.
  SMLoc findSMLocFromCoords(SourceCoords coords);

  /// Return an identifier for this buffer, typically the filename it was read
  /// from.
  llvm::StringRef getOriginalBufferIdentifier(unsigned bufId) const {
    return sm_.getMemoryBuffer(bufId)->getBufferIdentifier();
  }

  /// Get the user-specified source URL for this buffer, or a default identifier
  /// for it (typically the filename it was read from).
  llvm::StringRef getSourceUrl(unsigned bufId) const {
    const auto it = sourceUrls_.find(bufId);
    if (it != sourceUrls_.end()) {
      return it->second;
    }
    return getOriginalBufferIdentifier(bufId);
  }

  /// Print the passed source coordinates in human readable form for debugging.
  void dumpCoords(llvm::raw_ostream &OS, const SourceCoords &coords);

  /// If sucessfully decoded, print the passed source location in human readable
  /// form.
  void dumpCoords(llvm::raw_ostream &OS, SMLoc loc);

  void message(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);
  void message(DiagKind dk, SMRange sm, const Twine &msg);
  void message(DiagKind dk, SMLoc loc, const Twine &msg);

  void error(SMLoc loc, SMRange rng, const llvm::Twine &msg) {
    message(DK_Error, loc, rng, msg);
  }
  void warning(SMLoc loc, SMRange rng, const llvm::Twine &msg) {
    warning(Warning::Misc, loc, rng, msg);
  }
  void warning(Warning w, SMLoc loc, SMRange rng, const llvm::Twine &msg) {
    if (isWarningEnabled(w)) {
      message(DK_Warning, loc, rng, msg);
    }
  }
  void note(SMLoc loc, SMRange rng, const llvm::Twine &msg) {
    message(DK_Note, loc, rng, msg);
  }

  void error(SMRange rng, const llvm::Twine &msg) {
    message(DK_Error, rng, msg);
  }
  void warning(SMRange rng, const llvm::Twine &msg) {
    warning(Warning::Misc, rng, msg);
  }
  void warning(Warning w, SMRange rng, const llvm::Twine &msg) {
    if (isWarningEnabled(w)) {
      message(DK_Warning, rng, msg);
    }
  }
  void note(SMRange rng, const llvm::Twine &msg) {
    message(DK_Note, rng, msg);
  }

  void error(SMLoc loc, const llvm::Twine &msg) {
    message(DK_Error, loc, msg);
  }
  void warning(SMLoc loc, const llvm::Twine &msg) {
    warning(Warning::Misc, loc, msg);
  }
  void warning(Warning w, SMLoc loc, const llvm::Twine &msg) {
    if (isWarningEnabled(w)) {
      message(DK_Warning, loc, msg);
    }
  }
  void note(SMLoc loc, const llvm::Twine &msg) {
    message(DK_Note, loc, msg);
  }

  unsigned getMessageCount(DiagKind dk) const {
    assert(dk <= DK_Note);
    return messageCount_[dk];
  }

  unsigned getErrorCount() const {
    return getMessageCount(DK_Error);
  }
  unsigned getWarningCount() const {
    return getMessageCount(DK_Warning);
  }
  unsigned getNoteCount() const {
    return getMessageCount(DK_Note);
  }

  /// Combine two unsorted location into a range encompassing them both.
  static SMRange combineIntoRange(SMLoc a, SMLoc b) {
    if (a.getPointer() < b.getPointer())
      return SMRange(a, SMLoc::getFromPointer(b.getPointer() + 1));
    else
      return SMRange(b, SMLoc::getFromPointer(a.getPointer() + 1));
  }

  /// Extract the end location and convert it into an actual location by
  /// subtracting one and handling corner cases.
  static SMLoc convertEndToLocation(SMRange range);

  /// RAII to enable message suppression and restore the previous state of
  /// supression on destruction.
  class SaveAndSuppressMessages {
    SourceErrorManager *const sm_;
    bool const messagesSuppressed_;

   public:
    SaveAndSuppressMessages(SourceErrorManager *sm)
        : sm_(sm), messagesSuppressed_(sm->suppressMessages_) {
      sm->suppressMessages_ = true;
    }
    ~SaveAndSuppressMessages() {
      sm_->suppressMessages_ = messagesSuppressed_;
    }
  };

  /// RAII to enable message buffering and restore the previous state of
  /// buffering on destruction.
  class SaveAndBufferMessages {
    SourceErrorManager *const sm_;

   public:
    SaveAndBufferMessages(SourceErrorManager *sm) : sm_(sm) {
      sm->enableBuffering();
    }
    ~SaveAndBufferMessages() {
      sm_->disableBuffering();
    }
  };

 private:
  /// Optionally upgrade warnings into errors.
  void upgradeDiag(DiagKind &dk) {
    assert(dk <= DK_Note && "invalid DiagKind");
    if (warningsAreErrors_ && dk == DK_Warning)
      dk = DK_Error;
  }

  /// Implementation of generating a message.
  void doGenMessage(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);

  /// Actually print the message without performing any checks, buffering, etc.
  void doPrintMessage(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);
};

}; // namespace hermes

#endif // HERMES_SUPPORT_SOURCEERRORMANAGER_H
