/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Micro-benchmark: Closure and scope patterns for Static Hermes.
// Tests closure creation, scope chain access, and environment capture.

"use strict";

var ITERATIONS = 200000;

// --- Closure creation ---
function benchClosureCreation() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var fn = (function(x) {
      return function() { return x; };
    })(i);
    sum += fn();
  }
  return sum;
}

// --- Scope chain access (depth 1) ---
function benchScopeChain1() {
  var outer = 42;
  var sum = 0;
  var fn = function() { return outer; };
  for (var i = 0; i < ITERATIONS; i++) {
    sum += fn();
  }
  return sum;
}

// --- Scope chain access (depth 3) ---
function benchScopeChain3() {
  var sum = 0;
  (function() {
    var a = 10;
    (function() {
      var b = 20;
      (function() {
        var c = 30;
        var fn = function() { return a + b + c; };
        for (var i = 0; i < ITERATIONS; i++) {
          sum += fn();
        }
      })();
    })();
  })();
  return sum;
}

// --- Mutable closure variables ---
function benchMutableClosure() {
  var counter = 0;
  var inc = function() { counter++; };
  var get = function() { return counter; };
  for (var i = 0; i < ITERATIONS; i++) {
    inc();
  }
  return get();
}

// --- Closure in loop (let-like semantics) ---
function benchClosureInLoop() {
  var fns = [];
  for (var i = 0; i < 100; i++) {
    fns.push((function(x) {
      return function() { return x; };
    })(i));
  }
  var sum = 0;
  for (var iter = 0; iter < ITERATIONS / 100; iter++) {
    for (var j = 0; j < fns.length; j++) {
      sum += fns[j]();
    }
  }
  return sum;
}

// --- IIFE pattern ---
function benchIIFE() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    sum += (function(x) { return x * 2; })(i);
  }
  return sum;
}

// --- Closure over multiple variables ---
function benchMultiVarClosure() {
  var sum = 0;
  for (var i = 0; i < ITERATIONS / 10; i++) {
    var fn = (function(a, b, c, d) {
      return function() { return a + b + c + d; };
    })(i, i + 1, i + 2, i + 3);
    sum += fn();
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

runBench("closure_creation", benchClosureCreation);
runBench("scope_chain_1", benchScopeChain1);
runBench("scope_chain_3", benchScopeChain3);
runBench("mutable_closure", benchMutableClosure);
runBench("closure_in_loop", benchClosureInLoop);
runBench("iife", benchIIFE);
runBench("multi_var_closure", benchMultiVarClosure);
