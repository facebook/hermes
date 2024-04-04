/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/BCGen/LiteralBufferBuilder.h"
#include "hermes/BCGen/HBC/ConsecutiveStringStorage.h"

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
  Builder(
      Module *m,
      const std::function<bool(Function const *)> &shouldVisitFunction,
      SerializedLiteralGenerator::StringLookupFn getIdentifier,
      SerializedLiteralGenerator::StringLookupFn getString,
      bool optimize)
      : M_(m),
        shouldVisitFunction_(shouldVisitFunction),
        optimize_(optimize),
        literalGenerator_(getIdentifier, getString) {}

  /// Do everything: collect the literals, optionally deduplicate them.
  Result generate();

 private:
  /// Traverse the module, skipping functions that should not be visited,
  /// and collect all serialized array and object literals and the corresponding
  /// instruction.
  void traverse();

  /// Serialization handlers for different instructions.
  void serializeLiteralFor(AllocArrayInst *AAI);
  void serializeLiteralFor(HBCAllocObjectFromBufferInst *AOFB);

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
  const std::function<bool(const Function *)> &shouldVisitFunction_;
  /// Whether to deduplicate the serialized literals.
  bool const optimize_;

  /// The stateless generator object.
  SerializedLiteralGenerator literalGenerator_;

  /// Temporary buffer to serialize literals into. We keep it around instead
  /// of allocating a new one every time.
  std::vector<unsigned char> tempBuffer_{};

  /// Each element is a serialized array literal.
  UniquedStringVector arrays_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding index in \c arrays_.
  std::vector<const Instruction *> arraysInst_{};

  /// Each element is the keys portion of a serialized object literal.
  UniquedStringVector objKeys_{};

  /// Each element is the values portion of a serialized object literal.
  UniquedStringVector objVals_{};

  /// Each element records the instruction whose literal was serialized at the
  /// corresponding index in \c objKeys_/objVals_.
  std::vector<const Instruction *> objInst_{};
};

LiteralBufferBuilder::Result Builder::generate() {
  traverse();

  // Construct the serialized storage, optionally optimizing it.
  hbc::ConsecutiveStringStorage arrayStorage{
      arrays_.beginSet(), arrays_.endSet(), std::true_type{}, optimize_};
  hbc::ConsecutiveStringStorage keyStorage{
      objKeys_.beginSet(), objKeys_.endSet(), std::true_type{}, optimize_};
  hbc::ConsecutiveStringStorage valStorage{
      objVals_.beginSet(), objVals_.endSet(), std::true_type{}, optimize_};

  // Populate the offset map.
  LiteralOffsetMapTy literalOffsetMap{};

  // Visit all array literals.
  // Cast these to const to make sure we're calling the correct overload and no
  // longer modifying the underlying storage.
  auto arrayView =
      const_cast<const hbc::ConsecutiveStringStorage &>(arrayStorage)
          .getStringTableView();
  for (size_t i = 0, e = arraysInst_.size(); i != e; ++i) {
    assert(
        literalOffsetMap.count(arraysInst_[i]) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t arrayIndexInSet = arrays_.indexInSet(i);
    literalOffsetMap[arraysInst_[i]] =
        LiteralOffset{arrayView[arrayIndexInSet].getOffset(), UINT32_MAX};
  }

  // Visit all object literals - they are split in two buffers.
  auto keyView = const_cast<const hbc::ConsecutiveStringStorage &>(keyStorage)
                     .getStringTableView();
  auto valView = const_cast<const hbc::ConsecutiveStringStorage &>(valStorage)
                     .getStringTableView();
  for (size_t i = 0, e = objInst_.size(); i != e; ++i) {
    assert(
        literalOffsetMap.count(objInst_[i]) == 0 &&
        "instruction literal can't be serialized twice");
    uint32_t keyIndexInSet = objKeys_.indexInSet(i);
    uint32_t valIndexInSet = objVals_.indexInSet(i);
    literalOffsetMap[objInst_[i]] = LiteralOffset{
        keyView[keyIndexInSet].getOffset(), valView[valIndexInSet].getOffset()};
  }

  return {
      arrayStorage.acquireStringStorage(),
      keyStorage.acquireStringStorage(),
      valStorage.acquireStringStorage(),
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
            auto *AOFB = llvh::dyn_cast<HBCAllocObjectFromBufferInst>(&I)) {
          serializeLiteralFor(AOFB);
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

  serializeInto(arrays_, elements, false);
  arraysInst_.push_back(AAI);
}

void Builder::serializeLiteralFor(HBCAllocObjectFromBufferInst *AOFB) {
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

  serializeInto(objKeys_, objKeys, true);
  serializeInto(objVals_, objVals, false);
  assert(
      objKeys_.size() == objVals_.size() &&
      "objKeys_ and objVals_ must be the same size");
  objInst_.push_back(AOFB);
}
} // namespace

Result generate(
    Module *m,
    const std::function<bool(Function const *)> &shouldVisitFunction,
    const SerializedLiteralGenerator::StringLookupFn &getIdentifier,
    const SerializedLiteralGenerator::StringLookupFn &getString,
    bool optimize) {
  return Builder(m, shouldVisitFunction, getIdentifier, getString, optimize)
      .generate();
}

} // namespace LiteralBufferBuilder
} // namespace hermes
