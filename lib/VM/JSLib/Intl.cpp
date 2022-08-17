/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "JSLibInternal.h"

#include "hermes/Platform/Intl/PlatformIntl.h"

#ifdef HERMES_ENABLE_INTL

#include "hermes/VM/ArrayLike.h"
#include "hermes/VM/JSLib/DateUtil.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/Runtime.h"
#include "hermes/VM/StackFrame-inline.h"
#include "hermes/VM/StringView.h"

namespace hermes {
// TODO T65916424: Move this out of the hermes::vm namespace.
namespace vm {

// This implementation tries to avoid errors in platform code causing
// crashes or throwing JS exceptions.  However, it can still result in
// non-compliant behavior.  For example, resolvedOptions methods are
// required to have a particular set of properties, but if the
// platform result doesn't include them, then they will simply not be
// present.  This could manifest as a JS error later if one of the
// properties is referenced.

namespace {

CallResult<std::u16string> stringFromJS(
    Runtime &runtime,
    PseudoHandle<> value) {
  CallResult<PseudoHandle<StringPrimitive>> strRes =
      toString_RJS(runtime, runtime.makeHandle(std::move(value)));
  if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto view = vm::StringPrimitive::createStringView(
      runtime, runtime.makeHandle(std::move(*strRes)));
  return std::u16string(view.begin(), view.end());
}

CallResult<HermesValue> localesToJS(
    Runtime &runtime,
    CallResult<std::vector<std::u16string>> result) {
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<Handle<JSArray>> arrayRes =
      JSArray::create(runtime, result->size(), result->size());
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> array = *arrayRes;
  MutableHandle<> name{runtime};
  uint64_t index = 0;
  GCScopeMarkerRAII marker{runtime};
  for (auto &locale : *result) {
    marker.flush();
    CallResult<HermesValue> nameRes =
        StringPrimitive::createEfficient(runtime, std::move(locale));
    if (LLVM_UNLIKELY(nameRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    name = *nameRes;
    JSArray::setElementAt(array, runtime, index++, name);
  }
  return array.getHermesValue();
}

CallResult<HermesValue> optionsToJS(
    Runtime &runtime,
    platform_intl::Options result) {
  CallResult<PseudoHandle<JSObject>> objRes = JSObject::create(runtime);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> obj = runtime.makeHandle(std::move(*objRes));
  MutableHandle<> key{runtime};
  MutableHandle<> value{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (auto &kv : result) {
    marker.flush();
    CallResult<HermesValue> keyRes = StringPrimitive::createEfficient(
        runtime, createUTF16Ref(kv.first.c_str()));
    if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    key = *keyRes;
    if (kv.second.isBool()) {
      value = HermesValue::encodeBoolValue(kv.second.getBool());
    } else if (kv.second.isNumber()) {
      value = HermesValue::encodeNumberValue(kv.second.getNumber());
    } else {
      assert(kv.second.isString() && "Option is neither bool nor string");
      CallResult<HermesValue> strRes = StringPrimitive::createEfficient(
          runtime, std::move(kv.second.getString()));
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      value = *strRes;
    }
    auto putRes = JSObject::putComputed_RJS(obj, runtime, key, value);
    if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    assert(*putRes && "put returned false on a plain object");
  }
  return obj.getHermesValue();
}

CallResult<Handle<JSObject>> partToJS(
    Runtime &runtime,
    std::unordered_map<std::u16string, std::u16string> result) {
  CallResult<PseudoHandle<JSObject>> objRes = JSObject::create(runtime);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSObject> obj = runtime.makeHandle(std::move(*objRes));
  MutableHandle<> key{runtime};
  MutableHandle<> value{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (auto &kv : result) {
    marker.flush();

    CallResult<HermesValue> keyRes = StringPrimitive::createEfficient(
        runtime, createUTF16Ref(kv.first.c_str()));
    if (LLVM_UNLIKELY(keyRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    key = *keyRes;
    CallResult<HermesValue> valueRes =
        StringPrimitive::createEfficient(runtime, std::move(kv.second));
    if (LLVM_UNLIKELY(valueRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    value = *valueRes;
    auto putRes = JSObject::putComputed_RJS(obj, runtime, key, value);
    if (LLVM_UNLIKELY(putRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    assert(*putRes && "put returned false on a plain object");
  }

  return obj;
}

CallResult<HermesValue> partsToJS(
    Runtime &runtime,
    CallResult<std::vector<std::unordered_map<std::u16string, std::u16string>>>
        result) {
  if (LLVM_UNLIKELY(result == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<Handle<JSArray>> arrayRes =
      JSArray::create(runtime, result->size(), result->size());
  if (LLVM_UNLIKELY(arrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  Handle<JSArray> array = *arrayRes;
  uint64_t index = 0;
  GCScopeMarkerRAII marker{runtime};
  for (auto &part : *result) {
    marker.flush();
    CallResult<Handle<JSObject>> partRes = partToJS(runtime, std::move(part));
    if (LLVM_UNLIKELY(partRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    JSArray::setElementAt(array, runtime, index++, *partRes);
  }
  return array.getHermesValue();
}

CallResult<std::vector<std::u16string>> normalizeLocales(
    Runtime &runtime,
    Handle<> locales) {
  std::vector<std::u16string> ret;

  if (locales->isUndefined()) {
    return ret;
  }

  if (locales->isString()) {
    auto view = StringPrimitive::createStringView(
        runtime, Handle<StringPrimitive>::vmcast(locales));
    ret.emplace_back(view.begin(), view.end());
    return ret;
  }

  CallResult<HermesValue> objRes = toObject(runtime, locales);
  if (LLVM_UNLIKELY(objRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto localeObj = runtime.makeHandle(vmcast<JSObject>(*objRes));

  CallResult<uint64_t> lengthRes = getArrayLikeLength(localeObj, runtime);
  if (LLVM_UNLIKELY(lengthRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  bool isProxy = localeObj->isProxyObject();
  if (LLVM_UNLIKELY(
          createListFromArrayLike(
              localeObj,
              runtime,
              *lengthRes,
              [&ret, isProxy](
                  Runtime &runtime, uint64_t index, PseudoHandle<> value) {
                // When the locales list is a proxy object, gaps are allowed.
                if (isProxy && value->isUndefined())
                  return ExecutionStatus::RETURNED;

                if (!value->isString() && !value->isObject()) {
                  return runtime.raiseTypeError("Incorrect object type");
                }
                CallResult<std::u16string> strRes =
                    stringFromJS(runtime, std::move(value));
                if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
                  return ExecutionStatus::EXCEPTION;
                }
                ret.emplace_back(*strRes);
                return ExecutionStatus::RETURNED;
              }) == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return std::move(ret);
}

constexpr int kDateRequired = 1 << 0;
constexpr int kTimeRequired = 1 << 1;
constexpr int kDateDefault = 1 << 2;
constexpr int kTimeDefault = 1 << 3;

// To normalize options, we need to know the valid options names, and
// the expected type of each.
struct OptionData {
  std::u16string_view name;
  platform_intl::Option::Kind kind;
  int flags;
};

constexpr OptionData kSupportedLocalesOfOptions[] = {
    {u"localeMatcher", platform_intl::Option::Kind::String, 0}};

constexpr OptionData kCollatorOptions[] = {
    {u"usage", platform_intl::Option::Kind::String, 0},
    {u"localeMatcher", platform_intl::Option::Kind::String, 0},
    {u"numeric", platform_intl::Option::Kind::Bool, 0},
    {u"caseFirst", platform_intl::Option::Kind::String, 0},
    {u"sensitivity", platform_intl::Option::Kind::String, 0},
    {u"ignorePunctuation", platform_intl::Option::Kind::Bool, 0},
    {u"collation", platform_intl::Option::Kind::String, 0},
};

constexpr OptionData kDTFOptions[] = {
    {u"localeMatcher", platform_intl::Option::Kind::String, 0},
    {u"calendar", platform_intl::Option::Kind::String, 0},
    {u"numberingSystem", platform_intl::Option::Kind::String, 0},
    {u"hour12", platform_intl::Option::Kind::Bool, 0},
    {u"hourCycle", platform_intl::Option::Kind::String, 0},
    {u"timeZone", platform_intl::Option::Kind::String, 0},
    {u"formatMatcher", platform_intl::Option::Kind::String, 0},
    {u"weekday", platform_intl::Option::Kind::String, 0},
    {u"era", platform_intl::Option::Kind::String, 0},
    {u"year",
     platform_intl::Option::Kind::String,
     kDateRequired | kDateDefault},
    {u"month",
     platform_intl::Option::Kind::String,
     kDateRequired | kDateDefault},
    {u"day", platform_intl::Option::Kind::String, kDateRequired | kDateDefault},
    {u"hour",
     platform_intl::Option::Kind::String,
     kTimeRequired | kTimeDefault},
    {u"minute",
     platform_intl::Option::Kind::String,
     kTimeRequired | kTimeDefault},
    {u"second",
     platform_intl::Option::Kind::String,
     kTimeRequired | kTimeDefault},
    {u"timeZoneName", platform_intl::Option::Kind::String, 0},
    {u"dateStyle", platform_intl::Option::Kind::String, 0},
    {u"timeStyle", platform_intl::Option::Kind::String, 0},
    {u"fractionalSecondDigits", platform_intl::Option::Kind::Number, 0},
};

constexpr OptionData kNumberFormatOptions[] = {
    {u"localeMatcher", platform_intl::Option::Kind::String, 0},
    {u"numberingSystem", platform_intl::Option::Kind::String, 0},
    {u"style", platform_intl::Option::Kind::String, 0},
    {u"currency", platform_intl::Option::Kind::String, 0},
    {u"currencyDisplay", platform_intl::Option::Kind::String, 0},
    {u"currencySign", platform_intl::Option::Kind::String, 0},
    {u"unit", platform_intl::Option::Kind::String, 0},
    {u"unitDisplay", platform_intl::Option::Kind::String, 0},
    {u"notation", platform_intl::Option::Kind::String, 0},
    {u"minimumIntegerDigits", platform_intl::Option::Kind::Number, 0},
    {u"minimumFractionDigits", platform_intl::Option::Kind::Number, 0},
    {u"maximumFractionDigits", platform_intl::Option::Kind::Number, 0},
    {u"minimumSignificantDigits", platform_intl::Option::Kind::Number, 0},
    {u"maximumSignificantDigits", platform_intl::Option::Kind::Number, 0},
    {u"compactDisplay", platform_intl::Option::Kind::String, 0},
    {u"useGrouping", platform_intl::Option::Kind::Bool, 0},
    {u"signDisplay", platform_intl::Option::Kind::String, 0},
};

CallResult<platform_intl::Options> normalizeOptions(
    Runtime &runtime,
    Handle<> options,
    llvh::ArrayRef<OptionData> optionData) {
  platform_intl::Options ret;

  if (options->isNull())
    return runtime.raiseTypeError("Options object can't be null !");

  auto optionsObj = Handle<JSObject>::dyn_vmcast(options);
  if (!optionsObj) {
    return ret;
  }

  MutableHandle<> name{runtime};
  MutableHandle<> value{runtime};
  MutableHandle<StringPrimitive> strValue{runtime};
  GCScopeMarkerRAII marker{runtime};
  for (const OptionData &pod : optionData) {
    marker.flush();
    CallResult<HermesValue> nameRes = StringPrimitive::createEfficient(
        runtime, UTF16Ref{pod.name.data(), pod.name.size()});
    if (LLVM_UNLIKELY(nameRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    name = *nameRes;

    CallResult<PseudoHandle<>> valRes =
        JSObject::getComputed_RJS(optionsObj, runtime, name);
    if (LLVM_UNLIKELY(valRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    if ((*valRes)->isUndefined()) {
      continue;
    }

    if (pod.kind == platform_intl::Option::Kind::Bool) {
      ret.emplace(
          pod.name, platform_intl::Option(toBoolean(valRes->getHermesValue())));
    } else if (pod.kind == platform_intl::Option::Kind::Number) {
      value = std::move(*valRes);
      CallResult<HermesValue> numRes = toNumber_RJS(runtime, value);
      if (LLVM_UNLIKELY(numRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      ret.emplace(pod.name, platform_intl::Option(numRes->getNumber()));
    } else {
      assert(
          pod.kind == platform_intl::Option::Kind::String &&
          "Unknown option kind");
      value = std::move(*valRes);
      CallResult<PseudoHandle<StringPrimitive>> strRes =
          toString_RJS(runtime, value);
      if (LLVM_UNLIKELY(strRes == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      strValue = std::move(*strRes);
      auto view = StringPrimitive::createStringView(runtime, strValue);
      ret.emplace(
          pod.name,
          platform_intl::Option(std::u16string(view.begin(), view.end())));
    }
  }

  return std::move(ret);
}

template <typename T>
ExecutionStatus checkOptions(Runtime &runtime, const platform_intl::Options &) {
  return ExecutionStatus::RETURNED;
}

template <>
ExecutionStatus checkOptions<platform_intl::NumberFormat>(
    Runtime &runtime,
    const platform_intl::Options &options) {
  // As best as I can tell, there are only two places (in
  // SetNumberFormatUnitOptions) where option value handling can throw
  // a TypeError, and there is no other place where the platform code
  // needs a TypeError.  So, rather than have to deal with multiple
  // Java error classes, we just handle those two cases right here.

  auto styleIt = options.find(u"style");
  if (styleIt == options.end()) {
    return ExecutionStatus::RETURNED;
  }
  if (styleIt->second.isString() &&
      styleIt->second.getString() == u"currency" &&
      options.count(u"currency") == 0) {
    return runtime.raiseTypeError(
        "Style 'currency' requires option 'currency'");
  }
  if (styleIt->second.isString() && styleIt->second.getString() == u"unit" &&
      options.count(u"unit") == 0) {
    return runtime.raiseTypeError("Style 'unit' requires option 'unit'");
  }
  return ExecutionStatus::RETURNED;
}

// T here is one of the platform_intl types.  This exists to avoid
// duplicating code, since all the ctors are basically the same.
template <typename T>
CallResult<HermesValue> intlServiceConstructor(
    Runtime &runtime,
    NativeArgs args,
    llvh::ArrayRef<OptionData> optionData,
    Handle<JSObject> servicePrototype,
    unsigned int additionalSlots) {
  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<platform_intl::Options> optionsRes =
      normalizeOptions(runtime, args.getArgHandle(1), optionData);
  if (LLVM_UNLIKELY(optionsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // NumberFormat has a couple extra checks to make.
  if (LLVM_UNLIKELY(
          checkOptions<T>(runtime, *optionsRes) ==
          ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<std::unique_ptr<T>> nativeRes =
      T::create(runtime, *localesRes, *optionsRes);
  if (LLVM_UNLIKELY(nativeRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  std::unique_ptr<T> native = std::move(*nativeRes);

  auto typeHandle = runtime.makeHandle(
      HermesValue::encodeNumberValue((uint32_t)T::getNativeType()));
  auto setType = [&](Handle<DecoratedObject> obj) {
    auto res = JSObject::defineNewOwnProperty(
        obj,
        runtime,
        Predefined::getSymbolID(Predefined::InternalPropertyIntlNativeType),
        PropertyFlags::defaultNewNamedPropertyFlags(),
        typeHandle);
    (void)res;
    assert(res != ExecutionStatus::EXCEPTION && "Setting type cannot fail.");
  };

  // If constructor, use the allocated object
  if (args.isConstructorCall()) {
    Handle<DecoratedObject> selfHandle = args.vmcastThis<DecoratedObject>();
    selfHandle->setDecoration(std::move(native));
    setType(selfHandle);
    return HermesValue::encodeUndefinedValue();
  }

  // Otherwise allocate a new one.
  auto newHandle = runtime.makeHandle(DecoratedObject::create(
      runtime, servicePrototype, std::move(native), additionalSlots));
  setType(newHandle);
  return newHandle.getHermesValue();
}

// T is a class from platform_intl which has an appropriate
// supportedLocalesOf method.
template <typename T>
CallResult<HermesValue> intlServiceSupportedLocalesOf(
    Runtime &runtime,
    NativeArgs args) {
  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<platform_intl::Options> optionsRes = normalizeOptions(
      runtime, args.getArgHandle(1), kSupportedLocalesOfOptions);
  if (LLVM_UNLIKELY(optionsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return localesToJS(
      runtime, T::supportedLocalesOf(runtime, *localesRes, *optionsRes));
}

// This checks that the handle is valid (the caller may have used
// dyn_vmcast to make it), and that the native decoration on the
// handle corresponds to the expected class.
template <typename T>
CallResult<T *> verifyDecoration(
    Runtime &runtime,
    Handle<DecoratedObject> handle,
    const char *what) {
  if (!handle) {
    return runtime.raiseTypeError(
        TwineChar16(what) + " called with incompatible 'this'");
  }

  NamedPropertyDescriptor desc;
  bool exists = JSObject::getOwnNamedDescriptor(
      handle,
      runtime,
      Predefined::getSymbolID(Predefined::InternalPropertyIntlNativeType),
      desc);
  if (!exists) {
    return runtime.raiseTypeError(
        TwineChar16(what) + " called with incompatible 'this'");
  }
  auto val = JSObject::getNamedSlotValueUnsafe(*handle, runtime, desc);
  if (val.getNumber(runtime) != (uint32_t)T::getNativeType()) {
    return runtime.raiseTypeError(
        TwineChar16(what) + " called with incompatible 'this'");
  }
  return static_cast<T *>(handle->getDecoration());
}

} // namespace

namespace { // Collator impl stuff.

// Collator internal slots.
enum class CollatorSlotIndexes { boundCompare, COUNT };

PseudoHandle<NativeFunction> getBoundCompare(
    DecoratedObject *collator,
    Runtime &runtime) {
  return createPseudoHandle(dyn_vmcast<NativeFunction>(
      DecoratedObject::getAdditionalSlotValue(
          collator,
          runtime,
          static_cast<unsigned int>(CollatorSlotIndexes::boundCompare))
          .unboxToHV(runtime)));
}

void setBoundCompare(
    DecoratedObject *collator,
    Runtime &runtime,
    PseudoHandle<NativeFunction> value) {
  DecoratedObject::setAdditionalSlotValue(
      collator,
      runtime,
      static_cast<unsigned int>(CollatorSlotIndexes::boundCompare),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

// Collator compare internal slots
enum class CollatorCompareSlotIndexes { collator, COUNT };

PseudoHandle<DecoratedObject> getCollator(
    PseudoHandle<NativeFunction> compare,
    Runtime &runtime) {
  return createPseudoHandle(vmcast<DecoratedObject>(
      NativeFunction::getAdditionalSlotValue(
          compare.get(),
          runtime,
          static_cast<unsigned int>(CollatorCompareSlotIndexes::collator))
          .getObject(runtime)));
}

void setCollator(
    PseudoHandle<NativeFunction> compare,
    Runtime &runtime,
    PseudoHandle<DecoratedObject> value) {
  NativeFunction::setAdditionalSlotValue(
      compare.get(),
      runtime,
      static_cast<unsigned int>(CollatorCompareSlotIndexes::collator),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

CallResult<PseudoHandle<JSObject>>
intlCollatorCreator(Runtime &runtime, Handle<JSObject> parentHandle, void *) {
  return createPseudoHandle<JSObject>(
      DecoratedObject::create(
          runtime,
          parentHandle,
          nullptr,
          static_cast<unsigned int>(CollatorSlotIndexes::COUNT))
          .get());
}

void defineIntlCollator(Runtime &runtime, Handle<JSObject> intl) {
  // Create %CollatorPrototype% intrinsic.  Properties will be added later.
  Handle<JSObject> prototype = runtime.makeHandle(JSObject::create(runtime));
  runtime.intlCollatorPrototype = prototype.getHermesValue();

  // Create %Collator% intrinsic.
  Handle<NativeConstructor> constructor = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::Collator),
      intlCollatorConstructor,
      prototype,
      0,
      intlCollatorCreator,
      CellKind::DecoratedObjectKind);
  runtime.intlCollator = constructor.getHermesValue();

  {
    DefinePropertyFlags dpf{};
    dpf.setValue = 1;

    defineProperty(
        runtime,
        constructor,
        Predefined::getSymbolID(Predefined::prototype),
        prototype,
        dpf);
  }

  defineMethod(
      runtime,
      constructor,
      Predefined::getSymbolID(Predefined::supportedLocalesOf),
      nullptr,
      intlCollatorSupportedLocalesOf,
      1);

  // Add properties to prototype.

  defineProperty(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::constructor),
      constructor);

  {
    auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
    dpf.writable = 0;
    dpf.enumerable = 0;

    defineProperty(
        runtime,
        prototype,
        Predefined::getSymbolID(Predefined::SymbolToStringTag),
        runtime.getPredefinedStringHandle(Predefined::IntlCollator),
        dpf);
  }

  defineAccessor(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::compare),
      nullptr,
      intlCollatorPrototypeCompareGetter,
      nullptr,
      false,
      true);

  defineMethod(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::resolvedOptions),
      nullptr,
      intlCollatorPrototypeResolvedOptions,
      0);

  // Add Collator to Intl

  defineProperty(
      runtime,
      intl,
      Predefined::getSymbolID(Predefined::Collator),
      constructor);
}

} // namespace

CallResult<HermesValue>
intlCollatorConstructor(void *, Runtime &runtime, NativeArgs args) {
  return intlServiceConstructor<platform_intl::Collator>(
      runtime,
      args,
      kCollatorOptions,
      Handle<JSObject>::vmcast(&runtime.intlCollatorPrototype),
      static_cast<unsigned int>(CollatorSlotIndexes::COUNT));
}

CallResult<HermesValue>
intlCollatorSupportedLocalesOf(void *, Runtime &runtime, NativeArgs args) {
  return intlServiceSupportedLocalesOf<platform_intl::Collator>(runtime, args);
}

CallResult<HermesValue>
intlCollatorCompare(void *, Runtime &runtime, NativeArgs args) {
  auto *nf = vmcast<NativeFunction>(
      runtime.getCurrentFrame()->getCalleeClosureUnsafe());
  PseudoHandle<DecoratedObject> collatorHandle =
      getCollator(createPseudoHandle(nf), runtime);

  // Since collatorHandle came out of an internal slot, it's an
  // assertable failure if it has the wrong type.
  platform_intl::Collator *collator =
      static_cast<platform_intl::Collator *>(collatorHandle->getDecoration());
  assert(collator && "Intl.Collator platform part is nullptr");

  CallResult<std::u16string> xRes =
      stringFromJS(runtime, createPseudoHandle(args.getArg(0)));
  if (LLVM_UNLIKELY(xRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<std::u16string> yRes =
      stringFromJS(runtime, createPseudoHandle(args.getArg(1)));
  if (LLVM_UNLIKELY(yRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return HermesValue::encodeNumberValue(collator->compare(*xRes, *yRes));
}

CallResult<HermesValue>
intlCollatorPrototypeCompareGetter(void *, Runtime &runtime, NativeArgs args) {
  Handle<DecoratedObject> collatorHandle = args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::Collator *> collatorRes =
      verifyDecoration<platform_intl::Collator>(
          runtime, collatorHandle, "Intl.Collator.prototype.compare getter");
  if (LLVM_UNLIKELY(collatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  PseudoHandle<NativeFunction> boundCompare =
      getBoundCompare(*collatorHandle, runtime);
  if (boundCompare) {
    return boundCompare.getHermesValue();
  }

  Handle<NativeFunction> compare = NativeFunction::createWithoutPrototype(
      runtime,
      nullptr,
      intlCollatorCompare,
      Predefined::getSymbolID(Predefined::emptyString),
      2,
      static_cast<unsigned int>(CollatorCompareSlotIndexes::COUNT));
  setCollator(compare, runtime, collatorHandle);

  setBoundCompare(*collatorHandle, runtime, compare);

  return compare.getHermesValue();
}

CallResult<HermesValue> intlCollatorPrototypeResolvedOptions(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> collatorHandle = args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::Collator *> collatorRes =
      verifyDecoration<platform_intl::Collator>(
          runtime, collatorHandle, "Intl.Collator.prototype.resolvedOptions");
  if (LLVM_UNLIKELY(collatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return optionsToJS(runtime, (*collatorRes)->resolvedOptions());
}

namespace { // DateTimeFormat impl stuff.

// DateTimeFormat internal slots.
enum class DTFSlotIndexes { boundFormat, COUNT };

PseudoHandle<NativeFunction> getDTFBoundFormat(
    PseudoHandle<DecoratedObject> dtf,
    Runtime &runtime) {
  return createPseudoHandle(dyn_vmcast<NativeFunction>(
      DecoratedObject::getAdditionalSlotValue(
          dtf.get(),
          runtime,
          static_cast<unsigned int>(DTFSlotIndexes::boundFormat))
          .unboxToHV(runtime)));
}

void setDTFBoundFormat(
    PseudoHandle<DecoratedObject> dtf,
    Runtime &runtime,
    PseudoHandle<NativeFunction> value) {
  DecoratedObject::setAdditionalSlotValue(
      dtf.get(),
      runtime,
      static_cast<unsigned int>(DTFSlotIndexes::boundFormat),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

// DateTimeFormat format internal slots
enum class DTFFormatSlotIndexes { dateTimeFormat, COUNT };

PseudoHandle<DecoratedObject> getDateTimeFormat(
    PseudoHandle<NativeFunction> format,
    Runtime &runtime) {
  return createPseudoHandle(vmcast<DecoratedObject>(
      NativeFunction::getAdditionalSlotValue(
          format.get(),
          runtime,
          static_cast<unsigned int>(DTFFormatSlotIndexes::dateTimeFormat))
          .getObject(runtime)));
}

void setDateTimeFormat(
    PseudoHandle<NativeFunction> format,
    Runtime &runtime,
    PseudoHandle<DecoratedObject> value) {
  NativeFunction::setAdditionalSlotValue(
      format.get(),
      runtime,
      static_cast<unsigned int>(DTFFormatSlotIndexes::dateTimeFormat),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

CallResult<PseudoHandle<JSObject>> intlDateTimeFormatCreator(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    void *) {
  return createPseudoHandle<JSObject>(
      DecoratedObject::create(
          runtime,
          parentHandle,
          nullptr,
          static_cast<unsigned int>(DTFSlotIndexes::COUNT))
          .get());
}

void defineIntlDateTimeFormat(Runtime &runtime, Handle<JSObject> intl) {
  // Create %DateTimeFormatPrototype% intrinsic.  Properties will be added
  // later.
  Handle<JSObject> prototype = runtime.makeHandle(JSObject::create(runtime));
  runtime.intlDateTimeFormatPrototype = prototype.getHermesValue();

  // Create %DateTimeFormat% intrinsic.
  Handle<NativeConstructor> constructor = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::DateTimeFormat),
      intlDateTimeFormatConstructor,
      prototype,
      0,
      intlDateTimeFormatCreator,
      CellKind::DecoratedObjectKind);
  runtime.intlDateTimeFormat = constructor.getHermesValue();

  {
    DefinePropertyFlags dpf{};
    dpf.setValue = 1;

    defineProperty(
        runtime,
        constructor,
        Predefined::getSymbolID(Predefined::prototype),
        prototype,
        dpf);
  }

  defineMethod(
      runtime,
      constructor,
      Predefined::getSymbolID(Predefined::supportedLocalesOf),
      nullptr,
      intlDateTimeFormatSupportedLocalesOf,
      1);

  // Add properties to prototype.

  defineProperty(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::constructor),
      constructor);

  {
    auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
    dpf.writable = 0;
    dpf.enumerable = 0;

    defineProperty(
        runtime,
        prototype,
        Predefined::getSymbolID(Predefined::SymbolToStringTag),
        runtime.getPredefinedStringHandle(Predefined::IntlDateTimeFormat),
        dpf);
  }

  defineAccessor(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::format),
      nullptr,
      intlDateTimeFormatPrototypeFormatGetter,
      nullptr,
      false,
      true);

  defineMethod(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::formatToParts),
      nullptr,
      intlDateTimeFormatPrototypeFormatToParts,
      1);

  defineMethod(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::resolvedOptions),
      nullptr,
      intlDateTimeFormatPrototypeResolvedOptions,
      0);

  // Add DateTimeFormat to Intl

  defineProperty(
      runtime,
      intl,
      Predefined::getSymbolID(Predefined::DateTimeFormat),
      constructor);
}

} // namespace

CallResult<HermesValue>
intlDateTimeFormatConstructor(void *, Runtime &runtime, NativeArgs args) {
  return intlServiceConstructor<platform_intl::DateTimeFormat>(
      runtime,
      args,
      kDTFOptions,
      Handle<JSObject>::vmcast(&runtime.intlDateTimeFormatPrototype),
      static_cast<unsigned int>(DTFSlotIndexes::COUNT));
}

CallResult<HermesValue> intlDateTimeFormatSupportedLocalesOf(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  return intlServiceSupportedLocalesOf<platform_intl::DateTimeFormat>(
      runtime, args);
}

namespace {

CallResult<double> dateNowValue(Runtime &runtime, NativeArgs args) {
  // ECMA 402 13.1.5:
  // 1 and 2 are implicit in the callers.
  // 3. If date is not provided or is undefined, then
  if (args.getArg(0).isUndefined()) {
    // a. Let x be Call(%Date_now%, undefined).
    // Reusing NativeArgs is kind of a hack, but dateNow doesn't use
    // it (or the context arg), so this is ok.
    CallResult<HermesValue> dateRes = dateNow(nullptr, runtime, args);
    if (LLVM_UNLIKELY(dateRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    return dateRes->getNumber();
  }

  // 4.a. Let x be ? ToNumber(date).
  CallResult<HermesValue> xRes = toNumber_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(xRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  // ECMA 402 13.1.6, shared by the callers to this function:
  // 1. Let x be TimeClip(x).
  double x = timeClip(xRes->getNumber());
  // 2. If x is NaN, throw a RangeError exception.
  if (std::isnan(x)) {
    return runtime.raiseRangeError("Invalid time value");
  }

  return x;
}

} // namespace

CallResult<HermesValue>
intlDateTimeFormatFormat(void *, Runtime &runtime, NativeArgs args) {
  auto *nf = vmcast<NativeFunction>(
      runtime.getCurrentFrame()->getCalleeClosureUnsafe());
  PseudoHandle<DecoratedObject> dateTimeFormatHandle =
      getDateTimeFormat(createPseudoHandle(nf), runtime);

  // Since dateTimeFormatHandle came out of an internal slot, it's an
  // assertable failure if it has the wrong type.
  platform_intl::DateTimeFormat *dateTimeFormat =
      static_cast<platform_intl::DateTimeFormat *>(
          dateTimeFormatHandle->getDecoration());
  assert(dateTimeFormat && "Intl.DateTimeFormat platform part is nullptr");

  CallResult<double> dateRes = dateNowValue(runtime, args);
  if (LLVM_UNLIKELY(dateRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return StringPrimitive::createEfficient(
      runtime, dateTimeFormat->format(*dateRes));
}

CallResult<HermesValue> intlDateTimeFormatPrototypeFormatGetter(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> dateTimeFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::DateTimeFormat *> dateTimeFormatRes =
      verifyDecoration<platform_intl::DateTimeFormat>(
          runtime,
          dateTimeFormatHandle,
          "Intl.DateTimeFormat.prototype.format getter");
  if (LLVM_UNLIKELY(dateTimeFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  PseudoHandle<NativeFunction> boundFormat =
      getDTFBoundFormat(dateTimeFormatHandle, runtime);
  if (boundFormat) {
    return boundFormat.getHermesValue();
  }

  Handle<NativeFunction> format = NativeFunction::createWithoutPrototype(
      runtime,
      nullptr,
      intlDateTimeFormatFormat,
      Predefined::getSymbolID(Predefined::emptyString),
      1,
      static_cast<unsigned int>(DTFFormatSlotIndexes::COUNT));
  setDateTimeFormat(format, runtime, dateTimeFormatHandle);

  setDTFBoundFormat(dateTimeFormatHandle, runtime, format);

  return format.getHermesValue();
}

CallResult<HermesValue> intlDateTimeFormatPrototypeFormatToParts(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> dateTimeFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::DateTimeFormat *> dateTimeFormatRes =
      verifyDecoration<platform_intl::DateTimeFormat>(
          runtime,
          dateTimeFormatHandle,
          "Intl.DateTimeFormat.prototype.formatToParts");
  if (LLVM_UNLIKELY(dateTimeFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<double> dateRes = dateNowValue(runtime, args);
  if (LLVM_UNLIKELY(dateRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return partsToJS(runtime, (*dateTimeFormatRes)->formatToParts(*dateRes));
}

CallResult<HermesValue> intlDateTimeFormatPrototypeResolvedOptions(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> dateTimeFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::DateTimeFormat *> dateTimeFormatRes =
      verifyDecoration<platform_intl::DateTimeFormat>(
          runtime,
          dateTimeFormatHandle,
          "Intl.DateTimeFormat.prototype.resolvedOptions");
  if (LLVM_UNLIKELY(dateTimeFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return optionsToJS(runtime, (*dateTimeFormatRes)->resolvedOptions());
}

namespace { // NumberFormat impl stuff.

// NumberFormat internal slots.
enum class NFSlotIndexes { boundFormat, COUNT };

PseudoHandle<NativeFunction> getNFBoundFormat(
    PseudoHandle<DecoratedObject> nf,
    Runtime &runtime) {
  return createPseudoHandle(dyn_vmcast<NativeFunction>(
      DecoratedObject::getAdditionalSlotValue(
          nf.get(),
          runtime,
          static_cast<unsigned int>(NFSlotIndexes::boundFormat))
          .unboxToHV(runtime)));
}

void setNFBoundFormat(
    PseudoHandle<DecoratedObject> nf,
    Runtime &runtime,
    PseudoHandle<NativeFunction> value) {
  DecoratedObject::setAdditionalSlotValue(
      nf.get(),
      runtime,
      static_cast<unsigned int>(NFSlotIndexes::boundFormat),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

// NumberFormat format internal slots
enum class NFFormatSlotIndexes { numberFormat, COUNT };

PseudoHandle<DecoratedObject> getNumberFormat(
    PseudoHandle<NativeFunction> format,
    Runtime &runtime) {
  return createPseudoHandle(vmcast<DecoratedObject>(
      NativeFunction::getAdditionalSlotValue(
          format.get(),
          runtime,
          static_cast<unsigned int>(NFFormatSlotIndexes::numberFormat))
          .getObject(runtime)));
}

void setNumberFormat(
    PseudoHandle<NativeFunction> format,
    Runtime &runtime,
    PseudoHandle<DecoratedObject> value) {
  NativeFunction::setAdditionalSlotValue(
      format.get(),
      runtime,
      static_cast<unsigned int>(NFFormatSlotIndexes::numberFormat),
      SmallHermesValue::encodeObjectValue(value.get(), runtime));
}

CallResult<PseudoHandle<JSObject>> intlNumberFormatCreator(
    Runtime &runtime,
    Handle<JSObject> parentHandle,
    void *) {
  return createPseudoHandle<JSObject>(
      DecoratedObject::create(
          runtime,
          parentHandle,
          nullptr,
          static_cast<unsigned int>(NFSlotIndexes::COUNT))
          .get());
}

void defineIntlNumberFormat(Runtime &runtime, Handle<JSObject> intl) {
  // Create %NumberFormatPrototype% intrinsic.  Properties will be added later.
  Handle<JSObject> prototype = runtime.makeHandle(JSObject::create(runtime));
  runtime.intlNumberFormatPrototype = prototype.getHermesValue();

  // Create %NumberFormat% intrinsic.
  Handle<NativeConstructor> constructor = defineSystemConstructor(
      runtime,
      Predefined::getSymbolID(Predefined::NumberFormat),
      intlNumberFormatConstructor,
      prototype,
      0,
      intlNumberFormatCreator,
      CellKind::DecoratedObjectKind);
  runtime.intlNumberFormat = constructor.getHermesValue();

  {
    DefinePropertyFlags dpf{};
    dpf.setValue = 1;

    defineProperty(
        runtime,
        constructor,
        Predefined::getSymbolID(Predefined::prototype),
        prototype,
        dpf);
  }

  defineMethod(
      runtime,
      constructor,
      Predefined::getSymbolID(Predefined::supportedLocalesOf),
      nullptr,
      intlNumberFormatSupportedLocalesOf,
      1);

  // Add properties to prototype.

  defineProperty(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::constructor),
      constructor);

  {
    auto dpf = DefinePropertyFlags::getDefaultNewPropertyFlags();
    dpf.writable = 0;
    dpf.enumerable = 0;

    defineProperty(
        runtime,
        prototype,
        Predefined::getSymbolID(Predefined::SymbolToStringTag),
        runtime.getPredefinedStringHandle(Predefined::IntlNumberFormat),
        dpf);
  }

  defineAccessor(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::format),
      nullptr,
      intlNumberFormatPrototypeFormatGetter,
      nullptr,
      false,
      true);

#ifndef __APPLE__
  defineMethod(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::formatToParts),
      nullptr,
      intlNumberFormatPrototypeFormatToParts,
      1);
#endif

  defineMethod(
      runtime,
      prototype,
      Predefined::getSymbolID(Predefined::resolvedOptions),
      nullptr,
      intlNumberFormatPrototypeResolvedOptions,
      0);

  // Add NumberFormat to Intl

  defineProperty(
      runtime,
      intl,
      Predefined::getSymbolID(Predefined::NumberFormat),
      constructor);
}

} // namespace

CallResult<HermesValue>
intlNumberFormatConstructor(void *, Runtime &runtime, NativeArgs args) {
  return intlServiceConstructor<platform_intl::NumberFormat>(
      runtime,
      args,
      kNumberFormatOptions,
      Handle<JSObject>::vmcast(&runtime.intlNumberFormatPrototype),
      static_cast<unsigned int>(NFSlotIndexes::COUNT));
}

CallResult<HermesValue>
intlNumberFormatSupportedLocalesOf(void *, Runtime &runtime, NativeArgs args) {
  return intlServiceSupportedLocalesOf<platform_intl::NumberFormat>(
      runtime, args);
}

CallResult<HermesValue>
intlNumberFormatFormat(void *, Runtime &runtime, NativeArgs args) {
  auto *nf = vmcast<NativeFunction>(
      runtime.getCurrentFrame()->getCalleeClosureUnsafe());
  PseudoHandle<DecoratedObject> numberFormatHandle =
      getNumberFormat(createPseudoHandle(nf), runtime);

  // Since numberFormatHandle came out of an internal slot, it's an
  // assertable failure if it has the wrong type.
  platform_intl::NumberFormat *numberFormat =
      static_cast<platform_intl::NumberFormat *>(
          numberFormatHandle->getDecoration());
  assert(numberFormat && "Intl.NumberFormat platform part is nullptr");

  CallResult<HermesValue> xRes = toNumeric_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(xRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return StringPrimitive::createEfficient(
      runtime, numberFormat->format(xRes->getNumber()));
}

CallResult<HermesValue> intlNumberFormatPrototypeFormatGetter(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> numberFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::NumberFormat *> numberFormatRes =
      verifyDecoration<platform_intl::NumberFormat>(
          runtime,
          numberFormatHandle,
          "Intl.NumberFormat.prototype.format getter");
  if (LLVM_UNLIKELY(numberFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  PseudoHandle<NativeFunction> boundFormat =
      getNFBoundFormat(numberFormatHandle, runtime);
  if (boundFormat) {
    return boundFormat.getHermesValue();
  }

  Handle<NativeFunction> format = NativeFunction::createWithoutPrototype(
      runtime,
      nullptr,
      intlNumberFormatFormat,
      Predefined::getSymbolID(Predefined::emptyString),
      1,
      static_cast<unsigned int>(NFFormatSlotIndexes::COUNT));
  setNumberFormat(format, runtime, numberFormatHandle);

  setNFBoundFormat(numberFormatHandle, runtime, format);

  return format.getHermesValue();
}

CallResult<HermesValue> intlNumberFormatPrototypeFormatToParts(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> numberFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::NumberFormat *> numberFormatRes =
      verifyDecoration<platform_intl::NumberFormat>(
          runtime,
          numberFormatHandle,
          "Intl.NumberFormat.prototype.formatToParts");
  if (LLVM_UNLIKELY(numberFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<HermesValue> xRes = toNumeric_RJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(xRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return partsToJS(
      runtime, (*numberFormatRes)->formatToParts(xRes->getNumber()));
}

CallResult<HermesValue> intlNumberFormatPrototypeResolvedOptions(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  Handle<DecoratedObject> numberFormatHandle =
      args.dyncastThis<DecoratedObject>();

  CallResult<platform_intl::NumberFormat *> numberFormatRes =
      verifyDecoration<platform_intl::NumberFormat>(
          runtime,
          numberFormatHandle,
          "Intl.NumberFormat.prototype.resolvedOptions");
  if (LLVM_UNLIKELY(numberFormatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return optionsToJS(runtime, (*numberFormatRes)->resolvedOptions());
}

// ECMA 402 supersedes some definitionss in ECMA 262

namespace {

constexpr int kDTODate = 1 << 0;
constexpr int kDTOTime = 1 << 1;

void toDateTimeOptions(platform_intl::Options &options, int dtoFlags) {
  // The behavior of format with respect to default options is to
  // check if any of a set of date and time keys are present in
  // options.  If none are, then a default set of date keys is used.
  //
  // So, as long as this code sets any of the date or time keys, then
  // the default options behavior of format will not apply.

  bool needDefaults = true;
  for (const OptionData &pod : kDTFOptions) {
    if (dtoFlags & kDTODate && pod.flags & kDateRequired &&
        options.find(std::u16string(pod.name)) != options.end()) {
      needDefaults = false;
      break;
    }
    if (needDefaults) {
      if (dtoFlags & kDTOTime && pod.flags & kTimeRequired &&
          options.find(std::u16string(pod.name)) != options.end()) {
        needDefaults = false;
        break;
      }
    }
  }
  if (!needDefaults) {
    return;
  }
  for (const OptionData &pod : kDTFOptions) {
    if ((dtoFlags & kDTODate && pod.flags & kDateDefault) ||
        (dtoFlags & kDTOTime && pod.flags & kTimeDefault)) {
      options.emplace(pod.name, std::u16string(u"numeric"));
    }
  }
}

CallResult<HermesValue> intlDatePrototypeToSomeLocaleString(
    Runtime &runtime,
    const NativeArgs &args,
    JSDate *date,
    int dtoFlags) {
  double x = date->getPrimitiveValue();
  std::u16string str;
  if (std::isnan(x)) {
    str = u"Invalid Date";
  } else {
    CallResult<std::vector<std::u16string>> localesRes =
        normalizeLocales(runtime, args.getArgHandle(0));
    if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    CallResult<platform_intl::Options> optionsRes =
        normalizeOptions(runtime, args.getArgHandle(1), kDTFOptions);
    if (LLVM_UNLIKELY(optionsRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }
    toDateTimeOptions(*optionsRes, dtoFlags);

    CallResult<std::unique_ptr<platform_intl::DateTimeFormat>> dtfRes =
        platform_intl::DateTimeFormat::create(
            runtime, *localesRes, *optionsRes);
    if (LLVM_UNLIKELY(dtfRes == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    // Naively, the spec requires TimeClip to be called here, but
    // since in this code path, x comes from a Date slot which has
    // already been clipped, there's no reason to do it again.
    str = (*dtfRes)->format(x);
  }

  return StringPrimitive::createEfficient(runtime, std::move(str));
}

} // namespace

CallResult<HermesValue>
intlDatePrototypeToLocaleDateString(void *, Runtime &runtime, NativeArgs args) {
  JSDate *date = dyn_vmcast<JSDate>(args.getThisArg());
  if (!date) {
    return runtime.raiseTypeError(
        "Date.prototype.toLocaleString() called on non-Date object");
  }
  return intlDatePrototypeToSomeLocaleString(runtime, args, date, kDTODate);
}

CallResult<HermesValue>
intlDatePrototypeToLocaleString(void *, Runtime &runtime, NativeArgs args) {
  JSDate *date = dyn_vmcast<JSDate>(args.getThisArg());
  if (!date) {
    return runtime.raiseTypeError(
        "Date.prototype.toLocaleString() called on non-Date object");
  }
  return intlDatePrototypeToSomeLocaleString(
      runtime, args, date, kDTODate | kDTOTime);
}

CallResult<HermesValue>
intlDatePrototypeToLocaleTimeString(void *, Runtime &runtime, NativeArgs args) {
  JSDate *date = dyn_vmcast<JSDate>(args.getThisArg());
  if (!date) {
    return runtime.raiseTypeError(
        "Date.prototype.toLocaleString() called on non-Date object");
  }
  return intlDatePrototypeToSomeLocaleString(runtime, args, date, kDTOTime);
}

CallResult<HermesValue>
intlNumberPrototypeToLocaleString(void *, Runtime &runtime, NativeArgs args) {
  double x;
  if (args.getThisArg().isNumber()) {
    x = args.getThisArg().getNumber();
  } else {
    auto numPtr = Handle<JSNumber>::dyn_vmcast(args.getThisHandle());
    if (LLVM_UNLIKELY(!numPtr)) {
      return runtime.raiseTypeError(
          "Number.prototype.toLocaleString() can only be used on numbers");
    }
    x = numPtr->getPrimitiveNumber();
  }

  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<platform_intl::Options> optionsRes =
      normalizeOptions(runtime, args.getArgHandle(1), kNumberFormatOptions);
  if (LLVM_UNLIKELY(optionsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<std::unique_ptr<platform_intl::NumberFormat>> nfRes =
      platform_intl::NumberFormat::create(runtime, *localesRes, *optionsRes);
  if (LLVM_UNLIKELY(nfRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  return StringPrimitive::createEfficient(runtime, (*nfRes)->format(x));
}

CallResult<HermesValue>
intlStringPrototypeLocaleCompare(void *, Runtime &runtime, NativeArgs args) {
  if (args.getThisArg().isUndefined() || args.getThisArg().isNull()) {
    return runtime.raiseTypeError(
        "String.prototype.localeCompare called on null or undefined");
  }
  CallResult<std::u16string> thisRes =
      stringFromJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(thisRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<std::u16string> thatRes =
      stringFromJS(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(thatRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(1));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<platform_intl::Options> optionsRes =
      normalizeOptions(runtime, args.getArgHandle(2), kCollatorOptions);
  if (LLVM_UNLIKELY(optionsRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<std::unique_ptr<platform_intl::Collator>> collatorRes =
      platform_intl::Collator::create(runtime, *localesRes, *optionsRes);
  if (LLVM_UNLIKELY(collatorRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return HermesValue::encodeNumberValue(
      (*collatorRes)->compare(*thisRes, *thatRes));
}

CallResult<HermesValue> intlStringPrototypeToLocaleLowerCase(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  if (args.getThisArg().isUndefined() || args.getThisArg().isNull()) {
    return runtime.raiseTypeError(
        "String.prototype.localeCompare called on null or undefined");
  }
  CallResult<std::u16string> thisRes =
      stringFromJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(thisRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<std::u16string> lowerRes =
      platform_intl::toLocaleLowerCase(runtime, *localesRes, *thisRes);
  if (LLVM_UNLIKELY(lowerRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return StringPrimitive::createEfficient(runtime, std::move(*lowerRes));
}

CallResult<HermesValue> intlStringPrototypeToLocaleUpperCase(
    void *,
    Runtime &runtime,
    NativeArgs args) {
  if (args.getThisArg().isUndefined() || args.getThisArg().isNull()) {
    return runtime.raiseTypeError(
        "String.prototype.localeCompare called on null or undefined");
  }
  CallResult<std::u16string> thisRes =
      stringFromJS(runtime, args.getThisHandle());
  if (LLVM_UNLIKELY(thisRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  CallResult<std::vector<std::u16string>> localesRes =
      normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  CallResult<std::u16string> upperRes =
      platform_intl::toLocaleUpperCase(runtime, *localesRes, *thisRes);
  if (LLVM_UNLIKELY(upperRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }

  return StringPrimitive::createEfficient(runtime, std::move(*upperRes));
}

} // namespace vm

namespace intl {

namespace { // Intl impl stuff.

vm::CallResult<vm::HermesValue>
intlGetCanonicalLocales(void *, vm::Runtime &runtime, vm::NativeArgs args) {
  vm::CallResult<std::vector<std::u16string>> localesRes =
      vm::normalizeLocales(runtime, args.getArgHandle(0));
  if (LLVM_UNLIKELY(localesRes == vm::ExecutionStatus::EXCEPTION)) {
    return vm::ExecutionStatus::EXCEPTION;
  }

  return vm::localesToJS(
      runtime, platform_intl::getCanonicalLocales(runtime, *localesRes));
}

} // namespace

vm::Handle<vm::JSObject> createIntlObject(vm::Runtime &runtime) {
  vm::Handle<vm::JSObject> intl =
      runtime.makeHandle(vm::JSObject::create(runtime));
  defineMethod(
      runtime,
      intl,
      vm::Predefined::getSymbolID(vm::Predefined::getCanonicalLocales),
      nullptr,
      intlGetCanonicalLocales,
      1);

  {
    auto dpf = vm::DefinePropertyFlags::getDefaultNewPropertyFlags();
    dpf.writable = 0;
    dpf.enumerable = 0;

    defineProperty(
        runtime,
        intl,
        vm::Predefined::getSymbolID(vm::Predefined::SymbolToStringTag),
        runtime.getPredefinedStringHandle(vm::Predefined::Intl),
        dpf);
  }

  vm::defineIntlCollator(runtime, intl);
  vm::defineIntlDateTimeFormat(runtime, intl);
  vm::defineIntlNumberFormat(runtime, intl);
  return intl;
}

} // namespace intl
} // namespace hermes

#endif
