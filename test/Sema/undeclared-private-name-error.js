/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes %s -dump-sema -fno-std-globals 2>&1) | %FileCheck --match-full-lines %s

print(globalThis.#x);
// CHECK:{{.*}}undeclared-private-name-error.js:10:18: error: the private name "#x" was not declared in any enclosing class
// CHECK-NEXT:print(globalThis.#x);
// CHECK-NEXT:                 ^~
