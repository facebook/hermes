/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class Base {}
class Pair<A, B> {}
// Pair needs 2 arguments.
type Alias<T extends Pair<Base>> = T;
let z: Alias<Pair<Base, Base>>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-args-mismatch-error.js:13:22: error: type argument mismatch, expected 2, found 1
// CHECK-NEXT:type Alias<T extends Pair<Base>> = T;
// CHECK-NEXT:                     ^~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
