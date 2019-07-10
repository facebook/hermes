// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hdb %s < %s.debug | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
try {
  throw 'asdf';
} catch (e) {
  print('caught');
}

// CHECK: Break on 'debugger' statement in global: {{.*}}:9:1
// CHECK-NEXT: Stepped to global: {{.*}}:11:3
// CHECK-NEXT: Stepped to global: {{.*}}:13:3
// CHECK-NEXT: caught
