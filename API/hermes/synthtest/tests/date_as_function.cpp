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

const char *dateAsFunctionTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "mathRandomSeed": 0,
    "callsToDateNow": [],
    "callsToNewDate": [],
    "callsToDateAsFunction": [
      "Thu Feb 28 2019 13:48:22 GMT-0800 (Pacific Standard Time)",
      "hello"
    ],
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "BeginExecJSRecord",
      "time": 0
    },
    {
      "type": "EndExecJSRecord",
      "retval": "string:42",
      "time": 0
    }
  ]
}
)###";
}

const char *dateAsFunctionSource() {
  return R"###(
'use strict';

// Date should produce the same strings when it is replayed.
var a = [Date(), Date()];
// Note that arbitrary strings can be injected via env.callsToDateAsFunction, even ones that are not valid dates.
var b = ["Thu Feb 28 2019 13:48:22 GMT-0800 (Pacific Standard Time)", "hello"];

for (var i = 0; i < a.length; i++) {
  if (a[i] != b[i]) {
    throw new Error("No match: a[" + i + "] = " + a[i] + ", b[" + i + "] = " + b[i]);
  }
}
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
