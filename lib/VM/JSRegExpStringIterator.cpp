/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/JSRegExpStringIterator.h"

#include "hermes/VM/BuildMetadata.h"
#include "hermes/VM/StringPrimitive.h"

#include "JSLib/JSLibInternal.h"
#include "llvh/Support/Debug.h"
#define DEBUG_TYPE "serialize"

namespace hermes {
namespace vm {

//===----------------------------------------------------------------------===//
// class JSRegExpStringIterator

ObjectVTable JSRegExpStringIterator::vt{
    VTable(
        CellKind::RegExpStringIteratorKind,
        cellSize<JSRegExpStringIterator>()),
    JSRegExpStringIterator::_getOwnIndexedRangeImpl,
    JSRegExpStringIterator::_haveOwnIndexedImpl,
    JSRegExpStringIterator::_getOwnIndexedPropertyFlagsImpl,
    JSRegExpStringIterator::_getOwnIndexedImpl,
    JSRegExpStringIterator::_setOwnIndexedImpl,
    JSRegExpStringIterator::_deleteOwnIndexedImpl,
    JSRegExpStringIterator::_checkAllOwnIndexedImpl,
};

void RegExpStringIteratorBuildMeta(const GCCell *cell, Metadata::Builder &mb) {
  mb.addJSObjectOverlapSlots(
      JSObject::numOverlapSlots<JSRegExpStringIterator>());
  ObjectBuildMeta(cell, mb);
  const auto *self = static_cast<const JSRegExpStringIterator *>(cell);
  mb.addField("iteratedRegExp", &self->iteratedRegExp_);
  mb.addField("iteratedString", &self->iteratedString_);
}

#ifdef HERMESVM_SERIALIZE
JSRegExpStringIterator::JSRegExpStringIterator(Deserializer &d)
    : JSObject(d, &vt.base) {
  d.readRelocation(&iteratedRegExp_, RelocationKind::GCPointer);
  d.readRelocation(&iteratedString_, RelocationKind::GCPointer);
  global_ = d.readInt<bool>();
  unicode_ = d.readInt<bool>();
  done_ = d.readInt<bool>();
}

void RegExpStringIteratorSerialize(Serializer &s, const GCCell *cell) {
  auto *self = vmcast<const JSRegExpStringIterator>(cell);
  JSObject::serializeObjectImpl(
      s, cell, JSObject::numOverlapSlots<JSRegExpStringIterator>());
  s.writeRelocation(self->iteratedRegExp_.get(s.getRuntime()));
  s.writeRelocation(self->iteratedString_.get(s.getRuntime()));
  s.writeInt<bool>(self->global_);
  s.writeInt<bool>(self->unicode_);
  s.writeInt<bool>(self->done_);
  s.endObject(cell);
}

void RegExpStringIteratorDeserialize(Deserializer &d, CellKind kind) {
  assert(
      kind == CellKind::RegExpStringIteratorKind &&
      "Expected RegExpStringIterator");
  void *mem = d.getRuntime()->alloc(cellSize<JSRegExpStringIterator>());
  auto *cell = new (mem) JSRegExpStringIterator(d);
  d.endObject(cell);
}
#endif

/// ES11 21.2.5.8.1 CreateRegExpStringIterator ( R, S, global, fullUnicode )
PseudoHandle<JSRegExpStringIterator> JSRegExpStringIterator::create(
    Runtime *runtime,
    Handle<JSObject> R,
    Handle<StringPrimitive> S,
    bool global,
    bool fullUnicode) {
  auto proto =
      Handle<JSObject>::vmcast(&runtime->regExpStringIteratorPrototype);

  JSObjectAlloc<JSRegExpStringIterator> mem{runtime};
  return mem.initToPseudoHandle(new (mem) JSRegExpStringIterator(
      runtime,
      *proto,
      runtime->getHiddenClassForPrototypeRaw(
          *proto,
          numOverlapSlots<JSRegExpStringIterator>() + ANONYMOUS_PROPERTY_SLOTS),
      *R,
      *S,
      global,
      fullUnicode));
}

/// ES11 21.2.7.1.1 %RegExpStringIteratorPrototype%.next ( ) 4-11
CallResult<HermesValue> JSRegExpStringIterator::nextElement(
    Handle<JSRegExpStringIterator> O,
    Runtime *runtime) {
  // 4. If O.[[Done]] is true, then
  if (O->done_) {
    // a. Return ! CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
        .getHermesValue();
  }
  // 5. Let R be O.[[IteratingRegExp]].
  auto R = runtime->makeHandle(O->iteratedRegExp_);
  // 6. Let S be O.[[IteratedString]].
  auto S = runtime->makeHandle(O->iteratedString_);

  // 7. Let global be O.[[Global]].
  // 8. Let fullUnicode be O.[[Unicode]].
  // we will just use JSRegExpStringIterator's fields for non pointer types.

  // 9. Let match be ? RegExpExec(R, S).
  auto matchRes = regExpExec(runtime, R, S);
  if (LLVM_UNLIKELY(matchRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto match = matchRes.getValue();

  // 10. If match is null, then
  if (match.isNull()) {
    // a. Set O.[[Done]] to true.
    O->done_ = true;
    // b. Return ! CreateIterResultObject(undefined, true).
    return createIterResultObject(runtime, Runtime::getUndefinedValue(), true)
        .getHermesValue();
  } else {
    // 11. Else,
    auto matchObj = runtime->makeHandle<JSObject>(match);
    // a. If global is true, then
    if (O->global_) {
      //  i. Let matchStr be ? ToString(? Get(match, "0")).
      Handle<> zeroHandle =
          runtime->makeHandle(HermesValue::encodeNumberValue(0));
      auto propRes = JSObject::getComputed_RJS(matchObj, runtime, zeroHandle);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto matchStrRes =
          toString_RJS(runtime, runtime->makeHandle(std::move(*propRes)));
      if (matchStrRes == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      auto matchStr = runtime->makeHandle(std::move(*matchStrRes));

      //  ii. If matchStr is the empty String, then
      if (matchStr->getStringLength() == 0) {
        // 1. Let thisIndex be ? ToLength(? Get(R, "lastIndex")).
        auto lastIndexRes = runtime->getNamed(R, PropCacheID::RegExpLastIndex);
        if (lastIndexRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        auto lastIndex = runtime->makeHandle(std::move(*lastIndexRes));
        auto thisIndex = toLength(runtime, lastIndex);
        if (thisIndex == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        // 2. Let nextIndex be ! AdvanceStringIndex(S, thisIndex, fullUnicode).
        double nextIndex = advanceStringIndex(
            S.get(), thisIndex->getNumberAs<uint64_t>(), O->unicode_);
        // 3. Perform ? Set(R, "lastIndex", nextIndex, true).
        if (setLastIndex(R, runtime, nextIndex) == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      // iii. Return ! CreateIterResultObject(match, false).
      return createIterResultObject(runtime, matchObj, false).getHermesValue();
    }
    // b. Else,
    else {
      // i. Set O.[[Done]] to true.
      O->done_ = true;
      // ii. Return ! CreateIterResultObject(match, false).
      return createIterResultObject(runtime, matchObj, false).getHermesValue();
    }
  }
}

} // namespace vm
} // namespace hermes
