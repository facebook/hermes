/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/StringPrimitive.h"

#include "hermes/Support/Algorithms.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/FillerCell.h"
#include "hermes/VM/GCPointer-inline.h"
#include "hermes/VM/HermesValue-inline.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

void DynamicASCIIStringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&DynamicASCIIStringPrimitive::vt);
}
void DynamicUTF16StringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&DynamicUTF16StringPrimitive::vt);
}

#ifdef HERMESVM_SERIALIZE
template <typename T, bool Uniqued>
void serializeDynamicStringImpl(Serializer &s, const GCCell *cell) {
  const auto *self = vmcast<const DynamicStringPrimitive<T, Uniqued>>(cell);
  // Write string length.
  s.writeInt<uint32_t>(self->lengthAndUniquedFlag_);
  if (Uniqued) {
    // If isUniqued, also writes SymbolID.
    s.writeInt<uint32_t>(
        (self->isUniqued() ? self->getUniqueID() : SymbolID::empty())
            .unsafeGetRaw());
  }
  // Writes the actual string.
  s.writeData(self->getRawPointer(), self->getStringLength() * sizeof(T));
  s.endObject(cell);
}

template <typename T, bool Uniqued>
void deserializeDynamicStringImpl(Deserializer &d) {
  uint32_t lengthAndUniquedFlag = d.readInt<uint32_t>();
  uint32_t length = lengthAndUniquedFlag &
      ~DynamicStringPrimitive<T, Uniqued>::LENGTH_FLAG_UNIQUED;
  bool uniqued = lengthAndUniquedFlag &
      DynamicStringPrimitive<T, Uniqued>::LENGTH_FLAG_UNIQUED;
  SymbolID uniqueID{};
  if (Uniqued) {
    uniqueID = SymbolID::unsafeCreate(d.readInt<uint32_t>());
  }

  auto *cell =
      d.getRuntime()->makeAVariable<DynamicStringPrimitive<T, Uniqued>>(
          DynamicStringPrimitive<T, Uniqued>::allocationSize(length),
          d.getRuntime(),
          length);
  if (uniqued)
    cell->convertToUniqued(uniqueID);
  d.readData(cell->getRawPointerForWrite(), length * sizeof(T));

  d.endObject(cell);
}

void DynamicASCIIStringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeDynamicStringImpl<char, false>(s, cell);
}

void DynamicUTF16StringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeDynamicStringImpl<char16_t, false>(s, cell);
}

void DynamicUniquedASCIIStringPrimitiveSerialize(
    Serializer &s,
    const GCCell *cell) {
  serializeDynamicStringImpl<char, true>(s, cell);
}

void DynamicUniquedUTF16StringPrimitiveSerialize(
    Serializer &s,
    const GCCell *cell) {
  serializeDynamicStringImpl<char16_t, true>(s, cell);
}

void DynamicASCIIStringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::DynamicASCIIStringPrimitiveKind &&
      "Expected DynamicASCIIStringPrimitive");
  deserializeDynamicStringImpl<char, false>(d);
}

void DynamicUTF16StringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::DynamicUTF16StringPrimitiveKind &&
      "Expected DynamicUTF16StringPrimitive");
  deserializeDynamicStringImpl<char16_t, false>(d);
}

void DynamicUniquedASCIIStringPrimitiveDeserialize(
    Deserializer &d,
    CellKind kind) {
  assert(
      kind == CellKind::DynamicUniquedASCIIStringPrimitiveKind &&
      "Expected DynamicUniquedASCIIStringPrimitive");
  deserializeDynamicStringImpl<char, true>(d);
}

void DynamicUniquedUTF16StringPrimitiveDeserialize(
    Deserializer &d,
    CellKind kind) {
  assert(
      kind == CellKind::DynamicUniquedUTF16StringPrimitiveKind &&
      "Expected DynamicUniquedUTF16StringPrimitive");
  deserializeDynamicStringImpl<char16_t, true>(d);
}
#endif

void DynamicUniquedASCIIStringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&DynamicUniquedASCIIStringPrimitive::vt);
}

void DynamicUniquedUTF16StringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&DynamicUniquedUTF16StringPrimitive::vt);
}

void ExternalASCIIStringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&ExternalASCIIStringPrimitive::vt);
}

void ExternalUTF16StringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  mb.setVTable(&ExternalUTF16StringPrimitive::vt);
}

#ifdef HERMESVM_SERIALIZE
template <typename T>
void serializeExternalStringImpl(Serializer &s, const GCCell *cell) {
  const auto *self = vmcast<const ExternalStringPrimitive<T>>(cell);
  s.writeInt<uint32_t>(self->lengthAndUniquedFlag_);
  s.writeInt<uint32_t>(
      (self->isUniqued() ? self->getUniqueID() : SymbolID::empty())
          .unsafeGetRaw());
  // Writes the actual string.
  s.writeData(self->getRawPointer(), self->getStringLength() * sizeof(T));
  // contents_.data() is tracked by IDTracker for heapsnapshot. We should do
  // relocation for it.
  s.endObject((void *)self->contents_.data());

  s.endObject(cell);
}

template <typename T>
void deserializeExternalStringImpl(Deserializer &d) {
  // Deserialize the data.
  uint32_t lengthAndUniquedFlag = d.readInt<uint32_t>();
  uint32_t length =
      lengthAndUniquedFlag & ~ExternalStringPrimitive<T>::LENGTH_FLAG_UNIQUED;
  bool uniqued = length & ExternalStringPrimitive<T>::LENGTH_FLAG_UNIQUED;
  assert(
      ExternalStringPrimitive<T>::isSafeExternalLength(length) &&
      "length should be external");
  if (LLVM_UNLIKELY(length > ExternalStringPrimitive<T>::MAX_STRING_LENGTH))
    hermes_fatal("String length exceeds limit");
  uint32_t allocSize = length * sizeof(T);
  if (LLVM_UNLIKELY(
          !d.getRuntime()->getHeap().canAllocExternalMemory(allocSize))) {
    hermes_fatal("Cannot allocate an external string primitive.");
  }

  auto uniqueID = SymbolID::unsafeCreate(d.readInt<uint32_t>());
  std::basic_string<T> contents(length, '\0');
  d.readData(&contents[0], length * sizeof(T));

  // Construct an ExternalStringPrimitive from what we deserialized.
  auto *cell =
      d.getRuntime()
          ->makeAVariable<ExternalStringPrimitive<T>, HasFinalizer::Yes>(
              sizeof(ExternalStringPrimitive<T>),
              d.getRuntime(),
              std::move(contents));
  if (uniqued)
    cell->convertToUniqued(uniqueID);

  // contents_.data() is tracked by IDTracker for heapsnapshot. We should do
  // relocation for it.
  d.endObject((void *)cell->contents_.data());

  d.getRuntime()->getHeap().creditExternalMemory(
      cell, cell->calcExternalMemorySize());
  d.endObject(cell);
}

void ExternalASCIIStringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeExternalStringImpl<char>(s, cell);
}

void ExternalUTF16StringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeExternalStringImpl<char16_t>(s, cell);
}

void ExternalASCIIStringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::ExternalASCIIStringPrimitiveKind &&
      "Expected ExternalASCIIStringPrimitive");
  deserializeExternalStringImpl<char>(d);
}

void ExternalUTF16StringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::ExternalUTF16StringPrimitiveKind &&
      "Expected ExternalUTF16StringPrimitive");
  deserializeExternalStringImpl<char16_t>(d);
}
#endif

template <typename T>
CallResult<HermesValue> StringPrimitive::createEfficientImpl(
    Runtime *runtime,
    llvh::ArrayRef<T> str,
    std::basic_string<T> *optStorage) {
  constexpr bool charIs8Bit = std::is_same<T, char>::value;
  assert(
      (!optStorage ||
       str == llvh::makeArrayRef(optStorage->data(), optStorage->size())) &&
      "If optStorage is provided, it must equal the input string");
  assert(
      (!charIs8Bit || isAllASCII(str.begin(), str.end())) &&
      "8 bit strings must be ASCII");
  if (str.empty()) {
    return HermesValue::encodeStringValue(
        runtime->getPredefinedString(Predefined::emptyString));
  }
  if (str.size() == 1) {
    return runtime->getCharacterString(str[0]).getHermesValue();
  }

  // Check if we should acquire ownership of storage.
  if (optStorage != nullptr &&
      str.size() >= StringPrimitive::EXTERNAL_STRING_MIN_SIZE) {
    return ExternalStringPrimitive<T>::create(runtime, std::move(*optStorage));
  }

  // Check if we fit in ASCII.
  // We are ASCII if we are 8 bit, or we are 16 bit and all of our text is
  // ASCII.
  bool isAscii = charIs8Bit || isAllASCII(str.begin(), str.end());
  if (isAscii) {
    auto result = StringPrimitive::create(runtime, str.size(), isAscii);
    if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto output = runtime->makeHandle<StringPrimitive>(*result);
    // Copy directly into the StringPrimitive storage.
    std::copy(str.begin(), str.end(), output->castToASCIIPointerForWrite());
    return output.getHermesValue();
  }

  return StringPrimitive::create(runtime, str);
}

CallResult<HermesValue> StringPrimitive::createEfficient(
    Runtime *runtime,
    ASCIIRef str) {
  return createEfficientImpl(runtime, str);
}

CallResult<HermesValue> StringPrimitive::createEfficient(
    Runtime *runtime,
    UTF16Ref str) {
  return createEfficientImpl(runtime, str);
}

CallResult<HermesValue> StringPrimitive::createEfficient(
    Runtime *runtime,
    std::basic_string<char> &&str) {
  return createEfficientImpl(
      runtime, llvh::makeArrayRef(str.data(), str.size()), &str);
}

CallResult<HermesValue> StringPrimitive::createEfficient(
    Runtime *runtime,
    std::basic_string<char16_t> &&str) {
  return createEfficientImpl(
      runtime, llvh::makeArrayRef(str.data(), str.size()), &str);
}

CallResult<HermesValue> StringPrimitive::createDynamic(
    Runtime *runtime,
    UTF16Ref str) {
  if (LLVM_LIKELY(isAllASCII(str.begin(), str.end()))) {
    auto res = DynamicASCIIStringPrimitive::create(runtime, str.size());
    if (LLVM_UNLIKELY(res == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // Copy directly into the StringPrimitive storage.
    std::copy(
        str.begin(), str.end(), res->getString()->castToASCIIPointerForWrite());
    return res;
  } else {
    return DynamicUTF16StringPrimitive::create(runtime, str);
  }
}

bool StringPrimitive::sliceEquals(
    uint32_t start,
    uint32_t length,
    const StringPrimitive *other) const {
  if (isASCII()) {
    if (other->isASCII()) {
      return stringRefEquals(
          castToASCIIRef(start, length), other->castToASCIIRef());
    }
    return stringRefEquals(
        castToASCIIRef(start, length), other->castToUTF16Ref());
  }
  if (other->isASCII()) {
    return stringRefEquals(
        castToUTF16Ref(start, length), other->castToASCIIRef());
  }
  return stringRefEquals(
      castToUTF16Ref(start, length), other->castToUTF16Ref());
}

bool StringPrimitive::equals(const StringPrimitive *other) const {
  if (this == other) {
    return true;
  }
  return sliceEquals(0, getStringLength(), other);
}

bool StringPrimitive::equals(const StringView &other) const {
  if (isASCII()) {
    return other.equals(castToASCIIRef());
  }
  return other.equals(castToUTF16Ref());
}

int StringPrimitive::compare(const StringPrimitive *other) const {
  if (isASCII()) {
    if (other->isASCII()) {
      return stringRefCompare(castToASCIIRef(), other->castToASCIIRef());
    }
    return stringRefCompare(castToASCIIRef(), other->castToUTF16Ref());
  }
  if (other->isASCII()) {
    return stringRefCompare(castToUTF16Ref(), other->castToASCIIRef());
  }
  return stringRefCompare(castToUTF16Ref(), other->castToUTF16Ref());
}

CallResult<HermesValue> StringPrimitive::concat(
    Runtime *runtime,
    Handle<StringPrimitive> xHandle,
    Handle<StringPrimitive> yHandle) {
  auto *xPtr = xHandle.get();
  auto *yPtr = yHandle.get();

  auto xLen = xPtr->getStringLength();
  auto yLen = yPtr->getStringLength();
  if (!xLen) {
    // x is the empty string, just return y.
    return yHandle.getHermesValue();
  }
  if (!yLen) {
    // y is the empty string, just return x.
    return xHandle.getHermesValue();
  }

  SafeUInt32 xyLen(xLen);
  xyLen.add(yLen);
  if (xyLen.isOverflowed() || xyLen.get() > MAX_STRING_LENGTH) {
    return runtime->raiseRangeError("String length exceeds limit");
  }

  if (xyLen.get() >= CONCAT_STRING_MIN_SIZE ||
      isBufferedStringPrimitive(xPtr)) {
    if (LLVM_UNLIKELY(
            !runtime->getHeap().canAllocExternalMemory(xyLen.get()))) {
      return runtime->raiseRangeError(
          "Cannot allocate an external string primitive.");
    }
    return internalConcatStringPrimitives(runtime, xHandle, yHandle)
        .getHermesValue();
  }

  auto builder = StringBuilder::createStringBuilder(
      runtime, xyLen, xPtr->isASCII() && yPtr->isASCII());
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  builder->appendStringPrim(xHandle);
  builder->appendStringPrim(yHandle);
  return HermesValue::encodeStringValue(*builder->getStringPrimitive());
}

CallResult<HermesValue> StringPrimitive::slice(
    Runtime *runtime,
    Handle<StringPrimitive> str,
    size_t start,
    size_t length) {
  assert(
      start + length <= str->getStringLength() && "Invalid length for slice");

  SafeUInt32 safeLen(length);

  auto builder =
      StringBuilder::createStringBuilder(runtime, safeLen, str->isASCII());
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  if (str->isASCII()) {
    builder->appendASCIIRef(
        ASCIIRef(str->castToASCIIPointer() + start, length));
  } else {
    builder->appendUTF16Ref(
        UTF16Ref(str->castToUTF16Pointer() + start, length));
  }
  return HermesValue::encodeStringValue(*builder->getStringPrimitive());
}

StringView StringPrimitive::createStringView(
    Runtime *runtime,
    Handle<StringPrimitive> self) {
  ensureFlat(runtime, self);
  return createStringViewMustBeFlat(self);
}

void StringPrimitive::appendUTF16String(
    llvh::SmallVectorImpl<char16_t> &str) const {
  if (isASCII()) {
    const char *ptr = castToASCIIPointer();
    str.append(ptr, ptr + getStringLength());
  } else {
    const char16_t *ptr = castToUTF16Pointer();
    str.append(ptr, ptr + getStringLength());
  }
}

void StringPrimitive::appendUTF16String(char16_t *ptr) const {
  if (isASCII()) {
    const char *src = castToASCIIPointer();
    std::copy(src, src + getStringLength(), ptr);
  } else {
    const char16_t *src = castToUTF16Pointer();
    std::copy(src, src + getStringLength(), ptr);
  }
}

StringView StringPrimitive::createStringViewMustBeFlat(
    Handle<StringPrimitive> self) {
  return StringView(self);
}

std::string StringPrimitive::_snapshotNameImpl(GCCell *cell, GC *gc) {
  auto *const self = vmcast<StringPrimitive>(cell);
  // Only convert up to EXTERNAL_STRING_THRESHOLD characters, because large
  // strings can cause crashes in the snapshot visualizer.
  std::string out;
  bool fullyWritten = true;
  if (self->isASCII()) {
    auto ref = self->castToASCIIRef();
    out = std::string{
        ref.begin(),
        std::min(
            static_cast<uint32_t>(ref.size()),
            toRValue(EXTERNAL_STRING_THRESHOLD))};
    fullyWritten = ref.size() <= EXTERNAL_STRING_THRESHOLD;
  } else {
    fullyWritten = convertUTF16ToUTF8WithReplacements(
        out, self->castToUTF16Ref(), EXTERNAL_STRING_THRESHOLD);
  }
  if (!fullyWritten) {
    // The string was truncated, add a truncation message
    out += "...(truncated by snapshot)...";
  }
  return out;
}

template <typename T, bool Uniqued>
DynamicStringPrimitive<T, Uniqued>::DynamicStringPrimitive(
    Runtime *runtime,
    Ref src)
    : DynamicStringPrimitive(runtime, (uint32_t)src.size()) {
  hermes::uninitializedCopy(
      src.begin(), src.end(), this->template getTrailingObjects<T>());
}

template <typename T, bool Uniqued>
CallResult<HermesValue> DynamicStringPrimitive<T, Uniqued>::create(
    Runtime *runtime,
    Ref str) {
  assert(!isExternalLength(str.size()) && "length should not be external");
  auto *cell = runtime->makeAVariable<DynamicStringPrimitive<T, Uniqued>>(
      allocationSize((uint32_t)str.size()), runtime, str);
  return HermesValue::encodeStringValue(cell);
}

template <typename T, bool Uniqued>
CallResult<HermesValue> DynamicStringPrimitive<T, Uniqued>::createLongLived(
    Runtime *runtime,
    Ref str) {
  assert(!isExternalLength(str.size()) && "length should not be external");
  auto *obj = runtime->makeAVariable<
      DynamicStringPrimitive<T, Uniqued>,
      HasFinalizer::No,
      LongLived::Yes>(allocationSize((uint32_t)str.size()), runtime, str);
  return HermesValue::encodeStringValue(obj);
}

template <typename T, bool Uniqued>
CallResult<HermesValue> DynamicStringPrimitive<T, Uniqued>::create(
    Runtime *runtime,
    uint32_t length) {
  auto *cell = runtime->makeAVariable<DynamicStringPrimitive<T, Uniqued>>(
      allocationSize(length), runtime, length);
  return HermesValue::encodeStringValue(cell);
}

template class DynamicStringPrimitive<char16_t, true /* Uniqued */>;
template class DynamicStringPrimitive<char, true /* Uniqued */>;
template class DynamicStringPrimitive<char16_t, false /* not Uniqued */>;
template class DynamicStringPrimitive<char, false /* not Uniqued */>;

// NOTE: this is a template method in a template class, thus the two separate
// template<> lines.
template <typename T>
template <class BasicString>
ExternalStringPrimitive<T>::ExternalStringPrimitive(
    Runtime *runtime,
    BasicString &&contents)
    : SymbolStringPrimitive(
          runtime,
          &vt,
          sizeof(ExternalStringPrimitive<T>),
          contents.size()),
      contents_(std::forward<BasicString>(contents)) {
  static_assert(
      std::is_same<T, typename BasicString::value_type>::value,
      "ExternalStringPrimitive mismatched char type");
  assert(
      getStringLength() >= EXTERNAL_STRING_MIN_SIZE &&
      "ExternalStringPrimitive length must be at least EXTERNAL_STRING_MIN_SIZE");
}

// NOTE: this is a template method in a template class, thus the two separate
// template<> lines.
template <typename T>
template <class BasicString>
CallResult<HermesValue> ExternalStringPrimitive<T>::create(
    Runtime *runtime,
    BasicString &&str) {
  static_assert(
      std::is_same<T, typename BasicString::value_type>::value,
      "ExternalStringPrimitive mismatched char type");
  if (LLVM_UNLIKELY(str.size() > MAX_STRING_LENGTH))
    return runtime->raiseRangeError("String length exceeds limit");
  // We have to use a variable sized alloc here even though the size is already
  // known, because ExternalStringPrimitive is derived from
  // VariableSizeRuntimeCell
  auto *extStr =
      runtime->makeAVariable<ExternalStringPrimitive<T>, HasFinalizer::Yes>(
          sizeof(ExternalStringPrimitive<T>),
          runtime,
          std::forward<BasicString>(str));
  runtime->getHeap().creditExternalMemory(
      extStr, extStr->calcExternalMemorySize());
  auto res = HermesValue::encodeStringValue(extStr);
  return res;
}

template <typename T>
CallResult<HermesValue> ExternalStringPrimitive<T>::createLongLived(
    Runtime *runtime,
    StdString &&str) {
  if (LLVM_UNLIKELY(str.size() > MAX_STRING_LENGTH))
    return runtime->raiseRangeError("String length exceeds limit");
  if (LLVM_UNLIKELY(!runtime->getHeap().canAllocExternalMemory(
          str.capacity() * sizeof(T)))) {
    return runtime->raiseRangeError(
        "Cannot allocate an external string primitive.");
  }
  // Use variable size alloc since ExternalStringPrimitive is derived from
  // VariableSizeRuntimeCell.
  auto *extStr = runtime->makeAVariable<
      ExternalStringPrimitive<T>,
      HasFinalizer::Yes,
      LongLived::Yes>(
      sizeof(ExternalStringPrimitive<T>), runtime, std::move(str));
  runtime->getHeap().creditExternalMemory(
      extStr, extStr->calcExternalMemorySize());
  return HermesValue::encodeStringValue(extStr);
}

template <typename T>
CallResult<HermesValue> ExternalStringPrimitive<T>::create(
    Runtime *runtime,
    uint32_t length) {
  assert(isExternalLength(length) && "length should be external");
  if (LLVM_UNLIKELY(length > MAX_STRING_LENGTH))
    return runtime->raiseRangeError("String length exceeds limit");
  uint32_t allocSize = length * sizeof(T);
  if (LLVM_UNLIKELY(!runtime->getHeap().canAllocExternalMemory(allocSize))) {
    return runtime->raiseRangeError(
        "Cannot allocate an external string primitive.");
  }
  return create(runtime, StdString(length, T(0)));
}

template <typename T>
void ExternalStringPrimitive<T>::_finalizeImpl(GCCell *cell, GC *gc) {
  ExternalStringPrimitive<T> *self = vmcast<ExternalStringPrimitive<T>>(cell);
  // Remove the external string from the snapshot tracking system if it's being
  // tracked.
  gc->getIDTracker().untrackNative(self->contents_.data());
  gc->debitExternalMemory(self, self->calcExternalMemorySize());
  self->~ExternalStringPrimitive<T>();
}

template <typename T>
size_t ExternalStringPrimitive<T>::_mallocSizeImpl(GCCell *cell) {
  ExternalStringPrimitive<T> *self = vmcast<ExternalStringPrimitive<T>>(cell);
  return self->calcExternalMemorySize();
}

template <typename T>
gcheapsize_t ExternalStringPrimitive<T>::_externalMemorySizeImpl(
    hermes::vm::GCCell const *cell) {
  auto *self = vmcast<ExternalStringPrimitive<T>>(cell);
  return self->calcExternalMemorySize();
}

template <typename T>
void ExternalStringPrimitive<T>::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<ExternalStringPrimitive<T>>(cell);
  snap.addNamedEdge(
      HeapSnapshot::EdgeType::Internal,
      "externalString",
      gc->getNativeID(self->contents_.data()));
}

template <typename T>
void ExternalStringPrimitive<T>::_snapshotAddNodesImpl(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) {
  auto *const self = vmcast<ExternalStringPrimitive<T>>(cell);
  snap.beginNode();
  snap.endNode(
      HeapSnapshot::NodeType::Native,
      "ExternalStringPrimitive",
      gc->getNativeID(self->contents_.data()),
      self->contents_.size(),
      0);
}

template class ExternalStringPrimitive<char16_t>;
template class ExternalStringPrimitive<char>;

template CallResult<HermesValue> ExternalStringPrimitive<char>::create<
    std::basic_string<char>>(Runtime *runtime, std::basic_string<char> &&str);

template CallResult<HermesValue>
ExternalStringPrimitive<char16_t>::create<std::basic_string<char16_t>>(
    Runtime *runtime,
    std::basic_string<char16_t> &&str);

//===----------------------------------------------------------------------===//
// BufferedStringPrimitive<T>

void BufferedASCIIStringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  const auto *self = static_cast<const BufferedASCIIStringPrimitive *>(cell);
  mb.setVTable(&BufferedASCIIStringPrimitive::vt);
  mb.addField("storage", &self->concatBufferHV_);
}
void BufferedUTF16StringPrimitiveBuildMeta(
    const GCCell *cell,
    Metadata::Builder &mb) {
  const auto *self = static_cast<const BufferedUTF16StringPrimitive *>(cell);
  mb.setVTable(&BufferedUTF16StringPrimitive::vt);
  mb.addField("storage", &self->concatBufferHV_);
}

template <typename T>
PseudoHandle<StringPrimitive> BufferedStringPrimitive<T>::create(
    Runtime *runtime,
    uint32_t length,
    Handle<ExternalStringPrimitive<T>> storage) {
  // We have to use a variable sized alloc here even though the size is already
  // known, because BufferedStringPrimitive is derived from
  // VariableSizeRuntimeCell.
  auto *cell = runtime->makeAVariable<BufferedStringPrimitive<T>>(
      sizeof(BufferedStringPrimitive<T>), runtime, length, storage);
  return createPseudoHandle<StringPrimitive>(cell);
}

#ifndef NDEBUG
/// Assert the the combined length of the two strings is valid.
static void assertValidLength(StringPrimitive *a, StringPrimitive *b) {
  SafeUInt32 len(a->getStringLength());
  len.add(b->getStringLength());
  assert(
      !len.isOverflowed() && len.get() <= StringPrimitive::MAX_STRING_LENGTH &&
      "length must have been validated");
}
#else
static inline void assertValidLength(StringPrimitive *a, StringPrimitive *b) {}
#endif

template <>
void BufferedStringPrimitive<char>::appendToCopyableString(
    CopyableBasicString<char> &res,
    const StringPrimitive *str) {
  auto it = str->castToASCIIPointer();
  res.append(it, it + str->getStringLength());
}
template <>
void BufferedStringPrimitive<char16_t>::appendToCopyableString(
    CopyableBasicString<char16_t> &res,
    const StringPrimitive *str) {
  if (str->isASCII()) {
    auto it = (const uint8_t *)str->castToASCIIPointer();
    res.append(it, it + str->getStringLength());
  } else {
    auto it = str->castToUTF16Pointer();
    res.append(it, it + str->getStringLength());
  }
}

template <typename T>
PseudoHandle<StringPrimitive> BufferedStringPrimitive<T>::create(
    Runtime *runtime,
    Handle<StringPrimitive> leftHnd,
    Handle<StringPrimitive> rightHnd) {
  typename ExternalStringPrimitive<T>::CopyableStdString contents{};
  uint32_t len;

  {
    NoAllocScope noAlloc{runtime};
    auto *left = leftHnd.get();
    auto *right = rightHnd.get();

    assertValidLength(left, right);
    len = left->getStringLength() + right->getStringLength();

    contents.reserve(len);
    appendToCopyableString(contents, left);
    appendToCopyableString(contents, right);
  }

  auto storageHnd = runtime->makeHandle<ExternalStringPrimitive<T>>(
      runtime->ignoreAllocationFailure(
          ExternalStringPrimitive<T>::create(runtime, std::move(contents))));

  return create(runtime, len, storageHnd);
}

template <typename T>
PseudoHandle<StringPrimitive> BufferedStringPrimitive<T>::append(
    Handle<BufferedStringPrimitive<T>> selfHnd,
    Runtime *runtime,
    Handle<StringPrimitive> rightHnd) {
  NoAllocScope noAlloc{runtime};
  auto *self = selfHnd.get();
  auto *right = rightHnd.get();
  ExternalStringPrimitive<T> *storage = self->getConcatBuffer();

  assertValidLength(self, right);
  assert(
      (std::is_same<T, char16_t>::value || right->isASCII()) &&
      "cannot append UTF16 to ASCII");

  // Can't append if this is not the end of the string.
  if (self->getStringLength() != storage->contents_.size()) {
    noAlloc.release();
    return BufferedStringPrimitive<T>::create(runtime, selfHnd, rightHnd);
  }

  auto oldExternalMem = storage->calcExternalMemorySize();
  appendToCopyableString(storage->contents_, right);
  runtime->getHeap().creditExternalMemory(
      storage, storage->calcExternalMemorySize() - oldExternalMem);

  noAlloc.release();
  return BufferedStringPrimitive<T>::create(
      runtime, storage->contents_.size(), runtime->makeHandle(storage));
}

PseudoHandle<StringPrimitive> internalConcatStringPrimitives(
    Runtime *runtime,
    Handle<StringPrimitive> leftHnd,
    Handle<StringPrimitive> rightHnd) {
  auto *left = leftHnd.get();
  auto *right = rightHnd.get();

  assertValidLength(left, right);

  if (left->isASCII() && right->isASCII()) {
    if (auto *bufLeft = dyn_vmcast<BufferedASCIIStringPrimitive>(left)) {
      if (bufLeft->getStringLength() ==
          bufLeft->getConcatBuffer()->contents_.size())
        return BufferedASCIIStringPrimitive::append(
            Handle<BufferedASCIIStringPrimitive>::vmcast(leftHnd),
            runtime,
            rightHnd);
    }
    return BufferedASCIIStringPrimitive::create(runtime, leftHnd, rightHnd);
  } else {
    if (auto *bufLeft = dyn_vmcast<BufferedUTF16StringPrimitive>(left)) {
      if (bufLeft->getStringLength() ==
          bufLeft->getConcatBuffer()->contents_.size()) {
        return BufferedUTF16StringPrimitive::append(
            Handle<BufferedUTF16StringPrimitive>::vmcast(leftHnd),
            runtime,
            rightHnd);
      }
    }
    return BufferedUTF16StringPrimitive::create(runtime, leftHnd, rightHnd);
  }
}

#ifdef HERMESVM_SERIALIZE
template <typename T>
void serializeConcatStringImpl(Serializer &s, const GCCell *cell) {}

template <typename T>
void deserializeConcatStringImpl(Deserializer &d) {}

void BufferedASCIIStringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeConcatStringImpl<char>(s, cell);
}

void BufferedUTF16StringPrimitiveSerialize(Serializer &s, const GCCell *cell) {
  serializeConcatStringImpl<char16_t>(s, cell);
}

void BufferedASCIIStringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::BufferedASCIIStringPrimitiveKind &&
      "Expected BufferedASCIIStringPrimitive");
  deserializeConcatStringImpl<char>(d);
}

void BufferedUTF16StringPrimitiveDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::BufferedUTF16StringPrimitiveKind &&
      "Expected BufferedUTF16StringPrimitive");
  deserializeConcatStringImpl<char16_t>(d);
}
#endif

template <typename T>
void BufferedStringPrimitive<T>::_snapshotAddEdgesImpl(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) {}

template <typename T>
void BufferedStringPrimitive<T>::_snapshotAddNodesImpl(
    GCCell *cell,
    GC *gc,
    HeapSnapshot &snap) {}

template class BufferedStringPrimitive<char16_t>;
template class BufferedStringPrimitive<char>;
} // namespace vm
} // namespace hermes
