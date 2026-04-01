/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: String operations for Static Hermes.
// Tests concatenation, comparison, template literals, and slicing.

"use strict";

var ITERATIONS = 200000;

// --- String concatenation ---
function benchConcat() {
  var s = "";
  for (var i = 0; i < ITERATIONS / 10; i++) {
    s = "hello" + " " + "world" + " " + i;
  }
  return s.length;
}

// --- String comparison ---
function benchComparison() {
  var strings = ["apple", "banana", "cherry", "date", "elderberry"];
  var count = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var a = strings[i % 5];
    var b = strings[(i + 1) % 5];
    if (a < b) count++;
    if (a === b) count++;
  }
  return count;
}

// --- String slice/substring ---
function benchSlice() {
  var str = "The quick brown fox jumps over the lazy dog";
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var s = str.slice(i % 10, i % 10 + 10);
    sum += s.length;
  }
  return sum;
}

// --- String indexOf/search ---
function benchSearch() {
  var str = "The quick brown fox jumps over the lazy dog";
  var count = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    if (str.indexOf("fox") !== -1) count++;
    if (str.indexOf("cat") === -1) count++;
  }
  return count;
}

// --- String charAt/charCodeAt ---
function benchCharAt() {
  var str = "Hello, World! This is a test string for benchmarking.";
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += str.charCodeAt(i % str.length);
  }
  return sum;
}

// --- String replace ---
function benchReplace() {
  var str = "Hello World Hello World Hello World";
  var count = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    var result = str.replace("Hello", "Hi");
    count += result.length;
  }
  return count;
}

// --- String split/join ---
function benchSplitJoin() {
  var str = "one,two,three,four,five";
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    var parts = str.split(",");
    sum += parts.length;
    var joined = parts.join("-");
    sum += joined.length;
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

runBench("string_concat", benchConcat);
runBench("string_comparison", benchComparison);
runBench("string_slice", benchSlice);
runBench("string_search", benchSearch);
runBench("string_charAt", benchCharAt);
runBench("string_replace", benchReplace);
runBench("string_split_join", benchSplitJoin);
