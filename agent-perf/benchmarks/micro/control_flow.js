/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Control flow patterns for Static Hermes.
// Tests loops, switch statements, try/catch, and generators.

"use strict";

var ITERATIONS = 500000;

// --- While loop ---
function benchWhileLoop() {
  var sum = 0;
  var i = 0;
  while (i < ITERATIONS) {
    sum += i;
    i++;
  }
  return sum;
}

// --- For-in loop ---
function benchForIn() {
  var obj = {a: 1, b: 2, c: 3, d: 4, e: 5};
  var sum = 0;
  for (var iter = 0; iter < ITERATIONS / 10; iter++) {
    for (var key in obj) {
      sum += obj[key];
    }
  }
  return sum;
}

// --- Switch statement ---
function benchSwitch() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    switch (i % 8) {
      case 0: sum += 1; break;
      case 1: sum += 2; break;
      case 2: sum += 3; break;
      case 3: sum += 5; break;
      case 4: sum += 8; break;
      case 5: sum += 13; break;
      case 6: sum += 21; break;
      case 7: sum += 34; break;
    }
  }
  return sum;
}

// --- Try/catch (no throw) ---
function benchTryCatchNoThrow() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    try {
      sum += i;
    } catch (e) {
      sum -= 1;
    }
  }
  return sum;
}

// --- Try/catch (with occasional throw) ---
function benchTryCatchWithThrow() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    try {
      sum += i;
      if (i % 1000 === 999) {
        throw new Error("test");
      }
    } catch (e) {
      sum -= 1;
    }
  }
  return sum;
}

// --- Conditional (ternary) ---
function benchTernary() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += (i % 2 === 0) ? i : -i;
    sum += (i > ITERATIONS / 2) ? 1 : 0;
  }
  return sum;
}

// --- Logical operators as control flow ---
function benchLogicalOps() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var a = i > 100 && i;
    var b = i < 100 || i;
    sum += (a || 0) + (b || 0);
  }
  return sum;
}

// --- Nested loops ---
function benchNestedLoops() {
  var sum = 0;
  var outer = Math.round(Math.sqrt(ITERATIONS));
  for (var i = 0; i < outer; i++) {
    for (var j = 0; j < outer; j++) {
      sum += i * j;
    }
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

runBench("while_loop", benchWhileLoop);
runBench("for_in", benchForIn);
runBench("switch_stmt", benchSwitch);
runBench("try_catch_no_throw", benchTryCatchNoThrow);
runBench("try_catch_with_throw", benchTryCatchWithThrow);
runBench("ternary", benchTernary);
runBench("logical_ops", benchLogicalOps);
runBench("nested_loops", benchNestedLoops);
