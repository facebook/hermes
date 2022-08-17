/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

function betterToString(value) {
  if (Array.isArray(value)) {
    return('[' +
           ((typeof HermesInternal === 'object' && HermesInternal.isProxy(value))
            ? 'Proxy:' : '') +
           value.map(betterToString).join(',') +
           ']');
  } else if (typeof HermesInternal === 'object' && HermesInternal.isProxy(value)) {
    return '[Proxy]';
  } else {
    // This works reasonably if value is a symbol.
    return String(value);
  }
}

// This has a similar API to the Node.js assert object.
let assert = {
  _isEqual: function(a, b) {
    // Remember to check for NaN which does not compare as equal with itself.
    return a === b || (Number.isNaN(a) && Number.isNaN(b));
  },
  equal: function(actual, expected, msg) {
    if (!assert._isEqual(actual, expected)) {
      assert.fail(
        (msg ? msg + ' -- ' : '') +
          'Not equal: actual <' +
          betterToString(actual) +
          '>, and expected <' +
          betterToString(expected) +
          '>',
      );
    }
  },
  _isArrayEqual: function(a, b) {
    if (a.length !== b.length)
      return false;
    for (let i = 0; i < a.length; i++) {
      if (!assert._isEqual(a[i], b[i]))
        return false;
    }
    return true;
  },
  arrayEqual: function(actual, expected, msg) {
    if (!assert._isArrayEqual(actual, expected)) {
      assert.fail(
        (msg ? msg + ' -- ' : '') +
          'Array not equal: actual <' +
          betterToString(actual) +
          '>, and expected <' +
          betterToString(expected) +
          '>',
      );
    }
  },
  deepEqual: function(actual, expected, msg) {
    if (Array.isArray(actual)) {
      msg = (msg ? msg + ' -- ' : '');
      assert.equal(Array.isArray(expected), true, msg + 'length');
      assert.equal(actual.length, expected.length, msg + 'length');
      for (let i = 0; i < actual.length; i++) {
        assert.deepEqual(actual[i], expected[i], 'values');
      }
      assert.deepEqual(Object.getPrototypeOf(actual),
                       Object.getPrototypeOf(expected),
                       msg + 'prototype');
      return;
    }
    if (typeof actual === 'object') {
      assert.equal(typeof expected, 'object', msg);
      if (actual === null) {
        assert.equal(expected, null, msg);
        return;
      }
      assert.notEqual(expected, null, msg);
      msg = (msg ? msg + ' -- ' : '');
      assert.arrayEqual(Object.getOwnPropertyNames(expected).sort(),
                        Object.getOwnPropertyNames(actual).sort(),
                        msg + 'names');
      assert.arrayEqual(Object.getOwnPropertySymbols(expected),
                        Object.getOwnPropertySymbols(actual),
                        msg + 'symbols');

      for (p of Object.getOwnPropertyNames(expected)) {
        assert.deepEqual(expected[p], actual[p],
                         msg + 'property ' + betterToString(p));
      }
      for (p of Object.getOwnPropertySymbols(expected)) {
        assert.deepEqual(expected[p], actual[p],
                         msg + 'property ' + betterToString(p));
      }
      assert.deepEqual(Object.getPrototypeOf(actual),
                       Object.getPrototypeOf(expected),
                       msg);
      return;
    }
    assert.equal(actual, expected, msg);
  },
  notEqual: function(actual, expected, msg) {
    if (assert._isEqual(actual, expected)) {
      assert.fail(
        (msg ? msg + ' -- ' : '') +
          'Equal: actual <' +
          betterToString(actual) +
          '>, and expected <' +
          betterToString(expected) +
          '>',
      );
    }
  },
  ok: function(value, msg) {
    assert.equal(!!value, true, msg);
  },
  throws: function(block, error, msg) {
    try {
      block();
    } catch (e) {
      assert.equal(e.constructor, error, e.message + ' ' + msg);
      return;
    }
    // Can't put fail inside the try because it will catch the AssertionError.
    assert.fail((msg ? msg + ' -- ' : '') + 'Failed to throw');
  },
  fail: function(msg) {
    throw new Error('AssertionError: ' + (msg ? msg : 'Failed'));
  },
};

function spyTraps(output) {
  return {
    has(target, key) {
      output.push("has:" + betterToString(key));
      return key in target;
    },
    get(target, key) {
      output.push("get:" + betterToString(key));
      return target[key];
    },
    set(target, key, value) {
      output.push("set:" + betterToString(key));
      target[key] = value;
      return true;
    },
    defineProperty(target, key, desc) {
      output.push("defineProperty:" + betterToString(key));
      return Reflect.defineProperty(target, key, desc);
    },
    deleteProperty(target, key) {
      output.push("delete:" + betterToString(key));
      delete target[key];
      return true;
    },
    ownKeys(target) {
      output.push("ownKeys");
      return Object.getOwnPropertyNames(target).concat(
        Object.getOwnPropertySymbols(target));
    },
    getOwnPropertyDescriptor(target, key) {
      output.push("getOwnPropertyDescriptor:" + betterToString(key));
      return Object.getOwnPropertyDescriptor(target, key);
    },
  };
};

print('apply');
// CHECK-LABEL: apply

assert.arrayEqual(
  Reflect.apply(Array.prototype.concat, [10,20,30], [[40,50], [60,70]]),
  [10,20,30,40,50,60,70]);

assert.throws(
  _ => Reflect.apply(assert.fail, undefined, []),
  Error);

print('construct');
// CHECK-LABEL: construct

var a = Reflect.construct(Array, [10, 20, 30]);
assert.equal(a instanceof Array, true);
assert.arrayEqual(a, [10, 20, 30]);

var s = (new Date()).valueOf();
var x = Reflect.construct(Date, [s], RegExp);
assert.equal(x instanceof RegExp, true);
assert.equal(Object.prototype.toString.apply(x), '[object Date]');
assert.equal(Date.prototype.valueOf.apply(x), s);

assert.throws(
  _ => Reflect.construct(assert.fail, []),
  Error);

var nonobjProto = function() {};
nonobjProto.prototype = 1;
assert.equal(Reflect.construct(nonobjProto, []) instanceof Object, true);
assert.equal(Reflect.construct(Date, [], nonobjProto) instanceof Date, true);

print('defineProperty');
// CHECK-LABEL: defineProperty

var o = {};
assert.equal(
  Reflect.defineProperty(o, 'a', {value:1}),
  true);
assert.equal(o.a, 1);

assert.equal(
  Reflect.defineProperty(o, 'a', {value:2}),
  false);
assert.equal(o.a, 1);

var o = Object.preventExtensions({})
assert.equal(
  Reflect.defineProperty(o, 'a', {value:3}),
  false);
assert.equal('a' in o, false);

assert.throws(
  _ => Reflect.defineProperty(1, 'a', {value:3}),
  TypeError);

print('deleteProperty');
// CHECK-LABEL: deleteProperty

var o = {a:1};
assert.equal(
  Reflect.deleteProperty(o, 'a'),
  true);
assert.equal('a' in o, false);

assert.equal(
  Reflect.deleteProperty(o, 'a'),
  true);
assert.equal('a' in o, false);

var o = Object.defineProperty({}, 'a', {value:1});
assert.equal(
  Reflect.deleteProperty(o, 'a'),
  false);
assert.equal('a' in o, true);

assert.throws(
  _ => Reflect.deleteProperty(1, 'a', {value:3}),
  TypeError);

print('get');
// CHECK-LABEL: get

var o = {a:1, get b() { return this; }};
assert.equal(
  Reflect.get(o, 'a'),
  1);
assert.equal(
  Reflect.get(o, 'b'),
  o);
assert.equal(
  Reflect.get(o, 'c'),
  undefined);
assert.throws(
  _ => Reflect.get(1, 'd'),
  TypeError);

assert.equal(
  Reflect.get(o, 'a', 2),
  1);

var r = Reflect.get(o, 'b', 2);
assert.equal(typeof r, 'object');
assert.equal(Number(r), 2);
assert.equal(
  Reflect.get(o, 'c', 2),
  undefined);

var o = new Proxy({}, {
  get(t, k, r) {
    if (k === 'e') {
      return 5;
    } else if (k === 'f') {
      return r;
    } else {
      return t[k];
    }
  }
});
assert.equal(Reflect.get(o, 'e', 3),
             5);
assert.equal(Reflect.get(o, 'f', 3),
             3);

var output = [];
var o = new Proxy({g:7}, spyTraps(output));
assert.equal(Reflect.get(o, 'g'),
             7);
assert.arrayEqual(output, ['get:g']);

print('getOwnPropertyDescriptor');
// CHECK-LABEL: getOwnPropertyDescriptor

var o = Object.defineProperty({a:1}, 'b', {value:2});
assert.deepEqual(Reflect.getOwnPropertyDescriptor(o, 'a'),
                 {value:1, writable:true, configurable:true, enumerable:true});
assert.deepEqual(Reflect.getOwnPropertyDescriptor(o, 'b'),
                 {value:2, writable:false, configurable:false, enumerable:false});
assert.equal(Reflect.getOwnPropertyDescriptor(o, 'c'),
             undefined);
assert.throws(_ => Reflect.getOwnPropertyDescriptor(1, 'd'),
              TypeError);

print('getPrototypeOf');
// CHECK-LABEL: getPrototypeOf

var o = {};
assert.equal(Reflect.getPrototypeOf(o),
             Object.prototype);
assert.equal(Reflect.getPrototypeOf(Object.create(o)),
             o);
assert.throws(_ => Reflect.getPrototypeOf(1),
              TypeError);

print('has');
// CHECK-LABEL: has

var o = Object.create({a:1});
o.b = 2;
assert.equal(Reflect.has(o, 'a'),
             true);
assert.equal(Reflect.has(o, 'b'),
             true);
assert.equal(Reflect.has(o, 'c'),
             false);
assert.throws(_ => Reflect.has(1, 'd'),
             TypeError);

print('isExtensible');
// CHECK-LABEL: isExtensible
var o = {};
assert.equal(Reflect.isExtensible(o),
             true);
var o = Object.preventExtensions({});
assert.equal(Reflect.isExtensible(o),
             false);
assert.throws(_ => Reflect.isExtensible(1),
             TypeError);

print('ownKeys');
// CHECK-LABEL: ownKeys

var o = Object.create({a:1});
o.b = 2;
o[Symbol.for('c')] = 3;
Object.defineProperty(o, 'd', {value:4});
assert.arrayEqual(Reflect.ownKeys(o),
                  ['b', 'd', Symbol.for('c')])
assert.throws(_ => Reflect.ownKeys(1),
             TypeError);

print('preventExtensions');
// CHECK-LABEL: preventExtensions

var o = {};
assert.equal(Reflect.set(o, 'a', 1), true);
assert.equal(Reflect.preventExtensions(o),
             true);
assert.equal(Reflect.set(o, 'a', 11), true);
assert.equal(Reflect.set(o, 'b', 2), false);
assert.equal(Reflect.preventExtensions(o),
             true);
assert.throws(_ => Reflect.preventExtensions(1),
             TypeError);

print('set');
// CHECK-LABEL: set

var o = {};
assert.equal(
  Reflect.set(o, 'a', 1),
  true);
assert.equal(o.a, 1);

assert.equal(
  Reflect.set(o, 'a', 2),
  true);
assert.equal(o.a, 2);

Object.defineProperty(o, 'c', {value:3})
assert.equal(
  Reflect.set(o, 'c', 33),
  false);
assert.equal(o.c, 3);

var o = Object.preventExtensions({});
assert.equal(
  Reflect.set(o, 'a', 1),
  false);
assert.equal('c' in o, false);

assert.throws(
  _ => Reflect.set(1, 'a', 3),
  TypeError);

var o = {};
var r = {};
assert.equal(
  Reflect.set(o, 'a', 1, r),
  true);
assert.equal('a' in o, false);
assert.equal(r.a, 1);

assert.equal(
  Reflect.set(o, 'a', 2, r),
  true);
assert.equal('a' in o, false);
assert.equal(r.a, 2);

Object.defineProperty(o, 'c', {value:3})
assert.equal(
  Reflect.set(o, 'c', 33, r),
  false);
assert.equal(o.c, 3);
assert.equal('c' in r, false);

var output = [];
var o = new Proxy({}, spyTraps(output));
assert.equal(Reflect.set(o, 'a', 1),
             true);
assert.arrayEqual(output, ['set:a']);
assert.equal(o.a, 1);

var o = {a:1};
var rTraps = [];
var r = new Proxy({a:2}, spyTraps(rTraps));
assert.equal(Reflect.set(o, 'a', 3, r),
             true);
assert.arrayEqual(rTraps, [
  'getOwnPropertyDescriptor:a',
  'defineProperty:a']);
assert.equal(o.a, 1);
assert.equal(r.a, 3);

var o = Object.freeze({a:1});
var rTraps = [];
var r = new Proxy({a:2}, spyTraps(rTraps));
assert.equal(Reflect.set(o, 'a', 3, r),
             false);
assert.arrayEqual(rTraps, []);
assert.equal(o.a, 1);
assert.equal(r.a, 2);

// internalSetter on object but not receiver, sets on receiver
var o = {};
Reflect.set(Object.create([]),"length",10,o);
assert.equal(o.length, 10);

print('setPrototypeOf');
// CHECK-LABEL: setPrototypeOf

var proto = {};
var o = {};
assert.equal(Reflect.setPrototypeOf(o, proto),
             true);
assert.equal(Reflect.getPrototypeOf(o),
             proto);
assert.throws(_ => Reflect.setPrototypeOf(o, 1),
             TypeError);

assert.equal(Reflect.preventExtensions(o),
             true);
assert.equal(Reflect.setPrototypeOf(o, {}),
             false);
assert.throws(_ => Reflect.setPrototypeOf(1, proto),
             TypeError);

print('Symbol.toStringTag');
// CHECK-LABEL: Symbol.toStringTag

assert.equal(Reflect[Symbol.toStringTag], 'Reflect');

print('done');
// CHECK-LABEL: done
