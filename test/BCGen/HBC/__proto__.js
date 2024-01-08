/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// Code generation for static proto
function staticProto() {
  return {__proto__: null, a: 2, b: 3, c: 4};
}

function dynamicProto(func, getProto) {
  return {a: func(), b: 10, __proto__: getProto()};
}
