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

const char *hostFunctionCachesObjectTrace() {
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
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:23",
      "propName": "a",
      "value": "undefined:"
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
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 11,
      "propID": "propIDTag:23",
      "propName": "a",
      "value": "bool:true"
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

const char *hostFunctionCachesObjectSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function foo, which returns an object with one
  // property, a.
  // foo is called a second time, and is expected to return the same object, and
  // also set its a property to be true.
  var o = global.foo();
  if (!("a" in o) || o.a !== undefined) {
    throw new Error("o.a !== undefined");
  }
  var p = global.foo();
  if (o !== p) {
    throw new Error("Didn't cache the object");
  }
  if (o.a !== true || p.a !== true) {
    throw new Error("o.a !== true");
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
