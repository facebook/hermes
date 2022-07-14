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

const char *hostGlobalObjectTrace() {
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
      "type": "CreatePropNameRecord",
      "objID": 23,
      "encoding": "ASCII",
      "chars": "baz"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 23,
      "propName": "baz",
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
      "args": []
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 24,
      "encoding": "ASCII",
      "chars": "bar"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 24,
      "propName": "bar",
      "value": "object:12"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 12,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "CallToNativeRecord",
      "time": 0,
      "functionID": 11,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 25,
      "encoding": "ASCII",
      "chars": "quux"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 25,
      "propName": "quux",
      "value": "object:13"
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 26,
      "encoding": "ASCII",
      "chars": "b"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 13,
      "propID": 26,
      "propName": "b",
      "value": "bool:true"
    },
    {
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "CreateObjectRecord",
      "time": 0,
      "objID": 15
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 27,
      "encoding": "ASCII",
      "chars": "a"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 15,
      "propID": 27,
      "propName": "a",
      "value": "null:"
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
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "object:15"
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

const char *hostGlobalObjectSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates functions foo and baz.
  // foo calls bar on the global object, which sets quux on the global object, and calls baz.
  // baz checks that quux is an object with property b: true, and then returns.
  // foo then creates an object with a property a which is set to null, and
  // caches it.
  // The second time foo is called, it returns the cached object.
  global.bar = function() {
    global.quux = {b: true};
    global.baz();
  };
  global.foo();
  if (global.foo().a !== null) {
    throw new Error();
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
