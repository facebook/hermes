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

const char *hostCallsJSCallsHostTrace() {
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
      "type": "CreatePropNameIDRecord",
      "objID": 41,
      "encoding": "ASCII",
      "chars": "HostFunction2"
    },
    {
      "type": "CreateHostFunctionRecord",
      "time": 0,
      "objID": 11,
      "propNameID": 41,
      "functionName": "HostFunction2"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 23,
      "encoding": "ASCII",
      "chars": "bar"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 23,
      "propName": "bar",
      "value": "object:11"
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
      "args": ["object:12"]
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 24,
      "encoding": "ASCII",
      "chars": "f"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 24,
      "propName": "f",
      "value": "object:13"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 13,
      "thisArg": "undefined:",
      "args": ["object:12"]
    },
    {
      "type": "CallToNativeRecord",
      "time": 0,
      "functionID": 11,
      "thisArg": "undefined:",
      "args": ["object:14"]
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:14"
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "object:14"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:14"
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

const char *hostCallsJSCallsHostSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code create the function foo which takes an object.
  // foo calls f with that object as an argument.
  // f calls the native function bar with a new object which has the same
  // properties of the passed in object.
  // bar returns the object immediately.
  // That object is returned from f, which is returned from foo back to JS.
  global.f = function(a) {
    var o = {};
    o.x = a.x;
    return global.bar(o);
  };
  var a = {x: 2};
  var y = global.foo(a);
  if (a === y || y.x !== 2) {
    throw new Error();
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
