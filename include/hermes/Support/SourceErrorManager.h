/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_SUPPORT_SOURCEERRORMANAGER_H
#define HERMES_SUPPORT_SOURCEERRORMANAGER_H

#include "hermes/Support/OptValue.h"
#include "hermes/Support/StringSetVector.h"
#include "hermes/Support/Warning.h"

#include "llvh/ADT/DenseMap.h"
#include "llvh/ADT/DenseSet.h"
#include "llvh/ADT/SmallBitVector.h"
#include "llvh/Support/SourceMgr.h"

#include <cassert>
#include <string>

namespace hermes {

using llvh::DenseSet;
using llvh::SMLoc;
using llvh::SMRange;
using llvh::Twine;

class CollectMessagesRAII;

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

/// Allows the specification of the subsystem which generated a message.
enum class Subsystem {
  /// No specific system provided.
  /// While different functions may interpret this as "exclude no systems"
  /// or "include no systems", the general use case is akin to llvh::None.
  Unspecified,
  /// e.g. JSLexer or something with similar functionality.
  Lexer,
  /// e.g. JSParser, JSONParser or something with similar functionality.
  Parser,
};

/// A facade around llvh::SourceMgr which simplifies error output and counts the
/// errors.
class SourceErrorManager {
 public:
  enum DiagKind {
    DK_Error = llvh::SourceMgr::DK_Error,
    DK_Warning = llvh::SourceMgr::DK_Warning,
    DK_Note = llvh::SourceMgr::DK_Note,
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

  /// Result from looking for a line in an input buffer. Contains the buffer
  /// id, 1-based line number and a reference to the line itself in the buffer.
  struct LineCoord {
    /// 1-based buffer id.
    unsigned bufId = 0;
    /// 1-based line number.
    unsigned lineNo = 0;
    /// A reference to the line itself, including the EOL, if present.
    llvh::StringRef lineRef;
  };

  struct ICoordTranslator {
    virtual ~ICoordTranslator() = 0;
    virtual void translate(SourceCoords &coords) = 0;
  };

  using DiagHandlerTy = llvh::SourceMgr::DiagHandlerTy;

 private:
  llvh::SourceMgr sm_{};
  SourceErrorOutputOptions outputOptions_;
  std::shared_ptr<ICoordTranslator> translator_{};

  /// A cache to speed up finding locations. The assumption is that most lookups
  /// happen either in the current or the next source line, which would happen
  /// naturally if we are scanning the source left to right.
  /// If there is a cache hit in the current line, there is no lookup at all -
  /// just quick arithmetic to calculate the column offset. If the hit is in
  /// the next line, we "slide" the cache - the next line becomes the current
  /// one, and we fetch a reference to the next line, which is also an O(1)
  /// operation.
  struct FindLineCache {
    /// 1-based buffer ID. 0 means cache is invalid.
    unsigned bufferId = 0;
    /// 1-based line number.
    unsigned lineNo = 0;
    /// The last found line.
    llvh::StringRef lineRef{};
    /// The following line.
    llvh::StringRef nextLineRef{};

    /// Fill a SourceCoords instance under the assumption that it is a verified
    /// cache hit.
    void fillCoords(SMLoc loc, SourceCoords &result);
  } findLineCache_;

  /// Virtual buffers are tagged with the higest bit.
  static constexpr unsigned kVirtualBufIdTag = 1u
      << (sizeof(unsigned) * CHAR_BIT - 1);

  /// The names of virtual buffers.
  hermes::StringSetVector virtualBufferNames_{};

  static constexpr unsigned kMessageCountSize = 4;

  unsigned messageCount_[kMessageCountSize]{0, 0, 0, 0};

  /// Supress errors after this has been reached
  unsigned errorLimit_ = UINT_MAX;

  /// Set to true once the error limit has been reached.
  bool errorLimitReached_ = false;

  /// Mapping from warnings to true if enabled, false if disabled.
  /// All warnings default to enabled, and the numerical value of the Warning
  /// enum is the index in this vector.
  llvh::SmallBitVector warningStatuses_;

  /// Mapping from warnings to true if should be treated as an error, false if
  /// should be treated as a warning. All warnings default to being treated as
  /// warnings, and the numerical value of the Warning enum is the index in this
  /// vector.
  llvh::SmallBitVector warningsAreErrors_;

  /// If set, messages from the given subsystem are ignored.
  /// If set to Subsystem::Unspecified, then all messages are ignored.
  OptValue<Subsystem> suppressMessages_{llvh::None};

  /// Set to true if the last message was suppressed. Any following DK_Note
  /// messages will be automatically suppressed.
  bool lastMessageSuppressed_{false};

  /// Map of bufId to source mapping URLs.
  /// If an entry doesn't exist, then there is no source mapping URL.
  llvh::DenseMap<unsigned, std::string> sourceMappingUrls_{};

  /// Map of bufId to user-specified source URLs.
  /// If an entry doesn't exist, then there is no user-specified source URL.
  llvh::DenseMap<unsigned, std::string> sourceUrls_{};

  /// If larger than zero, messages are buffered and not immediately displayed.
  /// They will be displayed once the counter falls back to zero.
  unsigned bufferingEnabled_{0};

  /// Data for a single message.
  class MessageData {
   public:
    DiagKind dk;
    SMLoc loc;
    SMRange sm;
    std::string msg;

    MessageData(DiagKind dk, SMLoc loc, SMRange sm, std::string &&msg)
        : dk(dk), loc(loc), sm(sm), msg(std::move(msg)) {}
  };

  /// An instance of a buffered message, which will be printed later.
  class BufferedMessage : public MessageData {
   public:
    using MessageData::MessageData;

    /// Associate a note with a message.
    void addNote(
        std::vector<MessageData> &bufferedNotes,
        DiagKind dk,
        SMLoc loc,
        SMRange sm,
        std::string &&msg);

    llvh::iterator_range<const MessageData *> notes(
        const std::vector<MessageData> &bufferedNotes) const;

   private:
    /// Number of notes associated with this message.
    unsigned noteCount_ = 0;
    /// Index of the first associated note in the \c bufferedNotes_ vector.
    unsigned firstNote_;
  };

  /// If non-null, send messages to externalMessageBuffer_ instead of directly
  /// to bufferedMessages_ or printing to screen.
  CollectMessagesRAII *externalMessageBuffer_{nullptr};

  /// All buffered messages. This is empty if \c bufferingEnabled_ is zero.
  std::vector<BufferedMessage> bufferedMessages_{};

  /// The notes associated with the buffered messages.
  std::vector<MessageData> bufferedNotes_{};

  /// Diagnostic printer appropriate for setting via SourceMgr.setDiagHandler
  static void printDiagnostic(const llvh::SMDiagnostic &, void *ctx);

  friend class SaveAndSuppressMessages;
  friend class SaveAndBufferMessages;
  friend class CollectMessagesRAII;

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

  /// \return true if \c warning is treated as an error or false if it's treated
  /// as a warning.
  bool isWarningAnError(Warning warning) const {
    return warningsAreErrors_.test((unsigned)warning);
  }

  /// Set \c warning to be treated as an error if \c warningIsErrors is true
  /// or as a warning if it's false.
  void setWarningIsError(Warning warning, bool warningIsError) {
    warningsAreErrors_[(unsigned)warning] = warningIsError;
  }

  /// Set all warnings to be treated as errors if \c warningsAreErrors is true
  /// or as warnings if it's false.
  void setWarningsAreErrors(bool warningsAreErrors) {
    if (warningsAreErrors) {
      warningsAreErrors_.set();
    } else {
      warningsAreErrors_.reset();
    }
  }

  void disableAllWarnings() {
    warningStatuses_.reset();
  }

  void setWarningStatus(Warning warning, bool enabled) {
    warningStatuses_[(unsigned)warning] = enabled;
  }

  SourceErrorOutputOptions getOutputOptions() const {
    return outputOptions_;
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
  unsigned addNewSourceBuffer(std::unique_ptr<llvh::MemoryBuffer> f);

  /// Add a source buffer which maps to a filename. It doesn't contain any
  /// source and the only operation that can be performed on that buffer is to
  /// obtain the filename using \c getBufferFileName() or \c getSourceUrl().
  unsigned addNewVirtualSourceBuffer(llvh::StringRef fileName);

  /// \return true if this bufferId was created using \c
  /// addNewVirtualSourceBuffer().
  inline bool isVirtualBufferId(unsigned bufId) const {
    return (bufId & kVirtualBufIdTag) != 0;
  }

  /// \return the filename associated with the buffer id (which may be virtual).
  llvh::StringRef getBufferFileName(unsigned bufId) const;

  const llvh::MemoryBuffer *getSourceBuffer(unsigned bufId) const {
    assert(!isVirtualBufferId(bufId) && "virtual buffers cannot be accessed");
    return sm_.getMemoryBuffer(bufId);
  }

  /// Set the source mapping URL for the buffer \p bufId.
  /// If one was already set, overwrite it.
  void setSourceMappingUrl(unsigned bufId, llvh::StringRef url) {
    sourceMappingUrls_[bufId] = url;
  }

  /// Get the source mapping URL for file \p bufId.
  /// \return URL if it exists, else return empty string.
  llvh::StringRef getSourceMappingUrl(unsigned bufId) const {
    const auto it = sourceMappingUrls_.find(bufId);
    if (it == sourceMappingUrls_.end()) {
      return "";
    }
    return it->second;
  }

  /// Set the user-specified source URL for the buffer \p bufId.
  /// If one was already set, overwrite it.
  void setSourceUrl(unsigned bufId, llvh::StringRef url) {
    sourceUrls_[bufId] = url;
  }

  /// Find the bufferId of the specified location \p loc.
  uint32_t findBufferIdForLoc(SMLoc loc) const;

  /// Find the buffer ID and line of the specified location \p loc.
  /// \return the buffer ID and line of the location, or None on error.
  llvh::Optional<LineCoord> findBufferAndLine(SMLoc loc) const;

  /// Return a reference to the specified (1-based) line.
  /// If the line is greater than the last line in the buffer, an empty
  /// reference is returned.
  llvh::StringRef getLineRef(unsigned bufId, unsigned line) const {
    return sm_.getLineRef(line, bufId);
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
  const llvh::MemoryBuffer *findBufferForLoc(SMLoc loc) const;

  /// Find the SMLoc corresponding to the supplied source coordinates.
  SMLoc findSMLocFromCoords(SourceCoords coords);

  /// Given an SMDiagnostic, return {sourceLine, caretLine}, respecting the
  /// error output options
  static std::pair<std::string, std::string> buildSourceAndCaretLine(
      const llvh::SMDiagnostic &diag,
      SourceErrorOutputOptions opts);

  /// Get the user-specified source URL for this buffer, or a default identifier
  /// for it (typically the filename it was read from).
  llvh::StringRef getSourceUrl(unsigned bufId) const {
    const auto it = sourceUrls_.find(bufId);
    if (it != sourceUrls_.end()) {
      return it->second;
    }
    return getBufferFileName(bufId);
  }

  /// Print the passed source coordinates in human readable form for debugging.
  void dumpCoords(llvh::raw_ostream &OS, const SourceCoords &coords);

  /// If sucessfully decoded, print the passed source location in human readable
  /// form.
  void dumpCoords(llvh::raw_ostream &OS, SMLoc loc);

  void message(
      DiagKind dk,
      SMLoc loc,
      SMRange sm,
      const Twine &msg,
      Warning w,
      Subsystem subsystem);
  void message(
      DiagKind dk,
      SMLoc loc,
      SMRange sm,
      const Twine &msg,
      Subsystem subsystem);
  void message(DiagKind dk, SMRange sm, const Twine &msg, Subsystem subsystem);
  void message(DiagKind dk, SMLoc loc, const Twine &msg, Subsystem subsystem);

  void error(
      SMLoc loc,
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Error, loc, rng, msg, subsystem);
  }
  void warning(
      SMLoc loc,
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    warning(Warning::Misc, loc, rng, msg, subsystem);
  }
  void warning(
      Warning w,
      SMLoc loc,
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Warning, loc, rng, msg, w, subsystem);
  }
  void note(
      SMLoc loc,
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Note, loc, rng, msg, subsystem);
  }

  void error(
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Error, rng, msg, subsystem);
  }
  void warning(
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    warning(Warning::Misc, rng, msg, subsystem);
  }
  void warning(
      Warning w,
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Warning, rng.Start, rng, msg, w, subsystem);
  }
  void note(
      SMRange rng,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Note, rng, msg, subsystem);
  }

  void error(
      SMLoc loc,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Error, loc, msg, subsystem);
  }
  void warning(
      SMLoc loc,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    warning(Warning::Misc, loc, msg, subsystem);
  }
  void warning(
      Warning w,
      SMLoc loc,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Warning, loc, SMRange{}, msg, w, subsystem);
  }
  void note(
      SMLoc loc,
      const llvh::Twine &msg,
      Subsystem subsystem = Subsystem::Unspecified) {
    message(DK_Note, loc, msg, subsystem);
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
    OptValue<Subsystem> messagesSuppressed_;

   public:
    SaveAndSuppressMessages(
        SourceErrorManager *sm,
        Subsystem subsystem = Subsystem::Unspecified)
        : sm_(sm), messagesSuppressed_(sm->suppressMessages_) {
      sm->suppressMessages_ = subsystem;
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
  /// Increment the message counter and check if we've hit the error limit.
  void countAndGenMessage(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);

  /// Implementation of generating a message.
  void doGenMessage(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);

  /// Actually print the message without performing any checks, buffering, etc.
  void doPrintMessage(DiagKind dk, SMLoc loc, SMRange sm, const Twine &msg);

  /// Convert a virtual buffer ID to an index.
  unsigned virtualBufferIdToIndex(unsigned bufId) const {
    assert(isVirtualBufferId(bufId) && "bufId is not virtual");
    return bufId & ~kVirtualBufIdTag;
  }

  /// Convert an index to a virtual buffer ID.
  unsigned indexToVirtualBufferId(unsigned index) const {
    return index | kVirtualBufIdTag;
  }
};

/// RAII to enable message buffering and restore the previous state of
/// buffering on destruction, while discarding messages on request.
/// Enables the caller to decide whether or not to actually print the messages
/// upon destruction while this class is alive.
class CollectMessagesRAII {
  SourceErrorManager *const sm_;
  CollectMessagesRAII *oldExternalMessageBuffer_;

  /// Whether to show or discard messages upon destruction.
  bool discardMessages_;

  class StoredMessage {
   public:
    SourceErrorManager::DiagKind dk;
    SMLoc loc;
    SMRange sm;
    std::string msg;

    StoredMessage(
        SourceErrorManager::DiagKind dk,
        SMLoc loc,
        SMRange sm,
        Twine const &msg)
        : dk(dk), loc(loc), sm(sm), msg(msg.str()) {}
  };

  std::vector<StoredMessage> storage_{};

 public:
  CollectMessagesRAII(SourceErrorManager *sm, bool discardMessages)
      : sm_(sm), discardMessages_(discardMessages) {
    sm->enableBuffering();
    oldExternalMessageBuffer_ = sm->externalMessageBuffer_;
    sm->externalMessageBuffer_ = this;
  }

  ~CollectMessagesRAII() {
    if (!discardMessages_) {
      for (StoredMessage &msg : storage_) {
        sm_->countAndGenMessage(msg.dk, msg.loc, msg.sm, std::move(msg.msg));
      }
    }
    sm_->disableBuffering();
    sm_->externalMessageBuffer_ = oldExternalMessageBuffer_;
  }

  void addMessage(
      SourceErrorManager::DiagKind dk,
      SMLoc loc,
      SMRange sm,
      Twine const &msg) {
    storage_.emplace_back(dk, loc, sm, msg);
  }

  void setDiscardMessages(bool discardMessages) {
    discardMessages_ = discardMessages;
  }
};

} // namespace hermes

#endif // HERMES_SUPPORT_SOURCEERRORMANAGER_H
