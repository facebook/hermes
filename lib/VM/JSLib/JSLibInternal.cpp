/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/Regex/Executor.h"
#include "hermes/Regex/Regex.h"
#include "hermes/Regex/RegexTraits.h"
#include "hermes/VM/PropertyAccessor.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StringBuilder.h"
#include "hermes/VM/StringPrimitive.h"
#include "hermes/VM/StringView.h"

namespace hermes {
namespace vm {

Handle<NativeConstructor> defineSystemConstructor(
    Runtime &runtime,
    SymbolID name,
    NativeFunctionPtr nativeFunctionPtr,
    Handle<JSObject> prototypeObjectHandle,
    Handle<JSObject> constructorProtoObjectHandle,
    unsigned paramCount,
    NativeConstructor::CreatorFunction *creator,
    CellKind targetKind) {
  auto constructor = runtime.makeHandle(NativeConstructor::create(
      runtime,
      constructorProtoObjectHandle,
      nullptr,
      nativeFunctionPtr,
      paramCount,
      creator,
      targetKind));

  auto st = Callable::defineNameLengthAndPrototype(
      constructor,
      runtime,
      name,
      paramCount,
      prototypeObjectHandle,
      Callable::WritablePrototype::No,
      false);
  (void)st;
  assert(
      st != ExecutionStatus::EXCEPTION && "defineLengthAndPrototype() failed");

  // Define the global.
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();

  auto res = JSObject::defineOwnProperty(
      runtime.getGlobal(), runtime, name, dpf, constructor);
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
  (void)res;

  return constructor;
}

CallResult<HermesValue> defineMethod(
    Runtime &runtime,
    Handle<JSObject> objectHandle,
    SymbolID propertyName,
    SymbolID methodName,
    void *context,
    NativeFunctionPtr nativeFunctionPtr,
    unsigned paramCount,
    DefinePropertyFlags dpf) {
  GCScope gcScope{runtime};

  auto method = NativeFunction::create(
      runtime,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      context,
      nativeFunctionPtr,
      methodName,
      paramCount,
      Runtime::makeNullHandle<JSObject>());

  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, propertyName, dpf, method);
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");

  return method.getHermesValue();
}

Handle<NativeConstructor> defineSystemConstructor(
    Runtime &runtime,
    SymbolID name,
    NativeFunctionPtr nativeFunctionPtr,
    Handle<JSObject> prototypeObjectHandle,
    unsigned paramCount,
    NativeConstructor::CreatorFunction *creator,
    CellKind targetKind) {
  return defineSystemConstructor(
      runtime,
      name,
      nativeFunctionPtr,
      prototypeObjectHandle,
      Handle<JSObject>::vmcast(&runtime.functionPrototype),
      paramCount,
      creator,
      targetKind);
}

void defineMethod(
    Runtime &runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    void *context,
    NativeFunctionPtr nativeFunctionPtr,
    unsigned paramCount) {
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();
  (void)defineMethod(
      runtime, objectHandle, name, context, nativeFunctionPtr, paramCount, dpf);
}

void defineAccessor(
    Runtime &runtime,
    Handle<JSObject> objectHandle,
    SymbolID propertyName,
    SymbolID methodName,
    void *context,
    NativeFunctionPtr getterFunc,
    NativeFunctionPtr setterFunc,
    bool enumerable,
    bool configurable) {
  assert(
      (getterFunc || setterFunc) &&
      "at least a getter or a setter must be specified");
  ExecutionStatus status{};
  (void)status;

  GCScope gcScope{runtime};

  StringView nameView =
      runtime.getIdentifierTable().getStringView(runtime, methodName);
  assert(nameView.isASCII() && "Only ASCII accessors are supported");

  MutableHandle<NativeFunction> getter{runtime};
  if (getterFunc) {
    // Set the name by prepending "get ".
    llvh::SmallString<32> getterName{"get "};
    llvh::raw_svector_ostream os{getterName};
    os << nameView;

    auto strRes = runtime.ignoreAllocationFailure(
        StringPrimitive::create(runtime, getterName));
    SymbolID getterFuncName =
        runtime
            .ignoreAllocationFailure(
                runtime.getIdentifierTable().getSymbolHandleFromPrimitive(
                    runtime,
                    createPseudoHandle(vmcast<StringPrimitive>(strRes))))
            .get();

    auto funcRes = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime.functionPrototype),
        context,
        getterFunc,
        getterFuncName,
        0,
        Runtime::makeNullHandle<JSObject>());
    getter = funcRes.get();
  }

  MutableHandle<NativeFunction> setter{runtime};
  if (setterFunc) {
    // Set the name by prepending "set ".
    llvh::SmallString<32> setterName{"set "};
    llvh::raw_svector_ostream os{setterName};
    os << nameView;

    auto strRes = runtime.ignoreAllocationFailure(
        StringPrimitive::create(runtime, setterName));
    SymbolID setterFuncName =
        runtime
            .ignoreAllocationFailure(
                runtime.getIdentifierTable().getSymbolHandleFromPrimitive(
                    runtime,
                    createPseudoHandle(vmcast<StringPrimitive>(strRes))))
            .get();

    auto funcRes = NativeFunction::create(
        runtime,
        Handle<JSObject>::vmcast(&runtime.functionPrototype),
        context,
        setterFunc,
        setterFuncName,
        1,
        Runtime::makeNullHandle<JSObject>());
    setter = funcRes.get();
  }

  auto crtRes = PropertyAccessor::create(runtime, getter, setter);
  assert(crtRes != ExecutionStatus::EXCEPTION && "unable to define accessor");
  auto accessor = runtime.makeHandle<PropertyAccessor>(*crtRes);

  DefinePropertyFlags dpf{};
  dpf.setEnumerable = 1;
  dpf.setConfigurable = 1;
  dpf.setGetter = 1;
  dpf.setSetter = 1;
  dpf.enumerable = enumerable;
  dpf.configurable = configurable;

  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, propertyName, dpf, accessor);
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
}

void defineProperty(
    Runtime &runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    Handle<> value,
    DefinePropertyFlags dpf) {
  auto res = JSObject::defineOwnProperty(
      objectHandle, runtime, name, dpf, value, PropOpFlags());
  (void)res;
  assert(
      res != ExecutionStatus::EXCEPTION && *res &&
      "defineOwnProperty() failed");
}

void defineProperty(
    Runtime &runtime,
    Handle<JSObject> objectHandle,
    SymbolID name,
    Handle<> value) {
  DefinePropertyFlags dpf = DefinePropertyFlags::getNewNonEnumerableFlags();

  return defineProperty(runtime, objectHandle, name, value, dpf);
}

ExecutionStatus iteratorCloseAndRethrow(
    Runtime &runtime,
    Handle<JSObject> iterator) {
  auto completion = runtime.makeHandle(runtime.getThrownValue());
  if (isUncatchableError(completion.getHermesValue())) {
    // If an uncatchable exception was raised, do not swallow it, but instead
    // propagate it.
    return ExecutionStatus::EXCEPTION;
  }
  runtime.clearThrownValue();
  auto status = iteratorClose(runtime, iterator, completion);
  (void)status;
  assert(
      status == ExecutionStatus::EXCEPTION && "exception swallowed mistakenly");
  return ExecutionStatus::EXCEPTION;
}

static std::vector<uint8_t> getReturnThisRegexBytecode() {
  const char16_t *returnThisRE = uR"X(^\s*return[ \t]+this\s*;?\s*$)X";
  return regex::Regex<regex::UTF16RegexTraits>(returnThisRE).compile();
}

static bool isReturnThis(Handle<StringPrimitive> str, Runtime &runtime) {
  // Fast check for minimal version.
  {
    auto minimal = runtime.getPredefinedString(Predefined::returnThis);
    if (str->equals(minimal)) {
      return true;
    }
  }
  // Regex match for variants.
  auto input = StringPrimitive::createStringView(runtime, str);
  static auto bytecode = getReturnThisRegexBytecode();
  auto result = regex::MatchRuntimeResult::NoMatch;
  if (input.isASCII()) {
    const char *begin = input.castToCharPtr();
    result = regex::searchWithBytecode(
        bytecode,
        begin,
        0,
        input.length(),
        nullptr,
        regex::constants::matchDefault | regex::constants::matchInputAllAscii);
  } else {
    const char16_t *begin = input.castToChar16Ptr();
    result = regex::searchWithBytecode(
        bytecode,
        begin,
        0,
        input.length(),
        nullptr,
        regex::constants::matchDefault);
  }
  return result == regex::MatchRuntimeResult::Match;
}

CallResult<HermesValue> createDynamicFunction(
    Runtime &runtime,
    NativeArgs args,
    DynamicFunctionKind kind) {
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
  auto params = *arrRes;

  // Body of the resultant function.
  MutableHandle<StringPrimitive> body{runtime};

  // String length of the function in its entirety.
  // Account for commas in the argument list initially.
  // If at least two arguments to the function (3 in total), there's a comma.
  SafeUInt32 size{paramCount > 0 ? paramCount - 1 : 0};

  // es2020 19.2.1.1.1 Runtime Semantics: CreateDynamicFunction: If
  // NewTarget is given, such as with Reflect.construct, use
  // NewParent.prototype as the parent.  The prototype on NewParent
  // has already been looked up to use as the parent of 'this', so
  // instead of looking it up again, just use this's parent.  If
  // NewTarget isn't given, fall back to a default.
  MutableHandle<JSObject> fallbackProto{runtime};
  switch (kind) {
    case DynamicFunctionKind::Normal:
      fallbackProto = Handle<JSObject>::vmcast(&runtime.functionPrototype);
      break;
    case DynamicFunctionKind::Generator:
      fallbackProto =
          Handle<JSObject>::vmcast(&runtime.generatorFunctionPrototype);
      break;
    case DynamicFunctionKind::Async:
      fallbackProto = Handle<JSObject>::vmcast(&runtime.asyncFunctionPrototype);
      break;
    default:
      llvm_unreachable("unknown kind for CreateDynamicFunction.");
  }

  Handle<JSObject> parent = !args.getNewTarget().isUndefined()
      ? runtime.makeHandle(
            vmcast<JSObject>(args.getThisArg())->getParent(runtime))
      : fallbackProto;

  if (argCount == 0) {
    // No arguments, just set body to be the empty string.
    body = runtime.getPredefinedString(Predefined::emptyString);
  } else {
    // If there's arguments, store the parameters and the provided body.
    auto marker = gcScope.createMarker();
    for (uint32_t i = 0; i < paramCount; ++i) {
      gcScope.flushToMarker(marker);
      auto strRes = toString_RJS(runtime, args.getArgHandle(i));
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      auto param = runtime.makeHandle(std::move(*strRes));
      JSArray::setElementAt(params, runtime, i, param);
      size.add(param->getStringLength());
    }

    // Last parameter is the body.
    auto strRes = toString_RJS(runtime, args.getArgHandle(paramCount));
    if (strRes == ExecutionStatus::EXCEPTION) {
      return ExecutionStatus::EXCEPTION;
    }
    body = strRes->get();
    size.add(body->getStringLength());

    if (kind == DynamicFunctionKind::Normal && argCount == 1 &&
        isReturnThis(body, runtime)) {
      // If this raises an exception, we still return immediately.
      return JSFunction::create(
                 runtime,
                 runtime.makeHandle(Domain::create(runtime)),
                 parent,
                 Handle<Environment>(runtime, nullptr),
                 runtime.getReturnThisCodeBlock())
          .getHermesValue();
    }
  }

  // Constant parts of the function.
  ASCIIRef functionHeader;
  switch (kind) {
    case DynamicFunctionKind::Normal:
      functionHeader = createASCIIRef("(function (");
      break;
    case DynamicFunctionKind::Generator:
      functionHeader = createASCIIRef("(function*(");
      break;
    case DynamicFunctionKind::Async:
      functionHeader = createASCIIRef("(async function (");
      break;
    default:
      llvm_unreachable("unknown kind for CreateDynamicFunction.");
  };
  size.add(functionHeader.size());

  auto bodyHeader = createASCIIRef("){");
  size.add(bodyHeader.size());

  // Note: add a \n before the closing '}' in case the
  // function body ends with a line comment.
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
    element = params->at(runtime, i).getString(runtime);
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
      runtime.makeHandle(vmcast<JSFunction>(evalRes.getValue()));

  DefinePropertyFlags dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
  dpf.enumerable = 0;
  dpf.writable = 0;

  // Define the `name` correctly.
  if (JSObject::defineOwnProperty(
          function,
          runtime,
          Predefined::getSymbolID(Predefined::name),
          dpf,
          runtime.makeHandle(runtime.getStringPrimFromSymbolID(
              Predefined::getSymbolID(Predefined::anonymous)))) ==
      ExecutionStatus::EXCEPTION) {
    return ExecutionStatus::EXCEPTION;
  }

  // Set the parent.  This could be done by threading the argument
  // through to Runtime::runBytecode where the object is actually
  // created, but this is the only place we need to do this so it
  // keeps the code simpler.
  CallResult<bool> parentRes = JSObject::setParent(*function, runtime, *parent);
  if (LLVM_UNLIKELY(parentRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  assert(
      *parentRes && "Setting prototype on new dynamic function returned false");

  return function.getHermesValue();
}

} // namespace vm
} // namespace hermes
