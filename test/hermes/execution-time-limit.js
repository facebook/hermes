// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: (! %hermes -emit-async-break-check -time-limit=1000 %s 2>&1 ) | %FileCheck --match-full-lines %s

function entryPoint() {
  helper();
}

function helper() {
  var i = 0;
  while (true) {
    ++i;
  }
}

entryPoint();

//CHECK: Error: Javascript execution has timed out.
//CHECK: at helper ({{.*/execution-time-limit.js}}:15:5)
//CHECK-NEXT: at entryPoint ({{.*/execution-time-limit.js}}:9:9)
//CHECK-NEXT: at global ({{.*/execution-time-limit.js}}:19:11)
