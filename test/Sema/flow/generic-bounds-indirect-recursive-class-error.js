/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class Base {}
class Other {}

type Alias<T> = Node<T>;

class Node<T: Alias<Base>> {}

let bad: Node<Other>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-indirect-recursive-class-error.js:13:17: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:type Alias<T> = Node<T>;
// CHECK-NEXT:                ^~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-indirect-recursive-class-error.js:17:14: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:let bad: Node<Other>;
// CHECK-NEXT:             ^~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
