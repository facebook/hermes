/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s
"use strict";

var a = 'a';
for (var i = 0; i < 28; ++i) {
  a = a + a;
}

var evil = {toString: function() { return a; }};

try {
new Function("hi", evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil);
  print("Unexpected success");
} catch (e) {
  print("caught", e.name);
}
// CHECK: caught RangeError
