/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -fno-static-builtins %s | %FileCheck --match-full-lines %s --check-prefix=CHECK-NO-EXCEPTION

// REQUIRES: debug_options

// This test checks that the 'use static builtin' directive instructs the
// compiler to perform static builtin optimization, and the VM to freeze
// the builtins. Even when the directive is declared in a function, it
// should work for the whole program.
// The directive precedes the compiler flag '-fno-static-builtins'.
// It should also work in lazy compilation mode.
try {
  Array.isArray = 1;
  print('no exception');
} catch (e) {
  print(e.toString());
}
// CHECK: TypeError: Attempting to override read-only builtin method 'Array.isArray'
// CHECK-NO-EXCEPTION: no exception

function func() {
    'use static builtin';
    return;
}
