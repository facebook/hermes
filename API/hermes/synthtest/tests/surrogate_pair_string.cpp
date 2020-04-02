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

const char *surrogatePairStringTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "mathRandomSeed": 0,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
    "callsToHermesInternalGetInstrumentedStats": [],
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
        "type": "ArrayWriteRecord",
        "time": 0,
        "objID": 11,
        "index": 0,
        "value": "string:\ud83d\ude2c\ud801\udc37 abc \ud83e\ude00"
    },
    {
        "type": "ReturnFromNativeRecord",
        "time": 0,
        "retval": "undefined:"
    },
    {
      "type": "EndExecJSRecord",
      "time": 0,
      "retval": "undefined:"
    }
  ]
}
)###";
}

const char *surrogatePairStringSource() {
  return R"###(
'use strict';

(function(global) {
  var expectedName = "\ud83d\ude2c\ud801\udc37 abc \ud83e\ude00";
  var a = ["bad string"];
  global.foo(a);
  if (a[0] !== expectedName) {
    throw new Error("Expected a[0]: " + expectedName + ", got: " + a[0]);
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
