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

const char *globalReturnObjectTrace() {
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
      "type": "BeginExecJSRecord",
      "time": 0
    },
    {
      "type": "EndExecJSRecord",
      "retval": "object:10",
      "time": 0
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 10,
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
