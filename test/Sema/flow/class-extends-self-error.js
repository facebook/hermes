/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

class SelfA extends SelfA {}

class CycA extends CycB {}
class CycB extends CycA {}

class FwdB extends FwdA {}
class FwdA {}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-extends-self-error.js:10:21: error: ft: super class used before it is defined
// CHECK-NEXT:class SelfA extends SelfA {}
// CHECK-NEXT:                    ^
// CHECK-NEXT:{{.*}}class-extends-self-error.js:12:20: error: ft: super class used before it is defined
// CHECK-NEXT:class CycA extends CycB {}
// CHECK-NEXT:                   ^
// CHECK-NEXT:{{.*}}class-extends-self-error.js:15:20: error: ft: super class used before it is defined
// CHECK-NEXT:class FwdB extends FwdA {}
// CHECK-NEXT:                   ^
// CHECK-NEXT:Emitted 3 errors. exiting.
