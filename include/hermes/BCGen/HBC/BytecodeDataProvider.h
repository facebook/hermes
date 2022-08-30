/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_BCGEN_HBC_BYTECODEDATAPROVIDER_H
#define HERMES_BCGEN_HBC_BYTECODEDATAPROVIDER_H

#include "hermes/BCGen/HBC/BytecodeFileFormat.h"
#include "hermes/BCGen/HBC/DebugInfo.h"
#include "hermes/Public/Buffer.h"
#include "hermes/SourceMap/SourceMapGenerator.h"
#include "hermes/Support/BigIntSupport.h"
#include "hermes/Support/OSCompat.h"
#include "hermes/Support/PageAccessTracker.h"
#include "hermes/Support/RegExpSerialization.h"
#include "hermes/Support/StringTableEntry.h"

#include "llvh/ADT/ArrayRef.h"

#include <atomic>
#include <thread>

#pragma GCC diagnostic push

#ifdef HERMES_COMPILER_SUPPORTS_WSHORTEN_64_TO_32
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif

namespace hermes {
namespace hbc {
class BCProviderBase;
class BCProviderFromBuffer;

// Making BCProvider an alias to BCProviderFromBuffer when eval is disabled
// eliminates the cost of virtual function calls and allows for inlining.
#ifdef HERMESVM_LEAN
using BCProvider = BCProviderFromBuffer;
#else
using BCProvider = BCProviderBase;
#endif

using StringID = uint32_t;

/// Runtime reference to a function header. Values of this class should
/// be used in the VM to reference (not own) function headers.
class RuntimeFunctionHeader {
  /// Tagged 'ptr_' refers to a large header.
  const char *ptr_;

 public:
  explicit RuntimeFunctionHeader(const hbc::SmallFuncHeader *smallHeader)
      : ptr_(reinterpret_cast<const char *>(smallHeader)) {
    assert(!isLarge());
  }
  explicit RuntimeFunctionHeader(const hbc::FunctionHeader *large)
      : ptr_(reinterpret_cast<const char *>(large) + 1) {
    assert(isLarge());
  }

#define HEADER_FIELD_ACCESSOR(api_type, store_type, name, bits) \
  api_type name() const {                                       \
    if (LLVM_UNLIKELY(isLarge()))                               \
      return asLarge()->name;                                   \
    else                                                        \
      return asSmall()->name;                                   \
  }
  FUNC_HEADER_FIELDS(HEADER_FIELD_ACCESSOR)
  HEADER_FIELD_ACCESSOR(
      hbc::FunctionHeaderFlag,
      hbc::FunctionHeaderFlag,
      flags,
      8)
#undef HEADER_FIELD_ACCESSOR

 private:
  bool isLarge() const {
    return reinterpret_cast<uintptr_t>(ptr_) & 1;
  }

  const hbc::SmallFuncHeader *asSmall() const {
    assert(!isLarge());
    return reinterpret_cast<const hbc::SmallFuncHeader *>(ptr_);
  }

  const hbc::FunctionHeader *asLarge() const {
    assert(isLarge());
    return reinterpret_cast<const hbc::FunctionHeader *>(ptr_ - 1);
  }
};

static_assert(
    std::is_trivially_copyable<RuntimeFunctionHeader>::value,
    "RuntimeFunctionHeader should be trivially copyable");

/// Base class designed to provide bytecode data. We use this class
/// to abstract different ways of constructing bytecode from different
/// code paths: eval vs bytecode file. The design goal is to make the
/// code path of loading from bytecode file more efficient.
class BCProviderBase {
 protected:
  /// Storing information about the bytecode, needed when it is loaded by the
  /// runtime.
  BytecodeOptions options_{};

  /// Number of functions.
  uint32_t functionCount_{};

  /// Global functionID.
  uint32_t globalFunctionIndex_{};

  /// Below are all the different global data tables/storage.
  uint32_t stringCount_{};
  llvh::ArrayRef<StringKind::Entry> stringKinds_{};
  llvh::ArrayRef<uint32_t> identifierHashes_{};
  llvh::ArrayRef<unsigned char> stringStorage_{};

  llvh::ArrayRef<unsigned char> arrayBuffer_{};
  llvh::ArrayRef<unsigned char> objKeyBuffer_{};
  llvh::ArrayRef<unsigned char> objValueBuffer_{};

  llvh::ArrayRef<bigint::BigIntTableEntry> bigIntTable_{};
  llvh::ArrayRef<unsigned char> bigIntStorage_{};

  llvh::ArrayRef<RegExpTableEntry> regExpTable_{};
  llvh::ArrayRef<unsigned char> regExpStorage_{};

  /// The segment ID corresponding to the bytecode module.
  uint32_t segmentID_;

  /// Table which indicates where to find the different CommonJS modules.
  /// List of unsorted pairs from {filename ID => function index}.
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> cjsModuleTable_{};

  /// Table which indicates where to find the different CommonJS modules.
  /// List of unsorted pairs from {global module ID => function index}.
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> cjsModuleTableStatic_{};

  /// Table which indicates where to find the source string of functions.
  /// List of unsorted pairs from {function ID => string offset}.
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> functionSourceTable_{};

  /// Pointer to the global debug info. This will not be eagerly initialized
  /// when loading bytecode from a buffer. Instead it will be constructed
  /// when first needed. Most likely we should never need to use it.
  const hbc::DebugInfo *debugInfo_{};

  /// Error message when there is an error parsing the bytecode.
  /// We can use this to throw an exception to JSI.
  std::string errstr_{};

  /// Create the global debug info data, called only when first time needed.
  virtual void createDebugInfo() = 0;

 public:
  /// Getters for every private data member.
  BytecodeOptions getBytecodeOptions() const {
    return options_;
  }
  uint32_t getFunctionCount() const {
    return functionCount_;
  }
  uint32_t getGlobalFunctionIndex() const {
    return globalFunctionIndex_;
  }
  uint32_t getStringCount() const {
    return stringCount_;
  }
  uint32_t getBigIntCount() const {
    return bigIntTable_.size();
  }
  llvh::ArrayRef<StringKind::Entry> getStringKinds() const {
    return stringKinds_;
  }
  llvh::ArrayRef<uint32_t> getIdentifierHashes() const {
    return identifierHashes_;
  }
  llvh::ArrayRef<unsigned char> getStringStorage() const {
    return stringStorage_;
  }
  llvh::ArrayRef<unsigned char> getArrayBuffer() const {
    return arrayBuffer_;
  }
  llvh::ArrayRef<unsigned char> getObjectKeyBuffer() const {
    return objKeyBuffer_;
  }
  llvh::ArrayRef<unsigned char> getObjectValueBuffer() const {
    return objValueBuffer_;
  }
  llvh::ArrayRef<bigint::BigIntTableEntry> getBigIntTable() const {
    return bigIntTable_;
  }
  llvh::ArrayRef<unsigned char> getBigIntStorage() const {
    return bigIntStorage_;
  }
  llvh::ArrayRef<RegExpTableEntry> getRegExpTable() const {
    return regExpTable_;
  }
  llvh::ArrayRef<unsigned char> getRegExpStorage() const {
    return regExpStorage_;
  }
  uint32_t getSegmentID() const {
    return segmentID_;
  }
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTable() const {
    return cjsModuleTable_;
  }
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getCJSModuleTableStatic()
      const {
    return cjsModuleTableStatic_;
  }
  llvh::ArrayRef<std::pair<uint32_t, uint32_t>> getFunctionSourceTable() const {
    return functionSourceTable_;
  }
  const std::string getErrorStr() const {
    return errstr_;
  }

  virtual StringTableEntry getStringTableEntry(uint32_t index) const = 0;

  llvh::StringRef getStringRefFromID(StringID stringID) const {
    auto entry = getStringTableEntry(stringID);
    return llvh::StringRef(
        (const char *)getStringStorage().begin() + entry.getOffset(),
        entry.getLength());
  }

  /// Get the global debug info, lazily create it.
  const hbc::DebugInfo *getDebugInfo() const {
    if (!debugInfo_) {
      const_cast<BCProviderBase *>(this)->createDebugInfo();
    }
    return debugInfo_;
  }

  /// Get any trailing data after the real bytecode (only possible for buffers).
  virtual llvh::ArrayRef<uint8_t> getEpilogue() const {
    return llvh::ArrayRef<uint8_t>();
  }

  /// Get the hash of the source code that produced this bytecode.
  virtual SHA1 getSourceHash() const {
    return SHA1{};
  }

  /// Get a pointer to the function header.
  virtual RuntimeFunctionHeader getFunctionHeader(
      uint32_t functionID) const = 0;

  /// Get a pointer to the bytecode stream.
  virtual const uint8_t *getBytecode(uint32_t functionID) const = 0;

  /// Get the exception table for a given function with \p functionID.
  virtual llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t functionID) const = 0;

  /// Get the debug offsets for a given function with \p functionID.
  virtual const hbc::DebugOffsets *getDebugOffsets(
      uint32_t functionID) const = 0;

  /// Get the source text location of address \p offsetInFunction in funciton
  /// \p funcId.
  llvh::Optional<SourceMapTextLocation> getLocationForAddress(
      uint32_t funcId,
      uint32_t offsetInFunction) const;

  virtual ~BCProviderBase() = default;

  /// Check whether a function with \p functionID is lazy.
  virtual bool isFunctionLazy(uint32_t functionID) const = 0;

  /// Check whether the whole data provider is lazy.
  virtual bool isLazy() const = 0;

  /// Read some bytecode into OS page cache (only implemented for buffers).
  virtual void startWarmup(uint8_t percent) {}

  /// Issue an madvise call (only implemented for buffers).
  virtual void madvise(oscompat::MAdvice advice) {}

  /// Advise the provider that string table metadata is going to be accessed
  /// sequentially.  Only forwards this advice to the OS for buffers.
  virtual void adviseStringTableSequential() {}

  /// Advise the provider that string table metadata is going to be accessed
  /// in random order.  Only forwards this advice to the OS for buffers.
  virtual void adviseStringTableRandom() {}

  /// Advise the provider that string table metadata is going to be accessed
  /// soon.  Only forwards this information to the OS for buffers.
  virtual void willNeedStringTable() {}

  /// Start tracking I/O (only implemented for buffers). Any access before this
  /// call (e.g. reading header to construct the provider) will not be recorded.
  virtual void startPageAccessTracker() {}

  /// Access the page I/O tracker (only implemented for buffers).
  virtual volatile PageAccessTracker *getPageAccessTracker() {
    return nullptr;
  }

  /// Return the entire bytecode file (only implemented for buffers).
  virtual llvh::ArrayRef<uint8_t> getRawBuffer() const {
    return llvh::ArrayRef<uint8_t>();
  }

  /// Given the functionID and offset of the instruction where exception
  /// happened, \returns the offset of the exception handler to jump to.
  /// \returns -1 if a handler is not found.
  int32_t findCatchTargetOffset(uint32_t functionID, uint32_t exceptionOffset)
      const;

  /// When bytecode dedup optimization is enabled, different functions
  /// could end up with identical absolute bytecode offset, this could confuse
  /// symbolicator. This function computes the unique bytecode offset for a
  /// given function under a virtual scenario where no dedup happens, i.e.
  /// by accumulating the total size of all bytecode prior to this function.
  uint32_t getVirtualOffsetForFunction(uint32_t functionID) const;
};

/// BCProviderFromBuffer will be used when we are loading bytecode from
/// a bytecode buffer (from a file). In this code path, no extra classes need
/// to be created but we only need to maintain a few pointers to point into
/// the buffer in order to provide all the bytecode data.
class BCProviderFromBuffer final : public BCProviderBase {
  /// The continuous bytecode buffer.
  std::unique_ptr<const Buffer> buffer_;

  /// Pointer to buffer_->data(), to avoid calling it every time.
  const uint8_t *bufferPtr_;

  /// List of function headers.
  const hbc::SmallFuncHeader *functionHeaders_{};

  /// List of string table entries.
  /// Some of these may overflow, in which case their offset indexes into the
  /// overflow table below.
  const hbc::SmallStringTableEntry *stringTableEntries_{};

  /// List of overflow string table entries.
  llvh::ArrayRef<OverflowStringTableEntry> overflowStringTableEntries_{};

  /// Offset of the location to find debug info.
  uint32_t debugInfoOffset_{};

  /// If \p startWarmup has been called, this is the thread doing the warmup.
  llvh::Optional<std::thread> warmupThread_;

  /// Set by \p stopWarmup to tell any warmup thread to abort.
  std::atomic<bool> warmupAbortFlag_;

  std::unique_ptr<volatile PageAccessTracker> tracker_;

  /// End of the bytecode file.
  const uint8_t *end_;

  /// Tells any running warmup thread to abort and then joins that thread.
  void stopWarmup();

  explicit BCProviderFromBuffer(
      std::unique_ptr<const Buffer> buffer,
      BytecodeForm form);

  void createDebugInfo() override;

  /// Helper function to fetch the exception table data given \p functionID.
  /// \returns the ArrayRef to the exception table data, along with a pointer
  /// to the DebugOffsets (or nullptr if there is none).
  std::pair<
      llvh::ArrayRef<hbc::HBCExceptionHandlerInfo>,
      const hbc::DebugOffsets *>
  getExceptionTableAndDebugOffsets(uint32_t functionID) const;

 public:
  static std::pair<std::unique_ptr<BCProviderFromBuffer>, std::string>
  createBCProviderFromBuffer(
      std::unique_ptr<const Buffer> buffer,
      BytecodeForm form = BytecodeForm::Execution) {
    auto ret = std::unique_ptr<BCProviderFromBuffer>(
        new BCProviderFromBuffer(std::move(buffer), form));
    auto errstr = ret->getErrorStr();
    return {errstr.empty() ? std::move(ret) : nullptr, errstr};
  }

  /// Checks whether the data is actually bytecode.
  static bool isBytecodeStream(llvh::ArrayRef<uint8_t> aref) {
    const auto *header =
        reinterpret_cast<const hbc::BytecodeFileHeader *>(aref.data());
    return (
        aref.size() >= sizeof(hbc::BytecodeFileHeader) &&
        header->magic == hbc::MAGIC);
  }

  /// Checks whether the buffer is actually bytecode.
  static bool isBytecodeStream(const Buffer &buffer) {
    return isBytecodeStream(
        llvh::ArrayRef<uint8_t>(buffer.data(), buffer.size()));
  }

  /// Given a range of memory that contains a mapped bytecode file, issues
  /// madvise calls for portions of the bytecode file that will likely be
  /// used when loading the bytecode file and running its global function
  /// (such as the string table and the global function's body).
  static void prefetch(llvh::ArrayRef<uint8_t> aref);

  /// Returns data appened after the bytecode stream.
  static llvh::ArrayRef<uint8_t> getEpilogueFromBytecode(
      llvh::ArrayRef<uint8_t> buffer);

  static SHA1 getSourceHashFromBytecode(llvh::ArrayRef<uint8_t> buffer);

  /// Returns if aref points to valid bytecode and specifies why it may not
  /// in errorMessage (if supplied).
  static bool bytecodeStreamSanityCheck(
      llvh::ArrayRef<uint8_t> aref,
      std::string *errorMessage = nullptr);

  /// Given a valid bytecode buffer aref, returns whether its stored fileHash
  /// matches the actual hash of the buffer.
  static bool bytecodeHashIsValid(llvh::ArrayRef<uint8_t> aref);

  /// Given a writable valid bytecode buffer aref, update its fileHash field
  /// with the actual hash of the buffer.
  static void updateBytecodeHash(llvh::MutableArrayRef<uint8_t> aref);

  /// Returns the arrayref to small function headers;
  /// this is also the start of the function header section.
  const llvh::ArrayRef<hbc::SmallFuncHeader> getSmallFunctionHeaders() const {
    return llvh::ArrayRef<hbc::SmallFuncHeader>(
        functionHeaders_, functionCount_);
  }

  RuntimeFunctionHeader getFunctionHeader(uint32_t functionID) const override {
    const hbc::SmallFuncHeader &smallHeader = functionHeaders_[functionID];
    if (LLVM_UNLIKELY(smallHeader.flags.overflowed)) {
      auto large = reinterpret_cast<const hbc::FunctionHeader *>(
          bufferPtr_ + smallHeader.getLargeHeaderOffset());
      return RuntimeFunctionHeader(large);
    } else {
      return RuntimeFunctionHeader(&smallHeader);
    }
  }

  /// Returns the ArrayRef to the small string table entries.
  /// Some of these may be indexes into overflow string table entries.
  llvh::ArrayRef<SmallStringTableEntry> getSmallStringTableEntries() const {
    return llvh::makeArrayRef(stringTableEntries_, stringCount_);
  }

  /// Returns the arrayref to the small string table entries;
  /// this is also the start of the string table section.
  llvh::ArrayRef<OverflowStringTableEntry> getOverflowStringTableEntries()
      const {
    return overflowStringTableEntries_;
  }

  StringTableEntry getStringTableEntry(uint32_t index) const override {
    auto &smallHeader = stringTableEntries_[index];
    if (LLVM_UNLIKELY(smallHeader.isOverflowed())) {
      auto overflow = overflowStringTableEntries_[smallHeader.offset];
      StringTableEntry entry(
          overflow.offset, overflow.length, smallHeader.isUTF16);
      return entry;
    }
    StringTableEntry entry(
        smallHeader.offset, smallHeader.length, smallHeader.isUTF16);
    return entry;
  }

  const uint8_t *getBytecode(uint32_t functionID) const override {
    return bufferPtr_ + getFunctionHeader(functionID).offset();
  }

  llvh::ArrayRef<hbc::HBCExceptionHandlerInfo> getExceptionTable(
      uint32_t functionID) const override {
    return getExceptionTableAndDebugOffsets(functionID).first;
  }

  const hbc::DebugOffsets *getDebugOffsets(uint32_t functionID) const override {
    return getExceptionTableAndDebugOffsets(functionID).second;
  }

  llvh::ArrayRef<uint8_t> getEpilogue() const override;
  SHA1 getSourceHash() const override;

  void startWarmup(uint8_t percent) override;

  void madvise(oscompat::MAdvice advice) override;
  void adviseStringTableSequential() override;
  void adviseStringTableRandom() override;
  void willNeedStringTable() override;

  void startPageAccessTracker() override;

  volatile PageAccessTracker *getPageAccessTracker() override {
    return tracker_.get();
  }

  llvh::ArrayRef<uint8_t> getRawBuffer() const override {
    return llvh::ArrayRef<uint8_t>(bufferPtr_, buffer_->size());
  }

  ~BCProviderFromBuffer() override {
    stopWarmup();
    delete debugInfo_;
  }

  bool isFunctionLazy(uint32_t functionID) const override {
    return false;
  }

  bool isLazy() const override {
    return false;
  }
};

} // namespace hbc
} // namespace hermes
#pragma GCC diagnostic pop

#endif // HERMES_BCGEN_HBC_BYTECODEDATAPROVIDER_H
