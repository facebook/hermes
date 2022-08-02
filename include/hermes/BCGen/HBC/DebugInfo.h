/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_DEBUGINFO_H
#define HERMES_BCGEN_HBC_DEBUGINFO_H

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/HBC/StreamVector.h"
#include "hermes/BCGen/HBC/UniquingFilenameTable.h"
#include "hermes/Public/DebuggerTypes.h"
#include "hermes/Support/LEB128.h"
#include "hermes/Support/OptValue.h"
#include "hermes/Support/StringTable.h"
#include "hermes/Support/UTF8.h"
#include "llvh/ADT/StringRef.h"
#include "llvh/Support/Format.h"

#include <string>
#include <vector>

namespace llvh {
class raw_ostream;
} // namespace llvh

namespace hermes {
class SourceMapGenerator;
namespace hbc {

/// The file name, line and column associated with a bytecode address.
struct DebugSourceLocation {
  // The bytecode offset of this debug info.
  uint32_t address{0};
  // The filename index in the filename table.
  uint32_t filenameId{0};
  // The sourceMappingUrl index in the string table.
  // Use kInvalidBreakpoint for an invalid URL.
  uint32_t sourceMappingUrlId{facebook::hermes::debugger::kInvalidBreakpoint};
  // The line count, 1 based.
  uint32_t line{0};
  // The column count, 1 based.
  uint32_t column{0};
  // The statement at this location. 1 based, per function.
  // Initialized to 0, to show that no statements have been generated yet.
  // Thus, we can see which instructions aren't part of any user-written code.
  uint32_t statement{0};

  DebugSourceLocation() {}

  DebugSourceLocation(
      uint32_t address,
      uint32_t filenameId,
      uint32_t line,
      uint32_t column,
      uint32_t statement)
      : address(address),
        filenameId(filenameId),
        line(line),
        column(column),
        statement(statement) {}

  bool operator==(const DebugSourceLocation &rhs) const {
    return address == rhs.address && filenameId == rhs.filenameId &&
        line == rhs.line && column == rhs.column && statement == rhs.statement;
  }

  bool operator!=(const DebugSourceLocation &rhs) const {
    return !(*this == rhs);
  }
};

/// A type wrapping up the offsets into debugging data.
struct DebugOffsets {
  /// Offsets into the debugging data of the source locations
  /// (DebugSourceLocation).
  uint32_t sourceLocations = NO_OFFSET;

  /// Offset into the lexical data section of the debugging data.
  uint32_t lexicalData = NO_OFFSET;

  /// Sentinel value indicating no offset.
  static constexpr uint32_t NO_OFFSET = UINT32_MAX;

  /// Constructors.
  DebugOffsets() = default;
  DebugOffsets(uint32_t src, uint32_t lex)
      : sourceLocations(src), lexicalData(lex) {}
};

/// A result of a search for a bytecode offset for where a line/column fall.
struct DebugSearchResult {
  // Offset of the result function in the bytecode stream.
  uint32_t functionIndex{0};

  // Offset of the result instruction in the bytecode,
  // from the start of the function that it's in.
  uint32_t bytecodeOffset{0};

  /// The actual line that the search found.
  uint32_t line{0};

  /// The actual column that the search found.
  uint32_t column{0};

  DebugSearchResult() {}

  DebugSearchResult(
      uint32_t functionIndex,
      uint32_t bytecodeOffset,
      uint32_t line,
      uint32_t column)
      : functionIndex(functionIndex),
        bytecodeOffset(bytecodeOffset),
        line(line),
        column(column) {}
};

/// A data structure for storing debug info.
class DebugInfo {
 public:
  using DebugFileRegionList = llvh::SmallVector<DebugFileRegion, 1>;

 private:
  /// Filename table for mapping to offsets and lengths in filenameStorage_.
  std::vector<StringTableEntry> filenameTable_{};

  /// String storage for filenames.
  std::vector<unsigned char> filenameStorage_{};

  DebugFileRegionList files_{};
  uint32_t lexicalDataOffset_ = 0;
  StreamVector<uint8_t> data_{};

  /// Get source filename as string id.
  OptValue<uint32_t> getFilenameForAddress(uint32_t debugOffset) const;

 public:
  explicit DebugInfo() = default;
  /*implicit*/ DebugInfo(DebugInfo &&that) = default;

  explicit DebugInfo(
      ConsecutiveStringStorage &&filenameStrings,
      DebugFileRegionList &&files,
      uint32_t lexicalDataOffset,
      StreamVector<uint8_t> &&data)
      : filenameTable_(filenameStrings.acquireStringTable()),
        filenameStorage_(filenameStrings.acquireStringStorage()),
        files_(std::move(files)),
        lexicalDataOffset_(lexicalDataOffset),
        data_(std::move(data)) {}

  explicit DebugInfo(
      std::vector<StringTableEntry> &&filenameStrings,
      std::vector<unsigned char> &&filenameStorage,
      DebugFileRegionList &&files,
      uint32_t lexicalDataOffset,
      StreamVector<uint8_t> &&data)
      : filenameTable_(std::move(filenameStrings)),
        filenameStorage_(std::move(filenameStorage)),
        files_(std::move(files)),
        lexicalDataOffset_(lexicalDataOffset),
        data_(std::move(data)) {}

  DebugInfo &operator=(DebugInfo &&that) = default;

  const DebugFileRegionList &viewFiles() const {
    return files_;
  }
  const StreamVector<uint8_t> &viewData() const {
    return data_;
  }
  llvh::ArrayRef<StringTableEntry> getFilenameTable() const {
    return filenameTable_;
  }
  llvh::ArrayRef<unsigned char> getFilenameStorage() const {
    return filenameStorage_;
  }

  /// Retrieve the filename for a given \p id in the filename table.
  std::string getFilenameByID(uint32_t id) const {
    assert(id < filenameTable_.size() && "Filename ID out of bounds");
    std::string utf8Storage;
    return getStringFromEntry(filenameTable_[id], filenameStorage_, utf8Storage)
        .str();
  }

  uint32_t lexicalDataOffset() const {
    return lexicalDataOffset_;
  }

  /// Get the location of \p offsetInFunction, given the function's debug
  /// offset.
  OptValue<DebugSourceLocation> getLocationForAddress(
      uint32_t debugOffset,
      uint32_t offsetInFunction) const;

  /// Given a \p targetLine and optional \p targetColumn,
  /// find a bytecode address at which that location is listed in debug info.
  /// If \p targetColumn is None, then it tries to match at the first location
  /// in \p line, else it tries to match at column \p targetColumn.
  OptValue<DebugSearchResult> getAddressForLocation(
      uint32_t filenameId,
      uint32_t targetLine,
      OptValue<uint32_t> targetColumn) const;

  /// Read variable names at \p offset into the lexical data section
  /// of the debug info. \return the list of variable names.
  llvh::SmallVector<llvh::StringRef, 4> getVariableNames(uint32_t offset) const;

  /// Reads out the parent function ID of the function whose lexical debug data
  /// starts at \p offset. \return the ID of the parent function, or None if
  /// none.
  OptValue<uint32_t> getParentFunctionId(uint32_t offset) const;

 private:
  /// Accessors for portions of data_, which looks like this:
  /// [sourceLocations][lexicalData]
  ///                  ^ lexicalDataOffset_

  /// \return the slice of data_ reflecting the source locations.
  llvh::ArrayRef<uint8_t> sourceLocationsData() const {
    return data_.getData().slice(0, lexicalDataOffset_);
  }

  /// \return the slice of data_ reflecting the lexical data.
  llvh::ArrayRef<uint8_t> lexicalData() const {
    return data_.getData().slice(lexicalDataOffset_);
  }

  void disassembleFilenames(llvh::raw_ostream &OS) const;
  void disassembleFilesAndOffsets(llvh::raw_ostream &OS) const;
  void disassembleLexicalData(llvh::raw_ostream &OS) const;

 public:
  void disassemble(llvh::raw_ostream &OS) const {
    disassembleFilenames(OS);
    disassembleFilesAndOffsets(OS);
    disassembleLexicalData(OS);
  }

#ifndef HERMESVM_LEAN
  /// Populate the given source map \p sourceMap with debug information.
  /// Each opcode with line and column information is mapped to its absolute
  /// offset in the bytecode file. To determine these absolute offsets, the
  /// functionOffsets parameter maps functions (indexed by their id) to their
  /// start position in the bytecode file.
  void populateSourceMap(
      SourceMapGenerator *sourceMap,
      std::vector<uint32_t> &&functionOffsets,
      uint32_t segmentID) const;
#endif
};

class DebugInfoGenerator {
 private:
  /// An offset into Debug Lexical Table pointing to a special entry,
  /// representing the most common value in the Debug Lexical Table
  /// (vars count: 0, lexical parent: none). When compiled without -g,
  /// this common value applies to all functions without local variables.
  /// This optimization reduces hbc bundle size.
  ///
  /// When compiled with -g, the lexical parent is none for the global
  /// function, but not any other functions. As a result, this optimization
  /// does not provide value.
  static constexpr uint32_t kEmptyLexicalDataOffset = 0;

  bool validData{true};

  /// Serialized source location data.
  std::vector<uint8_t> sourcesData_{};

  /// String storage for filenames.
  /// ConsecutiveStringStorage is not copy-constructible or copy-assignable.
  ConsecutiveStringStorage filenameStrings_;

  /// List of files mapping file ID to source location offsets.
  DebugInfo::DebugFileRegionList files_{};

  /// Serialized lexical data, which contains information about the variables
  /// associated with each code block.
  std::vector<uint8_t> lexicalData_;

  int32_t delta(uint32_t to, uint32_t from) {
    int64_t diff = (int64_t)to - from;
    // It's unlikely that lines or columns will ever jump from 0 to 3 billion,
    // but if it ever happens we can extend to 64bit types.
    assert(
        diff <= INT32_MAX && diff >= INT32_MIN &&
        "uint32_t delta too large when encoding debug info");
    return (int32_t)diff;
  }

  /// Appends a string \p str to the given \p data.
  /// This first appends the string's length, followed by the string bytes.
  static void appendString(std::vector<uint8_t> &data, llvh::StringRef str) {
    appendSignedLEB128(data, int64_t(str.size()));
    data.insert(data.end(), str.begin(), str.end());
  }

  /// No copy constructor or copy assignment operator.
  /// Note that filenameStrings_ is of type ConsecutiveStringStorage, which
  /// is not copy-constructible or copy-assignable.
  DebugInfoGenerator(const DebugInfoGenerator &) = delete;
  DebugInfoGenerator &operator=(const DebugInfoGenerator &) = delete;

 public:
  explicit DebugInfoGenerator(UniquingFilenameTable &&filenameTable);

  DebugInfoGenerator(DebugInfoGenerator &&) = default;

  uint32_t appendSourceLocations(
      const DebugSourceLocation &start,
      uint32_t functionIndex,
      llvh::ArrayRef<DebugSourceLocation> offsets);

  /// Append lexical data including parent function \p parentFunctionIndex and
  /// list of variable names \p names to the debug data. \return the offset in
  /// the lexical section of the debug data.
  uint32_t appendLexicalData(
      OptValue<uint32_t> parentFunctionIndex,
      llvh::ArrayRef<Identifier> names);

  // Destructively move memory to a DebugInfo.
  DebugInfo serializeWithMove();
};

} // namespace hbc
} // namespace hermes
#endif // HERMES_BCGEN_HBC_DEBUGINFO_H
