#include "hermes/VM/JSLib/RuntimeFastJSONUtils.h"

#include "Object.h"

#include "hermes/Support/Compiler.h"
#include "hermes/Support/JSON.h"
#include "hermes/Support/UTF16Stream.h"
#include "hermes/VM/ArrayLike.h"
#include "hermes/VM/ArrayStorage.h"
#include "hermes/VM/Callable.h"
#include "hermes/VM/JSArray.h"
#include "hermes/VM/JSProxy.h"
#include "hermes/VM/PrimitiveBox.h"
#include "hermes/VM/JSLib/RuntimeCommonStorage.h"

#include "JSONLexer.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/SaveAndRestore.h"
#include "llvh/Support/ConvertUTF.h"

#include "simdjson/src/simdjson.h"
#include "simdutf/src/simdutf.h"

using namespace simdjson;

#define SIMDJSON_CALL(operation)            \
  error = operation;                        \
  if (LLVM_UNLIKELY(error)) {               \
    /* TODO: Actual error messsage */       \
    return rt.raiseSyntaxError(             \
      TwineChar16("JSON fastParse error")   \
    );                                      \
  }

namespace hermes {
namespace vm {

template<typename T>
CallResult<HermesValue> parseValue(Runtime &rt, T &value);

// FIXME: Copy&paste from StringPrimitive
static ExecutionStatus convertUtf8ToUtf16(
    Runtime &runtime,
    UTF8Ref utf8,
    bool IgnoreInputErrors,
    std::u16string &out) {
  out.resize(utf8.size());
  const llvh::UTF8 *sourceStart = (const llvh::UTF8 *)utf8.data();
  const llvh::UTF8 *sourceEnd = sourceStart + utf8.size();
  llvh::UTF16 *targetStart = (llvh::UTF16 *)&out[0];
  llvh::UTF16 *targetEnd = targetStart + out.size();
  llvh::ConversionResult cRes = llvh::ConvertUTF8toUTF16(
      &sourceStart,
      sourceEnd,
      &targetStart,
      targetEnd,
      llvh::lenientConversion);
  switch (cRes) {
    case llvh::ConversionResult::sourceExhausted:
      if (IgnoreInputErrors) {
        break;
      }
      return runtime.raiseRangeError(
          "Malformed UTF8 input: partial character in input");
    case llvh::ConversionResult::sourceIllegal:
      if (IgnoreInputErrors) {
        break;
      }
      return runtime.raiseRangeError("Malformed UTF8 input: illegal sequence");
    case llvh::ConversionResult::conversionOK:
      break;
    case llvh::ConversionResult::targetExhausted:
      return runtime.raiseRangeError(
          "Cannot allocate memory for UTF8 to UTF16 conversion.");
  }

  out.resize((char16_t *)targetStart - &out[0]);
  return ExecutionStatus::RETURNED;
}

Handle<HermesValue> parseString(Runtime &rt, std::string_view &stringView) {
  UTF8Ref utf8{(const uint8_t*)stringView.data(), stringView.size()};

  // std::u16string utf16str;
  // convertUtf8ToUtf16(rt, utf8, true, utf16str);
  // // TODO: check status
  // std::u16string_view utf16strView(utf16str);
  // UTF16Ref utf16{utf16strView.data(), utf16strView.size()};

  // if (auto existing = rt.getIdentifierTable().getExistingStringPrimitiveOrNull(rt, utf16)) {
  //   return rt.makeHandle(existing);
  // }

  auto string = StringPrimitive::createEfficient(rt, utf8);
  // auto string = StringPrimitive::create(rt, utf16);
  // if (LLVM_UNLIKELY(string == ExecutionStatus::EXCEPTION)) {
  //   return ExecutionStatus::EXCEPTION;
  // }
  return rt.makeHandle(*string);
}

CallResult<HermesValue> parseArray(Runtime &rt, ondemand::array &array) {
  simdjson::error_code error;

  auto jsArrayRes = JSArray::create(rt, 4, 0);
  if (LLVM_UNLIKELY(jsArrayRes == ExecutionStatus::EXCEPTION)) {
    return ExecutionStatus::EXCEPTION;
  }
  auto jsArray = *jsArrayRes;

  uint32_t index = 0;
  MutableHandle<> indexValue{rt};

  GCScope gcScope{rt};
  auto marker = gcScope.createMarker();

  for (auto valueRes : array) {
    gcScope.flushToMarker(marker);

    ondemand::value value;
    SIMDJSON_CALL(valueRes.get(value));

    auto jsValue = parseValue(rt, value);
    if (LLVM_UNLIKELY(jsValue == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    indexValue = HermesValue::encodeDoubleValue(index);
    (void)JSObject::defineOwnComputedPrimitive(
          jsArray,
          rt,
          indexValue,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          rt.makeHandle(*jsValue));

    index++;
  }

  return jsArray.getHermesValue();
}

CallResult<HermesValue> parseObject(Runtime &rt, ondemand::object &object) {
  simdjson::error_code error;

  auto jsObject = rt.makeHandle(JSObject::create(rt));

  MutableHandle<> jsKeyHandle{rt};

  GCScope gcScope{rt};
  auto marker = gcScope.createMarker();

  for (auto field : object) {
    gcScope.flushToMarker(marker);

    std::string_view key;
    SIMDJSON_CALL(field.unescaped_key().get(key));

    jsKeyHandle = parseString(rt, key);
    // if (LLVM_UNLIKELY(jsKey == ExecutionStatus::EXCEPTION)) {
    //   return ExecutionStatus::EXCEPTION;
    // }
    // jsKeyHandle = rt.makeHandle(*jsKey);

    ondemand::value value;
    SIMDJSON_CALL(field.value().get(value));

    auto jsValue = parseValue(rt, value);
    if (LLVM_UNLIKELY(jsValue == ExecutionStatus::EXCEPTION)) {
      return ExecutionStatus::EXCEPTION;
    }

    (void)JSObject::defineOwnComputedPrimitive(
          jsObject,
          rt,
          jsKeyHandle,
          DefinePropertyFlags::getDefaultNewPropertyFlags(),
          rt.makeHandle(*jsValue));
  }

  return jsObject.getHermesValue();
}

template<typename T>
CallResult<HermesValue> parseValue(Runtime &rt, T &value) {
  simdjson::error_code error;

  ondemand::json_type type;
  SIMDJSON_CALL(value.type().get(type));

  MutableHandle<> returnValue{rt};
  switch (type) {
    case ondemand::json_type::string: {
      std::string_view stringView;
      SIMDJSON_CALL(value.get(stringView));
      auto jsString = parseString(rt, stringView);
      // if (LLVM_UNLIKELY(jsString == ExecutionStatus::EXCEPTION)) {
      //   return ExecutionStatus::EXCEPTION;
      // }
      // returnValue = rt.makeHandle(*jsString);
      returnValue = jsString;
      break;
    }
    case ondemand::json_type::number:
      double doubleValue;
      SIMDJSON_CALL(value.get(doubleValue));
      returnValue = HermesValue::encodeDoubleValue(doubleValue);
      break;
    case ondemand::json_type::object: {
      ondemand::object objectValue;
      SIMDJSON_CALL(value.get(objectValue));
      auto jsObject = parseObject(rt, objectValue);
      if (LLVM_UNLIKELY(jsObject == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      returnValue = *jsObject;
      break;
    }
    case ondemand::json_type::array: {
      ondemand::array arrayValue;
      SIMDJSON_CALL(value.get(arrayValue));
      auto jsArray = parseArray(rt, arrayValue);
      if (LLVM_UNLIKELY(jsArray == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      returnValue = *jsArray;
      break;
    }
    case ondemand::json_type::boolean:
      bool boolValue;
      SIMDJSON_CALL(value.get(boolValue));
      returnValue = HermesValue::encodeBoolValue(boolValue);
      break;
    case ondemand::json_type::null:
      returnValue = HermesValue::encodeNullValue();
      break;
    default:
      return ExecutionStatus::EXCEPTION;
  }

  return returnValue.getHermesValue();
}

CallResult<HermesValue> runtimeFastJSONParse(
    Runtime &rt,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver) {
  simdjson::error_code error;

  auto commonStorage = rt.getCommonStorage();

  // TODO: Error handling
  // TODO: Support UTF-16

  // auto asciiRef = jsonString->getStringRef<char>();
  // auto json = padded_string(asciiRef.data(), asciiRef.size());

  // TODO: Avoid copying so much?
  UTF16Ref ref;
  SmallU16String<32> storage;
  if (LLVM_UNLIKELY(jsonString->isExternal() && !jsonString->isASCII())) {
    ref = jsonString->getStringRef<char16_t>();
  } else {
    StringPrimitive::createStringView(rt, jsonString)
        .appendUTF16String(storage);
    ref = storage;
  }
  std::string out;
  convertUTF16ToUTF8WithReplacements(out, ref);
  auto json = padded_string(out);

  ondemand::document doc;
  SIMDJSON_CALL(commonStorage->simdjsonParser.iterate(json).get(doc));

  return parseValue(rt, doc);
}

} // namespace vm
} // namespace hermes
