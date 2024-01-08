/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --check-prefix=CHKIR  --match-full-lines %s
// RUN: %hermes -O -dump-bytecode %s | %FileCheckOrRegen --check-prefix=CHKBC  --match-full-lines %s

// Strick equality check on int32 and int32
function test_int_int(x, y) {
  x = x | 0;
  y = y | 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on int32 and uint32
function test_int_uint(x, y) {
  x = x | 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on uint32 and uint32
function test_uint_uint(x, y) {
  x = x >>> 0;
  y = y >>> 0;
  if (x === y) {
    return x;
  } else {
    return undefined;
  }
}

// Strick equality check on values that could be int
function test_could_be_int(func) {
  var x = func() * 100;
  var a = func() ? (x | 0) : undefined;
  var b = x >>> 0;
  if (a === b) {
    return x;
  } else {
    return undefined;
  }
}
