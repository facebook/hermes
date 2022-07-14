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

const char *globalReturnObjectTrace() {
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
      "retval": "object:10",
      "time": 0
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 2,
      "encoding": "ASCII",
      "chars": "a"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 10,
      "propID": 2,
      "propName": "a",
      "value": "object:11"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 11,
      "thisArg": "undefined:",
      "args": []
    }
  ]
}
)###";
}

const char *globalReturnObjectSource() {
  return "({a:function() {}})";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
