/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
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
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "CreatePropNameIDRecord",
      "objID": 40,
      "encoding": "ASCII",
      "chars": "HostFunction1"
    },
    {
      "type": "CreateHostFunctionRecord",
      "time": 0,
      "objID": 10,
      "propNameID": 40,
      "functionName": "HostFunction1"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 22,
      "encoding": "ASCII",
      "chars": "foo"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": "propIDTag:22",
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
      "type": "CreatePropNameRecord",
      "objID": 23,
      "encoding": "ASCII",
      "chars": "o"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": "propIDTag:23",
      "propName": "o",
      "value": "object:11"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 24,
      "encoding": "ASCII",
      "chars": "a"
    },
    {
      "type": "CreateStringRecord",
      "objID": 30,
      "encoding": "ASCII",
      "chars": "hello"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:24",
      "propName": "a",
      "value": "string:30"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 25,
      "encoding": "ASCII",
      "chars": "b"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:25",
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
      "propID": "propIDTag:23",
      "propName": "o",
      "value": "object:11"
    },
    {
      "type": "CreateStringRecord",
      "objID": 31,
      "encoding": "ASCII",
      "chars": "bar"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:24",
      "propName": "a",
      "value": "string:31"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 26,
      "encoding": "ASCII",
      "chars": "c"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:26",
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
