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

const char *hostFunctionNameAndParamsTrace() {
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
      "chars": "foo"
    },
    {
      "type": "CreateHostFunctionRecord",
      "time": 0,
      "objID": 10,
      "propNameID": 40,
      "functionName": "foo",
      "parameterCount": 2
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
      "type": "EndExecJSRecord",
      "retval": "undefined:",
      "time": 0
    }
  ]
}
)###";
}

const char *hostFunctionNameAndParamsSource() {
  return R"###(
'use strict';

(function(global) {
  // Native code creates a function "foo" and attaches it to the property "foo".
  // Check that its name is "foo" and that it takes 2 arguments.
  if (global.foo.name !== "foo") {
    throw new Error('global.foo.name (which is "' + global.foo.name + '") !== "foo"');
  }
  if (global.foo.length !== 2) {
    throw new Error('global.foo.length (which is ' + global.foo.length + ') !== 2');
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
