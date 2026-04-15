/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Benchmark for float16 conversion performance.
// Run with old (hand-rolled) vs new (half.hpp) builds to compare.

(function() {
  var log = typeof print === "undefined" ? console.log : print;

  // --- Part 1: Math.f16round ---

  var ITERATIONS = 5000000;
  var f16round = Math.f16round;

  log("=== Part 1: Math.f16round ===");
  log("Iterations: " + ITERATIONS + " x 31 values");

  var start = Date.now();
  for (var i = 0; i < ITERATIONS; i++) {
    f16round(0);
    f16round(-0);
    f16round(NaN);
    f16round(Infinity);
    f16round(-Infinity);
    f16round(1);
    f16round(-1);
    f16round(2);
    f16round(-2);
    f16round(10);
    f16round(-10);
    f16round(100);
    f16round(-100);
    f16round(65504);
    f16round(-65504);
    f16round(6.103515625e-5);
    f16round(5.960464477539063e-8);
    f16round(0.1);
    f16round(0.2);
    f16round(0.3);
    f16round(1.1);
    f16round(3.14);
    f16round(2.718);
    f16round(65520);
    f16round(-65520);
    f16round(1e6);
    f16round(-1e6);
    f16round(1e-9);
    f16round(-1e-9);
    f16round(2049);
    f16round(2051);
  }
  var elapsed = Date.now() - start;
  log("f16round:          " + elapsed + " ms");

  // --- Part 2: Float16Array ---

  var ARRAY_SIZE = 10000;
  var ARRAY_ITERATIONS = 2000;

  log("");
  log("=== Part 2: Float16Array ===");
  log("Array size: " + ARRAY_SIZE + ", iterations: " + ARRAY_ITERATIONS);

  // 2a: Write doubles into Float16Array (double -> float16 conversion)
  var f16arr = new Float16Array(ARRAY_SIZE);
  var srcDoubles = new Float64Array(ARRAY_SIZE);
  for (var i = 0; i < ARRAY_SIZE; i++) {
    srcDoubles[i] = (Math.random() - 0.5) * 131008;
  }

  start = Date.now();
  for (var i = 0; i < ARRAY_ITERATIONS; i++) {
    for (var j = 0; j < ARRAY_SIZE; j++) {
      f16arr[j] = srcDoubles[j];
    }
  }
  elapsed = Date.now() - start;
  log("Write (f64->f16): " + elapsed + " ms");

  // 2b: Read Float16Array into doubles (float16 -> double conversion)
  var sink = 0;
  start = Date.now();
  for (var i = 0; i < ARRAY_ITERATIONS; i++) {
    for (var j = 0; j < ARRAY_SIZE; j++) {
      sink += f16arr[j];
    }
  }
  elapsed = Date.now() - start;
  log("Read  (f16->f64): " + elapsed + " ms");
  if (sink === 0.12345) log(sink);

  // 2c: Float16Array.set from Float64Array (bulk double -> float16)
  var f16dst = new Float16Array(ARRAY_SIZE);
  start = Date.now();
  for (var i = 0; i < ARRAY_ITERATIONS; i++) {
    f16dst.set(srcDoubles);
  }
  elapsed = Date.now() - start;
  log("set(Float64Array): " + elapsed + " ms");

  // 2d: Float16Array.sort
  var SORT_ITERATIONS = 500;
  var f16sort = new Float16Array(ARRAY_SIZE);
  var f16source = new Float16Array(ARRAY_SIZE);
  for (var i = 0; i < ARRAY_SIZE; i++) {
    f16source[i] = (Math.random() - 0.5) * 131008;
  }

  start = Date.now();
  for (var i = 0; i < SORT_ITERATIONS; i++) {
    f16sort.set(f16source);
    f16sort.sort();
  }
  elapsed = Date.now() - start;
  log("sort:              " + elapsed + " ms");
})();
