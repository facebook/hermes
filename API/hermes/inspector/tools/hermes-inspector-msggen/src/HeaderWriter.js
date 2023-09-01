/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

import {Writable} from 'stream';

import {GeneratedHeader} from './GeneratedHeader';
import {Property} from './Property';
import {PropsType, Type} from './Type';
import {Command} from './Command';
import {Event} from './Event';
import {toCppNamespace} from './Converters';

export class HeaderWriter {
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
    this.writeForwardDecls();
    this.writeRequestHandlerDecls();
    this.writeTypeDecls();
    this.writeRequestDecls();
    this.writeResponseDecls();
    this.writeNotificationDecls();
    this.writeEpilogue();
  }

  writePrologue() {
    this.stream.write(`${GeneratedHeader}

      #pragma once

      #include <hermes/inspector-modern/chrome/JSONValueInterfaces.h>
      #include <hermes/inspector-modern/chrome/MessageInterfaces.h>

      #include <optional>
      #include <vector>

      namespace facebook {
      namespace hermes {
      namespace inspector {
      namespace chrome {
      namespace message {

template<typename T>
void deleter(T* p);
using JSONBlob = std::string;
    `);
  }

  writeForwardDecls() {
    this.stream.write('struct UnknownRequest;\n\n');

    const namespaceMap: Map<string, Array<Type | Command | Event>> = new Map();
    const addToMap = function (type: Type | Command | Event) {
      const domain = type.domain;
      let types = namespaceMap.get(domain);
      if (!types) {
        types = [];
        namespaceMap.set(domain, types);
      }
      types.push(type);
    };

    this.types.forEach(addToMap);
    this.commands.forEach(addToMap);
    this.events.forEach(addToMap);

    for (const [domain, types] of namespaceMap) {
      types.sort((a, b) => {
        const nameA = a.getForwardDeclSortKey();
        const nameB = b.getForwardDeclSortKey();
        return nameA < nameB ? -1 : nameA > nameB ? 1 : 0;
      });

      const ns = toCppNamespace(domain);
      this.stream.write(`namespace ${ns} {\n`);

      for (const type of types) {
        for (const decl of type.getForwardDecls()) {
          this.stream.write(`${decl}\n`);
        }
      }

      this.stream.write(`} // namespace ${ns}\n\n`);
    }
  }

  writeRequestHandlerDecls() {
    this.stream.write(
      '\n/// RequestHandler handles requests via the visitor pattern.\n',
    );
    emitRequestHandlerDecl(this.stream, this.commands);

    this.stream.write(
      '\n/// NoopRequestHandler can be subclassed to only handle some requests.\n',
    );
    emitNoopRequestHandlerDecl(this.stream, this.commands);
  }

  writeTypeDecls() {
    this.stream.write('\n/// Types\n');

    for (const type of this.types) {
      if (type instanceof PropsType) {
        emitTypeDecl(this.stream, type);
      }
    }
  }

  writeRequestDecls() {
    this.stream.write('\n/// Requests\n');

    emitUnknownRequestDecl(this.stream);

    for (const command of this.commands) {
      emitRequestDecl(this.stream, command);
    }
  }

  writeResponseDecls() {
    this.stream.write('\n/// Responses\n');

    emitErrorResponseDecl(this.stream);
    emitOkResponseDecl(this.stream);

    for (const command of this.commands) {
      emitResponseDecl(this.stream, command);
    }
  }

  writeNotificationDecls() {
    this.stream.write('\n/// Notifications\n');

    for (const event of this.events) {
      emitNotificationDecl(this.stream, event);
    }
  }

  writeEpilogue() {
    this.stream.write(`
        } // namespace message
        } // namespace chrome
        } // namespace inspector
        } // namespace hermes
        } // namespace facebook
    `);
  }
}

function emitRequestHandlerDecl(stream: Writable, commands: Array<Command>) {
  stream.write(`struct RequestHandler {
    virtual ~RequestHandler() = default;

    virtual void handle(const UnknownRequest &req) = 0;
  `);

  for (const command of commands) {
    const cppNs = command.getCppNamespace();
    const cppType = command.getRequestCppType();

    stream.write(`virtual void handle(const ${cppNs}::${cppType} &req) = 0;`);
  }

  stream.write('};\n');
}

function emitNoopRequestHandlerDecl(
  stream: Writable,
  commands: Array<Command>,
) {
  stream.write(`struct NoopRequestHandler : public RequestHandler {
    void handle(const UnknownRequest &req) override {}
  `);

  for (const command of commands) {
    const cppNs = command.getCppNamespace();
    const cppType = command.getRequestCppType();

    stream.write(`void handle(const ${cppNs}::${cppType} &req) override {}`);
  }

  stream.write('};\n');
}

function emitProps(stream: Writable, props: ?Array<Property>) {
  if (!props || props.length === 0) {
    return;
  }

  stream.write('\n');

  for (const prop of props) {
    const fullCppType = prop.getFullCppType();
    const cppId = prop.getCppIdentifier();
    const init = prop.getInitializer();

    stream.write(`  ${fullCppType} ${cppId}${init};\n`);
  }
}

export function emitTypeDecl(stream: Writable, type: PropsType) {
  const cppNs = type.getCppNamespace();
  const cppType = type.getCppType();

  stream.write(`struct ${cppNs}::${cppType} : public Serializable {
    ${cppType}() = default;
    ${cppType}(${cppType}&&) = default;
    ${cppType}(const ${cppType}&) = delete;
    static std::unique_ptr<${cppType}> tryMake(const JSONObject *obj);
    JSONValue *toJsonVal(JSONFactory &factory) const override;
    ${cppType}& operator=(const ${cppType}&) = delete;
    ${cppType}& operator=(${cppType}&&) = default;
  `);

  if (type instanceof PropsType) {
    emitProps(stream, type.properties);
  }

  stream.write('};\n\n');
}

function emitUnknownRequestDecl(stream: Writable) {
  stream.write(`struct UnknownRequest : public Request {
    UnknownRequest();
    static std::unique_ptr<UnknownRequest> tryMake(const JSONObject *obj);

    JSONValue *toJsonVal(JSONFactory &factory) const override;
    void accept(RequestHandler &handler) const override;

    std::optional<JSONBlob> params;
  };

  `);
}

export function emitRequestDecl(stream: Writable, command: Command) {
  const cppNs = command.getCppNamespace();
  const cppType = command.getRequestCppType();

  stream.write(`struct ${cppNs}::${cppType} : public Request {
    ${cppType}();
    static std::unique_ptr<${cppType}> tryMake(const JSONObject *obj);

    JSONValue *toJsonVal(JSONFactory &factory) const override;
    void accept(RequestHandler &handler) const override;
  `);

  emitProps(stream, command.parameters);

  stream.write('};\n\n');
}

function emitErrorResponseDecl(stream: Writable) {
  stream.write(`struct ErrorResponse : public Response {
    ErrorResponse() = default;
    static std::unique_ptr<ErrorResponse> tryMake(const JSONObject *obj);
    JSONValue *toJsonVal(JSONFactory &factory) const override;

    long long code;
    std::string message;
    std::optional<JSONBlob> data;
  };

  `);
}

function emitOkResponseDecl(stream: Writable) {
  stream.write(`struct OkResponse : public Response {
    OkResponse() = default;
    static std::unique_ptr<OkResponse> tryMake(const JSONObject *obj);
    JSONValue *toJsonVal(JSONFactory &factory) const override;
  };

  `);
}

export function emitResponseDecl(stream: Writable, command: Command) {
  const cppNs = command.getCppNamespace();
  const cppType = command.getResponseCppType();
  if (!cppType) {
    return;
  }

  stream.write(`struct ${cppNs}::${cppType} : public Response {
    ${cppType}() = default;
    static std::unique_ptr<${cppType}> tryMake(const JSONObject *obj);
    JSONValue *toJsonVal(JSONFactory &factory) const override;
  `);

  emitProps(stream, command.returns);

  stream.write('};\n\n');
}

export function emitNotificationDecl(stream: Writable, event: Event) {
  const cppNs = event.getCppNamespace();
  const cppType = event.getCppType();

  stream.write(`struct ${cppNs}::${cppType} : public Notification {
    ${cppType}();
    static std::unique_ptr<${cppType}> tryMake(const JSONObject *obj);
    JSONValue *toJsonVal(JSONFactory &factory) const override;
  `);

  emitProps(stream, event.parameters);

  stream.write('};\n\n');
}
