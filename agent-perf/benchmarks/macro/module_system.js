/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Macro-benchmark: CommonJS module system simulation.
// Simulates a 60-module dependency graph with require(), module caching,
// factory execution, circular dependency handling, and initialization
// ordering. Exercises closures, map/object lookups, function calls,
// and dependency graph traversal.

"use strict";

var ITERATIONS = 500;
var NUM_MODULES = 60;

// --- Module registry ---
function createModuleSystem() {
  var modules = {};     // id -> {factory, exports, loaded, loading}
  var loadOrder = [];   // Track initialization order

  function define(id, deps, factory) {
    modules[id] = {
      factory: factory,
      deps: deps,
      exports: {},
      loaded: false,
      loading: false
    };
  }

  function require(id) {
    var mod = modules[id];
    if (!mod) {
      throw new Error("Module not found: " + id);
    }

    // Return cached exports if already loaded
    if (mod.loaded) {
      return mod.exports;
    }

    // Circular dependency: return partial exports
    if (mod.loading) {
      return mod.exports;
    }

    mod.loading = true;

    // Resolve dependencies first
    var resolvedDeps = [];
    for (var i = 0; i < mod.deps.length; i++) {
      resolvedDeps.push(require(mod.deps[i]));
    }

    // Execute factory
    var moduleObj = {exports: mod.exports};
    mod.factory(moduleObj, moduleObj.exports, require, resolvedDeps);
    mod.exports = moduleObj.exports;
    mod.loaded = true;
    mod.loading = false;
    loadOrder.push(id);

    return mod.exports;
  }

  function reset() {
    var ids = Object.keys(modules);
    for (var i = 0; i < ids.length; i++) {
      var mod = modules[ids[i]];
      mod.exports = {};
      mod.loaded = false;
      mod.loading = false;
    }
    loadOrder = [];
  }

  function getLoadOrder() {
    return loadOrder;
  }

  return {
    define: define,
    require: require,
    reset: reset,
    getLoadOrder: getLoadOrder
  };
}

// --- Register modules with realistic dependency patterns ---
function registerModules(sys) {
  // Utility modules (no deps) - modules 0..9
  for (var i = 0; i < 10; i++) {
    (function(id) {
      sys.define("util/" + id, [], function(module, exports) {
        exports.name = "util-" + id;
        exports.compute = function(x) { return x * (id + 1); };
        exports.transform = function(arr) {
          var result = [];
          for (var j = 0; j < arr.length; j++) {
            result.push(arr[j] + id);
          }
          return result;
        };
        exports.CONSTANT = id * 100;
      });
    })(i);
  }

  // Mid-level modules (depend on utils) - modules 10..29
  for (var i = 10; i < 30; i++) {
    (function(id) {
      var deps = [
        "util/" + (id % 10),
        "util/" + ((id + 3) % 10)
      ];
      sys.define("lib/" + id, deps, function(module, exports, req, resolved) {
        var util1 = resolved[0];
        var util2 = resolved[1];
        exports.name = "lib-" + id;
        exports.process = function(x) {
          return util1.compute(x) + util2.compute(x);
        };
        exports.data = util1.transform([1, 2, 3, 4, 5]);
        exports.config = {
          source: util1.name,
          helper: util2.name,
          level: id
        };
      });
    })(i);
  }

  // Service modules (depend on libs) - modules 30..49
  for (var i = 30; i < 50; i++) {
    (function(id) {
      var deps = [
        "lib/" + (10 + (id % 20)),
        "lib/" + (10 + ((id + 7) % 20))
      ];
      sys.define("service/" + id, deps, function(module, exports, req, resolved) {
        var lib1 = resolved[0];
        var lib2 = resolved[1];
        exports.name = "service-" + id;
        exports.execute = function(input) {
          return lib1.process(input) + lib2.process(input);
        };
        exports.metadata = {
          dependencies: [lib1.name, lib2.name],
          dataSize: lib1.data.length + lib2.data.length
        };
      });
    })(i);
  }

  // Circular dependency pair - modules 50, 51
  sys.define("circular/a", ["circular/b"], function(module, exports, req, resolved) {
    var b = resolved[0];
    exports.name = "circular-a";
    exports.getValue = function() { return 42; };
    exports.partner = b.name || "pending";
  });

  sys.define("circular/b", ["circular/a"], function(module, exports, req, resolved) {
    var a = resolved[0];
    exports.name = "circular-b";
    exports.getValue = function() { return 84; };
    exports.partner = a.name || "pending";
  });

  // App entry modules (depend on services + circular) - modules 52..59
  for (var i = 52; i < NUM_MODULES; i++) {
    (function(id) {
      var deps = [
        "service/" + (30 + (id % 20)),
        "service/" + (30 + ((id + 5) % 20)),
        "circular/a"
      ];
      sys.define("app/" + id, deps, function(module, exports, req, resolved) {
        var svc1 = resolved[0];
        var svc2 = resolved[1];
        var circA = resolved[2];
        exports.name = "app-" + id;
        exports.run = function() {
          return svc1.execute(id) + svc2.execute(id) + circA.getValue();
        };
        exports.info = {
          services: [svc1.name, svc2.name],
          circular: circA.partner
        };
      });
    })(i);
  }
}

// --- Benchmark: Full module graph initialization ---
function benchModuleInit() {
  var sys = createModuleSystem();
  registerModules(sys);
  var totalLoaded = 0;

  for (var i = 0; i < ITERATIONS; i++) {
    sys.reset();
    // Require all app entry points (triggers full graph resolution)
    for (var a = 52; a < NUM_MODULES; a++) {
      sys.require("app/" + a);
    }
    totalLoaded += sys.getLoadOrder().length;
  }
  return totalLoaded;
}

// --- Benchmark: Repeated require (cache hits) ---
function benchModuleCache() {
  var sys = createModuleSystem();
  registerModules(sys);
  var total = 0;

  // Initialize once
  for (var a = 52; a < NUM_MODULES; a++) {
    sys.require("app/" + a);
  }

  // Repeatedly require (should hit cache every time)
  for (var i = 0; i < ITERATIONS * 10; i++) {
    var modId = "service/" + (30 + (i % 20));
    var mod = sys.require(modId);
    total += mod.metadata.dataSize;
  }
  return total;
}

// --- Benchmark: Module execution workload ---
function benchModuleExecution() {
  var sys = createModuleSystem();
  registerModules(sys);
  var total = 0;

  // Initialize all modules
  for (var a = 52; a < NUM_MODULES; a++) {
    sys.require("app/" + a);
  }

  // Execute module functions
  for (var i = 0; i < ITERATIONS; i++) {
    for (var a = 52; a < NUM_MODULES; a++) {
      var app = sys.require("app/" + a);
      total += app.run();
    }
  }
  return total;
}

// --- Benchmark: Module system creation + teardown ---
function benchModuleLifecycle() {
  var total = 0;
  for (var i = 0; i < ITERATIONS; i++) {
    var sys = createModuleSystem();
    registerModules(sys);
    // Require a single entry point
    var app = sys.require("app/55");
    total += app.run();
    // System is GC'd on next iteration
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

runBench("module_init", benchModuleInit);
runBench("module_cache", benchModuleCache);
runBench("module_execution", benchModuleExecution);
runBench("module_lifecycle", benchModuleLifecycle);
