/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
//===----------------------------------------------------------------------===//
/// \file
/// ES5.1 15.3 Initialize the Function constructor.
//===----------------------------------------------------------------------===//
#include "JSLibInternal.h"

#include "hermes/Regex/Executor.h"
#include "hermes/Regex/RegexTraits.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/Operations.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

/// @name Function
/// @{

/// ES5.1 15.3.1.1 and 15.3.2.1. Function() invoked as a function and as a
/// constructor.
static CallResult<HermesValue>
functionConstructor(void *, Runtime *runtime, NativeArgs args);

/// @}

/// @name Function.prototype
/// @{

/// ES5.1 15.3.4.2.
static CallResult<HermesValue>
functionPrototypeToString(void *, Runtime *runtime, NativeArgs args);

/// ES5.1 15.3.4.3.
static CallResult<HermesValue>
functionPrototypeApply(void *, Runtime *runtime, NativeArgs args);

/// ES5.1 15.3.4.4.
static CallResult<HermesValue>
functionPrototypeCall(void *, Runtime *runtime, NativeArgs args);

/// ES5.1 15.3.4.5.
static CallResult<HermesValue>
functionPrototypeBind(void *, Runtime *runtime, NativeArgs args);

/// ES6.0 19.2.3.6.
static CallResult<HermesValue>
functionPrototypeSymbolHasInstance(void *, Runtime *runtime, NativeArgs args);

/// @}

//===----------------------------------------------------------------------===//
/// Function.

Handle<JSObject> createFunctionConstructor(Runtime *runtime) {
  auto functionPrototype =
      Handle<Callable>::vmcast(&runtime->functionPrototype);

  auto cons = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::Function),
      functionConstructor,
      functionPrototype,
      1,
      JSFunction::createWithNewDomain,
      CellKind::FunctionKind);

  // Function.prototype.xxx() methods.
  defineMethod(
      runtime,
      functionPrototype,
      Predefined::getSymbolID(Predefined::toString),
      nullptr,
      functionPrototypeToString,
      0);
  defineMethod(
      runtime,
      functionPrototype,
      Predefined::getSymbolID(Predefined::apply),
      nullptr,
      functionPrototypeApply,
      2);
  defineMethod(
      runtime,
      functionPrototype,
      Predefined::getSymbolID(Predefined::call),
      nullptr,
      functionPrototypeCall,
      1);
  defineMethod(
      runtime,
      functionPrototype,
      Predefined::getSymbolID(Predefined::bind),
      nullptr,
      functionPrototypeBind,
      1);

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.writable = 0;
  dpf.enumerable = 0;
  dpf.configurable = 0;
  (void)defineMethod(
      runtime,
      functionPrototype,
      Predefined::getSymbolID(Predefined::SymbolHasInstance),
      Predefined::getSymbolID(Predefined::squareSymbolHasInstance),
      nullptr,
      functionPrototypeSymbolHasInstance,
      1,
      dpf);

  return cons;
}

static std::vector<uint8_t> getReturnThisRegexBytecode() {
  const char16_t *returnThisRE = uR"X(^\s*return[ \t]+this\s*;?\s*$)X";
  regex::constants::SyntaxFlags nativeFlags = {};
  return regex::Regex<regex::U16RegexTraits>(returnThisRE, nativeFlags)
      .compile();
}

static bool isReturnThis(Handle<StringPrimitive> str, Runtime *runtime) {
  // Fast check for minimal version.
  {
    auto minimal = runtime->getPredefinedString(Predefined::returnThis);
    if (str->equals(minimal)) {
      return true;
    }
  }
  // Regex match for variants.
  auto input = StringPrimitive::createStringView(runtime, str);
  static auto bytecode = getReturnThisRegexBytecode();
  auto result = regex::MatchRuntimeResult::NoMatch;
  if (input.isASCII()) {
    regex::MatchResults<const char *> results;
    const char *begin = input.castToCharPtr();
    const char *end = begin + input.length();
    result = regex::searchWithBytecode(
        bytecode,
        begin,
        end,
        results,
        regex::constants::matchDefault | regex::constants::matchInputAllAscii);
  } else {
    regex::MatchResults<const char16_t *> results;
    const char16_t *begin = input.castToChar16Ptr();
    const char16_t *end = begin + input.length();
    result = regex::searchWithBytecode(
        bytecode, begin, end, results, regex::constants::matchDefault);
  }
  return result == regex::MatchRuntimeResult::Match;
}

static CallResult<HermesValue>
functionConstructor(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);

  // Number of arguments supplied to Function().
  uint32_t argCount = args.getArgCount();

  // Number of parameters in the resultant function.
  uint32_t paramCount = argCount > 0 ? argCount - 1 : 0;

  // List of the parameter strings for the resultant function..
  auto arrRes = JSArray::create(runtime, paramCount, paramCount);
  if (LLVM_UNLIKELY(arrRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto params = toHandle(runtime, std::move(*arrRes));

  // Body of the resultant function.
  MutableHandle<StringPrimitive> body{runtime};

  // String length of the function in its entirety.
  // Account for commas in the argument list initially.
  // If at least two arguments to the function (3 in total), there's a comma.
  SafeUInt32 size{paramCount > 0 ? paramCount - 1 : 0};

  if (argCount == 0) {
    // No arguments, just set body to be the empty string.
    body = runtime->getPredefinedString(Predefined::emptyString);
  } else {
    // If there's arguments, store the parameters and the provided body.
    auto marker = gcScope.createMarker();
    for (uint32_t i = 0; i < paramCount; ++i) {
      gcScope.flushToMarker(marker);
      auto strRes = toString(runtime, args.getArgHandle(runtime, i));
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto param = toHandle(runtime, std::move(*strRes));
      JSArray::setElementAt(params, runtime, i, param);
      size.add(param->getStringLength());
    }

    // Last parameter is the body.
    auto strRes = toString(runtime, args.getArgHandle(runtime, paramCount));
    if (strRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    body = strRes->get();
    size.add(body->getStringLength());

    if (argCount == 1 && isReturnThis(body, runtime)) {
      // If this raises an exception, we still return immediately.
      return JSFunction::create(
          runtime,
          toHandle(runtime, Domain::create(runtime)),
          Handle<JSObject>(runtime, nullptr),
          Handle<Environment>(runtime, nullptr),
          runtime->getReturnThisCodeBlock());
    }
  }

  // Constant parts of the function.
  auto functionHeader = createASCIIRef("(function (");
  size.add(functionHeader.size());

  auto bodyHeader = createASCIIRef("){");
  size.add(bodyHeader.size());

  // Note: add a \n before the closing '}' in case the function body ends with a
  // line comment.
  auto functionFooter = createASCIIRef("\n})");
  size.add(functionFooter.size());

  auto separator = createASCIIRef(",");

  auto builder = StringBuilder::createStringBuilder(runtime, size);
  if (builder == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }
  builder->appendASCIIRef(functionHeader);
  MutableHandle<StringPrimitive> element{runtime};
  for (uint32_t i = 0; i < paramCount; ++i) {
    // Copy params into str.
    element = params->at(i).getString();
    builder->appendStringPrim(element);
    if (i < paramCount - 1) {
      // If there's more params left to put, need to add a comma.
      // We wouldn't have entered the loop if paramCount == 0,
      // so there's no overflow here.
      builder->appendASCIIRef(separator);
    }
  }
  builder->appendASCIIRef(bodyHeader);
  builder->appendStringPrim(body);
  builder->appendASCIIRef(functionFooter);

  auto evalRes = directEval(runtime, builder->getStringPrimitive(), {}, true);
  if (evalRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  Handle<JSFunction> function =
      runtime->makeHandle(vmcast<JSFunction>(evalRes.getValue()));

  DefinePropertyFlags dpf{};
  dpf.clear();
  dpf.setValue = 1;
  dpf.setEnumerable = 1;
  dpf.setWritable = 1;
  dpf.setConfigurable = 1;
  dpf.enumerable = 0;
  dpf.writable = 0;
  dpf.configurable = 1;

  // Define the `name` correctly.
  if (JSObject::defineOwnProperty(
          function,
          runtime,
          Predefined::getSymbolID(Predefined::name),
          dpf,
          runtime->makeHandle(runtime->getStringPrimFromSymbolID(
              Predefined::getSymbolID(Predefined::anonymous)))) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  return function.getHermesValue();
}

static CallResult<HermesValue>
functionPrototypeToString(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope{runtime};

  auto func = args.dyncastThis<Callable>(runtime);
  if (!func) {
    return runtime->raiseTypeError(
        "Can't call Function.prototype.toString() on non-callable");
  }

  SmallU16String<64> strBuf{};
  strBuf.append("function ");

  // Extract the name.
  auto propRes = JSObject::getNamed(
      func, runtime, Predefined::getSymbolID(Predefined::name));
  if (propRes == ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Convert the name to string, unless it is undefined.
  if (!propRes->isUndefined()) {
    auto strRes = toString(runtime, runtime->makeHandle(*propRes));
    if (strRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    strRes->get()->copyUTF16String(strBuf);
  }

  // Append the named parameters.
  strBuf.append('(');

  // Extract ".length".
  auto lengthProp = Callable::extractOwnLengthProperty(func, runtime);
  if (lengthProp == ExecutionStatus::EXCEPTION)
    return ExecutionStatus::EXCEPTION;

  // The value of the property is not guaranteed to be meaningful, so clamp it
  // to [0..65535] for sanity.
  uint32_t paramCount = (uint32_t)std::min(65535.0, std::max(0.0, *lengthProp));

  for (uint32_t i = 0; i < paramCount; ++i) {
    if (i != 0)
      strBuf.append(", ");
    char buf[16];
    ::snprintf(buf, sizeof(buf), "a%u", i);
    strBuf.append(buf);
  }

  // The rest of the body.
  // http://tc39.github.io/Function-prototype-toString-revision
  strBuf.append(") { [ native code ] }");

  // Finally allocate a StringPrimitive.
  return StringPrimitive::create(runtime, strBuf);
}

static CallResult<HermesValue>
functionPrototypeApply(void *, Runtime *runtime, NativeArgs args) {
  GCScope gcScope(runtime);
  auto func = args.dyncastThis<Callable>(runtime);
  if (LLVM_UNLIKELY(!func)) {
    return runtime->raiseTypeError("Can't apply() to non-callable");
  }

  if (args.getArg(1).isNull() || args.getArg(1).isUndefined()) {
    ScopedNativeCallFrame newFrame{runtime, 0, *func, false, args.getArg(0)};
    if (LLVM_UNLIKELY(newFrame.overflowed()))
      return runtime->raiseStackOverflow();
    return Callable::call(func, runtime);
  }

  auto argObj =
      Handle<JSObject>::dyn_vmcast(runtime, args.getArgHandle(runtime, 1));
  if (LLVM_UNLIKELY(!argObj)) {
    return runtime->raiseTypeError(
        "Can't apply() with non-object arguments list");
  }

  auto propRes = JSObject::getNamed(
      argObj, runtime, Predefined::getSymbolID(Predefined::length));
  if (LLVM_UNLIKELY(propRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto intRes = toUInt32(runtime, runtime->makeHandle(*propRes));
  if (LLVM_UNLIKELY(intRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  uint32_t n = intRes->getNumber();

  ScopedNativeCallFrame newFrame{runtime, n, *func, false, args.getArg(0)};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();

  // Initialize the arguments to undefined because we might allocate and cause
  // a gc while populating them.
  // TODO: look into doing this lazily.
  newFrame.fillArguments(n, HermesValue::encodeUndefinedValue());

  Handle<ArrayImpl> argArray = Handle<ArrayImpl>::dyn_vmcast(runtime, argObj);
  MutableHandle<> iHandle{runtime, HermesValue::encodeNumberValue(0)};
  auto marker = gcScope.createMarker();
  if (LLVM_LIKELY(argArray)) {
    // Fast path: we already have an array, so try and bypass the getComputed
    // checks and the handle loads & stores. Directly call ArrayImpl::at,
    // and only call getComputed if the element is empty.
    for (uint32_t argIdx = 0; argIdx < n; ++argIdx) {
      HermesValue arg = argArray->at(argIdx);
      if (LLVM_LIKELY(!arg.isEmpty())) {
        newFrame->getArgRef(argIdx) = arg;
        continue;
      }
      // Slow path fallback: the actual getComputed on this,
      // because the real value could be up the prototype chain.
      iHandle = HermesValue::encodeDoubleValue(argIdx);
      gcScope.flushToMarker(marker);
      if (LLVM_UNLIKELY(
              (propRes = JSObject::getComputed(argObj, runtime, iHandle)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      newFrame->getArgRef(argIdx) = *propRes;
    }
  } else {
    // Not an array. Use this slow path.
    for (uint32_t argIdx = 0; argIdx < n; ++argIdx) {
      iHandle = HermesValue::encodeNumberValue(argIdx);
      gcScope.flushToMarker(marker);
      if (LLVM_UNLIKELY(
              (propRes = JSObject::getComputed(argObj, runtime, iHandle)) ==
              ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      newFrame->getArgRef(argIdx) = *propRes;
    }
  }

  gcScope.flushToMarker(marker);
  return Callable::call(func, runtime);
}

static CallResult<HermesValue>
functionPrototypeCall(void *, Runtime *runtime, NativeArgs args) {
  auto func = args.dyncastThis<Callable>(runtime);
  if (LLVM_UNLIKELY(!func)) {
    return runtime->raiseTypeError("Can't call() non-callable");
  }

  uint32_t argCount = args.getArgCount();
  ScopedNativeCallFrame newFrame{
      runtime, argCount ? argCount - 1 : 0, *func, false, args.getArg(0)};
  if (LLVM_UNLIKELY(newFrame.overflowed()))
    return runtime->raiseStackOverflow();
  for (uint32_t i = 1; i < argCount; ++i) {
    newFrame->getArgRef(i - 1) = args.getArg(i);
  }
  return Callable::call(func, runtime);
}

static CallResult<HermesValue>
functionPrototypeBind(void *, Runtime *runtime, NativeArgs args) {
  auto target = Handle<Callable>::dyn_vmcast(runtime, args.getThisHandle());
  if (!target) {
    return runtime->raiseTypeError("Can't bind() a non-callable");
  }

  return BoundFunction::create(
      runtime, target, args.getArgCount(), args.begin());
}

static CallResult<HermesValue>
functionPrototypeSymbolHasInstance(void *, Runtime *runtime, NativeArgs args) {
  /// 1. Let F be the this value.
  auto F = args.getThisHandle();
  /// 2. Return OrdinaryHasInstance(F, V).
  auto result = ordinaryHasInstance(runtime, F, args.getArgHandle(runtime, 0));
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeBoolValue(*result);
}

} // namespace vm
} // namespace hermes
