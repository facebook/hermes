// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
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

//CHECK: Break on 'debugger' statement in global: {{.*}}:13:1
//CHECK: Stepped to global: {{.*}}:14:9
//CHECK: Stepped to global: {{.*}}:15:1
//CHECK: Stepped to anonymous: {{.*}}:16:7
//CHECK: Stepped to anonymous: {{.*}}:16:3
//CHECK: Stepped to anonymous: {{.*}}:19:3
//CHECK: Stepped to anonymous: {{.*}}:15:23
//CHECK: Stepped to anonymous: {{.*}}:16:7
//CHECK: Stepped to anonymous: {{.*}}:17:5
//CHECK: Stepped to funky: {{.*}}:10:3
//CHECK: funky
//CHECK: Stepped to anonymous: {{.*}}:19:3
//CHECK: Stepped to anonymous: {{.*}}:15:23
//CHECK: Stepped to anonymous: {{.*}}:16:7
//CHECK: Stepped to global: {{.*}}:15:10
//CHECK: Stepped to global: {{.*}}:21:1
//CHECK: finish
