/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Macro-benchmark: JSON.stringify / JSON.parse round-trip processing.
// Simulates realistic API payloads of varying sizes, serializes and
// deserializes them, and validates the results. Exercises the JSON
// engine, string handling, object/array creation, and deep comparison.

"use strict";

var ITERATIONS_SMALL = 5000;
var ITERATIONS_MEDIUM = 1000;
var ITERATIONS_LARGE = 200;

// --- Generate realistic payloads ---

function makeUser(id) {
  return {
    id: id,
    name: "User " + id,
    email: "user" + id + "@example.com",
    active: id % 3 !== 0,
    age: 20 + (id % 40),
    score: (id * 17) % 1000 / 10.0,
    tags: ["tag" + (id % 5), "tag" + (id % 7), "tag" + (id % 11)]
  };
}

function makeSmallPayload() {
  return {
    status: "ok",
    code: 200,
    data: {id: 1, value: "hello"}
  };
}

function makeMediumPayload() {
  var users = [];
  for (var i = 0; i < 20; i++) {
    users.push(makeUser(i));
  }
  return {
    status: "ok",
    page: 1,
    totalPages: 10,
    count: 20,
    data: users,
    meta: {
      requestId: "abc-123-def-456",
      timestamp: 1700000000000,
      server: "api-west-2"
    }
  };
}

function makeLargePayload() {
  var categories = [];
  for (var c = 0; c < 5; c++) {
    var items = [];
    for (var i = 0; i < 30; i++) {
      items.push({
        id: c * 30 + i,
        title: "Item " + i + " in category " + c,
        description: "A longer description string that contains more text " +
          "to increase payload size for realistic benchmarking of JSON ops.",
        price: ((c * 30 + i) * 7 % 10000) / 100.0,
        inStock: (c + i) % 4 !== 0,
        attributes: {
          color: ["red", "green", "blue", "black", "white"][i % 5],
          size: ["S", "M", "L", "XL"][i % 4],
          weight: (i * 3 + 10) / 10.0
        },
        ratings: [
          {user: "u" + (i * 2), score: (i % 5) + 1},
          {user: "u" + (i * 2 + 1), score: ((i + 2) % 5) + 1}
        ]
      });
    }
    categories.push({
      name: "Category " + c,
      slug: "category-" + c,
      itemCount: items.length,
      items: items
    });
  }
  return {
    store: "Example Store",
    version: "2.1.0",
    categories: categories,
    config: {
      currency: "USD",
      locale: "en-US",
      features: {
        search: true,
        reviews: true,
        wishlist: false
      }
    }
  };
}

// --- Deep equality check (used to validate round-trip) ---
function deepEqual(a, b) {
  if (a === b) return true;
  if (a === null || b === null) return false;
  if (typeof a !== typeof b) return false;

  if (typeof a === "object") {
    var isArrayA = Array.isArray(a);
    var isArrayB = Array.isArray(b);
    if (isArrayA !== isArrayB) return false;

    if (isArrayA) {
      if (a.length !== b.length) return false;
      for (var i = 0; i < a.length; i++) {
        if (!deepEqual(a[i], b[i])) return false;
      }
      return true;
    }

    var keysA = Object.keys(a);
    var keysB = Object.keys(b);
    if (keysA.length !== keysB.length) return false;
    for (var k = 0; k < keysA.length; k++) {
      var key = keysA[k];
      if (!deepEqual(a[key], b[key])) return false;
    }
    return true;
  }

  return false;
}

// --- Benchmark: Small payload round-trip ---
function benchSmallPayload() {
  var verified = 0;
  for (var i = 0; i < ITERATIONS_SMALL; i++) {
    var payload = makeSmallPayload();
    var json = JSON.stringify(payload);
    var parsed = JSON.parse(json);
    if (deepEqual(payload, parsed)) verified++;
  }
  if (verified !== ITERATIONS_SMALL) {
    print("ERROR: small payload verification failed");
  }
  return verified;
}

// --- Benchmark: Medium payload round-trip ---
function benchMediumPayload() {
  var verified = 0;
  for (var i = 0; i < ITERATIONS_MEDIUM; i++) {
    var payload = makeMediumPayload();
    var json = JSON.stringify(payload);
    var parsed = JSON.parse(json);
    if (deepEqual(payload, parsed)) verified++;
  }
  if (verified !== ITERATIONS_MEDIUM) {
    print("ERROR: medium payload verification failed");
  }
  return verified;
}

// --- Benchmark: Large payload round-trip ---
function benchLargePayload() {
  var verified = 0;
  for (var i = 0; i < ITERATIONS_LARGE; i++) {
    var payload = makeLargePayload();
    var json = JSON.stringify(payload);
    var parsed = JSON.parse(json);
    if (deepEqual(payload, parsed)) verified++;
  }
  if (verified !== ITERATIONS_LARGE) {
    print("ERROR: large payload verification failed");
  }
  return verified;
}

// --- Benchmark: Stringify with replacer ---
function benchStringifyReplacer() {
  var payload = makeMediumPayload();
  var count = 0;
  var replacer = function(key, value) {
    if (key === "email") return undefined; // Strip emails
    if (typeof value === "number") return Math.round(value);
    return value;
  };
  for (var i = 0; i < ITERATIONS_MEDIUM; i++) {
    var json = JSON.stringify(payload, replacer);
    count += json.length;
  }
  return count;
}

// --- Benchmark: Parse with reviver ---
function benchParseReviver() {
  var payload = makeMediumPayload();
  var json = JSON.stringify(payload);
  var count = 0;
  var reviver = function(key, value) {
    if (typeof value === "string" && value.indexOf("@") !== -1) {
      return "[REDACTED]";
    }
    return value;
  };
  for (var i = 0; i < ITERATIONS_MEDIUM; i++) {
    var parsed = JSON.parse(json, reviver);
    count += Object.keys(parsed).length;
  }
  return count;
}

// --- Benchmark: Incremental JSON building ---
function benchIncrementalBuild() {
  var total = 0;
  for (var i = 0; i < ITERATIONS_MEDIUM; i++) {
    // Build up JSON string fragments then parse
    var parts = [];
    parts.push('{"items":[');
    for (var j = 0; j < 20; j++) {
      if (j > 0) parts.push(",");
      parts.push('{"id":' + j + ',"val":"item-' + j + '"}');
    }
    parts.push(']}');
    var json = parts.join("");
    var parsed = JSON.parse(json);
    total += parsed.items.length;
  }
  return total;
}

// Run all benchmarks
function runBench(name, fn, iters) {
  var start = Date.now();
  var result = fn();
  var elapsed = Date.now() - start;
  var opsPerSec = Math.round(iters / (elapsed / 1000));
  print("RESULT: " + name + " " + opsPerSec + " ops/sec");
  return result;
}

runBench("json_small_roundtrip", benchSmallPayload, ITERATIONS_SMALL);
runBench("json_medium_roundtrip", benchMediumPayload, ITERATIONS_MEDIUM);
runBench("json_large_roundtrip", benchLargePayload, ITERATIONS_LARGE);
runBench("json_stringify_replacer", benchStringifyReplacer, ITERATIONS_MEDIUM);
runBench("json_parse_reviver", benchParseReviver, ITERATIONS_MEDIUM);
runBench("json_incremental_build", benchIncrementalBuild, ITERATIONS_MEDIUM);
