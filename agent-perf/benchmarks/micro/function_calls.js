/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Function call patterns for Static Hermes.
// Tests direct calls, closures, bound functions, and method dispatch.

"use strict";

var ITERATIONS = 500000;

// --- Direct function call ---
function add(a, b) {
  return a + b;
}

function benchDirectCall() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum = add(sum, 1);
  }
  return sum;
}

// --- Method call ---
var obj = {
  value: 0,
  increment: function(n) {
    this.value += n;
    return this.value;
  }
};

function benchMethodCall() {
  obj.value = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    obj.increment(1);
  }
  return obj.value;
}

// --- Closure call ---
function makeAdder(base) {
  return function(n) {
    return base + n;
  };
}

function benchClosureCall() {
  var adder = makeAdder(100);
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum = adder(sum & 0xff);
  }
  return sum;
}

// --- Bound function call ---
function multiply(a, b) {
  return a * b;
}

function benchBoundCall() {
  var double = multiply.bind(null, 2);
  var val = 1;
  for (var i = 0; i < ITERATIONS; i++) {
    val = double(val & 0xff);
  }
  return val;
}

// --- Recursive call ---
function fib(n) {
  if (n <= 1) return n;
  return fib(n - 1) + fib(n - 2);
}

function benchRecursiveCall() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 100; i++) {
    sum += fib(20);
  }
  return sum;
}

// --- Apply/call ---
function sum3(a, b, c) {
  return a + b + c;
}

function benchApplyCall() {
  var total = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    total += sum3.call(null, 1, 2, 3);
    total += sum3.apply(null, [4, 5, 6]);
  }
  return total;
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

runBench("direct_call", benchDirectCall);
runBench("method_call", benchMethodCall);
runBench("closure_call", benchClosureCall);
runBench("bound_call", benchBoundCall);
runBench("recursive_call", benchRecursiveCall);
runBench("apply_call", benchApplyCall);
