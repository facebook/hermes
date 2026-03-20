/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Arithmetic operations for Static Hermes.
// Tests numeric operations, type coercion, and comparison operators.

"use strict";

var ITERATIONS = 1000000;

// --- Integer arithmetic ---
function benchIntArithmetic() {
  var a = 0, b = 1;
  for (var i = 0; i < ITERATIONS; i++) {
    var t = a + b;
    a = b;
    b = t & 0x7fffffff;
  }
  return b;
}

// --- Float arithmetic ---
function benchFloatArithmetic() {
  var x = 1.5;
  for (var i = 0; i < ITERATIONS; i++) {
    x = x * 1.0000001 + 0.0000001;
    x = x / 1.0000001 - 0.0000001;
  }
  return x;
}

// --- Bitwise operations ---
function benchBitwise() {
  var val = 0xDEADBEEF;
  for (var i = 0; i < ITERATIONS; i++) {
    val = (val << 1) | (val >>> 31);
    val = val ^ (i & 0xff);
    val = val & 0x7fffffff;
  }
  return val;
}

// --- Comparison operators ---
function benchComparisons() {
  var count = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    if (i > 100) count++;
    if (i < ITERATIONS - 100) count++;
    if (i >= 50) count++;
    if (i <= ITERATIONS - 50) count++;
    if (i === 500000) count++;
  }
  return count;
}

// --- Math operations ---
function benchMathOps() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += Math.abs(i - ITERATIONS / 2);
    sum += Math.min(i, ITERATIONS - i);
    sum += Math.max(i & 0xff, 128);
  }
  return sum;
}

// --- Type coercion arithmetic ---
function benchTypeCoercion() {
  var sum = 0;
  var str = "42";
  var bool = true;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += +str;
    sum += +bool;
    sum += ~~(i / 3);
  }
  return sum;
}

// --- Modulo and division ---
function benchModDiv() {
  var sum = 0;
  for (var i = 1; i < ITERATIONS; i++) {
    sum += i % 7;
    sum += (i / 3) | 0;
    sum += i % 13;
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

runBench("int_arithmetic", benchIntArithmetic);
runBench("float_arithmetic", benchFloatArithmetic);
runBench("bitwise_ops", benchBitwise);
runBench("comparisons", benchComparisons);
runBench("math_ops", benchMathOps);
runBench("type_coercion", benchTypeCoercion);
runBench("mod_div", benchModDiv);
