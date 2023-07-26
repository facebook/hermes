/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s

// https://tc39.es/ecma262/multipage/ordinary-and-exotic-objects-behaviours.html#sec-arraysetlength
// Ensure that toNumber() is called twice when the length is an object.

const spy = {
  [Symbol.toPrimitive](hint) {
    print(`toPrimitive ${hint}`);
    return hint === "number" ? 4 : "done";
  },
};
print(String([].length = spy));

//CHECK:      toPrimitive number
//CHECK-NEXT: toPrimitive number
//CHECK-NEXT: toPrimitive string
//CHECK-NEXT: done
