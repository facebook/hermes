/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -pretty-json %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

({
  ge\u0074 m() {}
});

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}get-escaped-error.js:11:12: error: ':' expected in property initialization
// CHECK-NEXT:  ge\u0074 m() {}
// CHECK-NEXT:  ~~~~~~~~~^
// CHECK-NEXT:Emitted 1 errors. exiting.
