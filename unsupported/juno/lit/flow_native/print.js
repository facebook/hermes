/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fn_dir/run_fnc.sh %fnc %s %t && %t | %FileCheck %s --match-full-lines

print("Hello world\n");
// CHECK: Hello world
print(10 + 10);
print("\n");
// CHECK-NEXT: 20.000000
print({});
print("\n");
// CHECK-NEXT: [Object]
print(function(){});
print("\n");
// CHECK-NEXT: [Closure]
