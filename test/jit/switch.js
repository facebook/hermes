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
    case 1:  print(1);  break;
    case 2:  print(2);  break;
    case 3:  print(3);  break;
    case 4:  print(4);  break;
    case 5:  print(5);  break;
    case 6:  print(6);  break;
    case 7:  print(7);  break;
    case 8:  print(8);  break;
    case 9:  print(9);  break;
    case 10: print(10); break;
    case 11: print(11); break;
    case 12: print(12); break;
    case 13: print(13); break;
    case 14: print(14); break;
    case 15: print(15); break;
    case 16: print(16); break;
    case 17: print(17); break;
    default:
      print('default');
      break;
  }
}
foo(1);
// CHECK: 1
foo(17);
// CHECK-NEXT: 17
foo(4);
// CHECK-NEXT: 4
foo(16);
// CHECK-NEXT: 16
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
