/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.10.3 RegExp (Regular Expression) Objects
//===----------------------------------------------------------------------===//

#include "JSLibInternal.h"

#include "hermes/VM/Operations.h"
#include "hermes/VM/SmallXString.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

/// Internal methods
/// {@

/// 21.2.3.2.1 Runtime Semantics: RegExpAlloc ( newTarget )
static CallResult<HermesValue> regExpAlloc(
    Runtime *runtime,
    PseudoHandle<> /* newTarget */);

/// 21.2.3.2.2 Runtime Semantics: RegExpInitialize ( obj, pattern, flags )
static CallResult<Handle<JSRegExp>> regExpInitialize(
    Runtime *runtime,
    Handle<> obj,
    Handle<> pattern,
    Handle<> flags);

/// ES6.0 21.2.5.2.1
static CallResult<HermesValue>
regExpExec(Runtime *runtime, Handle<JSObject> R, Handle<StringPrimitive> S);

/// ES6.0 21.2.5.2.2
static CallResult<HermesValue> regExpBuiltinExec(
    Runtime *runtime,
    Handle<JSRegExp> R,
    Handle<StringPrimitive> S);

/// ES6.0 21.2.5.2.3
static uint64_t advanceStringIndex(
    Runtime *runtime,
    PseudoHandle<StringPrimitive> /* S */,
    uint64_t index,
    bool unicode);

/// @}

// Several RegExp accessors are defined to do particular things when passed the
// RegExp object (which is not itself a RegExp). Centralize that logic here.
static inline bool thisIsRegExpProto(Runtime *runtime, NativeArgs args) {
  return args.dyncastThis<JSObject>().get() ==
      vmcast<JSObject>(runtime->regExpPrototype);
}

Handle<JSObject> createRegExpConstructor(Runtime *runtime) {
  auto proto = Handle<JSObject>::vmcast(&runtime->regExpPrototype);

  auto cons = defineSystemConstructor<JSRegExp>(
      runtime,
      Predefined::getSymbolID(Predefined::RegExp),
      regExpConstructor,
      proto,
      2,
      CellKind::RegExpKind);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::exec),
      nullptr,
      regExpPrototypeExec,
      1);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::test),
      nullptr,
      regExpPrototypeTest,
      1);

  defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::toString),
      nullptr,
      regExpPrototypeToString,
      0);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.enumerable = 0;
  (void)defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolMatch),
      Predefined::getSymbolID(Predefined::squareSymbolMatch),
      nullptr,
      regExpPrototypeSymbolMatch,
      1,
      dpf);

  (void)defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolSearch),
      Predefined::getSymbolID(Predefined::squareSymbolSearch),
      nullptr,
      regExpPrototypeSymbolSearch,
      1,
      dpf);

  (void)defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolReplace),
      Predefined::getSymbolID(Predefined::squareSymbolReplace),
      nullptr,
      regExpPrototypeSymbolReplace,
      2,
      dpf);

  (void)defineMethod(
      runtime,
      proto,
      Predefined::getSymbolID(Predefined::SymbolSplit),
      Predefined::getSymbolID(Predefined::squareSymbolSplit),
      nullptr,
      regExpPrototypeSymbolSplit,
      2,
      dpf);

  // The RegExp prototype and constructors have a variety of getters defined on
  // it; use this helper to define them. Note the context is passed as an
  // intptr_t which we convert to void*.
  auto defineGetter = [&](Handle<JSObject> obj,
                          Predefined::Str sym,
                          NativeFunctionPtr getter,
                          intptr_t ctx = 0) {
    defineAccessor(
        runtime,
        obj,
        Predefined::getSymbolID(sym),
        ctx ? reinterpret_cast<void *>(ctx) : nullptr,
        getter,
        nullptr,
        false,
        true);
  };

  defineGetter(proto, Predefined::source, regExpSourceGetter);
  defineGetter(proto, Predefined::flags, regExpFlagsGetter);
  defineGetter(proto, Predefined::multiline, regExpFlagPropertyGetter, 'm');
  defineGetter(proto, Predefined::ignoreCase, regExpFlagPropertyGetter, 'i');
  defineGetter(proto, Predefined::global, regExpFlagPropertyGetter, 'g');

  defineGetter(cons, Predefined::dollar1, regExpDollarNumberGetter, 1);
  defineGetter(cons, Predefined::dollar2, regExpDollarNumberGetter, 2);
  defineGetter(cons, Predefined::dollar3, regExpDollarNumberGetter, 3);
  defineGetter(cons, Predefined::dollar4, regExpDollarNumberGetter, 4);
  defineGetter(cons, Predefined::dollar5, regExpDollarNumberGetter, 5);
  defineGetter(cons, Predefined::dollar6, regExpDollarNumberGetter, 6);
  defineGetter(cons, Predefined::dollar7, regExpDollarNumberGetter, 7);
  defineGetter(cons, Predefined::dollar8, regExpDollarNumberGetter, 8);
  defineGetter(cons, Predefined::dollar9, regExpDollarNumberGetter, 9);
  defineGetter(cons, Predefined::leftContext, regExpLeftContextGetter);
  defineGetter(cons, Predefined::dollarBacktick, regExpLeftContextGetter);
  defineGetter(cons, Predefined::rightContext, regExpRightContextGetter);
  defineGetter(cons, Predefined::dollarApostrophe, regExpRightContextGetter);
  defineGetter(cons, Predefined::dollarUnderscore, regExpInputGetter);
  defineGetter(cons, Predefined::input, regExpInputGetter);
  defineGetter(cons, Predefined::dollarAmpersand, regExpLastMatchGetter);
  defineGetter(cons, Predefined::lastMatch, regExpLastMatchGetter);
  defineGetter(cons, Predefined::dollarPlus, regExpLastParenGetter);
  defineGetter(cons, Predefined::lastParen, regExpLastParenGetter);

  return cons;
}

/// 21.2.3.2.1 Runtime Semantics: RegExpAlloc ( newTarget )
// TODO: This is currently a wrapper around JSRegExp::create, and always creates
// an object with RegExp.prototype as its prototype. After supporting
// subclassing, this should be updated to allocate an object with its prototype
// set to newTarget.prototype.
static CallResult<HermesValue> regExpAlloc(
    Runtime *runtime,
    PseudoHandle<> /* newTarget */) {
  return JSRegExp::create(
      runtime, Handle<JSObject>::vmcast(&runtime->regExpPrototype));
}

/// 21.2.3.2.2 Runtime Semantics: RegExpInitialize ( obj, pattern, flags )
static CallResult<Handle<JSRegExp>> regExpInitialize(
    Runtime *runtime,
    Handle<> obj,
    Handle<> pattern,
    Handle<> flags) {
  auto regExpObj = Handle<JSRegExp>::dyn_vmcast(obj);
  if (LLVM_UNLIKELY(!regExpObj)) {
    return ExecutionStatus::EXCEPTION;
  }
  MutableHandle<StringPrimitive> P = runtime->makeMutableHandle(
      runtime->getPredefinedString(Predefined::emptyString));
  MutableHandle<StringPrimitive> F = runtime->makeMutableHandle(
      runtime->getPredefinedString(Predefined::emptyString));
  if (!pattern->isUndefined()) {
    auto strRes = toString_RJS(runtime, pattern);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    P = strRes->get();
  }
  if (!flags->isUndefined()) {
    auto strRes = toString_RJS(runtime, flags);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    F = strRes->get();
  }
  if (JSRegExp::initialize(regExpObj, runtime, P, F) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  return regExpObj;
}

/// ES6.0 21.2.3.2.3 Runtime Semantics: RegExpCreate ( P, F )
CallResult<Handle<JSRegExp>>
regExpCreate(Runtime *runtime, Handle<> P, Handle<> F) {
  auto objRes =
      regExpAlloc(runtime, createPseudoHandle(runtime->regExpPrototype));
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return regExpInitialize(
      runtime, runtime->makeHandle(objRes.getValue()), P, F);
}

CallResult<HermesValue>
regExpConstructor(void *, Runtime *runtime, NativeArgs args) {
  Handle<> pattern = args.getArgHandle(0);
  Handle<> flags = args.getArgHandle(1);
  // 1. Let patternIsRegExp be IsRegExp(pattern).
  // 2. ReturnIfAbrupt(patternIsRegExp).
  auto callRes = isRegExp(runtime, pattern);
  if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  bool patternIsRegExp = callRes.getValue();
  // 3. If NewTarget is not undefined, let newTarget be NewTarget.
  // Note: "NewTarget is not undefined" means we are in a constructor call.
  // TODO: right now the NewTarget can only be the RegExp constructor itself,
  // after supporting subclassing, NewTarget could be the constructor of the
  // derived class.
  auto propRes = JSObject::getNamed_RJS(
      runtime->getGlobal(),
      runtime,
      Predefined::getSymbolID(Predefined::RegExp));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> newTarget = runtime->makeHandle(propRes.getValue());
  // 4. Else,
  // This is not a constructor call.
  if (!args.isConstructorCall()) {
    // a. Let newTarget be the active function object.
    // Note: newTarget is the RegExp constructor, which is already set.
    // b. If patternIsRegExp is true and flags is undefined, then
    if (patternIsRegExp && flags->isUndefined()) {
      // i. Let patternConstructor be Get(pattern, "constructor").
      // ii. ReturnIfAbrupt(patternConstructor).
      auto patternObj = Handle<JSObject>::vmcast(pattern);
      auto patternConstructor = JSObject::getNamed_RJS(
          patternObj,
          runtime,
          Predefined::getSymbolID(Predefined::constructor));
      if (LLVM_UNLIKELY(patternConstructor == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // iii. If SameValue(newTarget, patternConstructor) is true, return
      // pattern.
      if (isSameValue(
              newTarget.getHermesValue(), patternConstructor.getValue())) {
        return pattern.getHermesValue();
      }
    }
  }

  MutableHandle<> P{runtime};
  MutableHandle<> F{runtime};
  // 5. If Type(pattern) is Object and pattern has a [[RegExpMatcher]] internal
  // slot, then
  if (auto patternAsRegExp = Handle<JSRegExp>::dyn_vmcast(pattern)) {
    // a. Let P be the value of pattern’s [[OriginalSource]] internal slot.
    P = JSRegExp::getPattern(patternAsRegExp.get(), runtime).getHermesValue();
    // b. If flags is undefined, let F be the value of pattern’s
    // [[OriginalFlags]] internal slot.
    if (flags->isUndefined()) {
      auto flagsStr = JSRegExp::getFlagBits(patternAsRegExp.get()).toString();
      auto strRes = StringPrimitive::create(runtime, flagsStr);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      F = strRes.getValue();
    } else {
      // c. Else, let F be flags.
      F = flags.getHermesValue();
    }
  } else if (patternIsRegExp) {
    // 6. Else if patternIsRegExp is true, then
    //   a. Let P be Get(pattern, "source").
    //   b. ReturnIfAbrupt(P).
    Handle<JSObject> patternObj = Handle<JSObject>::vmcast(pattern);
    propRes = JSObject::getNamed_RJS(
        patternObj, runtime, Predefined::getSymbolID(Predefined::source));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    P = propRes.getValue();
    //   c. If flags is undefined, then
    if (flags->isUndefined()) {
      //   i. Let F be Get(pattern, "flags").
      //   ii. ReturnIfAbrupt(F).
      propRes = JSObject::getNamed_RJS(
          patternObj, runtime, Predefined::getSymbolID(Predefined::flags));
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      F = propRes.getValue();
    } else {
      //   d. Else, let F be flags.
      F = flags.getHermesValue();
    }
  } else {
    // 7. Else,
    //   a. Let P be pattern.
    P = pattern.getHermesValue();
    //   b. Let F be flags.
    F = flags.getHermesValue();
  }
  // 8. Let O be RegExpAlloc(newTarget).
  // 9. ReturnIfAbrupt(O).
  auto objRes = regExpAlloc(runtime, newTarget);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 10. Return RegExpInitialize(O, P, F).
  auto regExpRes =
      regExpInitialize(runtime, runtime->makeHandle(objRes.getValue()), P, F);
  if (LLVM_UNLIKELY(regExpRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return regExpRes->getHermesValue();
}

/// Set the lastIndex property of \p regexp to \p value.
static ExecutionStatus
setLastIndex(Handle<JSObject> regexp, Runtime *runtime, HermesValue hv) {
  return runtime->putNamedThrowOnError(
      regexp, PropCacheID::RegExpLastIndex, hv);
}

static ExecutionStatus
setLastIndex(Handle<JSObject> regexp, Runtime *runtime, double value) {
  return setLastIndex(regexp, runtime, HermesValue::encodeNumberValue(value));
}

CallResult<Handle<JSArray>> directRegExpExec(
    Handle<JSRegExp> regexp,
    Runtime *runtime,
    Handle<StringPrimitive> S) {
  uint32_t length = S->getStringLength();
  MutableHandle<JSArray> A{runtime};
  GCScope gcScope{runtime};

  // "Let lastIndex be the result of calling the [[Get]] internal method
  // of R with argument "lastIndex". Let i be the value of ToInteger(lastIndex)"
  auto propRes = runtime->getNamed(regexp, PropCacheID::RegExpLastIndex);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  auto intRes = toInteger(runtime, runtime->makeHandle(*propRes));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  double i = intRes->getNumber();

  // "Let global be the result of calling the [[Get]] internal method of R
  // with argument "global". If global is false, then let i = 0."
  // global is non-configurable and read-only, so we can just refer to the flag
  // bits
  bool global = JSRegExp::getFlagBits(regexp.get()).global;
  if (!global) {
    i = 0;
  }

  CallResult<RegExpMatch> matchResult = RegExpMatch{};
  // Note that i == length is allowed, since we may match an empty range
  // at the end of the string
  if (i >= 0 && i <= length) {
    uint32_t searchStartOffset = static_cast<uint32_t>(i);
    matchResult = JSRegExp::search(regexp, runtime, S, searchStartOffset);
  }

  if (LLVM_UNLIKELY(matchResult == ExecutionStatus::EXCEPTION))
    return ExecutionStatus::EXCEPTION;

  auto match = *matchResult;

  if (match.empty()) {
    // No match
    // "Call the [[Put]] internal method of R with arguments "lastIndex", 0, and
    // true"
    if (LLVM_UNLIKELY(
            setLastIndex(regexp, runtime, 0) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return Runtime::makeNullHandle<JSArray>();
  }

  // We have a match!
  // "Let e be r's endIndex value. If global is true, Call the [[Put]] internal
  // method of R with arguments "lastIndex", e, and true"
  // Here 'e' is the end of the total match
  if (global) {
    auto totalMatch = *match.front();
    uint32_t e = totalMatch.location + totalMatch.length;
    if (LLVM_UNLIKELY(
            setLastIndex(regexp, runtime, e) == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }

  const auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();

  auto arrRes = JSArray::create(runtime, match.size(), 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  A = arrRes->get();

  CallResult<bool> defineResult = JSObject::defineOwnProperty(
      A,
      runtime,
      Predefined::getSymbolID(Predefined::index),
      dpf,
      runtime->makeHandle(
          HermesValue::encodeNumberValue(match.front()->location)));
  assert(
      defineResult != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  (void)defineResult;

  defineResult = JSObject::defineOwnProperty(
      A, runtime, Predefined::getSymbolID(Predefined::input), dpf, S);
  assert(
      defineResult != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  (void)defineResult;

  defineResult = JSObject::defineOwnProperty(
      A,
      runtime,
      Predefined::getSymbolID(Predefined::length),
      dpf,
      runtime->makeHandle(HermesValue::encodeNumberValue(match.size())));
  assert(
      defineResult != ExecutionStatus::EXCEPTION &&
      "defineOwnProperty() failed on a new object");
  (void)defineResult;

  // Set capture groups (including the initial full match)
  size_t idx = 0;
  auto marker = gcScope.createMarker();
  for (const auto &mg : match) {
    gcScope.flushToMarker(marker);
    if (!mg) {
      // Match groups that did not match anything become undefined.
      JSArray::setElementAt(A, runtime, idx, Runtime::getUndefinedValue());
    } else {
      assert(
          mg->location + mg->length >= mg->location &&
          mg->location + mg->length <= S->getStringLength() &&
          "Capture group range unexpectedly out of bounds");
      auto strRes =
          StringPrimitive::slice(runtime, S, mg->location, mg->length);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      JSArray::setElementAt(
          A, runtime, idx, runtime->makeHandle<StringPrimitive>(*strRes));
    }
    idx++;
  }
  if (JSArray::setLengthProperty(A, runtime, idx) == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;
  return A;
}

/// ES6.0 21.2.5.2.2 Runtime Semantics: RegExpBuiltinExec ( R, S )
/// TODO: this is a wrapper around \f directRegExpExec right now, and it is
/// semantically equivalent to the spec definition without unicode and sticky
/// support. After updating this to exactly match the spec, replace usages of
/// \f directRegExpExec with this function.
static CallResult<HermesValue> regExpBuiltinExec(
    Runtime *runtime,
    Handle<JSRegExp> R,
    Handle<StringPrimitive> S) {
  CallResult<Handle<JSArray>> result = directRegExpExec(R, runtime, S);
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // When the match is empty, result is a nullptr handle, which means
  // result.getValue().getHermesValue() is not HermesValue::encodeNullValue()
  // but HermesValue::encodeObjectValue(nullptr).
  if (!result.getValue())
    return HermesValue::encodeNullValue();
  return result->getHermesValue();
}

/// ES6.0 21.2.5.2.3
/// TODO: before supporting unicode regexp, all this function does is
/// incrementing the index by 1.
static uint64_t advanceStringIndex(
    Runtime *runtime,
    PseudoHandle<StringPrimitive> /* S */,
    uint64_t index,
    bool unicode) {
  // TODO: before implementing unicode regexp, the function can only be called
  // with unicode = false.
  assert(!unicode && "unicode regexp is not supported.");
  return index + 1;
}

/// ES6.0 21.2.5.2.1 Runtime Semantics: RegExpExec ( R, S )
static CallResult<HermesValue>
regExpExec(Runtime *runtime, Handle<JSObject> R, Handle<StringPrimitive> S) {
  // 3. Let exec be Get(R, "exec").
  // 4. ReturnIfAbrupt(exec).
  auto propRes = JSObject::getNamed_RJS(
      R, runtime, Predefined::getSymbolID(Predefined::exec));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto exec = runtime->makeHandle(propRes.getValue());
  // 5. If IsCallable(exec) is true, then
  if (auto execCallable = Handle<Callable>::dyn_vmcast(exec)) {
    // a. Let result be Call(exec, R, «S»).
    // b. ReturnIfAbrupt(result).
    auto callRes =
        Callable::executeCall1(execCallable, runtime, R, S.getHermesValue());
    if (callRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If Type(result) is neither Object or Null, throw a TypeError
    // exception.
    if (!callRes->isObject() && !callRes->isNull()) {
      return runtime->raiseTypeError(
          "The result of exec can only be object or null.");
    }
    // d. Return result.
    return callRes;
  }
  // 6. If R does not have a [[RegExpMatcher]] internal slot, throw a TypeError
  // exception.
  // In our implementation, having a [[RegExpMatcher]] internal slot is
  // equivalent to being a JSRegExp class instance.
  auto regExpObj = Handle<JSRegExp>::dyn_vmcast(R);
  if (LLVM_UNLIKELY(!regExpObj)) {
    return runtime->raiseTypeError(
        "Failed to execute an invalid regular expression object.");
  }
  // 7. Return RegExpBuiltinExec(R, S).
  return regExpBuiltinExec(runtime, regExpObj, S);
}

/// Implementation of RegExp.prototype.exec
/// Returns an Array if a match is found, null if no match is found
CallResult<HermesValue>
regExpPrototypeExec(void *, Runtime *runtime, NativeArgs args) {
  Handle<JSRegExp> regexp = args.dyncastThis<JSRegExp>();
  if (!regexp) {
    return runtime->raiseTypeError(
        "RegExp function called on non-RegExp object");
  }

  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (strRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> S = toHandle(runtime, std::move(*strRes));
  CallResult<Handle<JSArray>> result = directRegExpExec(regexp, runtime, S);
  if (LLVM_UNLIKELY(result.getStatus() == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // When the match is empty, result is a nullptr handle, which means
  // result.getValue().getHermesValue() is not HermesValue::encodeNullValue()
  // but HermesValue::encodeObjectValue(nullptr).
  if (!result.getValue())
    return HermesValue::encodeNullValue();
  return result.getValue().getHermesValue();
}

/// Implementation of RegExp.prototype.test
/// Returns true if a match is found, false otherwise
/// TODO optimization: avoid constructing the submatch array. Instead simply
/// check for a match.
CallResult<HermesValue>
regExpPrototypeTest(void *context, Runtime *runtime, NativeArgs args) {
  CallResult<HermesValue> res = regExpPrototypeExec(context, runtime, args);
  if (res == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  return HermesValue::encodeBoolValue(
      !runtime->makeHandle(res.getValue())->isNull());
}

// ES6 21.2.5.14
// Note there is no requirement that 'this' be a RegExp object.
CallResult<HermesValue>
regExpPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  Handle<JSObject> regexp = args.dyncastThis<JSObject>();
  if (!regexp) {
    return runtime->raiseTypeError(
        "RegExp.prototype.toString() called on non-object");
  }

  // Let pattern be ToString(Get(R, "source"))
  auto source = JSObject::getNamed_RJS(
      regexp, runtime, Predefined::getSymbolID(Predefined::source));
  if (LLVM_UNLIKELY(source == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto patternRes = toString_RJS(runtime, runtime->makeHandle(*source));
  if (LLVM_UNLIKELY(patternRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> pattern = toHandle(runtime, std::move(*patternRes));

  // Let flags be ToString(Get(R, "flags"))
  auto flagsObj = JSObject::getNamed_RJS(
      regexp, runtime, Predefined::getSymbolID(Predefined::flags));
  if (LLVM_UNLIKELY(flagsObj == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto flagsRes = toString_RJS(runtime, runtime->makeHandle(*flagsObj));
  if (LLVM_UNLIKELY(flagsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<StringPrimitive> flags = toHandle(runtime, std::move(*flagsRes));

  // 'Let result be the String value formed by concatenating "/", pattern, and
  // "/", and flags.' We expect 2 slashes plus at most 3 flags. Note we would
  // make this larger once sticky and unicode are supported.
  SmallU16String<32> result;
  result.reserve(pattern->getStringLength() + 5);

  result.push_back(u'/');
  pattern->copyUTF16String(result);
  result.push_back(u'/');
  flags->copyUTF16String(result);
  return StringPrimitive::create(runtime, result);
}

/// Return the ith capture group in the most recent succesful RegExp search.
/// If there was no ith capture group, return "".
CallResult<HermesValue>
regExpDollarNumberGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  size_t i = reinterpret_cast<size_t>(ctx);

  auto match = runtime->regExpLastMatch;
  if (match.size() >= i + 1 &&
      vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    const auto &cap = match[i];
    if (cap.hasValue()) {
      auto S = Handle<StringPrimitive>::vmcast(&runtime->regExpLastInput);
      auto strRes =
          StringPrimitive::slice(runtime, S, cap->location, cap->length);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return *strRes;
    }
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

// ES8 21.2.5.10
CallResult<HermesValue>
regExpSourceGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  // "If Type(R) is not Object, throw a TypeError exception"
  if (!args.dyncastThis<JSObject>()) {
    return runtime->raiseTypeError(
        "RegExp.prototype.source getter called on non-RegExp");
  }

  Handle<JSRegExp> R = args.dyncastThis<JSRegExp>();
  if (!R) {
    // "If SameValue(R, %RegExpPrototype%) is true, return "(?:)". Otherwise,
    // throw a TypeError exception.
    if (LLVM_UNLIKELY(thisIsRegExpProto(runtime, args))) {
      // Note we don't bother to predefine this string because this should
      // hardly ever be encountered.
      const char *emptyPattern = "(?:)";
      return StringPrimitive::create(
          runtime, ASCIIRef{emptyPattern, strlen(emptyPattern)});
    }
    return runtime->raiseTypeError(
        "RegExp.prototype.source getter called on non-RegExp");
  }

  // "Let src be the value of R’s [[OriginalSource]] internal slot."
  // "Let flags be the value of R’s [[OriginalFlags]] internal slot."
  // "Return EscapeRegExpPattern(src, flags)."
  // Note that ES6 specifies that we provide the flags to EscapeRegExpPattern,
  // however this is only to distinguish a Unicode from a non-Unicode regexp.
  // Beacuse we do not yet support Unicode regexps we can omit the flags.
  return JSRegExp::escapePattern(
      toHandle(runtime, JSRegExp::getPattern(R.get(), runtime)), runtime);
}

// ES8 21.2.5.3
// Note that we don't yet support unicode or sticky.
CallResult<HermesValue>
regExpFlagsGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  // Let R be the this value.
  // If Type(R) is not Object, throw a TypeError exception
  Handle<JSObject> R = args.dyncastThis<JSObject>();
  if (!R) {
    return runtime->raiseTypeError(
        "RegExp.prototype.flags getter called on non-object");
  }

  // Let global be ToBoolean(Get(R, "global")).
  // If global is true, append "g" as the last code unit of result.
  // Let ignoreCase be ToBoolean(Get(R, "ignoreCase")).
  // ReturnIfAbrupt(ignoreCase).
  // If ignoreCase is true, append "i" as the last code unit of result.
  // Let multiline be ToBoolean(Get(R, "multiline")).
  // ReturnIfAbrupt(multiline).
  // If multiline is true, append "m" as the last code unit of result.
  // TODO: support unicode and sticky
  llvm::SmallString<3> result;
  static const struct FlagProp {
    char flagChar;
    Predefined::Str name;
  } flagProps[] = {
      {'g', Predefined::global},
      {'i', Predefined::ignoreCase},
      {'m', Predefined::multiline},
  };
  for (FlagProp f : flagProps) {
    auto flagVal =
        JSObject::getNamed_RJS(R, runtime, Predefined::getSymbolID(f.name));
    if (LLVM_UNLIKELY(flagVal == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    if (toBoolean(*flagVal)) {
      result.push_back(f.flagChar);
    }
  }
  return StringPrimitive::create(runtime, result);
}

// ES8 21.2.5.4, 21.2.5.5, 21.2.5.7
CallResult<HermesValue>
regExpFlagPropertyGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  // Note in ES8, the standard specifies that the RegExp prototype object is not
  // a RegExp but that these accessors check for it as a special case.

  // "Let R be the this value."
  // "If Type(R) is not Object, throw a TypeError exception."
  // "If R does not have an [[OriginalFlags]] internal slot, throw a TypeError
  // exception." (Combine these two checks into a dyncast to JSRegExp).
  Handle<JSRegExp> R = args.dyncastThis<JSRegExp>();
  if (!R) {
    // "If SameValue(R, %RegExpPrototype%) is true, return undefined. Otherwise,
    // throw a TypeError exception"
    if (thisIsRegExpProto(runtime, args)) {
      return HermesValue::encodeUndefinedValue();
    }
    return runtime->raiseTypeError("RegExp getter called on non-RegExp");
  }

  // "Let flags be the value of R’s [[OriginalFlags]] internal slot."
  // "If flags contains the code unit "i/m/g", return true."
  // "Return false."
  auto flagBits = JSRegExp::getFlagBits(R.get());
  switch ((intptr_t)ctx) {
    case 'i':
      return HermesValue::encodeBoolValue(flagBits.ignoreCase);
    case 'm':
      return HermesValue::encodeBoolValue(flagBits.multiline);
    case 'g':
      return HermesValue::encodeBoolValue(flagBits.global);
    default:
      llvm_unreachable("Invalid flag passed to regExpFlagPropertyGetter");
      return HermesValue::encodeEmptyValue();
  }
}

CallResult<HermesValue>
regExpLeftContextGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  auto match = runtime->regExpLastMatch;
  if (match.size() >= 1 && vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    auto S = Handle<StringPrimitive>::vmcast(&runtime->regExpLastInput);
    auto strRes = StringPrimitive::slice(runtime, S, 0, match[0]->location);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return *strRes;
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

CallResult<HermesValue>
regExpRightContextGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  auto match = runtime->regExpLastMatch;
  if (match.size() >= 1 && vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    auto S = Handle<StringPrimitive>::vmcast(&runtime->regExpLastInput);
    if (match[0]->location + match[0]->length < S->getStringLength()) {
      auto startIdx = match[0]->location + match[0]->length;
      auto strRes = StringPrimitive::slice(
          runtime, S, startIdx, S->getStringLength() - startIdx);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return *strRes;
    }
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

CallResult<HermesValue>
regExpInputGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  if (vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    return runtime->regExpLastInput;
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

CallResult<HermesValue>
regExpLastMatchGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  auto match = runtime->regExpLastMatch;
  if (match.size() >= 1 && vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    auto S = Handle<StringPrimitive>::vmcast(&runtime->regExpLastInput);
    auto strRes = StringPrimitive::slice(
        runtime, S, match[0]->location, match[0]->length);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return *strRes;
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

CallResult<HermesValue>
regExpLastParenGetter(void *ctx, Runtime *runtime, NativeArgs args) {
  auto match = runtime->regExpLastMatch;
  if (match.size() >= 2 && vmisa<StringPrimitive>(runtime->regExpLastInput)) {
    auto S = Handle<StringPrimitive>::vmcast(&runtime->regExpLastInput);
    const auto &cap = match.back();
    if (cap.hasValue()) {
      auto strRes =
          StringPrimitive::slice(runtime, S, cap->location, cap->length);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return *strRes;
    }
  }

  return HermesValue::encodeStringValue(
      runtime->getPredefinedString(Predefined::emptyString));
}

// TODO: consider writing this in JS.
/// ES6.0 21.2.5.6
CallResult<HermesValue>
regExpPrototypeSymbolMatch(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let rx be the this value.
  Handle<JSObject> rx = args.dyncastThis<JSObject>();

  // 2. If Type(rx) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!rx)) {
    return runtime->raiseTypeError(
        "RegExp.prototype[@@match] should be called on a js object");
  }
  // 3. Let S be ToString(string)
  // 4. ReturnIfAbrupt(S).
  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (strRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  // 5. Let global be ToBoolean(Get(rx, "global")).
  // 6. ReturnIfAbrupt(global).
  auto propRes = JSObject::getNamed_RJS(
      rx, runtime, Predefined::getSymbolID(Predefined::global));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto global = toBoolean(*propRes);
  // 7. If global is false, then
  //   a. Return RegExpExec(rx, S).
  if (!global) {
    return regExpExec(runtime, rx, S);
  }
  // 8. Else global is true,
  // TODO: regexp does not have unicode support yet.
  // Skipping 8.a-b, and assume fullUnicode is always false.
  // c. Let setStatus be Set(rx, "lastIndex", 0, true).
  // d. ReturnIfAbrupt(setStatus).
  Handle<HermesValue> zeroHandle =
      runtime->makeHandle(HermesValue::encodeNumberValue(0));
  if (setLastIndex(rx, runtime, *zeroHandle) == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  // e. Let A be ArrayCreate(0).
  auto arrRes = JSArray::create(runtime, 0, 0);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto A = toHandle(runtime, std::move(*arrRes));
  // e. Let n be 0.
  uint32_t n = 0;

  // g. Repeat,
  MutableHandle<> propValue{runtime};
  MutableHandle<> result{runtime};
  MutableHandle<StringPrimitive> matchStr{runtime};
  auto marker = gcScope.createMarker();
  while (true) {
    gcScope.flushToMarker(marker);
    // i. Let result be RegExpExec(rx, S).
    auto callRes = regExpExec(runtime, rx, S);
    // ii. ReturnIfAbrupt(result).
    if (LLVM_UNLIKELY(callRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    result = callRes.getValue();
    // iii. If result is null, then
    if (result->isNull()) {
      if (n == 0) {
        return HermesValue::encodeNullValue();
      } else {
        if (LLVM_UNLIKELY(
                JSArray::setLengthProperty(A, runtime, n) ==
                ExecutionStatus::EXCEPTION))
          return ExecutionStatus::EXCEPTION;
        return A.getHermesValue();
      }
    }
    // iv. Else result is not null,
    auto resultObj = Handle<JSObject>::vmcast(result);
    // 1. Let matchStr be ToString(Get(result, "0")).
    // 2. ReturnIfAbrupt(matchStr).
    auto propRes2 = JSObject::getComputed_RJS(resultObj, runtime, zeroHandle);
    if (propRes2 == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    propValue = propRes2.getValue();
    auto strRes2 = toString_RJS(runtime, propValue);
    if (strRes2 == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    matchStr = strRes2->get();
    // 3. Let status be CreateDataProperty(A, ToString(n), matchStr).
    // 4. Assert: status is true.
    JSArray::setElementAt(A, runtime, n, matchStr);
    // 5. If matchStr is the empty String, then
    if (matchStr->getStringLength() == 0) {
      // a. Let thisIndex be ToLength(Get(rx, "lastIndex")).
      // b. ReturnIfAbrupt(thisIndex).
      if ((propRes = runtime->getNamed(rx, PropCacheID::RegExpLastIndex)) ==
          ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      propValue = propRes.getValue();
      auto thisIndex = toLength(runtime, propValue);
      if (thisIndex == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
      // c. Let nextIndex be AdvanceStringIndex(S, thisIndex, fullUnicode).
      // TODO: regexp doesn't have unicode support yet, so fullUnicode is always
      // false.
      double nextIndex = advanceStringIndex(
          runtime,
          S,
          thisIndex->getNumberAs<uint64_t>(),
          false /* fullUnicode */);
      // d. Let setStatus be Set(rx, "lastIndex", nextIndex, true).
      auto setStatus = setLastIndex(rx, runtime, nextIndex);
      // e. ReturnIfAbrupt(setStatus).
      if (setStatus == ExecutionStatus::EXCEPTION) {
        return ExecutionStatus::EXCEPTION;
      }
    }
    // 6. Increment n.
    n++;
  }
  llvm_unreachable(
      "RegExp.prototype[@@match] must stop when all matched results are returned.");
}

/// ES6.0 21.2.5.9
CallResult<HermesValue>
regExpPrototypeSymbolSearch(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let rx be the this value.
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  Handle<JSObject> rx = args.dyncastThis<JSObject>();
  if (!rx) {
    return runtime->raiseTypeError(
        "Calling regExp.prototype[@@search] on a non-object.");
  }
  // 3. Let S be ToString(string).
  // 4. ReturnIfAbrupt(S).
  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  // 5. Let previousLastIndex be Get(rx, "lastIndex").
  // 6. ReturnIfAbrupt(previousLastIndex).
  auto propRes = runtime->getNamed(rx, PropCacheID::RegExpLastIndex);
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> previousLastIndex = runtime->makeHandle(propRes.getValue());
  // 7. Let status be Set(rx, "lastIndex", 0, true).
  auto status = setLastIndex(rx, runtime, 0);
  // 8. ReturnIfAbrupt(status).
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 9. Let result be RegExpExec(rx, S).
  // 10. ReturnIfAbrupt(result).
  auto execRes = regExpExec(runtime, rx, S);
  if (LLVM_UNLIKELY(execRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<> result = runtime->makeHandle(execRes.getValue());
  // 11. Let status be Set(rx, "lastIndex", previousLastIndex, true).
  status = setLastIndex(rx, runtime, *previousLastIndex);
  // 12. ReturnIfAbrupt(status).
  if (LLVM_UNLIKELY(status == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  // 13. If result is null, return –1.
  if (result->isNull()) {
    return HermesValue::encodeNumberValue(-1);
  }
  // 14. Return Get(result, "index").
  auto resultObj = Handle<JSObject>::dyn_vmcast(result);
  if (LLVM_UNLIKELY(!resultObj)) {
    return ExecutionStatus::EXCEPTION;
  }
  return JSObject::getNamed_RJS(
      resultObj, runtime, Predefined::getSymbolID(Predefined::index));
}

/// ES6.0 21.1.3.14.1
/// Transforms a replacement string by substituting $ replacement strings.
/// \p captures can be a null pointer.
CallResult<HermesValue> getSubstitution(
    Runtime *runtime,
    Handle<StringPrimitive> matched,
    Handle<StringPrimitive> str,
    uint32_t position,
    Handle<ArrayStorage> captures,
    Handle<StringPrimitive> replacement) {
  // 1. Assert: Type(matched) is String.
  // 2. Let matchLength be the number of code units in matched.
  uint32_t matchLength = matched->getStringLength();
  // 3. Assert: Type(str) is String.
  // 4. Let stringLength be the number of code units in str.
  uint32_t stringLength = str->getStringLength();
  // 5. Assert: position is a nonnegative integer.
  // 6. Assert: position ≤ stringLength.
  assert(
      position <= stringLength &&
      "The matched position should be within the string length.");
  // 7. Assert: captures is a possibly empty List of Strings.
  // 8. Assert: Type(replacement) is String
  // 9. Let tailPos be position + matchLength.
  uint32_t tailPos = position + matchLength;
  // 10. Let m be the number of elements in captures.
  size_t m = captures ? captures->size() : 0;

  // 11. Let result be a String value derived from replacement by copying code
  // unit elements from replacement to result while performing replacements as
  // specified in Table 45. These $ replacements are done left-to- right, and,
  // once such a replacement is performed, the new replacement text is not
  // subject to further replacements.
  auto replacementView =
      StringPrimitive::createStringView(runtime, replacement);
  auto stringView = StringPrimitive::createStringView(runtime, str);
  auto matchedStrView = StringPrimitive::createStringView(runtime, matched);
  SmallU16String<32> result{};

  // Don't use a StringView iterator, as any calls to createStringView can
  // allocate and move the underlying char storage.
  for (size_t i = 0, e = replacementView.length(); i < e;) {
    // Go character by character and account for $ replacement strings.
    char16_t c0 = replacementView[i];
    if (c0 != u'$' || i + 1 == e) {
      // Not a special replacement string, just add the character.
      result.append(c0);
      ++i;
      continue;
    }

    // There's a character after the '$', check for replacement strings.
    char16_t c1 = replacementView[i + 1];
    if (c1 == u'$') {
      result.append(u'$');
      i += 2;
    } else if (c1 == u'&') {
      // The matched substring.
      matchedStrView.copyUTF16String(result);
      i += 2;
    } else if (c1 == u'`') {
      // Portion of string before the matched substring.
      stringView.slice(0, position).copyUTF16String(result);
      i += 2;
    } else if (c1 == u'\'') {
      // Portion of string after the matched substring.
      if (tailPos < stringLength) {
        stringView.slice(position + matchLength).copyUTF16String(result);
      }
      i += 2;
    } else if (u'0' <= c1 && c1 <= u'9') {
      // $n index.
      // '0' <= c1 <= '9' because $nn case can have 01 to 99.
      // If it ends up being the $n case instead of $nn,
      // then we can check to make sure 1 <= n <= 9.

      // Define a helper to access a submatch substring, or the empty
      // string if the submatch is undefined.
      auto submatchOrEmpty = [&](size_t idx) -> StringView {
        assert(
            captures && idx < captures->size() &&
            "Index into captures is out of bound.");
        if (captures->at(idx).isUndefined()) {
          // return empty string.
          return stringView.slice(str->getStringLength());
        }
        return StringPrimitive::createStringView(
            runtime,
            Handle<StringPrimitive>::vmcast(runtime, captures->at(idx)));
      };

      uint32_t n = c1 - u'0';
      if (i + 2 < e) {
        // Try for the $nn case if there's more characters available.
        char16_t c2 = replacementView[i + 2];
        // $nn index.
        uint32_t nn = (c1 - u'0') * 10 + (c2 - u'0');
        if ((u'0' <= c2 && c2 <= u'9') && (1 <= nn && nn <= m)) {
          // Valid $nn case.
          auto view = submatchOrEmpty(nn - 1);
          result.insert(result.end(), view.begin(), view.end());
          i += 3;
        } else if (1 <= n && n <= m) {
          // Try for the $n case first.
          auto view = submatchOrEmpty(n - 1);
          result.insert(result.end(), view.begin(), view.end());
          i += 2;
        } else {
          // No valid $n or $nn case, just append the characters and
          // move on.
          result.append({c0, c1});
          i += 2;
        }
      } else if (1 <= n && n <= m) {
        auto view = submatchOrEmpty(n - 1);
        result.insert(result.end(), view.begin(), view.end());
        i += 2;
      } else {
        // Not a valid $n.
        result.append({c0, c1});
        i += 2;
      }
    } else {
      // None of the replacement strings count, add both characters.
      result.append({c0, c1});
      i += 2;
    }
  }
  // 12. Return result.
  return StringPrimitive::create(runtime, result);
}

/// ES6.0 21.2.5.8
CallResult<HermesValue>
regExpPrototypeSymbolReplace(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  // 1. Let rx be the this value.
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  Handle<JSObject> rx = args.dyncastThis<JSObject>();
  if (LLVM_UNLIKELY(!rx)) {
    return runtime->raiseTypeError(
        "RegExp.prototype[@@replace] called on a non-object.");
  }
  // 3. Let S be ToString(string).
  // 4. ReturnIfAbrupt(S).
  auto strRes = toString_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto S = toHandle(runtime, std::move(*strRes));
  // 5. Let lengthS be the number of code unit elements in S.
  uint32_t lengthS = S->getStringLength();
  // 6. Let functionalReplace be IsCallable(replaceValue).
  Handle<> replaceValue = args.getArgHandle(1);
  MutableHandle<StringPrimitive> replaceValueStr{runtime};
  auto replaceFn = Handle<Callable>::dyn_vmcast(replaceValue);
  // 7. If functionalReplace is false, then
  if (!replaceFn) {
    //   a. Let replaceValue be ToString(replaceValue).
    //   b. ReturnIfAbrupt(replaceValue).
    auto strRes = toString_RJS(runtime, replaceValue);
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    replaceValueStr = strRes->get();
  }
  // 8. Let global be ToBoolean(Get(rx, "global")).
  // 9. ReturnIfAbrupt(global).
  auto propRes = JSObject::getNamed_RJS(
      rx, runtime, Predefined::getSymbolID(Predefined::global));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  bool global = toBoolean(propRes.getValue());
  // 10. If global is true, then
  Handle<> zeroHandle = runtime->makeHandle(HermesValue::encodeNumberValue(0));
  if (global) {
    //   a. Let fullUnicode be ToBoolean(Get(rx, "unicode")).
    //   b. ReturnIfAbrupt(fullUnicode).
    // TODO: skip above two steps since we don't support unicode regexp yet.
    //   c. Let setStatus be Set(rx, "lastIndex", 0, true).
    auto setStatus = setLastIndex(rx, runtime, *zeroHandle);
    //   d. ReturnIfAbrupt(setStatus).
    if (LLVM_UNLIKELY(setStatus == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
  }
  // 11. Let results be a new empty List.
  auto arrRes = ArrayStorage::create(runtime, 16 /* capacity */);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  MutableHandle<ArrayStorage> resultsHandle{
      runtime, vmcast<ArrayStorage>(arrRes.getValue())};

  // 12. Let done be false.
  bool done = false;

  // 13. Repeat, while done is false
  MutableHandle<> propValue{runtime};
  MutableHandle<StringPrimitive> matchStr{runtime};
  MutableHandle<HermesValue> nextIndex{runtime};
  MutableHandle<JSObject> result{runtime};
  auto stringView = StringPrimitive::createStringView(runtime, S);
  while (!done) {
    GCScopeMarkerRAII marker{runtime};
    // a. Let result be RegExpExec(rx, S).
    // b. ReturnIfAbrupt(result).
    auto execRes = regExpExec(runtime, rx, S);
    if (LLVM_UNLIKELY(execRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // c. If result is null, set done to true.
    if (execRes->isNull()) {
      done = true;
      break;
    }
    // d. Else result is not null,
    // If result is not null, it must be an object.
    result = vmcast<JSObject>(execRes.getValue());
    // i. Append result to the end of results.
    if (LLVM_UNLIKELY(resultsHandle->size() == ArrayStorage::maxElements())) {
      return runtime->raiseRangeError("Out of memory for regexp results.");
    }
    if (LLVM_UNLIKELY(
            ArrayStorage::push_back(resultsHandle, runtime, result) ==
            ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // ii. If global is false, set done to true.
    if (!global) {
      done = true;
    } else {
      // iii. Else,
      // 1. Let matchStr be ToString(Get(result, "0")).
      // 2. ReturnIfAbrupt(matchStr).
      propRes = JSObject::getComputed_RJS(result, runtime, zeroHandle);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      propValue = propRes.getValue();
      auto strRes = toString_RJS(runtime, propValue);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      matchStr = strRes->get();
      // 3. If matchStr is the empty String, then
      if (matchStr->getStringLength() == 0) {
        // a. Let thisIndex be ToLength(Get(rx, "lastIndex")).
        // b. ReturnIfAbrupt(thisIndex).
        if ((propRes = runtime->getNamed(rx, PropCacheID::RegExpLastIndex)) ==
            ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        propValue = propRes.getValue();
        auto thisIndex = toLength(runtime, propValue);
        if (thisIndex == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
        // c. Let nextIndex be AdvanceStringIndex(S, thisIndex, fullUnicode).
        // TODO: regexp doesn't have unicode support yet, so fullUnicode is
        // always false.
        nextIndex = HermesValue::encodeDoubleValue(advanceStringIndex(
            runtime,
            S,
            thisIndex->getNumberAs<uint64_t>(),
            false /* fullUnicode */));
        // d. Let setStatus be Set(rx, "lastIndex", nextIndex, true).
        auto setStatus = setLastIndex(rx, runtime, *nextIndex);
        // e. ReturnIfAbrupt(setStatus).
        if (setStatus == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
      }
    }
  }

  // 14. Let accumulatedResult be the empty String value.
  SmallU16String<32> accumulatedResult{};
  // 15. Let nextSourcePosition be 0.
  uint32_t nextSourcePosition = 0;
  // 16. Repeat, for each result in results,
  MutableHandle<> valueHandle{runtime};
  MutableHandle<> nHandle{runtime};
  for (uint32_t i = 0, size = resultsHandle->size(); i < size; ++i) {
    GCScopeMarkerRAII marker1{runtime};
    result = vmcast<JSObject>(resultsHandle->at(i));
    // a. Let nCaptures be ToLength(Get(result, "length")).
    // b. ReturnIfAbrupt(nCaptures).
    propRes = JSObject::getNamed_RJS(
        result, runtime, Predefined::getSymbolID(Predefined::length));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    valueHandle = propRes.getValue();
    auto lengthRes = toLength(runtime, valueHandle);
    if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // Use uint64_t so it can fit the result of toLength.
    uint64_t nCaptures = lengthRes->getNumberAs<uint64_t>();
    // c. Let nCaptures be max(nCaptures − 1, 0).
    nCaptures = nCaptures > 0 ? nCaptures - 1 : 0;
    // d. Let matched be ToString(Get(result, "0")).
    // e. ReturnIfAbrupt(matched).
    propRes = JSObject::getComputed_RJS(result, runtime, zeroHandle);
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto strRes =
        toString_RJS(runtime, runtime->makeHandle(propRes.getValue()));
    if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto matched = toHandle(runtime, std::move(*strRes));
    // f. Let matchLength be the number of code units in matched.
    uint32_t matchLength = matched->getStringLength();
    // g. Let position be ToInteger(Get(result, "index")).
    // h. ReturnIfAbrupt(position).
    propRes = JSObject::getNamed_RJS(
        result, runtime, Predefined::getSymbolID(Predefined::index));
    if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    auto intRes = toInteger(runtime, runtime->makeHandle(propRes.getValue()));
    if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    // position could potentially be negative here, so use int64_t.
    auto position = intRes->getNumberAs<int64_t>();
    // i. Let position be max(min(position, lengthS), 0).
    position = std::max(std::min(position, (int64_t)lengthS), (int64_t)0);
    // j. Let n be 1.
    // Match the type of nCaptures.
    uint64_t n = 1;
    // k. Let captures be an empty List.
    if (LLVM_UNLIKELY(nCaptures > ArrayStorage::maxElements())) {
      return runtime->raiseRangeError("Out of memory for capture groups.");
    }
    arrRes = ArrayStorage::create(runtime, nCaptures);
    if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    MutableHandle<ArrayStorage> capturesHandle{
        runtime, vmcast<ArrayStorage>(arrRes.getValue())};
    MutableHandle<> capN{runtime};
    // l. Repeat while n ≤ nCaptures
    while (n <= nCaptures) {
      GCScopeMarkerRAII marker2{runtime};
      // i. Let capN be Get(result, ToString(n)).
      // ii. ReturnIfAbrupt(capN).
      nHandle = HermesValue::encodeNumberValue(n);
      propRes = JSObject::getComputed_RJS(result, runtime, nHandle);
      if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      capN = propRes.getValue();
      // iii. If capN is not undefined, then
      if (!capN->isUndefined()) {
        // 1. Let capN be ToString(capN).
        // 2. ReturnIfAbrupt(capN).
        auto strRes = toString_RJS(runtime, capN);
        if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
          return ExecutionStatus::EXCEPTION;
        }
        capN = strRes->getHermesValue();
      }
      // iv. Append capN as the last element of captures.
      if (LLVM_UNLIKELY(
              ArrayStorage::push_back(capturesHandle, runtime, capN) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      // v. Let n be n+1
      n++;
    }
    // m. If functionalReplace is true, then
    MutableHandle<StringPrimitive> replacement{runtime};
    if (replaceFn) {
      CallResult<HermesValue> callRes{ExecutionStatus::EXCEPTION};
      {
        // i. Let replacerArgs be «matched».
        // Arguments: matched, captures, position, S.
        size_t replacerArgsCount = 1 + nCaptures + 2;
        if (LLVM_UNLIKELY(replacerArgsCount >= UINT32_MAX))
          return runtime->raiseStackOverflow(
              Runtime::StackOverflowKind::JSRegisterStack);
        ScopedNativeCallFrame newFrame{runtime,
                                       static_cast<uint32_t>(replacerArgsCount),
                                       *replaceFn,
                                       false,
                                       HermesValue::encodeUndefinedValue()};
        if (LLVM_UNLIKELY(newFrame.overflowed()))
          return runtime->raiseStackOverflow(
              Runtime::StackOverflowKind::NativeStack);

        uint32_t argIdx = 0;
        newFrame->getArgRef(argIdx++) = matched.getHermesValue();
        // ii. Append in list order the elements of captures to the end of the
        // List replacerArgs.
        for (; argIdx <= capturesHandle->size(); ++argIdx) {
          newFrame->getArgRef(argIdx) = capturesHandle->at(argIdx - 1);
        }
        // iii. Append position and S as the last two elements of replacerArgs.
        newFrame->getArgRef(argIdx++) =
            HermesValue::encodeNumberValue(position);
        newFrame->getArgRef(argIdx++) = S.getHermesValue();

        // iv. Let replValue be Call(replaceValue, undefined, replacerArgs).
        callRes = Callable::call(replaceFn, runtime);
        if (callRes == ExecutionStatus::EXCEPTION) {
          return ExecutionStatus::EXCEPTION;
        }
      }
      // v. Let replacement be ToString(replValue).
      auto strRes =
          toString_RJS(runtime, runtime->makeHandle(callRes.getValue()));
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      replacement = strRes->get();
    } else {
      // n. Else,
      // i. Let replacement be GetSubstitution(matched, S, position, captures,
      // replaceValue).
      auto callRes = getSubstitution(
          runtime, matched, S, position, capturesHandle, replaceValueStr);
      replacement = vmcast<StringPrimitive>(callRes.getValue());
    }
    // o. ReturnIfAbrupt(replacement).
    // p. If position ≥ nextSourcePosition, then
    if (position >= nextSourcePosition) {
      // i. NOTE position should not normally move backwards. If it does, it is
      // an indication of an ill-behaving RegExp subclass or use of an access
      // triggered side-effect to change the global flag or other
      // characteristics of rx. In such cases, the corresponding substitution is
      // ignored.
      // ii. Let accumulatedResult be the String formed by
      // concatenating the code units of the current value of accumulatedResult
      // with the substring of S consisting of the code units from
      // nextSourcePosition (inclusive) up to position (exclusive) and with the
      // code units of replacement.
      stringView.slice(nextSourcePosition, position - nextSourcePosition)
          .copyUTF16String(accumulatedResult);
      replacement->copyUTF16String(accumulatedResult);
      // iii. Let nextSourcePosition be position + matchLength.
      nextSourcePosition = position + matchLength;
    }
  }

  // 17. If nextSourcePosition ≥ lengthS, return accumulatedResult.
  if (nextSourcePosition >= lengthS) {
    return StringPrimitive::createEfficient(runtime, accumulatedResult);
  }
  // 18. Return the String formed by concatenating the code units of
  // accumulatedResult with the substring of S consisting of the code units from
  // nextSourcePosition (inclusive) up through the final code unit of S
  // (inclusive).
  stringView.slice(nextSourcePosition).copyUTF16String(accumulatedResult);
  return StringPrimitive::createEfficient(runtime, accumulatedResult);
}

/// ES6.0 21.2.5.11
/// Note: this implementation does not fully observe ES6 spec behaviors because
/// of lack of support for sticky matching and species constructors.
// TODO(T35212035): make this ES6 compliant once we support sticky matching and
// species constructor.
CallResult<HermesValue>
regExpPrototypeSymbolSplit(void *, Runtime *runtime, NativeArgs args) {
  // 2. If Type(rx) is not Object, throw a TypeError exception.
  if (LLVM_UNLIKELY(!vmisa<JSObject>(args.getThisArg()))) {
    return runtime->raiseTypeError(
        "Cannot call RegExp.protoype[Symbol.split] on a non-object.");
  }
  // We currently cannot support calling this method on a non-RegExp object, so
  // throw a TypeError in this case.
  if (LLVM_UNLIKELY(!vmisa<JSRegExp>(args.getThisArg()))) {
    return runtime->raiseTypeError(
        "Calling RegExp.protoype[Symbol.split] on a non-RegExp object is not supported yet.");
  }
  return splitInternal(
      runtime,
      args.getArgHandle(0),
      args.getArgHandle(1),
      args.getThisHandle());
}

} // namespace vm
} // namespace hermes
