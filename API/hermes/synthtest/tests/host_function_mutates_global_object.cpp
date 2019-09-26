/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the LICENSE
 * file in the root directory of this source tree.
 */
#include "TestFunctions.h"

namespace facebook {
namespace hermes {
namespace synthtest {

const char *hostFunctionMutatesGlobalObjectTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "mathRandomSeed": 0,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": []
  },
  "trace": [
    {
      "type": "CreateHostFunctionRecord",
      "time": 0,
      "objID": 10
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propName": "foo",
      "value": "object:10"
    },
    {
      "type": "BeginExecJSRecord",
      "time": 0
    },
    {
      "type": "CallToNativeRecord",
      "time": 0,
      "functionID": 10,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propName": "o",
      "value": "object:11"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propName": "a",
      "value": "string:hello"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propName": "b",
      "value": "bool:true"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "CallToNativeRecord",
      "time": 0,
      "functionID": 10,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propName": "o",
      "value": "object:11"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propName": "a",
      "value": "string:bar"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propName": "c",
      "value": "null:"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "EndExecJSRecord",
      "retval": "undefined:",
      "time": 0
    }
  ]
}
)###";
}

const char *hostFunctionMutatesGlobalObjectSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function foo.
  // JS source code creates a global object o.
  // The first time foo is called, it sets a and b on o to be "hello" and true respectively.
  // The second time it's called, it changes a to "bar", and adds a new property c which is set to null.
  global.o = {};
  global.foo();
  if (global.o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (global.o.b !== true) {
    throw new Error("o.b !== true");
  }
  global.foo();
  if (global.o.a !== "bar") {
    throw new Error("o.a !== \"bar\"");
  }
  if (global.o.b !== true) {
    throw new Error("o.b !== true after second mutation");
  }
  if (global.o.c !== null) {
    throw new Error("o.c !== null");
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
