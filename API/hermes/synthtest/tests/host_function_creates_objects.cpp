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

const char *hostFunctionCreatesObjectsTrace() {
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
      "propID": 22,
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
      "type": "CreateObjectRecord",
      "time": 0,
      "objID": 11
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 23,
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
      "propID": 23,
      "propName": "a",
      "value": "string:30"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 24,
      "encoding": "ASCII",
      "chars": "b"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": 24,
      "propName": "b",
      "value": "null:"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:11"
    },
    {
      "type": "CallToNativeRecord",
      "time": 0,
      "functionID": 10,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "CreateObjectRecord",
      "time": 0,
      "objID": 12
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 12,
      "propID": 23,
      "propName": "a",
      "value": "null:"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 12,
      "propID": 24,
      "propName": "b",
      "value": "string:30"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:12"
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

const char *hostFunctionCreatesObjectsSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function foo.
  // foo returns an object with two properties, a: "hello" and b: null.
  // The second time it is called, it should return an object with a: null, and
  // b: "hello".
  var o = global.foo();
  if (o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (o.b !== null) {
    throw new Error("o.b !== null");
  }
  // The second returned object should swap the properties.
  var p = global.foo();
  if (o === p) {
    throw new Error("o and p should be different objects");
  }
  if (p.a !== null) {
    throw new Error("o.a !== null");
  }
  if (p.b !== "hello") {
    throw new Error("o.b !== \"hello\"");
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
