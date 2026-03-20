/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Array operations for Static Hermes.
// Tests push/pop, indexed access, iteration, and higher-order methods.

"use strict";

var ITERATIONS = 200000;

// --- Array push/pop ---
function benchPushPop() {
  var arr = [];
  for (var i = 0; i < ITERATIONS; i++) {
    arr.push(i);
  }
  var sum = 0;
  while (arr.length > 0) {
    sum += arr.pop();
  }
  return sum;
}

// --- Indexed access ---
function benchIndexedAccess() {
  var arr = new Array(1000);
  for (var i = 0; i < 1000; i++) arr[i] = i;
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += arr[i % 1000];
    arr[(i + 500) % 1000] = sum & 0xff;
  }
  return sum;
}

// --- Array iteration (for loop) ---
function benchForLoop() {
  var arr = new Array(1000);
  for (var i = 0; i < 1000; i++) arr[i] = i;
  var total = 0;
  for (var iter = 0; iter < ITERATIONS / 100; iter++) {
    var sum = 0;
    for (var i = 0; i < arr.length; i++) {
      sum += arr[i];
    }
    total += sum;
  }
  return total;
}

// --- Array.map ---
function benchMap() {
  var arr = new Array(100);
  for (var i = 0; i < 100; i++) arr[i] = i;
  var sum = 0;
  for (var iter = 0; iter < ITERATIONS / 100; iter++) {
    var mapped = arr.map(function(x) { return x * 2; });
    sum += mapped[50];
  }
  return sum;
}

// --- Array.filter ---
function benchFilter() {
  var arr = new Array(100);
  for (var i = 0; i < 100; i++) arr[i] = i;
  var sum = 0;
  for (var iter = 0; iter < ITERATIONS / 100; iter++) {
    var filtered = arr.filter(function(x) { return x % 2 === 0; });
    sum += filtered.length;
  }
  return sum;
}

// --- Array.reduce ---
function benchReduce() {
  var arr = new Array(100);
  for (var i = 0; i < 100; i++) arr[i] = i;
  var total = 0;
  for (var iter = 0; iter < ITERATIONS / 100; iter++) {
    total += arr.reduce(function(acc, x) { return acc + x; }, 0);
  }
  return total;
}

// --- Array.sort ---
function benchSort() {
  var sum = 0;
  for (var iter = 0; iter < ITERATIONS / 1000; iter++) {
    var arr = new Array(100);
    for (var i = 0; i < 100; i++) arr[i] = (i * 17 + 31) % 100;
    arr.sort(function(a, b) { return a - b; });
    sum += arr[0] + arr[99];
  }
  return sum;
}

// --- Array spread / concat ---
function benchConcat() {
  var a = [1, 2, 3, 4, 5];
  var b = [6, 7, 8, 9, 10];
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    var c = a.concat(b);
    sum += c.length;
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

runBench("array_push_pop", benchPushPop);
runBench("indexed_access", benchIndexedAccess);
runBench("for_loop", benchForLoop);
runBench("array_map", benchMap);
runBench("array_filter", benchFilter);
runBench("array_reduce", benchReduce);
runBench("array_sort", benchSort);
runBench("array_concat", benchConcat);
