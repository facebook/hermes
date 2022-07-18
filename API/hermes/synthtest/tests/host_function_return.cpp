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

const char *hostFunctionReturnTrace() {
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
      "type": "ReturnFromNativeRecord",
      "time": 0,
      "retval": "undefined:"
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

const char *hostFunctionReturnSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function foo, that when called returns undefined.
  if (global.foo() !== undefined) {
    throw new Error();
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
