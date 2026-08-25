/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// An empty explicit type-argument list `<>` used to abort with an assertion
// failure (an empty ArrayRef aliased the DenseMap empty/tombstone key).

class BoxNew<T> {
  v: T;
  constructor(v: T) {
    this.v = v;
  }
}
new BoxNew<>(1);

class C {
  @Hermes.final
  m<T>(x: T): void {}
}
new C().m<>(1);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}regress-generic-empty-type-args.js:19:11: error: type argument mismatch, expected 1, found 0
// CHECK-NEXT:new BoxNew<>(1);
// CHECK-NEXT:          ^~
// CHECK-NEXT:{{.*}}regress-generic-empty-type-args.js:25:1: error: type argument mismatch, expected 1, found 0
// CHECK-NEXT:new C().m<>(1);
// CHECK-NEXT:^~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
