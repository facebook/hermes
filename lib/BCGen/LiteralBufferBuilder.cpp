/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/LiteralBufferBuilder.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"
#include "hermes/BCGen/SerializedLiteralParser.h"
#include "hermes/BCGen/ShapeTableEntry.h"
#include "hermes/IR/IR.h"
#include "hermes/IR/Instrs.h"
#include "hermes/Inst/Inst.h"
#include "hermes/Inst/InstDecode.h"

namespace hermes {
namespace LiteralBufferBuilder {
namespace {

/// A container that deduplicates byte sequences, while keeping the original
/// insertion order with the duplicates. Note: strings are used as a
/// representation for code reuse and simplicity, but the contents are meant to
/// be interpreted as unsigned bytes.
/// It maintains:
/// - a StringSetVector containing the uniqued strings.
/// - a vector mapping each originally inserted string in order to an index in
///   the StringSetVector.
/// This is a specialized class used only by \c LiteralBufferBuilder.
/// NOTE: We use std::string instead of std::vector<uint8_t> for code reuse.
class UniquedStringVector {
 public:
  /// Append a string.
  void push_back(llvh::StringRef str) {
    indexInSet_.push_back(set_.insert(str));
  }

  /// \return how many strings the vector contains, in other words, how many
  ///     times \c push_back() was called.
  size_t size() const {
    return indexInSet_.size();
  }

  /// \return the begin iterator over the uniqued set of strings in insertion
  ///  order.
  StringSetVector::const_iterator beginSet() const {
    return set_.begin();
  }
  /// \return the end iterator over the uniqued set of strings in insertion
  ///  order.
  StringSetVector::const_iterator endSet() const {
    return set_.end();
  }

  /// \return the index in the uniqued set corresponding to the insertion index
  ///     \p insertionIndex.
  uint32_t indexInSet(size_t insertionIndex) const {
    return indexInSet_[insertionIndex];
  }

 private:
  /// The uniqued string set in insertion order.
  StringSetVector set_{};
  /// Index into the set of each original non-deduplicated string in insertion
  /// order.
  std::vector<uint32_t> indexInSet_{};
};

/// A utility class which collects all serialized literals from a module,
/// optionally deduplicates them, and installs them in the
/// BytecodeModuleGenerator.
class Builder {
 public:
  /// Constructor.
  /// \param m the IR module to process.
  /// \param shouldVisitFunction a predicate indicating whether a function
  ///     should be processed or not. (In some cases like segment splitting we
  ///     want to exclude part of the module.)
  /// \param getIdentifier used to lookup identifiers.
  /// \param getString used to lookup string values.
  /// \param optimize whether to deduplicate the serialized literals.
  /// \param bcProvider optional base bytecode provider.
  Builder(
      Module *m,
      const std::function<bool(Function *)> &shouldVisitFunction,
      const SerializedLiteralGenerator::StringLookupFn &getIdentifier,
      const SerializedLiteralGenerator::StringLookupFn &getString,
      bool optimize,
      hbc::BCProviderBase *bcProvider)
      : M_(m),
        shouldVisitFunction_(shouldVisitFunction),
        optimize_(optimize),
        literalGenerator_(getIdentifier, getString),
        bcProvider_(bcProvider) {}

  /// Do everything: collect the literals, optionally deduplicate them.
  Result generate();

 private:
  /// Reseed the tables to be initialized with the base bytecode given.
  void reseedFromBaseBytecode();

  /// Traverse the module, skipping functions that should not be visited,
  /// and collect all serialized array and object literals and the corresponding
  /// instruction.
  void traverse();

  /// Make the underlying raw storage for the buffers.
  void makeBufferStorages();

  /// Serialization handlers for different instructions.
  void serializeLiteralFor(AllocArrayInst *AAI);
  void serializeLiteralFor(LIRAllocObjectFromBufferInst *AOFB);
  void serializeLiteralFor(CacheNewObjectInst *cacheNew);

  /// Serialize the the input literals \p elements into the UniquedStringVector
  /// \p dest.
  /// \p isKeyBuffer: whether this is generating object literal key buffer or
  /// not.
  void serializeInto(
      UniquedStringVector &dest,
      llvh::ArrayRef<Literal *> elements,
      bool isKeyBuffer);

 private:
  /// The IR module to process.
  Module *const M_;
  /// A predicate indicating whether a function should be processed or not. (In
  /// some cases like segment splitting we want to exclude part of the module.)
  const std::function<bool(Function *)> &shouldVisitFunction_;
  /// Whether to deduplicate the serialized literals.
  bool const optimize_;

  /// The stateless generator object.
  SerializedLiteralGenerator literalGenerator_;

  hbc::BCProviderBase *bcProvider_;

  /// Temporary buffer to serialize literals into. We keep it around instead
  /// of allocating a new one every time.
  std::vector<unsigned char> tempBuffer_{};

  /// Each element is the values portion of a serialized object literal or
  /// array.
  UniquedStringVector values_{};

  /// The underlying raw byte storage of the values buffer.
  hbc::ConsecutiveStringStorage valueStorage_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding index in \c values_.
  std::vector<std::pair<const Instruction *, size_t>> arraysInst_{};

  // This contains all the unique shapes of all the object literals in the
  // module.
  std::vector<ShapeTableEntry> objShapeTable_{};

  /// This maps a <keyBufferOffset, numProps> pair to an element index in \p
  /// objShapeTable_.
  llvh::DenseMap<std::pair<uint32_t, uint32_t>, uint32_t> keyOffsetToShapeIdx_;

  /// Each element is the keys portion of a serialized object literal.
  UniquedStringVector objKeys_{};

  /// The underlying raw byte storage of the keys buffer.
  hbc::ConsecutiveStringStorage keyStorage_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding indices in \c objKeys_/values_. Note that the indicies may
  /// not be the same, since values_ is shared between object and array literal
  /// values.
  std::vector<std::pair<
      const LIRAllocObjectFromBufferInst *,
      std::pair<size_t, size_t>>>
      objInst_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding indices in \c objKeys_.
  std::vector<std::pair<CacheNewObjectInst *, size_t>> cacheNewObjectInst_{};
};

void Builder::reseedFromBaseBytecode() {
  assert(bcProvider_ && "expected nonnull bytecode provider");
  llvh::ArrayRef<ShapeTableEntry> shapeTable =
      bcProvider_->getObjectShapeTable();
  llvh::ArrayRef<unsigned char> valueBuf = bcProvider_->getLiteralValueBuffer();
  // Pair of <offset, sizeInBytes>.
  llvh::DenseSet<std::pair<uint32_t, uint32_t>> seenValueBufferEntries;
  std::vector<StringTableEntry> valueStrTable;
  struct {
    void visitStringID(uint32_t id) {}
    void visitNumber(double d) {}
    void visitNull() {}
    void visitUndefined() {}
    void visitBool(bool b) {}
  } emptyVisitor;

  /// Process the usage of a <offset, numElements> usage from a bytecode
  /// instruction. If this pair has not been seen before, we add an entry to \c
  /// valueStrTable which describes the layout of the base bytecode value
  /// buffer.
  auto processValueUserOp = [this,
                             &valueBuf,
                             &emptyVisitor,
                             &valueStrTable,
                             &seenValueBufferEntries](
                                uint32_t valBufOffset, uint16_t numElements) {
    auto sizeInBytes = SerializedLiteralParser::parse(
        valueBuf.slice(valBufOffset), numElements, emptyVisitor);
    auto [_, inserted] =
        seenValueBufferEntries.insert({valBufOffset, sizeInBytes});
    if (inserted) {
      valueStrTable.push_back({valBufOffset, (uint32_t)sizeInBytes, false});
      values_.push_back(
          llvh::StringRef{
              reinterpret_cast<const char *>(valueBuf.data() + valBufOffset),
              sizeInBytes});
    }
  };

  // There is no convenient header describing the structure of the raw bytes for
  // the value buffer. Therefore, we reconstruct the individual elements in the
  // buffer by looking at the bytecode instructions that index into the value
  // buffer.
  for (unsigned funcId = 0; funcId < bcProvider_->getFunctionCount();
       ++funcId) {
    hbc::RuntimeFunctionHeader functionHeader =
        bcProvider_->getFunctionHeader(funcId);
    const uint8_t *bytecodeStart = bcProvider_->getBytecode(funcId);
    const uint8_t *bytecodeEnd =
        bytecodeStart + functionHeader.getBytecodeSizeInBytes();
    const uint8_t *cursor = bytecodeStart;
    while (cursor < bytecodeEnd) {
      auto *ip = reinterpret_cast<const inst::Inst *>(cursor);
      switch (ip->opCode) {
        case inst::OpCode::NewArrayWithBuffer:
          processValueUserOp(
              ip->iNewArrayWithBuffer.op4, ip->iNewArrayWithBuffer.op3);
          break;
        case inst::OpCode::NewArrayWithBufferLong:
          processValueUserOp(
              ip->iNewArrayWithBufferLong.op4, ip->iNewArrayWithBufferLong.op3);
          break;
        case inst::OpCode::NewObjectWithBuffer: {
          auto numElements = shapeTable[ip->iNewObjectWithBuffer.op2].numProps;
          processValueUserOp(ip->iNewObjectWithBuffer.op3, numElements);
          break;
        }
        case inst::OpCode::NewObjectWithBufferLong: {
          auto numElements =
              shapeTable[ip->iNewObjectWithBufferLong.op2].numProps;
          processValueUserOp(ip->iNewObjectWithBufferLong.op3, numElements);
          break;
        }
        case inst::OpCode::NewObjectWithBufferAndParent: {
          auto numElements =
              shapeTable[ip->iNewObjectWithBufferAndParent.op3].numProps;
          processValueUserOp(
              ip->iNewObjectWithBufferAndParent.op4, numElements);
          break;
        }
        default:
#ifndef NDEBUG
#define DEFINE_VALUE_BUFFER_USER(name)    \
  assert(                                 \
      ip->opCode != inst::OpCode::name && \
      "Value buffer user " #name " not handled");
#include "hermes/BCGen/HBC/BytecodeList.def"
#endif
          break;
      }
      cursor += inst::getInstSize(ip->opCode);
    }
  }
  valueStorage_ =
      hbc::ConsecutiveStringStorage{std::move(valueStrTable), valueBuf.vec()};

  // Now recreate the key storage. This is easier since the shape table is
  // a convenient place where the uniqued info can be found for all keys in the
  // key storage.
  objShapeTable_ = shapeTable.vec();
  llvh::ArrayRef<unsigned char> keyBuf = bcProvider_->getObjectKeyBuffer();
  std::vector<StringTableEntry> keyStrTable;
  keyStrTable.reserve(objShapeTable_.size());
  for (size_t i = 0, e = objShapeTable_.size(); i < e; ++i) {
    auto numProps = objShapeTable_[i].numProps;
    auto keyBufferOffset = objShapeTable_[i].keyBufferOffset;
    auto sizeInBytes = SerializedLiteralParser::parse(
        keyBuf.slice(keyBufferOffset), numProps, emptyVisitor);
    keyStrTable.push_back({keyBufferOffset, (uint32_t)sizeInBytes, false});
    objKeys_.push_back(
        llvh::StringRef{
            reinterpret_cast<const char *>(keyBuf.data() + keyBufferOffset),
            sizeInBytes});
    keyOffsetToShapeIdx_.insert({{keyBufferOffset, numProps}, (uint32_t)i});
  }
  keyStorage_ =
      hbc::ConsecutiveStringStorage{std::move(keyStrTable), keyBuf.vec()};
}

void Builder::makeBufferStorages() {
  assert(
      bcProvider_ ||
      (valueStorage_.count() == 0 && keyStorage_.count() == 0) &&
          "with no base bytecode, storages should be empty");
  valueStorage_.appendStorage(
      hbc::ConsecutiveStringStorage{
          values_.beginSet() + valueStorage_.count(),
          values_.endSet(),
          std::true_type{},
          optimize_});
  keyStorage_.appendStorage(
      hbc::ConsecutiveStringStorage{
          objKeys_.beginSet() + keyStorage_.count(),
          objKeys_.endSet(),
          std::true_type{},
          optimize_});
}

LiteralBufferBuilder::Result Builder::generate() {
  if (bcProvider_) {
    reseedFromBaseBytecode();
  }
  traverse();
  makeBufferStorages();

  // Populate the offset map.
  LiteralOffsetMapTy literalOffsetMap{};

  // Visit all object/array literal values.
  // Cast these to const to make sure we're calling the correct overload and no
  // longer modifying the underlying storage.
  auto valView =
      const_cast<const hbc::ConsecutiveStringStorage &>(valueStorage_)
          .getStringTableView();
  for (size_t i = 0, e = arraysInst_.size(); i != e; ++i) {
    const auto [Inst, idx] = arraysInst_[i];
    assert(
        literalOffsetMap.count(Inst) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t arrayIndexInSet = values_.indexInSet(idx);
    literalOffsetMap[Inst] = {UINT32_MAX, valView[arrayIndexInSet].getOffset()};
  }

  // Visit all object literals.
  auto keyView = const_cast<const hbc::ConsecutiveStringStorage &>(keyStorage_)
                     .getStringTableView();
  for (size_t i = 0, e = objInst_.size(); i != e; ++i) {
    const auto [Inst, indices] = objInst_[i];
    const auto [keyIdx, valIdx] = indices;
    const uint32_t len = Inst->getKeyValuePairCount();
    assert(
        literalOffsetMap.count(Inst) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t keyIndexInSet = objKeys_.indexInSet(keyIdx);
    uint32_t valIndexInSet = values_.indexInSet(valIdx);
    uint32_t keyBufferOffset = keyView[keyIndexInSet].getOffset();
    const auto [iter, success] = keyOffsetToShapeIdx_.insert(
        {{keyBufferOffset, len}, keyOffsetToShapeIdx_.size()});
    auto shapeID = iter->second;
    if (success) {
      // This is a new entry, add it to the shape table.
      objShapeTable_.push_back({keyBufferOffset, len});
    }
    literalOffsetMap[Inst] =
        LiteralOffset{shapeID, valView[valIndexInSet].getOffset()};
  }

  for (size_t i = 0, e = cacheNewObjectInst_.size(); i != e; ++i) {
    const auto [Inst, idx] = cacheNewObjectInst_[i];
    const uint32_t len = Inst->getNumKeys();
    assert(
        literalOffsetMap.count(Inst) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t keyIndexInSet = objKeys_.indexInSet(idx);
    uint32_t keyBufferOffset = keyView[keyIndexInSet].getOffset();
    const auto [iter, success] = keyOffsetToShapeIdx_.insert(
        {{keyBufferOffset, len}, keyOffsetToShapeIdx_.size()});
    uint32_t shapeID = iter->second;
    if (success) {
      // This is a new entry, add it to the shape table.
      objShapeTable_.push_back({keyBufferOffset, len});
    }
    literalOffsetMap[Inst] = LiteralOffset{shapeID, UINT32_MAX};
  }

  return {
      std::move(valueStorage_).acquireStringTableAndStorage().second,
      std::move(keyStorage_).acquireStringTableAndStorage().second,
      std::move(objShapeTable_),
      std::move(literalOffsetMap)};
}

void Builder::traverse() {
  for (auto &F : *M_) {
    if (!shouldVisitFunction_(&F))
      continue;

    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *AAI = llvh::dyn_cast<AllocArrayInst>(&I)) {
          serializeLiteralFor(AAI);
        } else if (
            auto *AOFB = llvh::dyn_cast<LIRAllocObjectFromBufferInst>(&I)) {
          serializeLiteralFor(AOFB);
        } else if (auto *cacheNew = llvh::dyn_cast<CacheNewObjectInst>(&I)) {
          serializeLiteralFor(cacheNew);
        }
      }
    }
  }
}

void Builder::serializeInto(
    UniquedStringVector &dest,
    llvh::ArrayRef<Literal *> elements,
    bool isKeyBuffer) {
  tempBuffer_.clear();
  literalGenerator_.serializeBuffer(elements, tempBuffer_, isKeyBuffer);
  dest.push_back(
      llvh::StringRef((const char *)tempBuffer_.data(), tempBuffer_.size()));
}

void Builder::serializeLiteralFor(AllocArrayInst *AAI) {
  unsigned e = AAI->getElementCount();
  if (!e)
    return;

  llvh::SmallVector<Literal *, 8> elements;
  for (unsigned i = 0; i < e; ++i) {
    elements.push_back(cast<Literal>(AAI->getArrayElement(i)));
  }

  arraysInst_.push_back({AAI, values_.size()});
  serializeInto(values_, elements, false);
}

void Builder::serializeLiteralFor(LIRAllocObjectFromBufferInst *AOFB) {
  unsigned e = AOFB->getKeyValuePairCount();
  if (!e)
    return;

  llvh::SmallVector<Literal *, 8> objKeys;
  llvh::SmallVector<Literal *, 8> objVals;
  for (unsigned ind = 0; ind != e; ++ind) {
    auto keyValuePair = AOFB->getKeyValuePair(ind);
    objKeys.push_back(cast<Literal>(keyValuePair.first));
    objVals.push_back(cast<Literal>(keyValuePair.second));
  }

  objInst_.push_back({AOFB, {objKeys_.size(), values_.size()}});
  serializeInto(objKeys_, objKeys, true);
  serializeInto(values_, objVals, false);
}

void Builder::serializeLiteralFor(CacheNewObjectInst *cacheNew) {
  unsigned e = cacheNew->getNumKeys();
  assert(e > 0 && "CacheNewObjectInst should have at least one key");

  llvh::SmallVector<Literal *, 8> objKeys;
  for (unsigned ind = 0; ind != e; ++ind) {
    Literal *key = cacheNew->getKey(ind);
    objKeys.push_back(key);
  }

  cacheNewObjectInst_.push_back({cacheNew, objKeys_.size()});
  serializeInto(objKeys_, objKeys, true);
}

} // namespace

Result generate(
    Module *m,
    const std::function<bool(Function *)> &shouldVisitFunction,
    const SerializedLiteralGenerator::StringLookupFn &getIdentifier,
    const SerializedLiteralGenerator::StringLookupFn &getString,
    bool optimize,
    hbc::BCProviderBase *bcProvider) {
  return Builder(
             m,
             shouldVisitFunction,
             getIdentifier,
             getString,
             optimize,
             bcProvider)
      .generate();
}

} // namespace LiteralBufferBuilder
} // namespace hermes
