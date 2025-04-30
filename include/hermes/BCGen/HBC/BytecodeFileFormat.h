/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEFILEFORMAT_H
#define HERMES_BCGEN_HBC_BYTECODEFILEFORMAT_H

#include "hermes/BCGen/FunctionInfo.h"
#include "hermes/BCGen/HBC/BytecodeVersion.h"
#include "hermes/BCGen/HBC/StringKind.h"
#include "hermes/BCGen/ShapeTableEntry.h"
#include "hermes/Regex/RegexSerialization.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/BitField.h"
#include "hermes/Support/Compiler.h"
#include "hermes/Support/SHA1.h"
#include "hermes/Support/StringTableEntry.h"

#include <cassert>
#include <cstdint>
#include <cstring>

namespace hermes {
namespace hbc {

// "Hermes" in ancient Greek encoded in UTF-16BE and truncated to 8 bytes.
const static uint64_t MAGIC = 0x1F1903C103BC1FC6;

// The "delta prepped" form: a different magic number indicating that the
// bytecode file is in a form suitable for delta diffing, not execution.
const static uint64_t DELTA_MAGIC = ~MAGIC;

/// Property cache index which indicates no caching.
static constexpr uint8_t PROPERTY_CACHING_DISABLED =
    std::numeric_limits<uint8_t>::max();

/// Alignment of data structures of in file.
static constexpr size_t BYTECODE_ALIGNMENT = alignof(uint32_t);

/// Bytecode forms
enum class BytecodeForm {
  /// Execution form (the default) is the bytecode prepared for execution.
  Execution,

  /// Delta form is the bytecode prepared to minimize binary diff size.
  Delta,
};

/// Storing information about the bytecode, needed when it is loaded by the
/// runtime.
struct BytecodeOptions {
  HERMES_FIRST_BITFIELD(uint8_t, flags, bool, StaticBuiltins, 1);
  HERMES_NEXT_BITFIELD(
      StaticBuiltins,
      flags,
      bool,
      CjsModulesStaticallyResolved,
      1);

  BytecodeOptions() : _flags(0) {}
};

// See BytecodeFileFormatTest for details about bit field layouts
static_assert(
    sizeof(BytecodeOptions) == 1,
    "BytecodeOptions should take up 1 byte total");

/**
 * Header of binary file.
 */
LLVM_PACKED_START
struct BytecodeFileHeader {
  uint64_t magic;
  uint32_t version;
  uint8_t sourceHash[SHA1_NUM_BYTES];
  uint32_t fileLength; // Until the end of the BytecodeFileFooter.
  uint32_t globalCodeIndex;
  uint32_t functionCount;
  uint32_t stringKindCount; // Number of string kind entries.
  uint32_t identifierCount; // Number of strings which are identifiers.
  uint32_t stringCount; // Number of strings in the string table.
  uint32_t overflowStringCount; // Number of strings in the overflow table.
  uint32_t stringStorageSize; // Bytes in the blob of string contents.
  uint32_t bigIntCount; // number of bigints in the bigint table.
  uint32_t bigIntStorageSize; // Bytes in the bigint table.
  uint32_t regExpCount;
  uint32_t regExpStorageSize;
  uint32_t literalValueBufferSize;
  uint32_t objKeyBufferSize;
  uint32_t objShapeTableCount; // Number of elements in the shape table.
  uint32_t segmentID; // The ID of this segment.
  uint32_t cjsModuleCount; // Number of modules.
  uint32_t functionSourceCount; // Number of function sources preserved.
  uint32_t debugInfoOffset;
  BytecodeOptions options;

  // Insert any padding to make function headers that follow this file header
  // less likely to cross cache lines.
  uint8_t padding[19];

  BytecodeFileHeader(
      uint64_t magic,
      uint32_t version,
      const SHA1 &sourceHash,
      uint32_t fileLength,
      uint32_t globalCodeIndex,
      uint32_t functionCount,
      uint32_t stringKindCount,
      uint32_t identifierCount,
      uint32_t stringCount,
      uint32_t overflowStringCount,
      uint32_t stringStorageSize,
      uint32_t bigIntCount,
      uint32_t bigIntStorageSize,
      uint32_t regExpCount,
      uint32_t regExpStorageSize,
      uint32_t literalValueBufferSize,
      uint32_t objKeyBufferSize,
      uint32_t objShapeTableCount,
      uint32_t segmentID,
      uint32_t cjsModuleCount,
      uint32_t functionSourceCount,
      uint32_t debugInfoOffset,
      BytecodeOptions options)
      : magic(magic),
        version(version),
        sourceHash(),
        fileLength(fileLength),
        globalCodeIndex(globalCodeIndex),
        functionCount(functionCount),
        stringKindCount(stringKindCount),
        identifierCount(identifierCount),
        stringCount(stringCount),
        overflowStringCount(overflowStringCount),
        stringStorageSize(stringStorageSize),
        bigIntCount(bigIntCount),
        bigIntStorageSize(bigIntStorageSize),
        regExpCount(regExpCount),
        regExpStorageSize(regExpStorageSize),
        literalValueBufferSize(literalValueBufferSize),
        objKeyBufferSize(objKeyBufferSize),
        objShapeTableCount(objShapeTableCount),
        segmentID(segmentID),
        cjsModuleCount(cjsModuleCount),
        functionSourceCount(functionSourceCount),
        debugInfoOffset(debugInfoOffset),
        options(options) {
    std::copy(sourceHash.begin(), sourceHash.end(), this->sourceHash);
    std::fill(padding, padding + sizeof(padding), 0);
  }
};

/**
 * Footer of binary file. Used for summary information that is *not*
 * read during normal execution (since that would hurt locality).
 */
struct BytecodeFileFooter {
  uint8_t fileHash[SHA1_NUM_BYTES]; // Hash of everything above the footer.

  // NOTE: If we ever add any non-byte fields, we need to ensure alignment
  // everywhere this struct is written.

  BytecodeFileFooter(const SHA1 &fileHash) {
    std::copy(fileHash.begin(), fileHash.end(), this->fileHash);
  }
};

/// The string table is an array of these entries, followed by an array of
/// OverflowStringTableEntry for the entries whose length or offset doesn't fit
/// into the bitfields.
struct SmallStringTableEntry {
  HERMES_FIRST_BITFIELD(uint32_t, data, bool, IsUTF16, 1);
  HERMES_NEXT_BITFIELD(IsUTF16, data, uint32_t, Offset, 23);
  HERMES_NEXT_BITFIELD(Offset, data, uint32_t, Length, 8);

  static constexpr uint32_t INVALID_OFFSET = (1 << 23);
  static constexpr uint32_t INVALID_LENGTH = (1 << 8) - 1;

  bool isOverflowed() const {
    return getLength() == INVALID_LENGTH;
  }

  /// Construct a small entry from 'entry'. If any fields overflow, then set
  /// 'overflowOffset' as the offset instead.
  SmallStringTableEntry(
      const StringTableEntry &entry,
      uint32_t overflowOffset) {
    setIsUTF16(entry.isUTF16());
    if (entry.getOffset() < INVALID_OFFSET &&
        entry.getLength() < INVALID_LENGTH) {
      setOffset(entry.getOffset());
      setLength(entry.getLength());
    } else {
      assert(overflowOffset < INVALID_OFFSET);
      setOffset(overflowOffset);
      setLength(INVALID_LENGTH);
    }
  }
};

// See BytecodeFileFormatTest for details about bit field layouts
static_assert(
    sizeof(SmallStringTableEntry) == 4,
    "SmallStringTableEntry should take up 4 bytes total");

/// These are indexed by the 'offset' field of overflowed SmallStringTableEntry.
struct OverflowStringTableEntry {
  uint32_t offset;
  uint32_t length;

  OverflowStringTableEntry(uint32_t offset, uint32_t length)
      : offset(offset), length(length) {}
};

struct FunctionHeaderFlag {
  /// Which kinds of calls are prohibited, constructed from enum
  /// ProhibitInvoke.
  HERMES_FIRST_BITFIELD(uint8_t, flags, uint8_t, ProhibitInvoke, 2);
  HERMES_NEXT_BITFIELD(ProhibitInvoke, flags, bool, StrictMode, 1);
  HERMES_NEXT_BITFIELD(StrictMode, flags, bool, HasExceptionHandler, 1);
  HERMES_NEXT_BITFIELD(HasExceptionHandler, flags, bool, HasDebugInfo, 1);
  HERMES_NEXT_BITFIELD(HasDebugInfo, flags, bool, Overflowed, 1);
  HERMES_NEXT_BITFIELD(Overflowed, flags, uint8_t, Kind, 2);

  FunctionHeaderFlag() {
    _flags = 0;
    setKind(FuncKind::Normal);
    setProhibitInvoke(ProhibitInvoke::None);
  }

  /// \return true if the specified kind of invocation is prohibited by the
  /// flags.
  bool isCallProhibited(bool construct) const {
    return getProhibitInvoke() == (uint8_t)construct;
  }
};

// See BytecodeFileFormatTest for details about bit field layouts
static_assert(
    sizeof(FunctionHeaderFlag) == 1,
    "FunctionHeaderFlag should take up 1 byte total");

/// FUNC_HEADER_FIELDS is a macro for defining function header fields. It
/// takes two macros as arguments:
///   F(storageType, storageName, apiType, name, bits)
/// for the first field in a "word", and
///   N(prevField, storageName, apiType, name, bits)
/// for the following fields.
#define FUNC_HEADER_FIELDS(F, N)                        \
  /* first word */                                      \
  F(uint32_t, w1, uint32_t, Offset, 25)                 \
  N(Offset, w1, uint32_t, ParamCount, 5)                \
  N(ParamCount, w1, uint32_t, LoopDepth, 2)             \
  /* second word */                                     \
  F(uint32_t, w2, uint32_t, BytecodeSizeInBytes, 14)    \
  N(BytecodeSizeInBytes, w2, uint32_t, FunctionName, 8) \
  N(FunctionName, w2, uint32_t, NumberRegCount, 5)      \
  N(NumberRegCount, w2, uint32_t, NonPtrRegCount, 5)    \
  /* third word, with flags below */                    \
  F(uint8_t, b1, uint32_t, FrameSize, 8)                \
  F(uint8_t, b2, uint8_t, ReadCacheSize, 8)             \
  F(uint8_t, b3, uint8_t, WriteCacheSize, 6)            \
  N(WriteCacheSize, b3, uint8_t, NumCacheNewObject, 1)  \
  N(NumCacheNewObject, b3, uint8_t, PrivateNameCacheSize, 1)

/**
 * Metadata of a function.
 */
struct FunctionHeader {
// Use api type here since FunctionHeader stores the full type.
#define DECLARE_FIELD(_, storage, api_type, name, bits) \
  api_type _##name;                                     \
  api_type get##name() const {                          \
    return _##name;                                     \
  }                                                     \
  void set##name(api_type value) {                      \
    _##name = value;                                    \
  }
  FUNC_HEADER_FIELDS(DECLARE_FIELD, DECLARE_FIELD)
#undef DECLARE_FIELD

  FunctionHeaderFlag flags{};

 public:
  FunctionHeader(
      uint32_t size,
      uint32_t paramCount,
      uint32_t loopDepth,
      uint32_t frameSize,
      uint32_t numberRegCount,
      uint32_t nonPtrRegCount,
      uint32_t functionNameID,
      uint8_t readCacheSize,
      uint8_t writeCacheSize,
      uint8_t numCacheNewObject,
      uint8_t privateCacheSize) {
    setOffset(0);
    setParamCount(paramCount);
    setLoopDepth(loopDepth);
    setBytecodeSizeInBytes(size);
    setFunctionName(functionNameID);
    setNumberRegCount(numberRegCount);
    setNonPtrRegCount(nonPtrRegCount);
    setFrameSize(frameSize);
    setReadCacheSize(readCacheSize);
    setWriteCacheSize(writeCacheSize);
    setNumCacheNewObject(numCacheNewObject);
    setPrivateNameCacheSize(privateCacheSize);
  }
};

/// Compact version of FunctionHeader. Fits most functions.
/// Has two possible states, indicated by 'overflowed' flag:
/// !overflowed: all fields are valid.
/// overflowed: only getLargeHeaderOffset() is valid, and at the
///             latter is a FunctionHeader.
/// Note that msvc and compatible compilers will not put bitfields
/// of the same type in the same memory, so don't mix uint8_t and
/// uint32_t if you want them packed next to each other.

struct SmallFuncHeader {
  // Use the store_type since SmallFuncHeader attempts to minimize storage.
  FUNC_HEADER_FIELDS(HERMES_FIRST_BITFIELD, HERMES_NEXT_BITFIELD)

  FunctionHeaderFlag flags{};

#define M(storageTypeOrPrevField, storageName, apiType, name, bits) \
  static constexpr apiType name##Max =                              \
      std::numeric_limits<apiType>::max() >> (sizeof(apiType) * 8 - bits);
  FUNC_HEADER_FIELDS(M, M)
#undef M

  /// Make a small header equivalent to \p large, which is known to fit.
  SmallFuncHeader(const FunctionHeader &large) {
    assert(canFitInSmallHeader(large) && "header does not fit");
    std::memset(this, 0, sizeof(SmallFuncHeader)); // Avoid leaking junk.

#define COPY_FIELD(_, storage, api_type, name, bits) \
  set##name(large.get##name());
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
    FUNC_HEADER_FIELDS(COPY_FIELD, COPY_FIELD)
#pragma GCC diagnostic pop
#undef COPY_FIELD

    flags = large.flags;
  }

  /// Make a small header that stores an offset to a full size FunctionHeader at
  /// \p largeHeaderOffset.
  SmallFuncHeader(uint32_t largeHeaderOffset) {
    std::memset(this, 0, sizeof(SmallFuncHeader)); // Avoid leaking junk.
    flags.setOverflowed(true);
    // Can use any fields to store the large offset; pick two big ones.
    setOffset(largeHeaderOffset & 0xffffff);
    setFunctionName((largeHeaderOffset >> 24) & 0xff);
    assert(
        getLargeHeaderOffset() == largeHeaderOffset &&
        "offset encoding is wrong");
  }

  /// Check if the fields in \p large will fit in a small header.
  /// \return true if it will fit, or false if any of the fields would overflow.
  static bool canFitInSmallHeader(const FunctionHeader &large) {
#define CHECK_FIELD(_, storage, api_type, name, bits) \
  if (large.get##name() > (1U << bits) - 1)           \
    return false;
    FUNC_HEADER_FIELDS(CHECK_FIELD, CHECK_FIELD)
#undef CHECK_FIELD
    return true;
  }

  /// Get the offset of the large header from a small header that is known to
  /// have overflowed.
  uint32_t getLargeHeaderOffset() const {
    assert(flags.getOverflowed());
    return (getFunctionName() << 24) | getOffset();
  }
};

static_assert(
    sizeof(SmallFuncHeader) == 12,
    "SmallFuncHeader should take up 12 bytes total");

// Sizes of file headers are tuned for good cache line packing.
// If you change their size, try to avoid headers crossing cache lines.
static_assert(
    sizeof(BytecodeFileHeader) % 32 == 0,
    "BytecodeFileHeader size should be cache friendly");

struct ExceptionHandlerTableHeader {
  uint32_t count;
};

/// We need HBCExceptionHandlerInfo other than using ExceptionHandlerInfo
/// directly because we don't need depth in HBC.
struct HBCExceptionHandlerInfo {
  /// Start offset of the try, inclusive.
  uint32_t start;
  /// End offset of the try, exclusive.
  uint32_t end;
  /// Handler offset.
  uint32_t target;
};

// The size of the file table and debug data.
struct DebugInfoHeader {
  // Number of filenames stored in the table.
  uint32_t filenameCount;
  // Bytes in the filename storage contents.
  uint32_t filenameStorageSize;

  // Count of the file table.
  uint32_t fileRegionCount;
  // Byte offset in the debug data for the lexical data.
  uint32_t lexicalDataOffset;
  // Size in bytes of the debug data.
  uint32_t debugDataSize;
};

// The string id of files for given offsets in debug info.
struct DebugFileRegion {
  uint32_t fromAddress;
  uint32_t filenameId;
  uint32_t sourceMappingUrlId;
};

LLVM_PACKED_END

/// Visit each segment in a bytecode file in order.
/// This function defines the order of the bytecode file segments.
template <typename Visitor>
void visitBytecodeSegmentsInOrder(Visitor &visitor) {
  visitor.visitFunctionHeaders();
  visitor.visitStringKinds();
  visitor.visitIdentifierHashes();
  visitor.visitSmallStringTable();
  visitor.visitOverflowStringTable();
  visitor.visitStringStorage();
  visitor.visitLiteralValueBuffer();
  visitor.visitObjectKeyBuffer();
  visitor.visitObjectShapeTable();
  visitor.visitBigIntTable();
  visitor.visitBigIntStorage();
  visitor.visitRegExpTable();
  visitor.visitRegExpStorage();
  visitor.visitCJSModuleTable();
  visitor.visitFunctionSourceTable();
}

/// BytecodeFileFields represents direct byte-level access to the structured
/// fields of a bytecode file, providing pointers and ArrayRefs referencing
/// directly into the buffer. Note some portions of the bytecode file are less
/// structured, such as the function info section; these are not exposed here.
/// Most clients will want to use an immutable BytecodeFileFields, which may be
/// initialized from a read-only buffer. Tools that want to modify the fields
/// in-place may initialize with Mutable=true.
template <bool Mutable>
struct BytecodeFileFields {
  template <typename T>
  using Pointer = typename std::conditional<Mutable, T *, const T *>::type;

  template <typename T>
  using Array = typename std::
      conditional<Mutable, llvh::MutableArrayRef<T>, llvh::ArrayRef<T>>::type;

  /// The file header.
  Pointer<BytecodeFileHeader> header{nullptr};

  /// List of function headers. Some of these may be overflow headers.
  Array<hbc::SmallFuncHeader> functionHeaders;

  /// The list of short string table entries.
  Array<hbc::SmallStringTableEntry> stringTableEntries{};

  /// Table for object shapes.
  Array<ShapeTableEntry> objShapeTable;

  /// Run-length encoding representing the kinds of strings in the table.
  Array<StringKind::Entry> stringKinds{};

  /// The list of identifier hashes.
  Array<uint32_t> identifierHashes{};

  /// The list of overflowed string table entries.
  Array<hbc::OverflowStringTableEntry> stringTableOverflowEntries{};

  /// The character buffer used for string storage.
  Array<uint8_t> stringStorage;

  /// Buffer for array/object value literals.
  Array<uint8_t> literalValueBuffer;

  /// Buffer for object keys.
  Array<uint8_t> objKeyBuffer;

  /// List of bigint literals.
  Array<bigint::BigIntTableEntry> bigIntTable;

  /// Storage for bigint bytecode.
  Array<uint8_t> bigIntStorage;

  /// List of regexp literals.
  Array<RegExpTableEntry> regExpTable;

  /// Storage for regexp bytecode.
  Array<uint8_t> regExpStorage;

  /// List of CJS modules.
  Array<std::pair<uint32_t, uint32_t>> cjsModuleTable;

  /// List of resolved CJS modules.
  Array<std::pair<uint32_t, uint32_t>> cjsModuleTableStatic;

  /// List of function source table entries.
  Array<std::pair<uint32_t, uint32_t>> functionSourceTable;

  /// Populate bytecode file fields from a buffer. The fields will point
  /// directly into the buffer and it is the caller's responsibility to ensure
  /// the result does not outlive the buffer.
  /// \p form contains the expected bytecode form (Execution or Delta).
  /// \return true on success, false on
  /// failure, in which case an error is returned by reference.
  bool populateFromBuffer(
      Array<uint8_t> bytes,
      std::string *outError,
      BytecodeForm form = BytecodeForm::Execution);
};

using ConstBytecodeFileFields = BytecodeFileFields<false>;
using MutableBytecodeFileFields = BytecodeFileFields<true>;

} // namespace hbc
} // namespace hermes

#endif // HERMES_BCGEN_HBC_BYTECODEFILEFORMAT_H
