/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

var C = class {
  static * #constructor() {}
};

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-constructor-private-error.js:11:10: error: constructor method must not be private
// CHECK-NEXT:  static * #constructor() {}
// CHECK-NEXT:         ^~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 1 errors. exiting.
