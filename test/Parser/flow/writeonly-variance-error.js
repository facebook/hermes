/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

// writeonly on object type method is an error
type T = {
  writeonly f(): number
}
// CHECK: {{.*}}:12:3: error: Unexpected variance sigil
// CHECK-NEXT:   writeonly f(): number
// CHECK-NEXT:   ^~~~~~~~~

// writeonly on class method is an error
class C {
  writeonly f() {}
}
// CHECK: {{.*}}:20:3: error: Unexpected variance sigil
// CHECK-NEXT:   writeonly f() {}
// CHECK-NEXT:   ^~~~~~~~~
