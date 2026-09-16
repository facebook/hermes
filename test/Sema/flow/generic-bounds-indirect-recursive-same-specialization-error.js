/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class Base {}

type Alias<T> = Node<T>;

class Node<T: Alias<Base>> {}

// Resolving Alias<Base> re-enters the in-flight Node<Base> specialization.
// The bound must not silently become any and accept Base.
let bad: Node<Base>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-indirect-recursive-same-specialization-error.js:12:17: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type Alias<T> = Node<T>;
// CHECK-NEXT:                ^~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
