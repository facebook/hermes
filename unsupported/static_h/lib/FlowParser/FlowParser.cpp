/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef HERMES_USE_FLOWPARSER

#define DEBUG_TYPE "flowparser"
#include "hermes/FlowParser/FlowParser.h"

#include "hermes/AST/ASTBuilder.h"
#include "hermes/AST/ESTreeJSONDumper.h"
#include "hermes/Support/Conversions.h"

#include "llvh/Support/Debug.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include "flowparser/libflowparser.h"
#pragma GCC diagnostic pop

namespace hermes {
namespace parser {

using namespace hermes::parser;
using llvh::cast;
using llvh::dyn_cast;
using llvh::dyn_cast_or_null;
using llvh::isa;

namespace {

class JSONTranslator final
    : public flowparser::AbstractTranslator<JSONValue *> {
 public:
  JSONTranslator(JSONFactory &factory) : factory_(factory) {}

  bool getErrorEncountered() const {
    return errorEncountered_;
  }

  virtual JSONValue *convert_string(char *str) {
    return factory_.getString(str);
  }
  virtual JSONValue *convert_number(double n) {
    return factory_.getNumber(n);
  }
  virtual JSONValue *convert_bool(long b) {
    return factory_.getBoolean(b);
  }
  virtual JSONValue *convert_null() {
    return factory_.getNull();
  }
  virtual JSONValue *convert_undefined() {
    // TODO: this probably indicates an error. There is no "undefined" in JSON.
    errorEncountered_ = true;
    return factory_.getNull();
  }

  virtual JSONValue *convert_object(value props) {
    llvh::SmallVector<JSONFactory::Prop, 4> propList;

    while (flowparser::list_has_next(props)) {
      value prop = flowparser::list_head(props);
      propList.emplace_back(
          factory_.getString(flowparser::get_prop_key(prop)),
          convert_json(flowparser::get_prop_value(prop)));
      props = flowparser::list_tail(props);
    }

    auto *res = factory_.newObject(propList.begin(), propList.end());
    // In case of failure (caused by duplicate properties), set the error flag.
    return res ? res : convert_undefined();
  }

  virtual JSONValue *convert_array(value items) {
    llvh::SmallVector<JSONValue *, 4> storage;

    while (flowparser::list_has_next(items)) {
      value head = flowparser::list_head(items);
      storage.push_back(convert_json(head));
      items = flowparser::list_tail(items);
    }
    return factory_.newArray(storage.size(), storage.begin(), storage.end());
  }

 private:
  JSONFactory &factory_;
  /// Set to true if we encountered an error.
  bool errorEncountered_{false};
};

#ifndef NDEBUG
void dump(llvh::raw_ostream &OS, const JSONValue *jsonValue, unsigned indent) {
  if (!jsonValue) {
    OS << "!!nullptr!!";
    return;
  }
  switch (jsonValue->getKind()) {
    case JSONKind::Object:
      OS << "{\n";
      indent += 4;
      for (auto pair : *cast<JSONObject>(jsonValue)) {
        OS.indent(indent);
        dump(OS, pair.first, indent);
        OS << ":";
        dump(OS, pair.second, indent);
        OS << ",\n";
      }
      indent -= 4;
      OS.indent(indent) << "}";
      break;
    case JSONKind::Array:
      OS << "[\n";
      indent += 4;
      for (auto *val : *cast<JSONArray>(jsonValue)) {
        OS.indent(indent);
        dump(OS, val, indent);
        OS << ",\n";
      }
      indent -= 4;
      OS.indent(indent) << "]";
      break;
    case JSONKind::String:
      OS << "\"" << cast<JSONString>(jsonValue)->str() << "\"";
      break;
    case JSONKind::Number: {
      char buf[NUMBER_TO_STRING_BUF_SIZE];
      numberToString(cast<JSONNumber>(jsonValue)->getValue(), buf, sizeof(buf));
      OS << buf;
      break;
    }
    case JSONKind::Boolean:
      OS << (cast<JSONBoolean>(jsonValue)->getValue() ? "true" : "false");
      break;
    case JSONKind::Null:
      OS << "null";
      break;
  }
}

void dump(llvh::raw_ostream &OS, const JSONValue *jsonValue) {
  dump(OS, jsonValue, 0);
  OS << "\n";
}
#endif

/// Print all recorded errors.
/// \return true if there were no errors.
bool printErrors(Context &context, uint32_t bufferId, JSONValue *v) {
  bool haveErrors = false;

  auto *obj = dyn_cast<JSONObject>(v);
  if (!obj)
    return false;

  auto *errors = dyn_cast_or_null<JSONArray>(obj->get("errors"));
  if (!errors)
    return false;

  auto parseLoc = [&context, bufferId](JSONValue *v) -> SMLoc {
    auto *obj = dyn_cast_or_null<JSONObject>(v);
    if (!obj)
      return {};
    auto *line = dyn_cast_or_null<JSONNumber>(obj->get("line"));
    if (!line)
      return {};
    auto *column = dyn_cast_or_null<JSONNumber>(obj->get("column"));
    if (!column)
      return {};

    SourceErrorManager::SourceCoords coords{
        bufferId, (unsigned)line->getValue(), (unsigned)column->getValue()};
    return context.getSourceErrorManager().findSMLocFromCoords(coords);
  };

  haveErrors = errors->size() != 0;

  for (auto *elem : *errors) {
    auto *error = dyn_cast<JSONObject>(elem);
    if (!error)
      continue;
    auto *loc = dyn_cast_or_null<JSONObject>(error->get("loc"));
    if (!loc)
      continue;
    SMLoc start = parseLoc(loc->get("start"));
    SMLoc end = parseLoc(loc->get("end"));

    auto *message = dyn_cast_or_null<JSONString>(error->get("message"));
    if (!message)
      continue;

    context.getSourceErrorManager().error({start, end}, message->str());
  }

  return !haveErrors;
}

} // anonymous namespace

llvh::Optional<ESTree::ProgramNode *> parseFlowParser(
    Context &context,
    uint32_t bufferId) {
  static const bool initFlowParser = (flowparser::init(), true);
  (void)initFlowParser;

  JSONFactory factory{context.getAllocator(), &context.getStringTable()};
  JSONTranslator tr{factory};

  auto *buffer = context.getSourceErrorManager().getSourceBuffer(bufferId);

  JSONValue *jsonValue = tr.parse(buffer->getBufferStart(), {});
  if (!jsonValue || tr.getErrorEncountered()) {
    context.getSourceErrorManager().error(
        SMLoc::getFromPointer(buffer->getBufferStart()), "Error parsing");
    return llvh::None;
  }

  LLVM_DEBUG(dump(llvh::dbgs(), jsonValue));

  if (!printErrors(context, bufferId, jsonValue))
    return llvh::None;

  auto parsedRes = ESTree::buildAST(context, jsonValue, buffer);
  if (!parsedRes)
    return llvh::None;

  ESTree::Node *parsed = *parsedRes;

  if (!isa<ESTree::ProgramNode>(parsed)) {
    context.getSourceErrorManager().error(
        SMLoc::getFromPointer(buffer->getBufferStart()), "Unexpected AST node");
    return llvh::None;
  }

  LLVM_DEBUG(hermes::dumpESTreeJSON(
      llvh::dbgs(), parsed, true /* pretty */, ESTreeDumpMode::HideEmpty));

  return cast<ESTree::ProgramNode>(parsed);
}

} // namespace parser
} // namespace hermes

#undef DEBUG_TYPE

#endif // HERMES_USE_FLOWPARSER
