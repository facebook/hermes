/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function funky() {
  print("funky");
}

debugger;
var x = [1,2,3];
x.forEach(function(i) {
  if (i == 2) {
    funky();
  }
  return 'myresult';
});
print("finish");

//CHECK: Break on 'debugger' statement in global: {{.*}}:15:1
//CHECK-NEXT: Stepped to global: {{.*}}:16:9
//CHECK-NEXT: Stepped to global: {{.*}}:17:1
//CHECK-NEXT: Stepped to anonymous: {{.*}}:18:7
//CHECK-NEXT: Stepped to anonymous: {{.*}}:18:3
//CHECK-NEXT: Stepped to anonymous: {{.*}}:21:3
//CHECK-NEXT: Stepped to anonymous: {{.*}}:18:7
//CHECK-NEXT: Stepped to anonymous: {{.*}}:19:5
//CHECK-NEXT: Stepped to funky: {{.*}}:12:3

// step out
//CHECK-NEXT: funky
//CHECK-NEXT: Stepped to anonymous: {{.*}}:21:3
//CHECK-NEXT: Stepped to anonymous: {{.*}}:18:7
//CHECK-NEXT: Stepped to anonymous: {{.*}}:18:3
//CHECK-NEXT: Stepped to global: {{.*}}:17:10
//CHECK-NEXT: Stepped to global: {{.*}}:23:1
//CHECK-NEXT: Continuing execution
//CHECK-NEXT: finish
