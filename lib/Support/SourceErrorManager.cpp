/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "hermes/Support/SourceErrorManager.h"
#include "hermes/Support/UTF8.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/raw_ostream.h"

namespace hermes {

namespace {

/// The location cache of a single memory buffer. Locations are cached in an
/// array, in strictly increasing offsets, storing the start of the line and the
/// line number corresponding to that location. When looking for the source
/// coordinates of a location, the cache finds the closest smaller cached
/// location using binary search and starting from there determines the
/// coordinates of the new location.
///
/// The cache ensures that cached locations are always about STEP bytes apart,
/// which guarantees a small upper bound for the scanning time between two
/// cached locations.
class BufferLocationCache {
  /// Cache locations are always "about" this distance from each other. The
  /// variation comes from the precise boundary being in the middle of an UTF-8
  /// code point or between '\n' and '\r'.
  static const unsigned STEP = 8 * 1024;

  /// The cached information for a location.
  struct LineInfo {
    /// The location itself.
    const char *ptr;
    /// The start of the line of the location.
    const char *lineStart;
    /// The number of the line.
    unsigned line;

    /// To facilitate binary search.
    bool operator<(const LineInfo &x) const {
      return ptr < x.ptr;
    }
  };

  /// The associated source buffer.
  const llvm::MemoryBuffer *const buf_;

  /// The cache itself. This array only grows - only the last element is ever
  /// updated, or new elements are pushed back.
  llvm::SmallVector<LineInfo, 128> lineCache_;

  /// The result of last lookup. nullptr indicates that it is empty.
  LineInfo last_{nullptr, nullptr, 0};
  /// The index in lineCache_ of the last lookup.
  unsigned lastIndex_{0};

 public:
  explicit BufferLocationCache(const llvm::MemoryBuffer *buf) : buf_(buf) {
    // Offset 0 is in line 1.
    lineCache_.push_back({buf->getBufferStart(), buf->getBufferStart(), 1});
  }

  /// Find the line and column of the specified location \p loc. Guaranteed to
  /// succeed because the location is within the source buffer.
  void findBufferLineAndLoc(
      SMLoc loc,
      SourceErrorManager::SourceCoords &result);

 private:
  /// Starting from the supplied cache location \p lineInfo, scan until location
  /// \p to and return a pair containing the start of the line of the location
  /// and its line number.
  std::pair<const char *, unsigned> scan(
      const LineInfo &lineInfo,
      const char *to) {
    unsigned line = lineInfo.line;
    const char *lineStart = lineInfo.lineStart;
    const char *cur = lineInfo.ptr;

    assert(
        *to != '\r' && !isUTF8ContinuationByte(*to) &&
        "cannot search for location in the middle of CRLF or UTF8 codepoint");

    // TODO: check for Unicode line separators?
    while (
        (cur = static_cast<const char *>(std::memchr(cur, '\n', to - cur)))) {
      ++cur;
      if (*cur == '\r')
        ++cur;
      lineStart = cur;
      ++line;
    }

    return std::make_pair(lineStart, line);
  }
};

using BufferCachePtr = std::shared_ptr<BufferLocationCache>;

void BufferLocationCache::findBufferLineAndLoc(
    SMLoc loc,
    SourceErrorManager::SourceCoords &result) {
  const char *ptr = loc.getPointer();

  // In the very unlikely case that `loc` points to a '\r', we skip backwards
  // until we find another character, while being careful not to fall off the
  // beginning of the buffer.
  if (LLVM_UNLIKELY(*ptr == '\r' || isUTF8ContinuationByte(*ptr))) {
    const char *bufStart = buf_->getBufferStart();
    do {
      if (ptr == bufStart) {
        // This is highly unlikely but theoretically possible. There were only
        // '\r' between `loc` and the start of the buffer.
        result.line = 1;
        result.col = 1;
        return;
      }
      --ptr;
    } while (*ptr == '\r' || isUTF8ContinuationByte(*ptr));
  }

  auto *back = &lineCache_.back();

  // Are we extending the end of the cache?
  if (ptr >= back->ptr) {
    const char *upto;
    std::pair<const char *, unsigned> scanRes;

    /// Loop in STEP chunks, adding cache entries on the way, until we reach
    /// our desired location.
    do {
      // Determine the step. Do we need to move the last cached location
      // forward, or add a new one.
      unsigned step = STEP;
      if (lineCache_.size() > 1) {
        unsigned ns = (unsigned)(back->ptr - back[-1].ptr);
        if (ns < STEP)
          step = STEP - ns;
      }
      upto = std::min(ptr, back->ptr + step);

      // Skip invalid locations. Note that we know that the 'ptr' is a valid
      // location, so it is safe to simply more forward here.
      while (LLVM_UNLIKELY(*upto == '\r' || isUTF8ContinuationByte(*upto)))
        ++upto;

      scanRes = scan(*back, upto);
      if (step == STEP) {
        lineCache_.push_back({upto, scanRes.first, scanRes.second});
        back = &lineCache_.back();
      } else {
        back->ptr = upto;
        back->lineStart = scanRes.first;
        back->line = scanRes.second;
      }
    } while (upto != ptr);

    result.line = scanRes.second;
    result.col = (unsigned)(ptr - scanRes.first) + 1;
    return;
  }

  // Are we between the last position and the next cache entry?
  if (last_.ptr && ptr >= last_.ptr && ptr < lineCache_[lastIndex_ + 1].ptr) {
    auto scanRes = scan(last_, ptr);

    // Update the last entry.
    last_.ptr = ptr;
    last_.lineStart = scanRes.first;
    last_.line = scanRes.second;

    result.line = scanRes.second;
    result.col = (unsigned)(ptr - scanRes.first) + 1;
    return;
  }

  LineInfo LF = {ptr, nullptr, 0};
  // Locate the closest offset in the cache using binary search.
  auto upper = std::upper_bound(lineCache_.begin(), lineCache_.end(), LF);
  assert(upper != lineCache_.begin() && "element must be inside the cache");
  assert(upper != lineCache_.end() && "element must be inside the cache");

  --upper; // Point to the cache element that is <= that 'ptr'.
  auto scanRes = scan(*upper, ptr);
  // Update the last entry.
  last_.ptr = ptr;
  last_.lineStart = scanRes.first;
  last_.line = scanRes.second;
  lastIndex_ = (unsigned)std::distance(lineCache_.begin(), upper);

  result.line = scanRes.second;
  result.col = (unsigned)(ptr - scanRes.first) + 1;
}
}; // anonymous namespace

class SourceLocationCache {
  llvm::SourceMgr &sm_;
  llvm::DenseMap<unsigned, BufferCachePtr> bufferMap_{};

 public:
  explicit SourceLocationCache(llvm::SourceMgr &sm) : sm_(sm) {}

  /// Find the bufferId, line and column of the specified location \p loc.
  /// This is a very slow method that should be used only for error generation.
  /// \return true on success, false if could not be found, in which case
  ///     result.isValid() would also return false.
  bool findBufferLineAndLoc(
      SMLoc loc,
      SourceErrorManager::SourceCoords &result);
};

bool SourceLocationCache::findBufferLineAndLoc(
    SMLoc loc,
    SourceErrorManager::SourceCoords &result) {
  if (!loc.isValid()) {
    result.bufId = 0;
    return false;
  }

  unsigned bufId = sm_.FindBufferContainingLoc(loc);
  if (!bufId) {
    result.bufId = 0;
    return false;
  }

  auto &bufPtrRef = bufferMap_[bufId];
  if (!bufPtrRef)
    bufPtrRef =
        std::make_shared<BufferLocationCache>(sm_.getMemoryBuffer(bufId));

  result.bufId = bufId;
  bufPtrRef->findBufferLineAndLoc(loc, result);

  return true;
}

SourceErrorManager::ICoordTranslator::~ICoordTranslator() = default;

SourceErrorManager::SourceErrorManager()
    : cache_(new SourceLocationCache(sm_)),
      warningStatuses_((unsigned)Warning::_NumWarnings, true) {
  sm_.setDiagHandler(SourceErrorManager::printDiagnostic, this);
}

void SourceErrorManager::enableBuffering() {
  ++bufferingEnabled_;
  assert(bufferingEnabled_ != 0 && "unsigned counter overflow");
}

void SourceErrorManager::disableBuffering() {
  assert(bufferingEnabled_ != 0 && "unsigned counter underflow");

  if (--bufferingEnabled_ != 0)
    return;

  // Sort all messages.
  std::sort(
      bufferedMessages_.begin(),
      bufferedMessages_.end(),
      [](const BufferedMessage &a, const BufferedMessage &b) {
        return a.coords.less(b.coords);
      });

  // Print them.
  for (const auto &bm : bufferedMessages_) {
    doPrintMessage(bm.dk, bm.loc, bm.sm, bm.msg);
  }

  // Clean the buffer.
  bufferedMessages_.clear();
}

uint32_t SourceErrorManager::addNewVirtualSourceBuffer(
    llvm::StringRef bufferName) {
  return addNewSourceBuffer(
      llvm::MemoryBuffer::getMemBuffer("", bufferName, true));
}

void SourceErrorManager::dumpCoords(
    llvm::raw_ostream &OS,
    const SourceCoords &coords) {
  if (coords.isValid()) {
    OS << getSourceUrl(coords.bufId) << ":" << coords.line << "," << coords.col;
  } else {
    OS << "none:0,0";
  }
}

void SourceErrorManager::dumpCoords(llvm::raw_ostream &OS, SMLoc loc) {
  SourceCoords coords;
  findBufferLineAndLoc(loc, coords);
  dumpCoords(OS, coords);
}

void SourceErrorManager::doGenMessage(
    hermes::SourceErrorManager::DiagKind dk,
    llvm::SMLoc loc,
    llvm::SMRange sm,
    llvm::Twine const &msg) {
  if (bufferingEnabled_) {
    SourceCoords coords;
    findBufferLineAndLoc(loc, coords);
    bufferedMessages_.emplace_back(dk, loc, sm, msg.str(), coords);
  } else {
    doPrintMessage(dk, loc, sm, msg);
  }
}

void SourceErrorManager::doPrintMessage(
    DiagKind dk,
    SMLoc loc,
    SMRange sm,
    const Twine &msg) {
  sm_.PrintMessage(
      loc,
      static_cast<llvm::SourceMgr::DiagKind>(dk),
      msg,
      sm.isValid() ? llvm::ArrayRef<SMRange>(sm)
                   : llvm::ArrayRef<SMRange>(llvm::None),
      llvm::None,
      outputOptions_.showColors);
}

void SourceErrorManager::message(
    DiagKind dk,
    SMLoc loc,
    SMRange sm,
    const Twine &msg) {
  assert(dk <= DK_Note);
  if (suppressMessages_)
    return;
  upgradeDiag(dk);
  assert(static_cast<unsigned>(dk) < kMessageCountSize && "bounds check");

  // Supress all messages once the error limit has been reached.
  if (LLVM_UNLIKELY(errorLimitReached_))
    return;

  ++messageCount_[dk];
  doGenMessage(dk, loc, sm, msg);

  if (LLVM_UNLIKELY(dk == DK_Error && messageCount_[DK_Error] == errorLimit_)) {
    errorLimitReached_ = true;
    doGenMessage(DK_Error, {}, {}, "too many errors emitted");
  }
}

void SourceErrorManager::message(DiagKind dk, SMRange sm, const Twine &msg) {
  message(dk, sm.Start, sm, msg);
}

void SourceErrorManager::message(DiagKind dk, SMLoc loc, const Twine &msg) {
  message(dk, loc, SMRange{}, msg);
}

bool SourceErrorManager::findBufferLineAndLoc(SMLoc loc, SourceCoords &result) {
  return cache_->findBufferLineAndLoc(loc, result);
}

bool SourceErrorManager::findBufferLineAndLoc(
    llvm::SMLoc loc,
    hermes::SourceErrorManager::SourceCoords &result,
    bool translate) {
  if (!findBufferLineAndLoc(loc, result))
    return false;
  if (translate && translator_)
    translator_->translate(result);
  return true;
}

const llvm::MemoryBuffer *SourceErrorManager::findBufferForLoc(
    SMLoc loc) const {
  uint32_t bufID = sm_.FindBufferContainingLoc(loc);
  if (bufID == 0) {
    return nullptr;
  }
  return sm_.getMemoryBuffer(bufID);
}

SMLoc SourceErrorManager::findSMLocFromCoords(SourceCoords coords) {
  if (!coords.isValid())
    return {};

  // TODO: optimize this with caching, etc.
  auto *buffer = getSourceBuffer(coords.bufId);
  if (!buffer)
    return {};

  const char *cur = buffer->getBufferStart();
  const char *end = buffer->getBufferEnd();

  // Loop until we find the line or we reach EOF.
  unsigned lineNumber = 1;
  const char *lineEnd;
  while ((lineEnd = (const char *)std::memchr(cur, '\n', end - cur)) !=
             nullptr &&
         lineNumber != coords.line) {
    ++lineNumber;
    cur = lineEnd + 1;
  }

  // If we didn't find LF, the end of the buffer is the end of the line.
  if (!lineEnd)
    lineEnd = end;

  // The last line we found is [cur..lineEnd) and its number is lineNumber.
  // Is it the right one?
  if (lineNumber != coords.line)
    return {};

  // Trim a CR at start and end to account for all crazy line endings.
  if (cur != lineEnd && *cur == '\r')
    ++cur;
  if (cur != lineEnd && *(lineEnd - 1) == '\r')
    --lineEnd;

  // Special case for empty line.
  if (cur == lineEnd) {
    // Column 1 or 0 in an empty line should work.
    if (coords.col <= 1)
      return SMLoc::getFromPointer(cur);
    return {};
  }

  // Check for presence of UTF-8.
  bool utf8 = false;
  for (const char *p = cur; p != lineEnd; ++p) {
    if (LLVM_UNLIKELY(*p & 0x80)) {
      utf8 = true;
      break;
    }
  }

  // ASCII is easy - just add the offset.
  if (LLVM_LIKELY(!utf8)) {
    // Is the column in range?
    if (coords.col > (size_t)(lineEnd - cur))
      return {};
    return SMLoc::getFromPointer(cur + coords.col - 1);
  }

  // Scan for the column while accounting for multi-byte characters.
  unsigned column = 0;
  for (; cur != lineEnd; ++cur) {
    // Skip continuation bytes.
    if (isUTF8ContinuationByte(*cur))
      continue;
    if (++column == coords.col)
      return SMLoc::getFromPointer(cur);
  }

  return {};
}

/// Given an SMDiagnostic, return {sourceLine, caretLine}, respecting the error
/// output options
static std::pair<std::string, std::string> buildSourceAndCaretLine(
    const llvm::SMDiagnostic &diag,
    SourceErrorOutputOptions opts) {
  // Decode our source line to UTF-32
  // Ignore errors (UTF-8 errors will become replacement character)
  // Don't try to decode past embedded nulls
  // Map from narrow byte to column as we go
  std::vector<uint32_t> narrowByteToColumn;
  std::u32string sourceLine;
  std::string narrowSourceLine = diag.getLineContents();
  const char *cursor = narrowSourceLine.c_str();
  while (*cursor) {
    const char *prev = cursor;
    sourceLine.push_back(decodeUTF8<true>(cursor, [](const llvm::Twine &) {}));
    while (prev++ < cursor) {
      narrowByteToColumn.push_back(sourceLine.size() - 1);
    }
  }
  const size_t numColumns = sourceLine.size();

  // Widening helper
  auto widenColumn = [&](unsigned narrowColumn) -> unsigned {
    return narrowColumn < narrowByteToColumn.size()
        ? narrowByteToColumn[narrowColumn]
        : numColumns;
  };

  // Widen the caret column and ranges using our map
  // Ranges are of the form [first, last)
  assert(diag.getColumnNo() >= 0);
  const size_t columnNo = widenColumn(diag.getColumnNo());
  std::vector<std::pair<unsigned, unsigned>> ranges;
  for (const auto &r : diag.getRanges()) {
    ranges.emplace_back(widenColumn(r.first), widenColumn(r.second));
  }

  // Build the line with the caret and ranges.
  std::string caretLine(numColumns + 1, ' ');
  for (const auto range : ranges) {
    if (range.first < caretLine.size()) {
      std::fill(
          &caretLine[range.first],
          &caretLine[std::min((size_t)range.second, caretLine.size())],
          '~');
    }
  }
  caretLine[std::min(size_t(columnNo), numColumns)] = '^';
  caretLine.erase(caretLine.find_last_not_of(' ') + 1);

  // Expand tabs to spaces in both the source and caret line
  const size_t tabStop = SourceErrorOutputOptions::TabStop;
  for (size_t pos = sourceLine.find('\t'); pos < sourceLine.size();
       pos = sourceLine.find('\t', pos)) {
    size_t expandCount = tabStop - (pos % tabStop);
    sourceLine.replace(pos, 1, expandCount, ' ');
    if (pos < caretLine.size()) {
      // Reuse the character in the caretLine, so that tabs in tildes expand to
      // more tildes
      caretLine.replace(pos, 1, expandCount, caretLine[pos]);
    }
    pos += expandCount;
  }

  // Trim the lines to respect preferredMaxErrorWidth
  // "Focus" around the caret, and any range intersecting it
  // Note ranges are of the form [start, end) and not [start, length)
  int focusStart = columnNo;
  int focusLength = 1;
  for (const auto r : ranges) {
    if (r.first <= size_t(columnNo) && size_t(columnNo) < r.second) {
      focusStart = r.first;
      focusLength = r.second - r.first;
      break;
    }
  }
  size_t desiredLineLength = std::max(
      opts.preferredMaxErrorWidth,
      focusLength + SourceErrorOutputOptions::MinimumSourceContext);
  if (sourceLine.size() > desiredLineLength) {
    int focusCenter = focusStart + focusLength / 2;
    int leftTrimAmount = focusCenter - desiredLineLength / 2;
    if (leftTrimAmount > 0) {
      caretLine.erase(0, leftTrimAmount);
      sourceLine.erase(0, leftTrimAmount);
      std::fill(sourceLine.begin(), sourceLine.begin() + 3, '.');
    }
    if (sourceLine.size() > desiredLineLength) {
      // Trim on the right
      caretLine.erase(std::min(caretLine.size(), desiredLineLength));
      sourceLine.erase(desiredLineLength);
      std::fill(sourceLine.end() - 3, sourceLine.end(), '.');
    }
  }

  // Convert sourceLine back to narrow
  narrowSourceLine.clear();
  for (uint32_t c : sourceLine) {
    char buffer[UTF8CodepointMaxBytes] = {};
    char *buffCursor = buffer;
    encodeUTF8(buffCursor, c);
    narrowSourceLine.append(buffer, buffCursor);
  }
  return {std::move(narrowSourceLine), std::move(caretLine)};
}

void SourceErrorManager::printDiagnostic(
    const llvm::SMDiagnostic &diag,
    void *ctx) {
  using llvm::raw_ostream;
  const SourceErrorManager *self = static_cast<SourceErrorManager *>(ctx);
  const SourceErrorOutputOptions opts = self->outputOptions_;
  auto &S = llvm::errs();

  llvm::StringRef filename = diag.getFilename();
  int lineNo = diag.getLineNo();
  int columnNo = diag.getColumnNo();

  // Helpers to conditionally set or reset a color
  auto changeColor = [&](raw_ostream::Colors color) {
    if (opts.showColors)
      S.changeColor(color, true);
  };

  auto resetColor = [&]() {
    if (opts.showColors)
      S.resetColor();
  };

  changeColor(raw_ostream::SAVEDCOLOR);
  if (!filename.empty()) {
    S << (filename == "-" ? "<stdin>" : filename);
    if (lineNo != -1) {
      S << ':' << lineNo;
      if (columnNo != -1)
        S << ':' << (columnNo + 1);
    }
    S << ": ";
  }

  switch (diag.getKind()) {
    case llvm::SourceMgr::DK_Error:
      changeColor(raw_ostream::RED);
      S << "error: ";
      break;
    case llvm::SourceMgr::DK_Warning:
      changeColor(raw_ostream::MAGENTA);
      S << "warning: ";
      break;
    case llvm::SourceMgr::DK_Note:
      changeColor(raw_ostream::BLACK);
      S << "note: ";
      break;
    case llvm::SourceMgr::DK_Remark:
      changeColor(raw_ostream::BLACK);
      S << "remark: ";
      break;
  }

  resetColor();
  changeColor(raw_ostream::SAVEDCOLOR);
  S << diag.getMessage() << '\n';
  resetColor();

  if (lineNo == -1 || columnNo == -1)
    return;

  std::string sourceLine;
  std::string caretLine;
  std::tie(sourceLine, caretLine) = buildSourceAndCaretLine(diag, opts);

  // Check for non-ASCII characters, which may have a width > 1
  // If we find them, don't try to show the caret line
  // TODO: bravely teach buildSourceAndCaretLine to use wcwidth(), lifting this
  // restriction
  bool showCaret = isAllASCII(sourceLine.begin(), sourceLine.end());

  S << sourceLine << '\n';
  if (showCaret) {
    changeColor(raw_ostream::GREEN);
    S << caretLine << '\n';
    resetColor();
  }
}

SMLoc SourceErrorManager::convertEndToLocation(SMRange range) {
  // If the range is empty, return the starting point.
  if (range.Start == range.End)
    return range.Start;

  return SMLoc::getFromPointer(range.End.getPointer() - 1);
}

} // namespace hermes
