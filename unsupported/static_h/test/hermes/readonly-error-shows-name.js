/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

function putnamed() {
  try {
    a.mypropname = 42;
  } catch(e) {
    print('caught named', e.name, e.message);
  }
}

var mypropname = "mypropname";
function putcomputed() {
  try {
    a[mypropname] = 42;
  } catch(e) {
    print('caught computed', e.name, e.message);
  }
}

var a = {}
Object.defineProperty(a, 'mypropname', { writable: false });
putnamed();
// CHECK: caught named TypeError Cannot assign to read-only property 'mypropname'
putcomputed();
// CHECK: caught computed TypeError Cannot assign to read-only property 'mypropname'

a = {get mypropname() { return 42; }};
putnamed();
// CHECK: caught named TypeError Cannot assign to property 'mypropname' which has only a getter
putcomputed();
// CHECK: caught computed TypeError Cannot assign to property 'mypropname' which has only a getter
