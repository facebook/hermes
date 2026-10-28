/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -Werror -typed -dump-ir %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// The FlowChecker rejects computed access on class types, except on Array<T>,
// so a subclass of Array is the only way to reach IRGen with a computed
// 'super'. IRGen has no typed lowering for it: without the error the store
// would fall through to the untyped path, which looks the property up starting
// at the receiver rather than the superclass.
class A extends Array<number> {
  set2(i: number, v: number): void {
    super[i] = v;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-super-computed-error.js:19:5: error: unsupported use of 'super'
// CHECK-NEXT:    super[i] = v;
// CHECK-NEXT:    ^~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
