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

#include "JSONLexer.h"

#include "llvh/ADT/SmallString.h"
#include "llvh/Support/SaveAndRestore.h"

#include "simdjson/src/simdjson.h"

using namespace simdjson;

namespace hermes {
namespace vm {

CallResult<HermesValue> parseValue(Runtime &rt, ondemand::document &doc) {
  simdjson::error_code error;

  auto &value = doc;

  ondemand::json_type type;
  error = value.type().get(type);
  switch (type) {
  //   case ondemand::json_type::array:
  //   case ondemand::json_type::object:
    case ondemand::json_type::number:
      double doubleValue;
      error = value.get(doubleValue);
      return HermesValue::encodeDoubleValue(doubleValue);
  //   case ondemand::json_type::string:
  //     std::string_view stringView = value;
    case ondemand::json_type::boolean:
      bool boolValue;
      error = value.get(boolValue);
      return HermesValue::encodeBoolValue(boolValue);
    case ondemand::json_type::null:
      return HermesValue::encodeNullValue();
    default:
      return ExecutionStatus::EXCEPTION;
  }
}

CallResult<HermesValue> runtimeFastJSONParse(
    Runtime &runtime,
    Handle<StringPrimitive> jsonString,
    Handle<Callable> reviver) {
  ondemand::parser parser;
  simdjson::error_code error;
  ondemand::document doc;

  // TODO: Support UTF-16
  auto asciiRef = jsonString->getStringRef<char>();
  auto json = padded_string(asciiRef.data(), asciiRef.size());
  error = parser.iterate(json).get(doc);

  return parseValue(runtime, doc);
}

} // namespace vm
} // namespace hermes
