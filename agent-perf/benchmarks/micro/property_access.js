/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Property access patterns for Static Hermes.
// Tests named property get/put, computed access, and prototype chain lookups.

"use strict";

var ITERATIONS = 500000;

// --- Named property access (monomorphic) ---
function benchNamedAccess() {
  var obj = {x: 1, y: 2, z: 3};
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += obj.x;
    sum += obj.y;
    sum += obj.z;
    obj.x = sum & 0xff;
  }
  return sum;
}

// --- Computed property access ---
function benchComputedAccess() {
  var obj = {a: 1, b: 2, c: 3};
  var keys = ["a", "b", "c"];
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    for (var k = 0; k < keys.length; k++) {
      sum += obj[keys[k]];
    }
  }
  return sum;
}

// --- Prototype chain lookup ---
function benchProtoChain() {
  function Base() {}
  Base.prototype.value = 42;
  function Mid() {}
  Mid.prototype = Object.create(Base.prototype);
  Mid.prototype.mid = 10;
  function Leaf() { this.own = 1; }
  Leaf.prototype = Object.create(Mid.prototype);

  var obj = new Leaf();
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += obj.own;    // Own property
    sum += obj.mid;    // 1 level up
    sum += obj.value;  // 2 levels up
  }
  return sum;
}

// --- Property addition (hidden class transitions) ---
function benchPropertyAddition() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    var obj = {};
    obj.a = 1;
    obj.b = 2;
    obj.c = 3;
    obj.d = 4;
    sum += obj.a + obj.b + obj.c + obj.d;
  }
  return sum;
}

// --- hasOwnProperty checks ---
function benchHasOwnProperty() {
  var obj = {x: 1, y: 2, z: 3};
  var count = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    if (obj.hasOwnProperty("x")) count++;
    if (obj.hasOwnProperty("y")) count++;
    if (!obj.hasOwnProperty("w")) count++;
  }
  return count;
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

runBench("named_access", benchNamedAccess);
runBench("computed_access", benchComputedAccess);
runBench("proto_chain", benchProtoChain);
runBench("property_addition", benchPropertyAddition);
runBench("has_own_property", benchHasOwnProperty);
