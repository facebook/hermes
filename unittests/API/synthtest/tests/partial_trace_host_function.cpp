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

const char *partialTraceHostFunctionTrace() {
  return R"###(
{
  "globalObjID": 1,
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
      "propID": "propIDTag:2",
      "propName": "f",
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "object:10"
    },
    {
      "type": "CreatePropNameIDRecord",
      "objID": 40,
      "encoding": "ASCII",
      "chars": "HostFunction1"
    },
    {
      "type": "CreateHostFunctionRecord",
      "time": 0,
      "objID": 11,
      "propNameID": 40,
      "functionName": "HostFunction1"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 10,
      "thisArg": "undefined:",
      "args": ["object:11"]
    }
  ]
}
)###";
}

const char *partialTraceHostFunctionSource() {
  return R"###(
'use strict';

(function(global) {
  // callbacks execute f
  // read the zeroth element of the return result,
  // execute that as a function with no args,
  // read the zeroth element of the return value and expect it to be false.
  global.f = function(nativeFunc) {
    nativeFunc();
  };
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
