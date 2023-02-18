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
  {
  UTF8Ref utf8{(const uint8_t*)stringView.data(), stringView.size()};
  auto string = StringPrimitive::createEfficient(rt, utf8);
  // if (LLVM_UNLIKELY(string == ExecutionStatus::EXCEPTION)) {
  //   return ExecutionStatus::EXCEPTION;
  // }
  return rt.makeHandle(*string);
  }


  // check if ascii
  // TODO: Surprisingly, isAllASCII is faster (for small strings) than simdutf::validate_ascii?
  if (isAllASCII(stringView.data(), stringView.data() + stringView.size())) {
    ASCIIRef ascii{stringView.data(), stringView.size()};
    auto string = StringPrimitive::createEfficient(rt, ascii);
    return rt.makeHandle(*string);
  }

  // convert to utf16
  auto utf16_expected_size = simdutf::utf16_length_from_utf8(stringView.data(), stringView.size());
  std::unique_ptr<char16_t[]> utf16_output{new char16_t[utf16_expected_size]};
  auto utf16_words = simdutf::convert_utf8_to_utf16(stringView.data(), stringView.size(), utf16_output.get());
  // if (!utf16_words) {
  //   return ExecutionStatus::EXCEPTION;
  // }
  UTF16Ref utf16{utf16_output.get(), utf16_words};
  auto string = StringPrimitive::create(rt, utf16);
  return rt.makeHandle(*string);


  // if (auto existing = rt.getIdentifierTable().getExistingStringPrimitiveOrNull(rt, utf16)) {
  //   return rt.makeHandle(existing);
  // }
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

  switch (type) {
    case ondemand::json_type::string: {
      std::string_view stringView;
      SIMDJSON_CALL(value.get(stringView));
      auto jsString = parseString(rt, stringView);
      // if (LLVM_UNLIKELY(jsString == ExecutionStatus::EXCEPTION)) {
      //   return ExecutionStatus::EXCEPTION;
      // }
      return jsString.getHermesValue();
    }
    case ondemand::json_type::number: {
      double doubleValue;
      SIMDJSON_CALL(value.get(doubleValue));
      return HermesValue::encodeDoubleValue(doubleValue);
    }
    case ondemand::json_type::object: {
      ondemand::object objectValue;
      SIMDJSON_CALL(value.get(objectValue));
      auto jsObject = parseObject(rt, objectValue);
      if (LLVM_UNLIKELY(jsObject == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return jsObject;
    }
    case ondemand::json_type::array: {
      ondemand::array arrayValue;
      SIMDJSON_CALL(value.get(arrayValue));
      auto jsArray = parseArray(rt, arrayValue);
      if (LLVM_UNLIKELY(jsArray == ExecutionStatus::EXCEPTION)) {
        return ExecutionStatus::EXCEPTION;
      }
      return jsArray;
    }
    case ondemand::json_type::boolean: {
      bool boolValue;
      SIMDJSON_CALL(value.get(boolValue));
      return HermesValue::encodeBoolValue(boolValue);
    }
    case ondemand::json_type::null:
      return HermesValue::encodeNullValue();
    default:
      return ExecutionStatus::EXCEPTION;
  }
}

CallResult<HermesValue> runtimeFastJSONParse(
    Runtime &rt,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver) {
  simdjson::error_code error;

  auto commonStorage = rt.getCommonStorage();

  // TODO: Error handling

  // auto asciiRef = jsonString->getStringRef<char>();
  // auto json = padded_string(asciiRef.data(), asciiRef.size());

  // NOTE: We need to copy (I think) because strings can move during GC
  // but I'm not sure why we seemingly need to copy multiple times...
  UTF16Ref ref;
  SmallU16String<32> storage;
  if (LLVM_UNLIKELY(jsonString->isExternal() && !jsonString->isASCII())) {
    ref = jsonString->getStringRef<char16_t>();
  } else {
    StringPrimitive::createStringView(rt, jsonString)
        .appendUTF16String(storage);
    ref = storage;
  }

  // convert to utf8
  auto utf8_expected_size = simdutf::utf8_length_from_utf16(ref.data(), ref.size());
  std::unique_ptr<char[]> utf8_output{new char[utf8_expected_size]};
  auto utf8_size = simdutf::convert_utf16_to_utf8(ref.data(), ref.size(), utf8_output.get());
  if (!utf8_size) {
    return ExecutionStatus::EXCEPTION;
  }

  // parse json
  auto json = padded_string(utf8_output.get(), utf8_size);
  ondemand::document doc;
  SIMDJSON_CALL(commonStorage->simdjsonParser.iterate(json).get(doc));

  return parseValue(rt, doc);
}

} // namespace vm
} // namespace hermes
