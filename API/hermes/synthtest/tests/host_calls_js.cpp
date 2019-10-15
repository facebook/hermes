/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "TestFunctions.h"

namespace facebook {
namespace hermes {
namespace synthtest {

const char *hostCallsJSTrace() {
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
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propName": "f",
      "value": "object:12"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 12,
      "thisArg": "undefined:",
      "args": ["object:11"]
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "object:11"
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

const char *hostCallsJSSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a foo function on the global object.
  // foo takes an object, and calls f with it.
  // f then sets that object's x property to 5, and returns it.
  // foo returns the object it got from f.
  global.f = function(a) {
    if (a.x !== 2) {
      throw new Error();
    }
    a.x = 5;
    return a;
  };
  var a = {x: 2};
  var y = global.foo(a);
  if (a !== y || y.x !== 5) {
    throw new Error();
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
