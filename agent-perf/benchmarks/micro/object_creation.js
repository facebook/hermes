/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Object creation patterns for Static Hermes.
// Tests literal objects, constructor calls, and property initialization.

"use strict";

var ITERATIONS = 200000;

// --- Object literal creation ---
function benchObjectLiteral() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var obj = {x: i, y: i + 1, z: i + 2};
    sum += obj.x + obj.y + obj.z;
  }
  return sum;
}

// --- Constructor call ---
function Point(x, y, z) {
  this.x = x;
  this.y = y;
  this.z = z;
}

function benchConstructor() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var p = new Point(i, i + 1, i + 2);
    sum += p.x + p.y + p.z;
  }
  return sum;
}

// --- Object.create ---
function benchObjectCreate() {
  var proto = {getValue: function() { return this.x + this.y; }};
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var obj = Object.create(proto);
    obj.x = i;
    obj.y = i + 1;
    sum += obj.getValue();
  }
  return sum;
}

// --- Nested object creation ---
function benchNestedObjects() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 2; i++) {
    var obj = {
      position: {x: i, y: i + 1},
      size: {width: 100, height: 200},
      style: {color: "red", opacity: 0.5}
    };
    sum += obj.position.x + obj.size.width;
  }
  return sum;
}

// --- Object with methods ---
function benchObjectWithMethods() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 2; i++) {
    var obj = {
      value: i,
      double: function() { return this.value * 2; },
      square: function() { return this.value * this.value; }
    };
    sum += obj.double() + obj.square();
  }
  return sum;
}

// --- Object spread / Object.assign ---
function benchObjectAssign() {
  var defaults = {x: 0, y: 0, width: 100, height: 100};
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 2; i++) {
    var obj = Object.assign({}, defaults, {x: i, y: i + 1});
    sum += obj.x + obj.y + obj.width;
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

runBench("object_literal", benchObjectLiteral);
runBench("constructor", benchConstructor);
runBench("object_create", benchObjectCreate);
runBench("nested_objects", benchNestedObjects);
runBench("object_with_methods", benchObjectWithMethods);
runBench("object_assign", benchObjectAssign);
