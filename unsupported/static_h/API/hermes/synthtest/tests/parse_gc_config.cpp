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

const char *parseGCConfigTrace() {
  return R"###(
{
  "globalObjID": 0,
  "runtimeConfig": {
    "gcConfig": {
      "initHeapSize": 100,
      "maxHeapSize": 16777216
    }
  },
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
    }
  ]
}
)###";
}

const char *parseGCConfigSource() {
  // JS doesn't need to run, only need to parse and initialize the GC.
  return "// doesn't matter";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
