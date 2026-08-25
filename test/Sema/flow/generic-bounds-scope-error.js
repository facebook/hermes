/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// An alias bound resolves at the declaration site, not the instantiation site.
// The bound 'Narrow' is the top-level class; at the instantiation site it is
// shadowed by an alias to Wide. Resolving there would wrongly accept Arg (which
// extends Wide); resolving at the declaration site correctly rejects it.

class Wide {}
class Narrow extends Wide {}
class Arg extends Wide {}

type BoundAlias<T: Narrow> = T;
{
  type Narrow = Wide;
  let x: BoundAlias<Arg> = new Arg();
  let invalidRecursive: Recursive<RecursiveOther>;
}

class RecursiveBase {}
class RecursiveOther {}

type Recursive<T: Recursive<RecursiveBase>> = T;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-bounds-scope-error.js:22:10: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:  let x: BoundAlias<Arg> = new Arg();
// CHECK-NEXT:         ^~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}generic-bounds-scope-error.js:23:25: error: ft: type argument for type parameter 'T' is incompatible with its bound
// CHECK-NEXT:  let invalidRecursive: Recursive<RecursiveOther>;
// CHECK-NEXT:                        ^~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
