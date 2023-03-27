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
#include "llvh/ADT/DenseMap.h"
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
  static constexpr uint32_t NO_REG = UINT32_MAX;
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
  // The offset source-level scope descriptor that is "active" for this
  // location. May temporarily hold a relocation that's resolved right
  // before the source location is serialized.
  uint32_t scopeAddress{0};
  // The register holding the Environment with the active scope.
  uint32_t envReg{NO_REG};

  DebugSourceLocation() {}

  DebugSourceLocation(
      uint32_t address,
      uint32_t filenameId,
      uint32_t line,
      uint32_t column,
      uint32_t statement,
      uint32_t scopeAddress,
      uint32_t envReg)
      : address(address),
        filenameId(filenameId),
        line(line),
        column(column),
        statement(statement),
        scopeAddress(scopeAddress),
        envReg(envReg) {}

  bool operator==(const DebugSourceLocation &rhs) const {
    return address == rhs.address && filenameId == rhs.filenameId &&
        line == rhs.line && column == rhs.column &&
        statement == rhs.statement && scopeAddress == rhs.scopeAddress &&
        envReg == rhs.envReg;
  }

  bool operator!=(const DebugSourceLocation &rhs) const {
    return !(*this == rhs);
  }
};

/// A deserialized scope descriptor.
struct DebugScopeDescriptor {
  /// Various flags about this scope descriptor. See the Bits enum below for a
  /// description of each flag.
  struct Flags {
    /// Constructs a new Flags object with the given \p bits.
    explicit Flags(uint32_t bits = 0);

    /// Serializes this Flags object.
    uint32_t toUint32() const;

    /// This scope descriptor is an inner scope (i.e., a scope that's a child of
    /// a Function outermost scope).
    bool isInnerScope;

    /// This scope descriptor is dynamic (i.e., it is a loop's top level scope).
    bool isDynamic;

   private:
    enum class Bits {
      InnerScope,
      Dynamic,
    };
  };

  /// The offset into the scope descriptor debug info where the descriptor for
  /// this scope's parents is.
  OptValue<unsigned> parentOffset;

  Flags flags;

  /// The names for the variables in this scope.
  llvh::SmallVector<llvh::StringRef, 4> names;
};

/// The string representing a textual name for a call instruction's callee
/// argument.
struct DebugTextifiedCallee {
  // The bytecode offset of this debug info.
  uint32_t address{0};
  // A textual name for the function being called. Must be a valid UTF8 string.
  Identifier textifiedCallee;
};

/// A type wrapping up the offsets into debugging data.
struct DebugOffsets {
  /// Offsets into the debugging data of the source locations
  /// (DebugSourceLocation).
  uint32_t sourceLocations = NO_OFFSET;

  /// Offset into the scope descriptor data section of the debugging data.
  uint32_t scopeDescData = NO_OFFSET;

  /// Offset into the textified callee data section of the debugging data.
  uint32_t textifiedCallees = NO_OFFSET;

  /// Sentinel value indicating no offset.
  static constexpr uint32_t NO_OFFSET = UINT32_MAX;

  /// Constructors.
  DebugOffsets() = default;
  DebugOffsets(uint32_t src, uint32_t scopeDesc, uint32_t tCallee)
      : sourceLocations(src),
        scopeDescData(scopeDesc),
        textifiedCallees(tCallee) {}
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
  uint32_t scopeDescDataOffset_ = 0;
  uint32_t textifiedCalleeOffset_ = 0;
  uint32_t stringTableOffset_ = 0;
  StreamVector<uint8_t> data_{};

  /// Get source filename as string id.
  OptValue<uint32_t> getFilenameForAddress(uint32_t debugOffset) const;

  /// Decodes a string at offset \p offset in \p data, updating offset in-place.
  /// \return the decoded string.
  llvh::StringRef decodeString(
      uint32_t *inoutOffset,
      llvh::ArrayRef<uint8_t> data) const;

 public:
  explicit DebugInfo() = default;
  /*implicit*/ DebugInfo(DebugInfo &&that) = default;

  explicit DebugInfo(
      ConsecutiveStringStorage &&filenameStrings,
      DebugFileRegionList &&files,
      uint32_t scopeDescDataOffset,
      uint32_t textifiedCalleeOffset,
      uint32_t stringTableOffset,
      StreamVector<uint8_t> &&data)
      : filenameTable_(filenameStrings.acquireStringTable()),
        filenameStorage_(filenameStrings.acquireStringStorage()),
        files_(std::move(files)),
        scopeDescDataOffset_(scopeDescDataOffset),
        textifiedCalleeOffset_(textifiedCalleeOffset),
        stringTableOffset_(stringTableOffset),
        data_(std::move(data)) {}

  explicit DebugInfo(
      std::vector<StringTableEntry> &&filenameStrings,
      std::vector<unsigned char> &&filenameStorage,
      DebugFileRegionList &&files,
      uint32_t scopeDescDataOffset,
      uint32_t textifiedCalleeOffset,
      uint32_t stringTableOffset,
      StreamVector<uint8_t> &&data)
      : filenameTable_(std::move(filenameStrings)),
        filenameStorage_(std::move(filenameStorage)),
        files_(std::move(files)),
        scopeDescDataOffset_(scopeDescDataOffset),
        textifiedCalleeOffset_(textifiedCalleeOffset),
        stringTableOffset_(stringTableOffset),
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

  uint32_t scopeDescDataOffset() const {
    return scopeDescDataOffset_;
  }

  uint32_t textifiedCalleeOffset() const {
    return textifiedCalleeOffset_;
  }

  uint32_t stringTableOffset() const {
    return stringTableOffset_;
  }

  /// Get the location of \p offsetInFunction, given the function's debug
  /// offset.
  OptValue<DebugSourceLocation> getLocationForAddress(
      uint32_t debugOffset,
      uint32_t offsetInFunction) const;

  /// \return the name of the textified callee for the function called in the
  /// given \p offsetInFunction. Encoding is UTF8.
  OptValue<llvh::StringRef> getTextifiedCalleeUTF8(
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

  /// Read the variable names at \p offset into the scope descriptor section
  /// of the debug info. \return the list of variable names.
  DebugScopeDescriptor getScopeDescriptor(uint32_t offset) const;

  /// Reads out the parent function ID of the function whose lexical debug data
  /// starts at \p offset. \return the ID of the parent function, or None if
  /// none.
  OptValue<uint32_t> getParentFunctionId(uint32_t offset) const;

  /// \return the size in bytes of the source locations data.
  uint32_t getSourceLocationsDataSizeBytes() const {
    return scopeDescDataOffset_ - 0;
  }

  /// \return the size in bytes of the scope desc table data.
  uint32_t getScopeDescDataSizeBytes() const {
    return textifiedCalleeOffset_ - scopeDescDataOffset_;
  }

  /// \return the size in bytes of the textified callee data.
  uint32_t getTextifiedCalleesDataSizeBytes() const {
    return stringTableOffset_ - textifiedCalleeOffset_;
  }

  /// \return the size in bytes of the string table data.
  uint32_t getStringTableSizeBytes() const {
    return data_.size() - stringTableOffset_;
  }

 private:
  // clang-format off
  /// Accessors for portions of data_, which looks like this:
  /// [sourceLocations][scopeDescData][textifiedCallee][stringTable]
  ///                  |              |                ^ stringTableOffset_
  ///                  |              ^ textifiedCalleeOffset_
  ///                  ^ scopeDescDataOffset_
  // clang-format on

  /// \return the slice of data_ reflecting the source locations.
  llvh::ArrayRef<uint8_t> sourceLocationsData() const {
    return data_.getData().slice(0, getSourceLocationsDataSizeBytes());
  }

  /// \return the slice of data_ reflecting the scope desc data
  llvh::ArrayRef<uint8_t> scopeDescData() const {
    return data_.getData().slice(
        scopeDescDataOffset_, getScopeDescDataSizeBytes());
  }

  /// \return the slice of data_ reflecting the textified callee table.
  llvh::ArrayRef<uint8_t> textifiedCalleeData() const {
    return data_.getData().slice(
        textifiedCalleeOffset_, getTextifiedCalleesDataSizeBytes());
  }

  /// \return the slice of data_ reflecting the string table data.
  llvh::ArrayRef<uint8_t> stringTableData() const {
    return data_.getData().slice(stringTableOffset_);
  }

  void disassembleFilenames(llvh::raw_ostream &OS) const;
  void disassembleFilesAndOffsets(llvh::raw_ostream &OS) const;
  void disassembleScopeDescData(llvh::raw_ostream &OS) const;
  void disassembleTextifiedCallee(llvh::raw_ostream &OS) const;
  void disassembleStringTable(llvh::raw_ostream &OS) const;

 public:
  void disassemble(llvh::raw_ostream &OS) const {
    disassembleFilenames(OS);
    disassembleFilesAndOffsets(OS);
    disassembleScopeDescData(OS);
    disassembleTextifiedCallee(OS);
    disassembleStringTable(OS);
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
  /// A special offset for representing the most common entry in its table.
  ///
  /// For Scope Desc Table, it represents the most common info (vars count: 0,
  /// lexical parent: none). When compiled without -g, this common value applies
  /// to all functions without local variables. This optimization reduces hbc
  /// bundle size; When compiled with -g, the lexical parent is none for the
  /// global function, but not any other functions. As a result, this
  /// optimization does not provide value.
  ///
  /// For textified callee table, it represents an empty table.
  static constexpr uint32_t kMostCommonEntryOffset = 0;

  bool validData{true};

  /// Serialized source location data.
  std::vector<uint8_t> sourcesData_{};

  /// String storage for filenames.
  /// ConsecutiveStringStorage is not copy-constructible or copy-assignable.
  ConsecutiveStringStorage filenameStrings_;

  /// List of files mapping file ID to source location offsets.
  DebugInfo::DebugFileRegionList files_{};

  /// Serialized scope descriptors.
  std::vector<uint8_t> scopeDescData_;

  /// Serialized textified callee table.
  std::vector<uint8_t> textifiedCallees_;

  /// The debug info string table. All string entries in the debug info records
  /// point to an entry in this table. Strings are encoded as size-prefixed,
  /// UTF8-encoded payloads.
  std::vector<uint8_t> stringTable_;

  /// An index for strings in stringTable_.
  llvh::DenseMap<UniqueString *, uint32_t> stringTableIndex_;

  int32_t delta(uint32_t to, uint32_t from) {
    int64_t diff = (int64_t)to - from;
    // It's unlikely that lines or columns will ever jump from 0 to 3 billion,
    // but if it ever happens we can extend to 64bit types.
    assert(
        diff <= INT32_MAX && diff >= INT32_MIN &&
        "uint32_t delta too large when encoding debug info");
    return (int32_t)diff;
  }

  /// Appends \p str to stringTable_ if not already present, then
  /// appends \p str's offset in stringTable_ to the given \p data.
  void appendString(std::vector<uint8_t> &data, Identifier str);

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

  /// Appends a scope descriptor with the given \p parentScopeID and
  /// \p names to the scope descriptor table.
  ///
  /// \p names is the list of variables that live in the scope.
  ///
  /// \p flags contains the flags for \p names. See DebugScopeDescriptor::Flags.
  ///
  /// \returns the offset of the new scope descriptor in the table.
  uint32_t appendScopeDesc(
      OptValue<uint32_t> parentScopeID,
      DebugScopeDescriptor::Flags flags,
      llvh::ArrayRef<Identifier> names);

  /// Append the textified callee data to the debug data. \return the offset in
  /// the textified callee table of the debug data.
  uint32_t appendTextifiedCalleeData(
      llvh::ArrayRef<DebugTextifiedCallee> textifiedCallees);

  // Destructively move memory to a DebugInfo.
  DebugInfo serializeWithMove();
};

} // namespace hbc
} // namespace hermes
#endif // HERMES_BCGEN_HBC_DEBUGINFO_H
