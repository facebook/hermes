/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

// FastArray's `length` property is non-writable at runtime, and Array<T> has
// no other public fields. Without the FlowChecker diagnostic, IRGen falls
// through to emitTypedFieldStore and asserts on the failed field lookup.

(function main() {
    const a: number[] = [1, 2, 3];
    a.length = 0;
})();

//CHECK: {{.*}}array-length-assign-error.js:16:5: error: ft: cannot assign to property 'length' of typed Array
