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

const char *getInstrumentedStatsTrace() {
  return R"###(
{
  "globalObjID": 1,
  "runtimeConfig" : {
     "enableSampledStats": true,
   },
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [
      {
         "js_heapSize": 100,
         "js_bytecodePagesAccessed": 17,
         "js_bytecodePagesTraceSample": "xyz",
      },
      {
         "js_threadCPU": 2,
         "js_hermesInvolCtxSwitches": 1000,
      },
    ],
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

const char *getInstrumentedStatsSource() {
  return R"###(
'use strict';

// If values are recorded for Hermes.getInstrumentedStats in a trace, calls
// during replay of that trace should used those values.
var stats = [HermesInternal.getInstrumentedStats(),
             HermesInternal.getInstrumentedStats()];
var expected = [{js_heapSize: 100, js_bytecodePagesTraceSample: "xyz"},
                {js_threadCPU: 2, js_hermesInvolCtxSwitches: 1000}];

for (var i = 0; i < expected.length; i++) {
  if (Object.keys(stats[i]).length < Object.keys(expected[i]).length) {
    throw new Error("No match: stats[" + i + "] has " +
                    Object.keys(stats[i]).length +
                    " props, but expected[" + i + "] has " +
                    Object.keys(expected[i]).length);
  }
  for (const prop in expected[i]) {
    if (!stats[i].hasOwnProperty(prop)) {
      throw new Error("No match: expected[" + i + "] has property " + prop +
                      ", but stats[" + i + "] does not.");
    }
    if (stats[i][prop] != expected[i][prop]) {
      throw new Error("No match: stats[" + i + "][" + prop +
                      "] = " + stats[i][prop] + ", but expected[" + i + "][" +
                      prop + "] = " + expected[i][prop]);
    }
  }
}
)###";
}

const char *getInstrumentedStatsAllowsEmptyTrace() {
  return R"###(
{
  "globalObjID": 1,
  "runtimeConfig" : {
     "enableSampledStats": true,
   },
  "env": {
    "mathRandomSeed": 0,
    "callsToNewDate": [],
    "callsToDateAsFunction": [],  // Empty.
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

const char *getInstrumentedStatsAllowsEmptySource() {
  return R"###(
'use strict';

// This just test that the call doesn't crash, given that we do have a
// recorded environment, but one without any recorded value for the call.
var stat = HermesInternal.getInstrumentedStats();

)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
