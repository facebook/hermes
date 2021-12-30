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

const char *dateNowTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "mathRandomSeed": 0,
    "callsToDateNow": [1, 2, 3],
    "callsToNewDate": [],
    "callsToDateAsFunction": [],
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

const char *dateNowSource() {
  return R"###(
'use strict';

// Date.now should produce the same numbers when it is replayed.
var a = [Date.now(), Date.now(), Date.now()];
var b = [1, 2, 3];

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
