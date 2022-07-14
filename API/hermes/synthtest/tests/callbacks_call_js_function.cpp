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

const char *callbacksCallJSFunctionTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "BeginExecJSRecord",
      "time": 0
    },
    {
      "type": "EndExecJSRecord",
      "retval": "undefined:",
      "time": 0
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 2,
      "encoding": "ASCII",
      "chars": "f"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": 2,
      "propName": "f",
      "value": "object:10"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 10,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "object:11"
    },
    {
      "type": "ArrayReadRecord",
      "time": 0,
      "objID": 11,
      "index": 0,
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
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "object:13"
    },
    {
      "type": "ArrayReadRecord",
      "time": 0,
      "objID": 13,
      "index": 0,
      "value": "bool:false"
    }
  ]
}
)###";
}

const char *callbacksCallJSFunctionSource() {
  return R"###(
'use strict';

(function(global) {
  // callbacks execute f
  // read the zeroth element of the return result,
  // execute that as a function with no args,
  // read the zeroth element of the return value and expect it to be false.
  global.f = function() {
    return [
      function() {
        return [false];
      }
    ];
  };
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
