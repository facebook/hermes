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

const char *hostFunctionMutatesObjectTrace() {
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
      "args": ["object:11"]
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
      "value": "bool:false"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:11"
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

const char *hostFunctionMutatesObjectSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function foo.
  // It takes in an object, sets a to "hello" and b to false, and then returns it.
  // It should return the same object it takes in.
  var o = {};
  var x = global.foo(o);
  if (x !== o) {
    throw new Error("Didn't return the same object it took in");
  }
  if (o.a !== "hello") {
    throw new Error("o.a !== \"hello\"");
  }
  if (o.b !== false) {
    throw new Error("o.b !== false");
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
