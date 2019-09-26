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

const char *hostFunctionCreatesObjectsTrace() {
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
      "type": "CreateObjectRecord",
      "time": 0,
      "objID": 11
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
      "propName": "a",
      "value": "null:"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 12,
      "propName": "b",
      "value": "string:hello"
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
