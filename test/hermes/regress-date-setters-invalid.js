/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
"use strict";

// For an invalid (NaN) date, the setter methods snapshot [[DateValue]] before
// calling ToNumber on their arguments, then return NaN without writing the
// snapshot back. Since ToNumber can mutate [[DateValue]] via a valueOf side
// effect (here, setTime(0)), the side effect must survive: the setter must not
// clobber it by writing the stale NaN snapshot back. We verify that valueOf is
// called exactly once, the setter returns NaN, and the side effect's value (0)
// is preserved.

var methods = [
  "setMilliseconds",
  "setUTCMilliseconds",
  "setSeconds",
  "setUTCSeconds",
  "setMinutes",
  "setUTCMinutes",
  "setHours",
  "setUTCHours",
  "setDate",
  "setUTCDate",
  "setMonth",
  "setUTCMonth",
];

for (var i = 0; i < methods.length; ++i) {
  var method = methods[i];
  var dt = new Date(NaN);
  var calls = 0;
  var value = {
    valueOf: function() {
      ++calls;
      dt.setTime(0);
      return 1;
    },
  };
  var result = dt[method](value);
  print(method, calls, result, dt.getTime());
}

// CHECK-LABEL: setMilliseconds 1 NaN 0
// CHECK-NEXT: setUTCMilliseconds 1 NaN 0
// CHECK-NEXT: setSeconds 1 NaN 0
// CHECK-NEXT: setUTCSeconds 1 NaN 0
// CHECK-NEXT: setMinutes 1 NaN 0
// CHECK-NEXT: setUTCMinutes 1 NaN 0
// CHECK-NEXT: setHours 1 NaN 0
// CHECK-NEXT: setUTCHours 1 NaN 0
// CHECK-NEXT: setDate 1 NaN 0
// CHECK-NEXT: setUTCDate 1 NaN 0
// CHECK-NEXT: setMonth 1 NaN 0
// CHECK-NEXT: setUTCMonth 1 NaN 0
