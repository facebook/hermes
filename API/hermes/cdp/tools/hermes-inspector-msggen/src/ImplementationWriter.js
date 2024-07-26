/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {Writable} from 'stream';

import {GeneratedHeader} from './GeneratedHeader';
import {PropsType, Type} from './Type';
import {Command} from './Command';
import {Event} from './Event';

export class ImplementationWriter {
  stream: Writable;
  types: Array<Type>;
  commands: Array<Command>;
  events: Array<Event>;

  constructor(
    stream: Writable,
    types: Array<Type>,
    commands: Array<Command>,
    events: Array<Event>,
  ) {
    this.stream = stream;
    this.types = types;
    this.commands = commands;
    this.events = events;
  }

  write() {
    this.writePrologue();
    this.writeRequestParser();
    this.writeTypeDefs();
    this.writeRequestDefs();
    this.writeResponseDefs();
    this.writeNotificationDefs();
    this.writeEpilogue();
  }

  writePrologue() {
    this.stream.write(`${GeneratedHeader}

      #include "MessageTypes.h"

      #include "MessageTypesInlines.h"

      namespace facebook {
      namespace hermes {
      namespace cdp {
      namespace message {

    `);
  }

  writeRequestParser() {
    emitRequestParser(this.stream, this.commands);
  }

  writeTypeDefs() {
    this.stream.write('\n/// Types\n');

    for (const type of this.types) {
      if (type instanceof PropsType) {
        emitTypeDef(this.stream, type);
      }
    }
  }

  writeRequestDefs() {
    this.stream.write('\n/// Requests\n');

    emitUnknownRequestDef(this.stream);

    for (const command of this.commands) {
      emitRequestDef(this.stream, command);
    }
  }

  writeResponseDefs() {
    this.stream.write('\n/// Responses\n');

    emitErrorResponseDef(this.stream);
    emitOkResponseDef(this.stream);

    for (const command of this.commands) {
      emitResponseDef(this.stream, command);
    }
  }

  writeNotificationDefs() {
    this.stream.write('\n/// Notifications\n');

    for (const event of this.events) {
      emitNotificationDef(this.stream, event);
    }
  }

  writeEpilogue() {
    this.stream.write(`
      } // namespace message
      } // namespace cdp
      } // namespace hermes
      } // namespace facebook
    `);
  }
}

function emitRequestParser(stream: Writable, commands: Array<Command>) {
  stream.write(`
    using RequestBuilder = std::unique_ptr<Request> (*)(const JSONObject *obj);

    namespace {

    template <typename T>
    std::unique_ptr<Request> tryMake(const JSONObject *obj) {
      std::unique_ptr<T> t = T::tryMake(obj);
      if (t == nullptr) {
        return nullptr;
      }
      return t;
    }

    #define TRY_ASSIGN(lhs, obj, key) \
      if (!assign(lhs, obj, key)) {   \
        return nullptr;               \
      }
    #define TRY_ASSIGN_JSON_BLOB(lhs, obj, key) \
      if (!assignJsonBlob(lhs, obj, key)) {     \
        return nullptr;                         \
      }
    #define TRY_ASSIGN_OPTIONAL_JSON_BLOB(lhs, obj, key) \
      if (!assignOptionalJsonBlob(lhs, obj, key)) {     \
        return nullptr;                         \
      }

    bool assignJsonBlob(
        std::string &field,
        const JSONObject *obj,
        const std::string &key) {
      JSONValue *v = obj->get(key);
      if (v == nullptr) {
        return false;
      }
      field = jsonValToStr(v);
      return true;
    }

    bool assignOptionalJsonBlob(
        optional<std::string> &field,
        const JSONObject *obj,
        const std::string &key) {
      JSONValue *v = obj->get(key);
      if (v != nullptr) {
        field = jsonValToStr(v);
      } else {
        field.reset();
      }
      return true;
    }

    void putJsonBlob(
        Properties &props,
        const std::string &key,
        std::string blob,
        JSONFactory &factory) {
      JSONString *jsStr = factory.getString(key);
      std::optional<JSONValue *> jsVal = parseStr(blob, factory);

      // Expecting the conversion from string to JSONValue to succeed because
      // it was originally parsed via assignJsonBlob.
      assert(jsVal);

      props.push_back({jsStr, *jsVal});
    }

    void putOptionalJsonBlob(
        Properties &props,
        const std::string &key,
        optional<std::string> blob,
        JSONFactory &factory) {
      if (blob.has_value()) {
        putJsonBlob(props, key, *blob, factory);
      }
    }

    } // namespace

    std::unique_ptr<Request> Request::fromJson(const std::string &str) {
      static std::unordered_map<std::string, RequestBuilder> builders = {
  `);

  for (const command of commands) {
    const cppNs = command.getCppNamespace();
    const cppType = command.getRequestCppType();
    const dbgName = command.getDebuggerName();

    stream.write(`{"${dbgName}", tryMake<${cppNs}::${cppType}>},\n`);
  }

  stream.write(`};

    JSLexer::Allocator alloc;
    JSONFactory factory(alloc);
    std::optional<JSONObject *> parseResult = parseStrAsJsonObj(str, factory);
    if (!parseResult) {
      return nullptr;
    }
    JSONObject *jsonObj = *parseResult;

    std::string method;

    TRY_ASSIGN(method, jsonObj, "method");

    auto it = builders.find(method);
    if (it == builders.end()) {
      return UnknownRequest::tryMake(jsonObj);
    }

    auto builder = it->second;
    return builder(jsonObj);
  }\n\n`);

  stream.write('\n');
}

function emitPropAssign(stream: Writable, pointerName: string, prop: Array<Property>, objName: string = 'obj') {
  const id = prop.getCppIdentifier();
  const name = prop.name;
  const type = prop.getFullCppType();
  const assignMethod = type == 'std::optional<JSONBlob>' ?
    'TRY_ASSIGN_OPTIONAL_JSON_BLOB' :
    (type == 'JSONBlob' ?
      'TRY_ASSIGN_JSON_BLOB' :
      'TRY_ASSIGN');
  stream.write(`${assignMethod}(${pointerName}->${id}, ${objName}, "${name}");
  `);
}

function emitPropPut(stream: Writable, prop: Array<Property>, propsName: string = 'props'){
  const id = prop.getCppIdentifier();
  const name = prop.name;
  const type = prop.getFullCppType();
  const putMethod = type == 'std::optional<JSONBlob>' ?
    'putOptionalJsonBlob' :
    (type == 'JSONBlob' ?
      'putJsonBlob' :
      'put');
  stream.write(`${putMethod}(${propsName}, "${name}", ${id}, factory);\n`);
}

export function emitTypeDef(stream: Writable, type: PropsType) {
  const cppNs = type.getCppNamespace();
  const cppType = type.getCppType();
  const props = type.properties || [];

  // From-dynamic constructor
  stream.write(`std::unique_ptr<${cppNs}::${cppType}> ${cppNs}::${cppType}::tryMake(const JSONObject *obj) {
    std::unique_ptr<${cppNs}::${cppType}> type = std::make_unique<${cppNs}::${cppType}>();
  `);

  for (const prop of props) {
    emitPropAssign(stream, "type", prop);
  }

  stream.write('  return type;\n}\n\n');

  // toJsonVal()
  stream.write(`JSONValue *${cppNs}::${cppType}::toJsonVal(JSONFactory &factory) const {
    llvh::SmallVector<JSONFactory::Prop, ${props.length}> props;\n\n`);

  for (const prop of props) {
    emitPropPut(stream, prop);
  }

  stream.write('return factory.newObject(props.begin(), props.end());\n}\n\n');
}

function emitErrorResponseDef(stream: Writable) {
  stream.write(`std::unique_ptr<ErrorResponse> ErrorResponse::tryMake(const JSONObject *obj) {
    std::unique_ptr<ErrorResponse> resp = std::make_unique<ErrorResponse>();
    TRY_ASSIGN(resp->id, obj, "id");

    JSONValue *v = obj->get("error");
    if (v == nullptr) {
      return nullptr;
    }
    auto convertResult = valueFromJson<JSONObject*>(v);
    if (!convertResult) {
      return nullptr;
    }
    auto *error = *convertResult;

    TRY_ASSIGN(resp->code, error, "code");
    TRY_ASSIGN(resp->message, error, "message");
    TRY_ASSIGN_OPTIONAL_JSON_BLOB(resp->data, error, "data");

    return resp;
  }

  JSONValue *ErrorResponse::toJsonVal(JSONFactory &factory) const {
    llvh::SmallVector<JSONFactory::Prop, 3> errProps;
    put(errProps, "code", code, factory);
    put(errProps, "message", message, factory);
    putOptionalJsonBlob(errProps, "data", data, factory);

    llvh::SmallVector<JSONFactory::Prop, 2> props;
    put(props, "id", id, factory);
    put(props, "error", factory.newObject(errProps.begin(), errProps.end()), factory);
    return factory.newObject(props.begin(), props.end());
  }\n\n`);
}

function emitOkResponseDef(stream: Writable) {
  stream.write(`std::unique_ptr<OkResponse> OkResponse::tryMake(const JSONObject *obj) {
    std::unique_ptr<OkResponse> resp = std::make_unique<OkResponse>();
    TRY_ASSIGN(resp->id, obj, "id");
    return resp;
  }

  JSONValue *OkResponse::toJsonVal(JSONFactory &factory) const {
    llvh::SmallVector<JSONFactory::Prop, 0> resProps;

    llvh::SmallVector<JSONFactory::Prop, 2> props;
    put(props, "id", id, factory);
    put(props, "result", factory.newObject(resProps.begin(), resProps.end()), factory);
    return factory.newObject(props.begin(), props.end());
  }\n\n`);
}

function emitUnknownRequestDef(stream: Writable) {
  stream.write(`UnknownRequest::UnknownRequest() {}

std::unique_ptr<UnknownRequest> UnknownRequest::tryMake(const JSONObject *obj) {
  std::unique_ptr<UnknownRequest> req = std::make_unique<UnknownRequest>();
  TRY_ASSIGN(req->id, obj, "id");
  TRY_ASSIGN(req->method, obj, "method");
  TRY_ASSIGN_OPTIONAL_JSON_BLOB(req->params, obj, "params");
  return req;
}

JSONValue *UnknownRequest::toJsonVal(JSONFactory &factory) const {
  llvh::SmallVector<JSONFactory::Prop, 3> props;
  put(props, "id", id, factory);
  put(props, "method", method, factory);
  putOptionalJsonBlob(props, "params", params, factory);
  return factory.newObject(props.begin(), props.end());
}

void UnknownRequest::accept(RequestHandler &handler) const {
  handler.handle(*this);
}\n\n`);
}

export function emitRequestDef(stream: Writable, command: Command) {
  const cppNs = command.getCppNamespace();
  const cppType = command.getRequestCppType();
  const dbgName = command.getDebuggerName();
  const props = command.parameters || [];

  // Default constructor
  stream.write(`${cppNs}::${cppType}::${cppType}()
      : Request("${dbgName}") {}\n\n`);

  // From-dynamic constructor
  stream.write(`std::unique_ptr<${cppNs}::${cppType}> ${cppNs}::${cppType}::tryMake(const JSONObject *obj) {
    std::unique_ptr<${cppNs}::${cppType}> req = std::make_unique<${cppNs}::${cppType}>();
    TRY_ASSIGN(req->id, obj, "id");
    TRY_ASSIGN(req->method, obj, "method");\n\n`);

  if (props.length > 0) {
    const optionalParams = props.every(p => p.optional);
    if (optionalParams) {
      stream.write(`
        JSONValue *p = obj->get("params");
        if (p != nullptr) {
          auto convertResult = valueFromJson<JSONObject*>(p);
          if (!convertResult) {
            return nullptr;
          }
          auto *params = *convertResult;
      `);
    } else {
      stream.write(`JSONValue *v = obj->get("params");
      if (v == nullptr) {
        return nullptr;
      }
      auto convertResult = valueFromJson<JSONObject*>(v);
      if (!convertResult) {
        return nullptr;
      }
      auto *params = *convertResult;
      `);
    }

    for (const prop of props) {
      emitPropAssign(stream, "req", prop, "params");
    }

    if (optionalParams) {
      stream.write('}');
    }
  }

  stream.write('  return req;\n}\n\n');

  // toJsonVal
  stream.write(`JSONValue *${cppNs}::${cppType}::toJsonVal(JSONFactory &factory) const {\n`);

  if (props.length > 0) {
    stream.write(
      `llvh::SmallVector<JSONFactory::Prop, ${props.length}> paramsProps;\n`,
    );

    for (const prop of props) {
      emitPropPut(stream, prop, "paramsProps");
    }
  }

  stream.write(`
    llvh::SmallVector<JSONFactory::Prop, ${
      2 + props.length > 0 ? 1 : 0
    }> props;
    put(props, "id", id, factory);
    put(props, "method", method, factory);
  `);

  if (props.length > 0) {
    stream.write('put(props, "params", factory.newObject(paramsProps.begin(), paramsProps.end()), factory);\n');
  }

  stream.write(`return factory.newObject(props.begin(), props.end());
    }\n\n`);

  // visitor
  stream.write(`void ${cppNs}::${cppType}::accept(RequestHandler &handler) const {
    handler.handle(*this);
  }\n\n`);
}

export function emitResponseDef(stream: Writable, command: Command) {
  const cppNs = command.getCppNamespace();
  const cppType = command.getResponseCppType();
  if (!cppType) {
    return;
  }

  // From-dynamic constructor
  stream.write(`std::unique_ptr<${cppNs}::${cppType}> ${cppNs}::${cppType}::tryMake(const JSONObject *obj) {
    std::unique_ptr<${cppNs}::${cppType}> resp = std::make_unique<${cppNs}::${cppType}>();
    TRY_ASSIGN(resp->id, obj, "id");\n\n`);

  const props = command.returns || [];
  if (props.length > 0) {
    stream.write(`JSONValue *v = obj->get("result");
    if (v == nullptr) {
      return nullptr;
    }
    auto convertResult = valueFromJson<JSONObject*>(v);
    if (!convertResult) {
      return nullptr;
    }
    auto *res = *convertResult;
    `);

    for (const prop of props) {
      emitPropAssign(stream, "resp", prop, "res");
    }
  }

  stream.write('  return resp;\n}\n\n');

  // toJsonVal
  stream.write(`JSONValue *${cppNs}::${cppType}::toJsonVal(JSONFactory &factory) const {\n`);

  if (props.length > 0) {
    stream.write(
      `llvh::SmallVector<JSONFactory::Prop, ${props.length}> resProps;\n`,
    );

    for (const prop of props) {
      emitPropPut(stream, prop, "resProps");
    }
  }

  stream.write(`
    llvh::SmallVector<JSONFactory::Prop, 2> props;
    put(props, "id", id, factory);
    put(props, "result", factory.newObject(resProps.begin(), resProps.end()), factory);
    return factory.newObject(props.begin(), props.end());
  }\n\n`);
}

export function emitNotificationDef(stream: Writable, event: Event) {
  const cppNs = event.getCppNamespace();
  const cppType = event.getCppType();
  const dbgName = event.getDebuggerName();
  const props = event.parameters || [];

  // Default constructor
  stream.write(`${cppNs}::${cppType}::${cppType}()
      : Notification("${dbgName}") {}\n\n`);

  // From-dynamic constructor
  stream.write(`std::unique_ptr<${cppNs}::${cppType}> ${cppNs}::${cppType}::tryMake(const JSONObject *obj) {
    std::unique_ptr<${cppNs}::${cppType}> notif = std::make_unique<${cppNs}::${cppType}>();
    TRY_ASSIGN(notif->method, obj, "method");\n\n`);

  if (props.length > 0) {
    stream.write(`JSONValue *v = obj->get("params");
    if (v == nullptr) {
      return nullptr;
    }
    auto convertResult = valueFromJson<JSONObject*>(v);
    if (!convertResult) {
      return nullptr;
    }
    auto *params = *convertResult;
    `);

    for (const prop of props) {
      emitPropAssign(stream, "notif", prop, "params");
    }
  }

  stream.write('  return notif;\n}\n\n');

  // toJsonVal
  stream.write(`JSONValue *${cppNs}::${cppType}::toJsonVal(JSONFactory &factory) const {\n`);

  if (props.length > 0) {
    stream.write(
      `llvh::SmallVector<JSONFactory::Prop, ${props.length}> paramsProps;\n`,
    );

    for (const prop of props) {
      emitPropPut(stream, prop, "paramsProps");
    }
  }

  stream.write(`
    llvh::SmallVector<JSONFactory::Prop, ${
      1 + props.length > 0 ? 1 : 0
    }> props;
    put(props, "method", method, factory);
  `);

  if (props.length > 0) {
    stream.write('put(props, "params", factory.newObject(paramsProps.begin(), paramsProps.end()), factory);\n');
  }

  stream.write(`return factory.newObject(props.begin(), props.end());
    }\n\n`);
}
