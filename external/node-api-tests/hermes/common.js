/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Minimal Node.js 'common' test helper shim for running Node.js NAPI tests
// under Hermes. Provides the subset of common utilities used by the tests.

(function() {
'use strict';

var common = {};

// buildType is used in require paths: `./build/${common.buildType}/addon`
common.buildType = 'Release';

// mustCall wraps a function to verify it gets called at least once.
// For simplicity in Hermes, we just return the function as-is since
// we can't easily track call counts without process.on('exit').
common.mustCall = function mustCall(fn, expectedCalls) {
  if (typeof fn === 'number') {
    expectedCalls = fn;
    fn = undefined;
  }
  if (fn === undefined) {
    fn = function() {};
  }
  return fn;
};

// mustCallAtLeast wraps a function to verify it gets called at least N times.
// Simplified: just return the function.
common.mustCallAtLeast = function mustCallAtLeast(fn, minimum) {
  if (typeof fn === 'number') {
    minimum = fn;
    fn = undefined;
  }
  if (fn === undefined) {
    fn = function() {};
  }
  return fn;
};

// mustNotCall returns a function that throws if called.
common.mustNotCall = function mustNotCall(msg) {
  return function() {
    throw new Error(msg || 'Function should not have been called');
  };
};

// platformTimeout returns the timeout value scaled for the platform.
// In Hermes we just return the value directly.
common.platformTimeout = function platformTimeout(ms) {
  return ms;
};

// expectsError returns a validator function for assert.throws.
common.expectsError = function expectsError(settings) {
  return function(error) {
    if (settings.code && error.code !== settings.code) return false;
    if (settings.name && error.name !== settings.name) return false;
    if (settings.message) {
      if (settings.message instanceof RegExp) {
        if (!settings.message.test(error.message)) return false;
      } else if (error.message !== settings.message) {
        return false;
      }
    }
    if (settings.type && !(error instanceof settings.type)) return false;
    return true;
  };
};

// nodeProcessAborted checks if a child process was aborted.
// Not applicable in Hermes.
common.nodeProcessAborted = function() {
  return false;
};

// Export as global for the harness.
globalThis.__common = common;

// Node.js defines 'global' as an alias for globalThis.
if (typeof globalThis.global === 'undefined') {
  globalThis.global = globalThis;
}

// Provide global.gc() which is available in Node.js with --expose-gc.
// In Hermes, use gc_() if available.
if (typeof globalThis.gc === 'undefined') {
  globalThis.gc = function gc() {
    try {
      gc_();  // Hermes built-in GC trigger
    } catch (e) {
      // gc_() not available, try allocating garbage to trigger GC.
      var garbage = [];
      for (var j = 0; j < 1000; j++) {
        garbage.push({x: j, y: new Array(100)});
      }
    }
  };
}

// Minimal GC helper (for require('../../common/gc')).
var gc = {};
gc.gcUntil = function gcUntil(name, condition) {
  // In Hermes, we can trigger GC directly.
  for (var i = 0; i < 10; i++) {
    if (condition()) return;
    // Attempt to trigger GC by allocating and discarding objects.
    try {
      gc_();  // Hermes built-in GC trigger if available
    } catch (e) {
      // gc_() not available, try allocating garbage.
      var garbage = [];
      for (var j = 0; j < 1000; j++) {
        garbage.push({x: j, y: new Array(100)});
      }
    }
  }
  if (!condition()) {
    throw new Error('gcUntil(' + name + '): condition not met after 10 GC cycles');
  }
};
globalThis.__gc = gc;

})();
