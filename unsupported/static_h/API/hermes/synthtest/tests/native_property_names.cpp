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

const char *nativePropertyNamesTrace() {
  return R"###(
{
  "globalObjID": 1,
  "env": {
    "callsToHermesInternalGetInstrumentedStats": [],
  },
  "trace": [
    {
      "type": "CreateHostObjectRecord",
      "time": 0,
      "objID": 10
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 22,
      "encoding": "ASCII",
      "chars": "hostObj1"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": "propIDTag:22",
      "propName": "hostObj1",
      "value": "object:10"
    },
    {
      "type": "CreateHostObjectRecord",
      "time": 0,
      "objID": 11
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 23,
      "encoding": "ASCII",
      "chars": "hostObj2"
    },
    {
      "type": "SetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": "propIDTag:23",
      "propName": "hostObj2",
      "value": "object:11"
    },
    {
      "type": "BeginExecJSRecord",
      "time": 0
    },
    {
      "type": "GetNativePropertyNamesRecord",
      "time": 0,
      "hostObjectID": 10
    },
    {
      "type": "CreatePropNameRecord",
      "objID": 24,
      "encoding": "ASCII",
      "chars": "jsFunc"
    },
    {
      "type": "GetPropertyRecord",
      "time": 0,
      "objID": 1,
      "propID": "propIDTag:24",
      "propName": "jsFunc",
      "value": "object:12"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 12,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "GetNativePropertyNamesRecord",
      "time": 0,
      "hostObjectID": 11
    },
    {
      "type": "GetNativePropertyNamesReturnRecord",
      "time": 0,
      "properties": [
        "baz",
        "quux"
      ],
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "CallFromNativeRecord",
      "time": 0,
      "functionID": 12,
      "thisArg": "undefined:",
      "args": []
    },
    {
      "type": "GetNativePropertyNamesRecord",
      "time": 0,
      "hostObjectID": 11
    },
    {
      "type": "GetNativePropertyNamesReturnRecord",
      "time": 0,
      "properties": [
        "baz",
        "quux"
      ],
    },
    {
      "type": "ReturnToNativeRecord",
      "time": 0,
      "retval": "undefined:"
    },
    {
      "type": "GetNativePropertyNamesReturnRecord",
      "time": 0,
      "properties": [
        "foo",
        "bar"
      ],
    }
  ]
}
)###";
}

const char *nativePropertyNamesSource() {
  return R"###(
'use strict';

(function(global) {
  var expectedHostObj1Props = ["foo", "bar"];
  var expectedHostObj2Props = ["baz", "quux", "baz", "quux"];
  var hostObj1Props = [];
  var hostObj2Props = [];
  var numTimesCalled = 0;
  global.jsFunc = function jsFunc() {
    for (var prop in global.hostObj2) {
      hostObj2Props.push(prop);
    }
    numTimesCalled++;
  };
  for (var prop in global.hostObj1) {
    hostObj1Props.push(prop);
  }

  for (var i = 0; i < expectedHostObj1Props.length; i++) {
    var expected = expectedHostObj1Props[i];
    var actual = hostObj1Props[i];
    if (expected !== actual) {
      throw new Error('Expected hostObj1Props[' + i + '] to be "' + expected + '", but got "' + actual + '"');
    }
  }

  for (var i = 0; i < expectedHostObj2Props.length; i++) {
    var expected = expectedHostObj2Props[i];
    var actual = hostObj2Props[i];
    if (expected !== actual) {
      throw new Error('Expected hostObj2Props[' + i + '] to be "' + expected + '", but got "' + actual + '"');
    }
  }

  if (numTimesCalled !== 2) {
    throw new Error('Expected numTimesCalled to be 2, but got ' + numTimesCalled);
  }
})(this);
)###";
}

} // namespace synthtest
} // namespace hermes
} // namespace facebook
