/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xjit=force -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo(x) {
  switch(x) {
    case 0xdeadbe01:  print(1);  break;
    case 0xdeadbe02:  print(2);  break;
    case 0xdeadbe03:  print(3);  break;
    case 0xdeadbe04:  print(4);  break;
    case 0xdeadbe05:  print(5);  break;
    case 0xdeadbe06:  print(6);  break;
    case 0xdeadbe07:  print(7);  break;
    case 0xdeadbe08:  print(8);  break;
    case 0xdeadbe09:  print(9);  break;
    case 0xdeadbe0a:  print(10);  break;
    case 0xdeadbe0b:  print(11);  break;
    case 0xdeadbe0c:  print(12);  break;
    case 0xdeadbe0d:  print(13);  break;
    case 0xdeadbe0e:  print(14);  break;
    case 0xdeadbe0f:  print(15);  break;
    case 0xdeadbe10:  print(16);  break;
    case 0xdeadbe11:  print(17);  break;
    case 0xdeadbe12:  print(18);  break;
    default:
      print('default');
      break;
  }
}
foo(0xdeadbe04);
// CHECK: 4
foo(0x12);
// CHECK: default
foo(0xdfffffff);
// CHECK: default
foo('a');
// CHECK-NEXT: default
foo(123.38);
// CHECK-NEXT: default
foo(4.1);
// CHECK-NEXT: default
foo(123);
// CHECK-NEXT: default
foo(0);
// CHECK-NEXT: default
