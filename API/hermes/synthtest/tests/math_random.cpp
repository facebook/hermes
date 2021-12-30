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

const char *mathRandomTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "mathRandomSeed": 10,
    "callsToDateNow": [],
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

const char *mathRandomSource() {
  return R"###(
'use strict';

// Math random should produce the same numbers when it is replayed.
var a = [Math.random(), Math.random(), Math.random()];
// These were the numbers that came out for the seed 10 for the first three
// calls to Math.random. This test ensures that the seed is set when replaying
// and mocking out Math.random.
var b = [0.8503244913652883, 0.9161127707202693, 0.8968977185307782];

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
