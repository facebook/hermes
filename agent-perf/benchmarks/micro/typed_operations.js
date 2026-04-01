/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Operations that benefit from typed mode (--typed).
// When compiled with shermes --typed, Flow type annotations enable
// direct C operations instead of generic runtime calls.
//
// Run in two modes to compare:
//   shermes -exec typed_operations.js        # untyped
//   shermes -typed -exec typed_operations.js  # typed

"use strict";

var ITERATIONS = 1000000;

// --- Typed arithmetic (number + number) ---
function benchTypedAdd() {
  var a = 1.5;
  var b = 2.5;
  var sum = 0.0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum = sum + a + b;
    a = (sum % 100) + 0.5;
    b = (sum % 50) + 0.5;
  }
  return sum;
}

// --- Typed comparison ---
function benchTypedCompare() {
  var count = 0;
  var a = 0;
  var b = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    a = i;
    b = ITERATIONS - i;
    if (a < b) count = count + 1;
    if (a === b) count = count + 1;
  }
  return count;
}

// --- Typed array indexing ---
function benchTypedArrayIndex() {
  var arr = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0];
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var idx = i % 10;
    arr[idx] = i;
    sum = sum + arr[idx];
  }
  return sum;
}

// --- Typed property access on known shape ---
function benchTypedPropertyAccess() {
  var obj = {x: 0, y: 0};
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    obj.x = i;
    obj.y = i + 1;
    sum = sum + obj.x + obj.y;
  }
  return sum;
}

// --- Typed function call (known signature) ---
function typedAdd(a, b) {
  return a + b;
}

function benchTypedFunctionCall() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum = typedAdd(sum, 1);
  }
  return sum;
}

// --- Typed boolean operations ---
function benchTypedBoolean() {
  var count = 0;
  var flag = true;
  for (var i = 0; i < ITERATIONS; i++) {
    if (flag) count = count + 1;
    flag = !flag;
  }
  return count;
}

// --- Typed loop with narrow integer range ---
function benchTypedIntLoop() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum = (sum + i) | 0;
  }
  return sum;
}

// Run all benchmarks
function runBench(name, fn) {
  var start = Date.now();
  var result = fn();
  var elapsed = Date.now() - start;
  var opsPerSec = Math.round(ITERATIONS / (elapsed / 1000));
  print("RESULT: " + name + " " + opsPerSec + " ops/sec");
  return result;
}

runBench("typed_add", benchTypedAdd);
runBench("typed_compare", benchTypedCompare);
runBench("typed_array_index", benchTypedArrayIndex);
runBench("typed_property_access", benchTypedPropertyAccess);
runBench("typed_function_call", benchTypedFunctionCall);
runBench("typed_boolean", benchTypedBoolean);
runBench("typed_int_loop", benchTypedIntLoop);
