/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Minimal Node.js 'assert' module shim for running Node.js NAPI tests
// under Hermes. Provides the subset of assert methods used by the tests.

(function() {
'use strict';

function formatValue(v) {
  if (v === undefined) return 'undefined';
  if (v === null) return 'null';
  if (typeof v === 'string') return JSON.stringify(v);
  if (typeof v === 'symbol') return v.toString();
  if (typeof v === 'function') return '[Function' +
    (v.name ? ': ' + v.name : '') + ']';
  if (typeof v === 'bigint') return v.toString() + 'n';
  if (typeof v === 'number') {
    if (Object.is(v, -0)) return '-0';
    return String(v);
  }
  if (Array.isArray(v)) {
    var items = [];
    for (var i = 0; i < v.length; i++) {
      items.push(formatValue(v[i]));
    }
    return '[ ' + items.join(', ') + ' ]';
  }
  if (typeof v === 'object') {
    try {
      return JSON.stringify(v);
    } catch (e) {
      return String(v);
    }
  }
  return String(v);
}

// Deep strict equality check.
function deepStrictEqual(a, b) {
  if (Object.is(a, b)) return true;
  if (typeof a !== typeof b) return false;
  if (a === null || b === null) return false;
  if (typeof a !== 'object') return false;

  // Array check.
  if (Array.isArray(a)) {
    if (!Array.isArray(b)) return false;
    if (a.length !== b.length) return false;
    for (var i = 0; i < a.length; i++) {
      if (!deepStrictEqual(a[i], b[i])) return false;
    }
    return true;
  }
  if (Array.isArray(b)) return false;

  // TypedArray / ArrayBuffer check.
  if (ArrayBuffer.isView(a) && ArrayBuffer.isView(b)) {
    if (a.constructor !== b.constructor) return false;
    if (a.byteLength !== b.byteLength) return false;
    var va = new Uint8Array(a.buffer, a.byteOffset, a.byteLength);
    var vb = new Uint8Array(b.buffer, b.byteOffset, b.byteLength);
    for (var i = 0; i < va.length; i++) {
      if (va[i] !== vb[i]) return false;
    }
    return true;
  }

  // Plain object check.
  var keysA = Object.keys(a);
  var keysB = Object.keys(b);
  if (keysA.length !== keysB.length) return false;
  for (var i = 0; i < keysA.length; i++) {
    var key = keysA[i];
    if (!b.hasOwnProperty(key)) return false;
    if (!deepStrictEqual(a[key], b[key])) return false;
  }
  return true;
}

function AssertionError(message, actual, expected, operator) {
  var err = new Error(message);
  err.name = 'AssertionError';
  err.actual = actual;
  err.expected = expected;
  err.operator = operator;
  return err;
}

function assert(value, message) {
  if (!value) {
    throw AssertionError(
      message || 'Expected truthy value, got ' + formatValue(value),
      value, true, '==');
  }
}

assert.ok = function ok(value, message) {
  if (!value) {
    throw AssertionError(
      message || 'Expected truthy value, got ' + formatValue(value),
      value, true, '==');
  }
};

assert.strictEqual = function strictEqual(actual, expected, message) {
  if (!Object.is(actual, expected)) {
    throw AssertionError(
      message || 'Expected ' + formatValue(expected) +
        ' but got ' + formatValue(actual),
      actual, expected, '===');
  }
};

assert.notStrictEqual = function notStrictEqual(actual, expected, message) {
  if (Object.is(actual, expected)) {
    throw AssertionError(
      message || 'Expected value not to be ' + formatValue(expected),
      actual, expected, '!==');
  }
};

assert.deepStrictEqual = function(actual, expected, message) {
  if (!deepStrictEqual(actual, expected)) {
    throw AssertionError(
      message || 'Expected deep equality.\n' +
        '  actual:   ' + formatValue(actual) + '\n' +
        '  expected: ' + formatValue(expected),
      actual, expected, 'deepStrictEqual');
  }
};

assert.throws = function throws(fn, expected, message) {
  var threw = false;
  var actual;
  try {
    fn();
  } catch (e) {
    threw = true;
    actual = e;
  }
  if (!threw) {
    throw AssertionError(
      message || 'Expected function to throw',
      undefined, expected, 'throws');
  }
  if (expected !== undefined) {
    if (typeof expected === 'function') {
      if (expected.prototype !== undefined &&
          actual instanceof expected) {
        // Error class check passed.
      } else if (expected(actual)) {
        // Validator function returned true.
      } else {
        throw AssertionError(
          message || 'Thrown error did not match expected',
          actual, expected, 'throws');
      }
    } else if (expected instanceof RegExp) {
      if (!expected.test(String(actual))) {
        throw AssertionError(
          message || 'Thrown error message did not match: ' +
            formatValue(actual.message) +
            ' vs ' + expected,
          actual, expected, 'throws');
      }
    } else if (typeof expected === 'object' && expected !== null) {
      // Check properties of expected against actual.
      var keys = Object.keys(expected);
      for (var i = 0; i < keys.length; i++) {
        var key = keys[i];
        if (key === 'message') {
          if (typeof expected.message === 'string') {
            if (actual.message !== expected.message) {
              throw AssertionError(
                message || 'Error message mismatch: ' +
                  formatValue(actual.message) +
                  ' !== ' + formatValue(expected.message),
                actual, expected, 'throws');
            }
          } else if (expected.message instanceof RegExp) {
            if (!expected.message.test(actual.message)) {
              throw AssertionError(
                message || 'Error message did not match regex: ' +
                  formatValue(actual.message),
                actual, expected, 'throws');
            }
          }
        } else if (key === 'name') {
          if (actual.name !== expected.name) {
            throw AssertionError(
              message || 'Error name mismatch: ' +
                formatValue(actual.name) +
                ' !== ' + formatValue(expected.name),
              actual, expected, 'throws');
          }
        } else if (key === 'code') {
          if (actual.code !== expected.code) {
            throw AssertionError(
              message || 'Error code mismatch: ' +
                formatValue(actual.code) +
                ' !== ' + formatValue(expected.code),
              actual, expected, 'throws');
          }
        }
      }
    }
  }
};

assert.match = function match(string, regexp, message) {
  if (typeof string !== 'string') {
    throw new TypeError('The "string" argument must be of type string');
  }
  if (!(regexp instanceof RegExp)) {
    throw new TypeError('The "regexp" argument must be an instance of RegExp');
  }
  if (!regexp.test(string)) {
    throw AssertionError(
      message || 'Expected ' + formatValue(string) +
        ' to match ' + regexp,
      string, regexp, 'match');
  }
};

assert.ifError = function ifError(err) {
  if (err !== null && err !== undefined) {
    throw err;
  }
};

assert.fail = function fail(message) {
  throw AssertionError(
    typeof message === 'string' ? message : 'assert.fail()',
    undefined, undefined, 'fail');
};

assert.AssertionError = AssertionError;

// Export as global for the harness.
globalThis.__assert = assert;

})();
