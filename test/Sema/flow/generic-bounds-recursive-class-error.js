/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// A generic class whose bound is a closed instantiation of itself is circular:
// computing the bound requires the specialization, which itself requires
// checking the bound. It must be reported, not silently accepted (which would
// let any argument satisfy the bound).

class Base {}
class Node<T extends Node<Base>> extends Base {}

// Can't reference the class name in the generic bounds.
let bad: Node<Base>;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-recursive-class-error.js:16:22: error: ft: type parameter bound cannot reference the generic being declared
// CHECK-NEXT:class Node<T extends Node<Base>> extends Base {}
// CHECK-NEXT:                     ^~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
