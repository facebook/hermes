/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSRegExp.h"

#include "hermes/Regex/Executor.h"
#include "hermes/Regex/Regex.h"
#include "hermes/Regex/RegexTraits.h"
#include "hermes/Support/UTF8.h"
#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/RegExpMatch.h"
#include "hermes/VM/StringView.h"

#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSRegExp

ObjectVTable JSRegExp::vt{
    VTable(
        CellKind::RegExpKind,
        cellSize<JSRegExp>(),
        JSRegExp::_finalizeImpl,
        nullptr,
        JSRegExp::_mallocSizeImpl,
        nullptr,
        nullptr,
        nullptr, // externalMemorySize
        VTable::HeapSnapshotMetadata{HeapSnapshot::NodeType::Regexp,
                                     JSRegExp::_snapshotNameImpl,
                                     JSRegExp::_snapshotAddEdgesImpl,
                                     JSRegExp::_snapshotAddNodesImpl,
                                     nullptr}),
    JSRegExp::_getOwnIndexedRangeImpl,
    JSRegExp::_haveOwnIndexedImpl,
    JSRegExp::_getOwnIndexedPropertyFlagsImpl,
    JSRegExp::_getOwnIndexedImpl,
    JSRegExp::_setOwnIndexedImpl,
    JSRegExp::_deleteOwnIndexedImpl,
    JSRegExp::_checkAllOwnIndexedImpl,
};

void RegExpBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(JSObject::numOverlapSlots<JSRegExp>());
  ObjectBuildMeta(cell, mb);
}

#ifdef HERMESVM_SERIALIZE
JSRegExp::JSRegExp(Deserializer &d) : JSObject(d, &vt.base) {
  uint32_t size = d.readInt<uint32_t>();
  initializeBytecode(d.readArrayRef<uint8_t>(size), d.getRuntime());
  // bytecode_ is tracked by IDTracker for heapsnapshot. We should do
  // relocation for it.
  d.endObject(bytecode_);

  d.readData(&syntaxFlags_, sizeof(syntaxFlags_));
}

void RegExpSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSRegExp>(cell);
  JSObject::serializeObjectImpl(s, cell, JSObject::numOverlapSlots<JSRegExp>());
  s.writeInt<uint32_t>(self->bytecodeSize_);
  s.writeData(self->bytecode_, self->bytecodeSize_);
  // bytecode_ is tracked by IDTracker for heapsnapshot. We should do
  // relocation for it.
  s.endObject(self->bytecode_);

  s.writeData(&self->syntaxFlags_, sizeof(self->syntaxFlags_));

  s.endObject(cell);
}

void RegExpDeserialize(Deserializer &d, CellKind kind) {
  assert(kind == CellKind::RegExpKind && "Expected RegExp");
  void *mem = d.getRuntime()->alloc</*fixedSize*/ true, HasFinalizer::Yes>(
      cellSize<JSRegExp>());
  auto *cell = new (mem) JSRegExp(d);
  d.endObject(cell);
}
#endif

Handle<JSRegExp> JSRegExp::create(
    Runtime *runtime,
    Handle<JSObject> parentHandle) {
  JSObjectAlloc<JSRegExp, HasFinalizer::Yes> mem{runtime};
  auto selfHandle = mem.initToHandle(new (mem) JSRegExp(
      runtime,
      *parentHandle,
      runtime->getHiddenClassForPrototypeRaw(
          *parentHandle,
          numOverlapSlots<JSRegExp>() + ANONYMOUS_PROPERTY_SLOTS)));

  JSObject::setInternalProperty(
      *selfHandle,
      runtime,
      patternPropIndex(),
      HermesValue::encodeStringValue(
          runtime->getPredefinedString(Predefined::emptyString)));

  return selfHandle;
}

void JSRegExp::initializeProperties(
    Handle<JSRegExp> selfHandle,
    Runtime *runtime,
    Handle<StringPrimitive> pattern) {
  JSObject::setInternalProperty(
      selfHandle.get(), runtime, patternPropIndex(), pattern.getHermesValue());

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.enumerable = 0;
  dpf.configurable = 0;

  auto res = JSObject::defineOwnProperty(
      selfHandle,
      runtime,
      Predefined::getSymbolID(Predefined::lastIndex),
      dpf,
      runtime->makeHandle(HermesValue::encodeNumberValue(0)));
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
}

ExecutionStatus JSRegExp::initialize(
    Handle<JSRegExp> selfHandle,
    Runtime *runtime,
    Handle<JSRegExp> otherHandle,
    Handle<StringPrimitive> flags) {
  llvh::SmallVector<char16_t, 16> flagsText16;
  flags->appendUTF16String(flagsText16);

  auto sflags = regex::SyntaxFlags::fromString(flagsText16);
  if (!sflags) {
    return runtime->raiseSyntaxError("Invalid RegExp: Invalid flags");
  }

  auto pattern = runtime->makeHandle(getPattern(otherHandle.get(), runtime));

  // Fast path to avoid recompiling the RegExp if the flags match
  if (LLVM_LIKELY(
          sflags->toByte() == getSyntaxFlags(otherHandle.get()).toByte())) {
    initializeProperties(selfHandle, runtime, pattern);
    selfHandle->syntaxFlags_ = *sflags;
    return selfHandle->initializeBytecode(
        {otherHandle->bytecode_, otherHandle->bytecodeSize_}, runtime);
  }
  return initialize(selfHandle, runtime, pattern, flags, llvh::None);
}

/// ES11 21.2.3.2.2 RegExpInitialize ( obj, pattern, flags )
ExecutionStatus JSRegExp::initialize(
    Handle<JSRegExp> selfHandle,
    Runtime *runtime,
    Handle<StringPrimitive> pattern,
    Handle<StringPrimitive> flags,
    OptValue<llvh::ArrayRef<uint8_t>> bytecode) {
  assert(
      pattern && flags &&
      "Null pattern and/or flags passed to JSRegExp::initialize");
  initializeProperties(selfHandle, runtime, pattern);

  if (bytecode) {
    return selfHandle->initializeBytecode(*bytecode, runtime);
  } else {
    llvh::SmallVector<char16_t, 6> flagsText16;
    flags->appendUTF16String(flagsText16);

    llvh::SmallVector<char16_t, 16> patternText16;
    pattern->appendUTF16String(patternText16);

    // Build the regex.
    regex::Regex<regex::UTF16RegexTraits> regex(patternText16, flagsText16);

    if (!regex.valid()) {
      return runtime->raiseSyntaxError(
          TwineChar16("Invalid RegExp: ") +
          regex::constants::messageForError(regex.getError()));
    }
    // The regex is valid. Compile and store its bytecode.
    auto bytecode = regex.compile();
    return selfHandle->initializeBytecode(bytecode, runtime);
  }
}

ExecutionStatus JSRegExp::initializeBytecode(
    llvh::ArrayRef<uint8_t> bytecode,
    Runtime *runtime) {
  size_t sz = bytecode.size();
  if (sz > std::numeric_limits<decltype(bytecodeSize_)>::max()) {
    return runtime->raiseRangeError("RegExp size overflow");
  }
  auto header =
      reinterpret_cast<const regex::RegexBytecodeHeader *>(bytecode.data());
  syntaxFlags_ = regex::SyntaxFlags::fromByte(header->syntaxFlags);
  bytecodeSize_ = sz;
  bytecode_ = (uint8_t *)checkedMalloc(sz);
  memcpy(bytecode_, bytecode.data(), sz);
  return ExecutionStatus::RETURNED;
}

PseudoHandle<StringPrimitive> JSRegExp::getPattern(
    JSRegExp *self,
    PointerBase *base) {
  return createPseudoHandle(
      JSObject::getInternalProperty(self, base, patternPropIndex())
          .getString());
}

template <typename CharT, typename Traits>
CallResult<RegExpMatch> performSearch(
    Runtime *runtime,
    llvh::ArrayRef<uint8_t> bytecode,
    const CharT *start,
    uint32_t stringLength,
    uint32_t searchStartOffset,
    regex::constants::MatchFlagType matchFlags) {
  std::vector<regex::CapturedRange> nativeMatchRanges;
  auto matchResult = regex::searchWithBytecode(
      bytecode,
      start,
      searchStartOffset,
      stringLength,
      &nativeMatchRanges,
      matchFlags);
  if (matchResult == regex::MatchRuntimeResult::StackOverflow) {
    return runtime->raiseRangeError("Maximum regex stack depth reached");
  } else if (matchResult == regex::MatchRuntimeResult::NoMatch) {
    return RegExpMatch{}; // not found.
  }
  size_t matchRangeCount = nativeMatchRanges.size();
  assert(matchRangeCount > 0);
  RegExpMatch match;
  match.reserve(matchRangeCount);
  for (size_t i = 0; i < matchRangeCount; i++) {
    const auto &submatch = nativeMatchRanges[i];
    if (!submatch.matched()) {
      assert(i > 0 && "match_result[0] should always match");
      match.push_back(llvh::None);
    } else {
      uint32_t pos = submatch.start;
      uint32_t length = submatch.end - submatch.start;
      match.push_back(RegExpMatchRange{pos, length});
    }
  }
  assert(!match.empty() && "Unexpected empty match");
  return match;
}

CallResult<RegExpMatch> JSRegExp::search(
    Handle<JSRegExp> selfHandle,
    Runtime *runtime,
    Handle<StringPrimitive> strHandle,
    uint32_t searchStartOffset) {
  assert(selfHandle->bytecode_ && "Missing bytecode");
  auto input = StringPrimitive::createStringView(runtime, strHandle);

  // Note we may still have a match if searchStartOffset == str.size(),
  // if the regexp can match an empty string
  if (searchStartOffset > input.length()) {
    return RegExpMatch{}; // no match possible
  }

  auto matchFlags = regex::constants::matchDefault;

  // Respect the sticky flag, which forces us to match only at the given
  // location.
  if (selfHandle->syntaxFlags_.sticky) {
    matchFlags |= regex::constants::matchOnlyAtStart;
  }

  CallResult<RegExpMatch> matchResult = RegExpMatch{};
  if (input.isASCII()) {
    matchFlags |= regex::constants::matchInputAllAscii;
    matchResult = performSearch<char, regex::ASCIIRegexTraits>(
        runtime,
        llvh::makeArrayRef(selfHandle->bytecode_, selfHandle->bytecodeSize_),
        input.castToCharPtr(),
        input.length(),
        searchStartOffset,
        matchFlags);
  } else {
    matchResult = performSearch<char16_t, regex::UTF16RegexTraits>(
        runtime,
        llvh::makeArrayRef(selfHandle->bytecode_, selfHandle->bytecodeSize_),
        input.castToChar16Ptr(),
        input.length(),
        searchStartOffset,
        matchFlags);
  }

  // Only update on successful match.
  if (LLVM_UNLIKELY(matchResult == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  } else if (!matchResult->empty()) {
    runtime->regExpLastInput = strHandle.getHermesValue();
    runtime->regExpLastRegExp = selfHandle.getHermesValue();
    runtime->regExpLastMatch = *matchResult;
  }
  return matchResult;
}

JSRegExp::~JSRegExp() {
  free(bytecode_);
}

void JSRegExp::_finalizeImpl(GCCell *cell, GC *gc) {
  JSRegExp *self = vmcast<JSRegExp>(cell);
  if (self->bytecode_) {
    gc->getIDTracker().untrackNative(self->bytecode_);
  }
  self->~JSRegExp();
}

size_t JSRegExp::_mallocSizeImpl(GCCell *cell) {
  auto *self = vmcast<JSRegExp>(cell);
  return self->bytecodeSize_;
}

std::string JSRegExp::_snapshotNameImpl(GCCell *cell, GC *gc) {
  auto *const self = vmcast<JSRegExp>(cell);
  return converter(getPattern(self, gc->getPointerBase()).get());
}

void JSRegExp::_snapshotAddEdgesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap) {
  auto *const self = vmcast<JSRegExp>(cell);
  // Call the super type to add any other custom edges.
  JSObject::_snapshotAddEdgesImpl(self, gc, snap);
  if (self->bytecode_) {
    snap.addNamedEdge(
        HeapSnapshot::EdgeType::Internal,
        "bytecode",
        gc->getNativeID(self->bytecode_));
  }
}

void JSRegExp::_snapshotAddNodesImpl(GCCell *cell, GC *gc, HeapSnapshot &snap) {
  auto *const self = vmcast<JSRegExp>(cell);
  if (self->bytecode_) {
    // Add a native node for regex bytecode, to account for native size
    // directly owned by the regex.
    snap.beginNode();
    snap.endNode(
        HeapSnapshot::NodeType::Native,
        "RegExpBytecode",
        gc->getNativeID(self->bytecode_),
        self->bytecodeSize_,
        0);
  }
}

/// \return an escaped string equivalent to \p pattern.
/// This is used to construct the 'source' property of RegExp. This requires
/// us to return a string from which the regexp may be reconstructed as if
/// from a /foo/ style literal. Note this is different from the RegExp
/// constructor that takes a string, e.g. new RegExp("/") returns a regexp
/// that matches /, but
/// /// does not (it's a comment!). So we may have to perform surgery on the
/// pattern.
CallResult<HermesValue> JSRegExp::escapePattern(
    Handle<StringPrimitive> pattern,
    Runtime *runtime) {
  SmallU16String<32> result;
  result.reserve(pattern->getStringLength());
  auto patternView = StringPrimitive::createStringView(runtime, pattern);
  bool isBackslashed = false;
  for (char16_t c : patternView) {
    switch (c) {
      case u'/':
        // Avoid premature end of regex.
        // TODO nice to have: don't do this if we are in square brackets.
        // /[/]/ is valid and the middle / does not need to be escaped.
        // However /[\/]/ is also valid and means the same thing
        // (CharacterEscape production from regexp grammar). Still it would be
        // nice to not unnecessarily mangle the user's supplied pattern.
        result.append(isBackslashed ? "/" : "\\/");
        break;

      // Escape line terminators. See ES5.1 7.3.
      case u'\n':
        result.append(isBackslashed ? "n" : "\\n");
        break;

      case u'\r':
        result.append(isBackslashed ? "r" : "\\r");
        break;

      case 0x2028:
        result.append(isBackslashed ? "u2028" : "\\u2028");
        break;

      case 0x2029:
        result.append(isBackslashed ? "u2029" : "\\u2029");
        break;

      default:
        result.append(c);
        break;
    }
    isBackslashed = (c == u'\\') && !isBackslashed;
  }
  // "If P is the empty String, this specification can be met by letting S be
  // '(?:)'."
  if (result.empty()) {
    result = u"(?:)";
  }

  // Avoid unnecessary allocation in the likely event the source and pattern
  // match.
  if (patternView.equals(result.arrayRef())) {
    return pattern.getHermesValue();
  }
  return StringPrimitive::create(runtime, result);
}

} // namespace vm
} // namespace hermes
