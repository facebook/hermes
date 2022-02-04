/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods -Xes6-proxy -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

let isStrictMode = (function() { return this === undefined; })();

function betterToString(value) {
  if (Array.isArray(value)) {
    return('[' +
           ((typeof HermesInternal === 'object' && HermesInternal.isProxy(value))
            ? "Proxy:" : "") +
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
      assert.equal(Array.isArray(expected), true, msg + "length");
      assert.equal(actual.length, expected.length, msg + "length");
      for (let i = 0; i < actual.length; i++) {
        assert.deepEqual(actual[i], expected[i], "values");
      }
      assert.deepEqual(Object.getPrototypeOf(actual),
                       Object.getPrototypeOf(expected),
                       msg + "prototype");
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
      assert.arrayEqual(Object.getOwnPropertyNames(actual).sort(),
                        Object.getOwnPropertyNames(expected).sort(),
                        msg + "names");
      assert.arrayEqual(Object.getOwnPropertySymbols(actual),
                        Object.getOwnPropertySymbols(expected),
                        msg + "symbols");

      for (p of Object.getOwnPropertyNames(expected)) {
        assert.deepEqual(actual[p], expected[p],
                         msg + "property " + betterToString(p));
      }
      for (p of Object.getOwnPropertySymbols(expected)) {
        assert.deepEqual(actual[p], expected[p],
                         msg + "property " + betterToString(p));
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

var traps = [];
var currentProxy = undefined;

function trapReturns(trapName, checkArgs, ret, target) {
  let handler = {};
  let proxy = new Proxy(target || {}, handler);
  handler[trapName] = function trap(...trapArgs) {
    print(trapName + ' trap (returns)');
    traps.push(trapName);
    assert.equal(this, handler);
    assert.equal(trapArgs[0], target);
    checkArgs(_ => trapArgs.slice(1));
    return ret;
  };
  return proxy;
}

function TrapError(message) {
  this.message = message || '';
}

TrapError.prototype.toString = function () {
  return 'TrapError: ' + this.message;
};

function trapThrows(trap) {
  function throwTarget() {}
  return new Proxy(throwTarget, {
    [trap]: function() {
      print(trap + ' trap (throws)');
      traps.push(trap);
      throw new TrapError(trap);
    }
  });
}

function checkValue(value) {
  return function checkValue(func, msg) {
    assert.equal(func(), value, msg);
  };
}

function checkArray(arr, alter = _ => _) {
  return function checkArray(func, msg) {
    assert.arrayEqual(alter(func()), arr, msg);
  };
}

function checkDeep(expected) {
  return function checkDeep(func, msg) {
    assert.deepEqual(func(), expected, msg);
  }
}

function checkElements(...checkers) {
  return function checkElements(func, msg) {
    let actual = func();
    assert.equal(actual.length, checkers.length);
    for (let i = 0; i < actual.length; ++i) {
      checkers[i](_ => actual[i], msg);
    }
  };
}

function checkDesc(expected) {
  return function checkDesc(func, msg) {
    let actual = func();
    if (expected === undefined && actual == undefined) {
      return;
    }
    assert.notEqual(expected, undefined);
    assert.notEqual(actual, undefined);
    for (let p of ['configurable', 'enumerable', 'value', 'writable', 'get', 'set']) {
      assert.equal(expected[p], actual[p], msg ? (msg + ' ' + p) : 'for ' + p);
    }
  };
}

function checkThrows(ex) {
  return function checkThrows(func, msg) {
    assert.throws(func, ex, msg);
  };
}

function checkIf(pred) {
  return function checkIf(func, msg) {
    assert.equal(pred(func()), true, msg);
  };
}

function checkStrictValue(value) {
  if (isStrictMode) {
    return checkThrows(TypeError);
  } else {
    return checkValue(value);
  }
}

function checkUnimplemented(ex) {
  return ((typeof HermesInternal === 'object')
          ? checkThrows(TypeError)
          : checkDeep(ex));
}

function checkTraps(arr) {
  return function checkTraps(func, msg) {
    assert.arrayEqual(traps, arr, msg);
  };
}

function checkUnimplementedTraps(ex) {
  return ((typeof HermesInternal === 'object')
          ? checkArray([])
          : checkArray(ex));
}

function checkProxy() {
  return function checkProxy(func, msg) {
    assert.equal(func(), currentProxy, msg);
  };
}

function checkBoxed(value) {
  return function checkBoxed(func, msg) {
    let actual = func();
    if (actual.constructor === Number) {
      assert.equal(Number(actual), value);
    } else if (actual.constructor === Boolean) {
      assert.equal(Boolean(actual), value);
    } else {
      assert.equal(actual, value);
    }
  }
}

function checkAll(...funcs) {
  return function checkAll(func, msg) {
    let actual = func();
    for (let f of funcs) {
      f(_ => actual, msg);
    }
  };
}

// This function is used to describe a battery of tests for a
// particular trap.  For each test:
// * a target is created by calling targetFactory
// * a Proxy is created using that target
// * func is called on the proxy and target
// * the result of calling the function is checked.
//
// a checker is a function which takes a function to call, calls it,
// and asserts if it did something unexpected.  The checker can check
// the return value, if an exception is thrown, etc.  The Function to
// call calls func on an appropriately constructed proxy as below.
//
// This happens several times.
// A. the noTrapChecker is called on a Proxy with no traps.
// B. for each of a list of [value, checker], the checker is called
//   on a Proxy with a trap which returns value.  The first argument to the trap
//   is checked against target, and the rest with checkArgs.
// C. a trap which throws a TrapError is checked that the exception
//   makes it out to the caller.
// D. the noTrapChecker is called on a revocable Proxy with no traps.
//   The proxy is revoked, and the function is called again, expecting
//   a TypeError.
//
// For each test, there is also a check that the expected trap was
// called.
function proxyTests(trap, func, targetFactory, noTrapChecker, checkArgs,
                    trapCalls) {
  // A.
  let target = targetFactory();
  traps = [];
  currentProxy = new Proxy(target, {});
  try {
    noTrapChecker(_ => func(currentProxy, target),
                  'for noTrap');
  } catch (e) {
    e.message += ' for noTrap';
    throw e;
  }
  assert.arrayEqual(traps, [], 'for noTrap');
  // B.
  let i = 0;
  for (let [trapRet, checker, trapChecker] of trapCalls) {
    traps = [];
    let target = targetFactory();
    let msg = 'for trapRet ' + i + ' ' + betterToString(trapRet);
    try {
      currentProxy = trapReturns(trap, checkArgs, trapRet, target);
      checker(_ => func(currentProxy, target), msg);
    } catch (e) {
      e.message += ' ' + msg;
      throw e;
    }
    if (trapChecker) {
      trapChecker(_ => traps, msg);
    } else {
      assert.arrayEqual(traps, [trap], msg);
    }
    ++i;
  }
  // C.
  target = targetFactory();
  traps = [];
  currentProxy = trapThrows(trap);
  checkThrows(TrapError)(_ => func(currentProxy, target),
                         'for trapThrows');
  assert.arrayEqual(traps, [trap], 'for trapThrows');
  // D.
  target = targetFactory();
  traps = [];
  let pr = Proxy.revocable(target, {});
  currentProxy = pr.proxy;
  try {
    noTrapChecker(_ => func(currentProxy, target),
                  'for revocable');
  } catch (e) {
    e.message += ' for revocable';
    throw e;
  }
  pr.revoke();
   checkThrows(TypeError)(
    _ => func(currentProxy, target),
    ' for revoked');
  assert.arrayEqual(traps, [], 'for revoked');
  traps = [];
  currentProxy = undefined;
}

function restorePrototype(obj, func) {
  let save = Object.getPrototypeOf(obj);
  func();
  Object.setPrototypeOf(obj, save);
}

let base = {};
let obj = {a:1};

// end helpers and standard values

print('getPrototypeOf');
// CHECK-LABEL: getPrototypeOf

// extensible target with non-null parent
for (let func of [Object.getPrototypeOf, proxy => proxy.__proto__]) {
  proxyTests(
    // This is the trap we are testing.
    'getPrototypeOf',
    // This is the function
    func,
    _ => Object.create(base),
    checkValue(base),
    checkArray([]),
    [[base, checkValue(base)],
     [obj, checkValue(obj)],
     [17, checkThrows(TypeError)],
     [null, checkValue(null)]]);
}

proxyTests(
  'getPrototypeOf',
  proxy => base.isPrototypeOf(proxy),
  _ => Object.create(base),
  checkValue(true),
  checkArray([]),
  [[base, checkValue(true)],
   [obj, checkValue(false)],
   [17, checkThrows(TypeError)],
   [null, checkValue(false)]]);

// non-extensible target with non-null parent
proxyTests(
  'getPrototypeOf',
  Object.getPrototypeOf,
  _ => Object.preventExtensions(Object.create(base)),
  checkValue(base),
  checkArray([]),
  [[base, checkValue(base)],
   [obj, checkThrows(TypeError)],
   [17, checkThrows(TypeError)],
   [null, checkThrows(TypeError)]]);

// extensible target with null parent
proxyTests(
  'getPrototypeOf',
  Object.getPrototypeOf,
  _ => Object.create(null),
  checkValue(null),
  checkArray([]),
  [[base, checkValue(base)],
   [obj, checkValue(obj)],
   [17, checkThrows(TypeError)],
   [null, checkValue(null)]]);

// non-extensible target with null parent
proxyTests(
  'getPrototypeOf',
  Object.getPrototypeOf,
  _ => Object.preventExtensions(Object.create(null)),
  checkValue(null),
  checkArray([]),
  [[base, checkThrows(TypeError)],
   [obj, checkThrows(TypeError)],
   [17, checkThrows(TypeError)],
   [null, checkValue(null)]]);

let ctorProto = {};
function Ctor() { return this; };
Ctor.prototype = ctorProto;
let ctorObj = new Ctor();

proxyTests(
  'getPrototypeOf',
  proxy => proxy instanceof Ctor,
  _ => ctorObj,
  checkValue(true),
  checkArray([]),
  [[ctorObj, checkValue(true)],
   [ctorProto, checkValue(true)],
   [{}, checkValue(false)],
   [17, checkThrows(TypeError)]]);

print('setPrototypeOf');
// CHECK-LABEL: setPrototypeOf

for (let func of [proxy => assert.equal(Object.setPrototypeOf(proxy, base), proxy),
                  proxy => proxy.__proto__ = base]) {
  proxyTests(
    'setPrototypeOf',
    function (proxy, target) {
      func(proxy);
      let a = Object.getPrototypeOf(target);
      let b = Object.getPrototypeOf(proxy);
      assert.equal(a, b);
      return a;
    },
    _ => ({}),
    checkValue(base),
    checkArray([base]),
    [[true, checkValue(Object.prototype)],
     [false, checkThrows(TypeError)]]);
}

print('isExtensible');
// CHECK-LABEL: isExtensible

// extensible object
proxyTests(
  'isExtensible',
  Object.isExtensible,
  _ => ({}),
  checkValue(true),
  checkArray([]),
  [[true, checkValue(true)],
   [false, checkThrows(TypeError)]]);

// non-extensible object
proxyTests(
  'isExtensible',
  Object.isExtensible,
  _ => Object.preventExtensions({}),
  checkValue(false),
  checkArray([]),
  [[false, checkValue(false)],
   [true, checkThrows(TypeError)]]);

print('preventExtensions');
// CHECK-LABEL: preventExtensions

// extensible object
proxyTests(
  'preventExtensions',
  function (proxy, target) {
    assert.equal(Object.preventExtensions(proxy), proxy);
    let a = Object.isExtensible(target);
    let b = Object.isExtensible(proxy);
    assert.equal(a, b);
    return !a;
  },
  _ => ({}),
  checkValue(true),
  checkArray([]),
  [[true, checkThrows(TypeError)],
   [false, checkThrows(TypeError)]]);

// non-extensible object
proxyTests(
  'preventExtensions',
  function (proxy, target) {
    assert.equal(Object.preventExtensions(proxy), proxy);
    let a = Object.isExtensible(target);
    let b = Object.isExtensible(proxy);
    assert.equal(a, b);
    return !a;
  },
  _ => Object.preventExtensions({}),
  checkValue(true),
  checkArray([]),
  [[true, checkValue(true)],
   [false, checkThrows(TypeError)]]);

print('getOwnPropertyDescriptor');
// CHECK-LABEL: getOwnPropertyDescriptor

// writable value prop
function getter() { return 'foo'; }
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => Object.getOwnPropertyDescriptor(proxy, 'prop'),
  _ => ({prop: 1}),
  checkDesc({configurable: true, enumerable: true, writable: true, value: 1}),
  checkArray(['prop']),
  [[2,
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, writable: true, value: 3},
    checkDesc({configurable: true, enumerable: true, writable: true, value: 3})],
   [{configurable: true, enumerable: true, writable: false, value: 4},
    checkDesc({configurable: true, enumerable: true, writable: false, value: 4})],
   [{configurable: false, enumerable: true, writable: false, value: 5},
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, get: getter},
    checkDesc({configurable: true, enumerable: true, get: getter})]]);

// writable accessor prop
function getfoo() { return 'foo'; }
function getbar() { return 'bar'; }
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => Object.getOwnPropertyDescriptor(proxy, 'prop'),
  _ => Object.create(Object.prototype,
                     {prop: {configurable: true, enumerable: true, get: getfoo}}),
  checkDesc({configurable: true, enumerable: true, get: getfoo}),
  checkArray(['prop']),
  [[2,
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, writable: true, value: 3},
    checkDesc({configurable: true, enumerable: true, writable: true, value: 3})],
   [{configurable: true, enumerable: true, writable: false, value: 4},
    checkDesc({configurable: true, enumerable: true, writable: false, value: 4})],
   [{configurable: false, enumerable: true, writable: false, value: 5},
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, get: getbar},
    checkDesc({configurable: true, enumerable: true, get: getbar})]]);

// non-configurable, non-writable value prop
proxyTests(
  'getOwnPropertyDescriptor',
  function (proxy) {
    return Object.getOwnPropertyDescriptor(proxy, 'prop');
  },
  _ => Object.freeze({prop: 1}),
  checkDesc({configurable: false, enumerable: true, writable: false, value: 1}),
  checkArray(['prop']),
  [[2,
    checkThrows(TypeError)],
   [{configurable: false, enumerable: true, writable: true, value: 1},
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, writable: false, value: 1},
    checkThrows(TypeError)],
   [{configurable: false, enumerable: true, writable: false, value: 4},
    checkThrows(TypeError)],
   [{configurable: false, enumerable: true, get: getter},
    checkThrows(TypeError)],
   [{configurable: false, enumerable: true, writable: false, value: 1},
    checkDesc({configurable: false, enumerable: true, writable: false, value: 1})]]);

// no prop
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => Object.getOwnPropertyDescriptor(proxy, 'prop'),
  _ => ({}),
  checkValue(undefined),
  checkArray(['prop']),
  [[2,
    checkThrows(TypeError)],
   [{configurable: false, enumerable: true, writable: true, value: 1},
    checkThrows(TypeError)],
   [{configurable: true, enumerable: true, writable: false, value: 2},
    checkDesc({configurable: true, enumerable: true, writable: false, value: 2})],
   [{configurable: true, enumerable: true, writable: true, value: 3},
    checkDesc({configurable: true, enumerable: true, writable: true, value: 3})],
   [{configurable: true, enumerable: true, get: getter},
    checkDesc({configurable: true, enumerable: true, get: getter})]]);

// hasOwnProperty true.  doesn't call 'has' trap
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.hasOwnProperty('prop'),
  _ => ({prop: 1}),
  checkValue(true),
  checkArray(['prop']),
  [[{configurable: true, enumerable: true, writable: true, value: 1},
    checkValue(true)],
   [{configurable: true, enumerable: false, writable: true, value: 1},
    checkValue(true)],
   [undefined,
    checkValue(false)]]);

// hasOwnProperty false.  doesn't call 'has' trap
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.hasOwnProperty('prop'),
  _ => ({}),
  checkValue(false),
  checkArray(['prop']),
  [[{configurable: true, enumerable: true, writable: true, value: 1},
    checkValue(true)],
   [{configurable: true, enumerable: false, writable: true, value: 1},
    checkValue(true)],
   [undefined,
    checkValue(false)]]);

proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.propertyIsEnumerable('prop'),
  _ => ({}),
  checkValue(false),
  checkArray(['prop']),
  [[{configurable: true, enumerable: true, writable: true, value: 1},
    checkValue(true)],
   [{configurable: true, enumerable: false, writable: true, value: 1},
    checkValue(false)],
   [undefined,
    checkValue(false)]]);

proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.propertyIsEnumerable('prop'),
  _ => Object.create(Object.prototype,
                     {prop: {configurable: true, enumerable: false,
                             writeable: true, value: 1}}),
  checkValue(false),
  checkArray(['prop']),
  [[{configurable: true, enumerable: true, writable: true, value: 1},
    checkValue(true)],
   [{configurable: true, enumerable: false, writable: true, value: 1},
    checkValue(false)],
   [undefined,
    checkValue(false)]]);

function setfoo() {}
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.__lookupGetter__('prop'),
  _ => Object.create(Object.prototype, {prop: {
    configurable: true,
    enumerable: true,
    get: getfoo,
  }}),
  checkValue(getfoo),
  checkArray(['prop']),
  [[{configurable: true, enumerable: true, writable: true, value: 3},
    checkValue(undefined)],
   [{configurable: true, enumerable: true, get: getbar},
    checkValue(getbar)]]);

// Function.prototype.bind uses getOwnPropertyDescriptor trap and get
// to determine length (see ES9 19.2.3.2)
proxyTests(
  'getOwnPropertyDescriptor',
  proxy => proxy.bind(obj).length,
  _ => function(one, two, three) {},
  checkValue(3),
  checkArray(['length']),
  []);

// Try above test with lazy and non-lazy functions

print('defineProperty');
// CHECK-LABEL: defineProperty

// add new property
var desc = {
  configurable: true,
  enumerable: true,
  writable: true,
  value: 1,
};
proxyTests(
  'defineProperty',
  function (proxy, target) {
    let ret = Object.defineProperty(proxy, 'prop', desc);
    assert.equal(ret, proxy);
    let a = Object.getOwnPropertyDescriptor(target, 'prop');
    let b = Object.getOwnPropertyDescriptor(proxy, 'prop');
    if (a !== undefined || b !== undefined) {
      checkDesc(a)(_ => b);
    }
    return a;
  },
  _ => ({}),
  checkDesc(desc),
  checkElements(
    checkValue('prop'),
    checkDesc(desc)),
  [[true, checkValue(undefined)],
   [false, checkThrows(TypeError)]]);

// modify existing property
proxyTests(
  'defineProperty',
  function (proxy, target) {
    let ret = Object.defineProperty(proxy, 'prop', desc);
    assert.equal(ret, proxy);
    let a = Object.getOwnPropertyDescriptor(target, 'prop');
    let b = Object.getOwnPropertyDescriptor(proxy, 'prop');
    if (a !== undefined || b !== undefined) {
      checkDesc(a)(_ => b);
    }
    return a;
  },
  _ => Object.create(Object.prototype, {prop: {
    configurable: true,
    enumerable: true,
    writable: false,
    value: 2,
  }}),
  checkDesc(desc),
  checkElements(
    checkValue('prop'),
    checkDesc(desc)),
  [[true,
    checkDesc({configurable: true, enumerable: true, writable: false, value: 2})],
   [false,
    checkThrows(TypeError)]]);

// proxy validation failure
var desc = {
  configurable: false,
  enumerable: true,
  writable: true,
  value: 1,
};
proxyTests(
  'defineProperty',
  function (proxy, target) {
    let ret = Object.defineProperty(proxy, 'prop', desc);
    assert.equal(ret, proxy);
    let a = Object.getOwnPropertyDescriptor(target, 'prop');
    let b = Object.getOwnPropertyDescriptor(proxy, 'prop');
    checkDesc(a)(_ => b);
    return a;
  },
  _ => Object.create(Object.prototype, {prop: {
    configurable: true,
    enumerable: true,
    writable: false,
    value: 2,
  }}),
  checkDesc(desc),
  checkElements(
    checkValue('prop'),
    checkDesc(desc)),
  [[true,
    checkThrows(TypeError)],
   [false,
    checkThrows(TypeError)]]);

print('has');
// CHECK-LABEL: has

proxyTests(
  'has',
  proxy => 'prop' in proxy,
  _ => ({prop:1}),
  checkValue(true),
  checkArray(['prop']),
  [[false, checkValue(false)],
   [true, checkValue(true)]]);

proxyTests(
  'has',
  proxy => 'prop' in proxy,
  _ => ({prop:1}),
  checkValue(true),
  checkArray(['prop']),
  [[false, checkValue(false)],
   [true, checkValue(true)]]);

proxyTests(
  'has',
  proxy => 'prop' in proxy,
  _ => ({}),
  checkValue(false),
  checkArray(['prop']),
  [[false, checkValue(false)],
   [true, checkValue(true)]]);

proxyTests(
  'has',
  proxy => 'prop' in proxy,
  _ => ({prop:1}),
  checkValue(true),
  checkArray(['prop']),
  [[false, checkValue(false)],
   [true, checkValue(true)]]);

proxyTests(
  'has',
  proxy => 'prop' in proxy,
  _ => Object.preventExtensions({prop:1}),
  checkValue(true),
  checkArray(['prop']),
  [[false, checkThrows(TypeError)],
   [true, checkValue(true)]]);

print('get');
// CHECK-LABEL: get

// Using computedProp as a key should guarantee 'getComputed' behavior.
var computedProp = 'prop';
for (let func of [proxy => proxy.prop,
                  proxy => proxy[computedProp]]) {
  // prop does not exist
  proxyTests(
    'get',
    func,
    _ => ({}),
    checkValue(undefined),
    checkElements(
      checkValue('prop'),
      checkProxy()),
    [[false, checkValue(false)],
     ['hello', checkValue('hello')]]);

  // prop exists
  proxyTests(
    'get',
    func,
    _ => ({prop:1}),
    checkValue(1),
    checkElements(
      checkValue('prop'),
      checkProxy()),
    [[false, checkValue(false)],
     ['hello', checkValue('hello')]]);

  // validation fail
  proxyTests(
    'get',
    func,
    _ => Object.create(
      Object.prototype,
      {prop: {configurable: false, enumerable: true, writable: false, value: 1}}),
    checkValue(1),
    checkElements(
      checkValue('prop'),
      checkProxy()),
    [[1, checkValue(1)],
     ['hello', checkThrows(TypeError)]]);
}

// parent is proxy
var child = {};
proxyTests(
  'get',
  function (proxy) {
    Object.setPrototypeOf(child, proxy);
    return child.prop;
  },
  _ => ({prop:1}),
  checkValue(1),
  checkElements(
    checkValue('prop'),
    checkValue(child)),
  [[false, checkValue(false)],
   ['hello', checkValue('hello')]]);

// numeric prop (trap gets a string)
proxyTests(
  'get',
  proxy => proxy[3],
  _ => [1,2,3,4,5],
  checkValue(4),
  checkElements(
    checkValue("3"),
    checkProxy()),
  [[false, checkValue(false)],
   ['hello', checkValue('hello')]]);

// symbol prop
proxyTests(
  'get',
  proxy => proxy[Symbol.for('symprop')],
  _ => ({[Symbol.for('symprop')]:1}),
  checkValue(1),
  checkElements(
    checkValue(Symbol.for('symprop')),
    checkProxy()),
  [[false, checkValue(false)],
   ['hello', checkValue('hello')]]);

// transient get
restorePrototype(Number.prototype, _ => proxyTests(
  'get',
  function (proxy, target) {
    Object.setPrototypeOf(Number.prototype, proxy);
    return (5).prop;
  },
  _ => ({}),
  checkValue(undefined),
  checkArray(['prop', 5]),
  [[false, checkValue(false)],
   ['hello', checkValue('hello')]]));

// transient get has prop
restorePrototype(Number.prototype, _ => proxyTests(
  'get',
  function (proxy, target) {
    Object.setPrototypeOf(Number.prototype, proxy);
    return (5).prop;
  },
  _ => ({prop:1}),
  checkValue(1),
  checkArray(['prop', 5]),
  [[false, checkValue(false)],
   ['hello', checkValue('hello')]]));

print('set');
// CHECK-LABEL: set

for (let func of [proxy => proxy.prop = 1,
                  proxy => proxy[computedProp] = 1]) {
  // set new property
  proxyTests(
    'set',
    function (proxy, target) {
      func(proxy);
      let a = target.prop;
      let b = proxy.prop;
      assert.equal(a, b);
      return a;
    },
    _ => ({}),
    checkValue(1),
    checkElements(
      checkValue('prop'),
      checkValue(1),
      checkProxy()),
    [[false, checkStrictValue(undefined)],
     [true, checkValue(undefined)]]);

  // overwrite existing property
  proxyTests(
    'set',
    function (proxy, target) {
      func(proxy);
      let a = target.prop;
      let b = proxy.prop;
      assert.equal(a, b);
      return a;
    },
    _ => ({prop:2}),
    checkValue(1),
    checkElements(
      checkValue('prop'),
      checkValue(1),
      checkProxy()),
    [[false, checkStrictValue(2)],
     [true, checkValue(2)]]);

  // validation fail
  proxyTests(
    'set',
    func,
    _ => Object.create(
      Object.prototype,
      {prop: {configurable: false, enumerable: true, writable: false, value: 2}}),
    checkStrictValue(1),
    checkElements(
      checkValue('prop'),
      checkValue(1),
      checkProxy()),
    [[false, checkStrictValue(1)],
     [true, checkThrows(TypeError)]]);
}

// parent is proxy
var child = {};
proxyTests(
  'set',
  function (proxy) {
    // child gets reused, so clean it up, if we modified it.
    delete child.prop;
    Object.setPrototypeOf(child, proxy);
    child.prop = 2;
    return child.prop;
  },
  _ => ({prop:1}),
  checkValue(2),
  checkElements(
    checkValue('prop'),
    checkValue(2),
    checkValue(child)),
  [[false, checkStrictValue(1)],
   [true, checkValue(1)]]);

// symbol prop
proxyTests(
  'set',
  function (proxy, target) {
    proxy[Symbol.for('symprop')] = 1;
    let a = target[Symbol.for('symprop')];
    let b = proxy[Symbol.for('symprop')];
    assert.equal(a, b);
    return a;
  },
  _ => ({[Symbol.for('symprop')]:2}),
  checkValue(1),
  checkElements(
    checkValue(Symbol.for('symprop')),
    checkValue(1),
    checkProxy()),
  [[false, checkStrictValue(2)],
   [true, checkValue(2)]]);

// transient set
restorePrototype(Number.prototype, _ => proxyTests(
  'set',
  function (proxy, target) {
    Object.setPrototypeOf(Number.prototype, proxy);
    (5).prop = 10;
  },
  _ => ({}),
  checkStrictValue(undefined),
  checkArray(['prop', 10, 5]),
  [[false, checkStrictValue(undefined)],
   [true, checkValue(undefined)]]));

print('deleteProperty');
// CHECK-LABEL: deleteProperty

for (let func of [proxy => delete proxy.prop,
                  proxy => delete proxy[computedProp]]) {
  // delete non-existent property
  proxyTests(
    'deleteProperty',
    function (proxy, target) {
      func(proxy);
      let a = target.prop;
      let b = proxy.prop;
      assert.equal(a, b);
      return a;
    },
    _ => ({}),
    checkValue(undefined),
    checkArray(['prop']),
    [[false, checkStrictValue(undefined)],
     [true, checkValue(undefined)]]);

  // delete existing property
  proxyTests(
    'deleteProperty',
    function (proxy, target) {
      func(proxy);
      let a = target.prop;
      let b = proxy.prop;
      assert.equal(a, b);
      return a;
    },
    _ => ({prop:2}),
    checkValue(undefined),
    checkArray(['prop']),
    [[false, checkStrictValue(2)],
     [true, checkValue(2)]]);

  // validation fail
  proxyTests(
    'deleteProperty',
    function (proxy, target) {
      func(proxy);
      let a = target.prop;
      let b = proxy.prop;
      assert.equal(a, b);
      return a;
    },
    _ => Object.create(
      Object.prototype,
      {prop: {configurable: false, enumerable: true, writable: false, value: 2}}),
    checkStrictValue(2),
    checkArray(['prop']),
    [[false, checkStrictValue(2)],
     [true, checkThrows(TypeError)]]);
}

print('ownKeys');
// CHECK-LABEL: ownKeys

proxyTests(
  'ownKeys',
  Object.getOwnPropertyNames,
  _ => ({a:1, b:2, [Symbol.for('c')]:3, [Symbol.for('d')]:4}),
  checkArray(['a', 'b']),
  checkArray([]),
  [[['e', Symbol.for('f'), 'g', Symbol.for('h')],
    checkArray(['e', 'g'])]]);

proxyTests(
  'ownKeys',
  Object.getOwnPropertySymbols,
  _ => ({a:1, b:2, [Symbol.for('c')]:3, [Symbol.for('d')]:4}),
  checkArray([Symbol.for('c'), Symbol.for('d')]),
  checkArray([]),
  [[['e', Symbol.for('f'), 'g', Symbol.for('h')],
    checkArray([Symbol.for('f'), Symbol.for('h')])]]);

// validation fail
proxyTests(
  'ownKeys',
  Object.getOwnPropertyNames,
  _ => Object.preventExtensions({a:1, b:2}),
  checkArray(['a', 'b']),
  checkArray([]),
  [[['a', 'b'],
    checkArray(['a', 'b'])],
   [['e', 'g'],
    checkThrows(TypeError)]]);

print('apply');
// CHECK-LABEL: apply

// call with no this specified
proxyTests(
  'apply',
  proxy => proxy(1),
  _ => function (a) { return [this, a + 2]; },
  checkArray([isStrictMode ? undefined : globalThis, 3]),
  checkDeep([undefined, [1]]),
  [[4, checkValue(4)],
   ['hello', checkValue('hello')]]);

// call throws exception
proxyTests(
  'apply',
  proxy => proxy(1),
  _ => function (a) { throw new Error("fail"); },
  checkThrows(Error),
  checkDeep([undefined, [1]]),
  [])

// specify undefined, null this
for (let testThis of [undefined, null]) {
  for (let func of [proxy => proxy.bind(testThis)(1),
                    proxy => proxy.call(testThis, 1)]) {
    proxyTests(
      'apply',
      func,
      _ => function (a) { return [this, a + 2]; },
      checkArray([isStrictMode ? testThis : globalThis, 3]),
      checkElements(
        checkValue(testThis),
        checkArray([1])),
      [[4, checkValue(4)],
       ['hello', checkValue('hello')]]);
  }
}

// specify some this objects
for (let testThis of [obj, [1,2,3], _ => 4]) {
  for (let func of [proxy => proxy.bind(testThis)(1),
                    proxy => proxy.call(testThis, 1)]) {
    proxyTests(
      'apply',
      func,
      _ => function (a) { return [this, a + 2]; },
      checkArray([testThis, 3]),
      checkElements(
        checkValue(testThis),
        checkArray([1])),
      [[4, checkValue(4)],
       ['hello', checkValue('hello')]]);
  }
}

// Specify some this primitives (they get boxed)
for (let testThis of [5, true]) {
  for (let func of [proxy => proxy.bind(testThis)(1),
                    proxy => proxy.call(testThis, 1)]) {
    proxyTests(
      'apply',
      func,
      _ => function (a) { return [this, a + 2]; },
      checkElements(
        checkBoxed(testThis),
        checkValue(3)),
      checkElements(
        checkValue(testThis),
        checkArray([1])),
      [[4, checkValue(4)],
       ['hello', checkValue('hello')]]);
  }
}

print('construct');
// CHECK-LABEL: construct

function testTargetCtor (a) { this.a = a; return this; }
function testCtor1 (a) { return this; }
function testCtor2 (a) { this.a = 2; return this; }
var x = new Ctor();
print(x, x.constructor);
proxyTests(
  'construct',
  function (proxy) {
    let o = new proxy(1);
    return [o.constructor, o.a];
  },
  _ => testTargetCtor,
  checkArray([testTargetCtor, 1]),
  checkElements(
    checkArray([1]),
    checkProxy()),
  [[new testCtor1(),
    checkArray([testCtor1, undefined])],
   [new testCtor2(),
    checkArray([testCtor2, 2])],
   ['hello',
    checkThrows(TypeError)]]);

// test target is not a ctor.  Note that the trap isn't called, so
// proxyTests's assumptions would fail.
var p = new Proxy(_ => _, {});
assert.throws(_ => new p(), TypeError);
var p = new Proxy(_ => _, { construct() { assert.fail("trap called"); }});
assert.throws(_ => new p(), TypeError);

print('ProxyCreate');
// CHECK-LABEL: ProxyCreate

for (let val of [undefined, null, true, 17, "string"]) {
  assert.throws(_ => new Proxy(val, {}), TypeError);
  assert.throws(_ => new Proxy({}, val), TypeError);
}

// Check that adding slots to the revoker function works as expected.

var pr = Proxy.revocable({}, {});
pr.revoke.prop = 1;
assert.equal(pr.revoke.prop, 1);
pr.revoke();
assert.equal(pr.revoke.prop, 1);

assert.throws(_=> pr.proxy.foo, TypeError);
assert.ok(_=> new Proxy(pr.proxy, {}), "ProxyCreate using revoked proxies should be allowed.");
assert.ok(_=> new Proxy({}, pr.proxy), "ProxyCreate using revoked proxies should be allowed.");

print('Array.isArray');
// CHECK-LABEL: Array.isArray

var a = [];
var ap = new Proxy(a, {});
var app = new Proxy(ap, {});
assert.equal(Array.isArray(app), true);

print('multitraps');
// CHECK-LABEL: multitraps

function spyTraps(output) {
  return {
    has(target, key) {
      output.push("has:" + betterToString(key));
      print(output[output.length - 1]);
      return key in target;
    },
    get(target, key) {
      output.push("get:" + betterToString(key));
      print(output[output.length - 1]);
      return target[key];
    },
    set(target, key, value) {
      output.push("set:" + betterToString(key));
      print(output[output.length - 1]);
      target[key] = value;
      return true;
    },
    deleteProperty(target, key) {
      output.push("delete:" + betterToString(key));
      print(output[output.length - 1]);
      delete target[key];
      return true;
    },
    ownKeys(target) {
      output.push("ownKeys");
      print(output[output.length - 1]);
      return Object.getOwnPropertyNames(target).concat(
        Object.getOwnPropertySymbols(target));
    },
    getOwnPropertyDescriptor(target, key) {
      output.push("getOwnPropertyDescriptor:" + betterToString(key));
      print(output[output.length - 1]);
      return Object.getOwnPropertyDescriptor(target, key);
    },
  };
};

var sourceParent = Object.defineProperties({}, {
  pa:{value:11, writable:true, configurable:true, enumerable: true},
  pb:{value:12, writable:true, configurable:true, enumerable: false},
  [Symbol.for('pc')]:{value:13, writable:true, configurable:true, enumerable: true},
  [Symbol.for('pd')]:{value:14, writable:true, configurable:true, enumerable: false},
});

var source = Object.create(sourceParent, {
  ca:{value:21, writable:true, configurable:true, enumerable: true},
  cb:{value:22, writable:true, configurable:true, enumerable: false},
  [Symbol.for('cc')]:{value:23, writable:true, configurable:true, enumerable: true},
  [Symbol.for('cd')]:{value:24, writable:true, configurable:true, enumerable: false},
});

var sourceArray = [10,20,30];

assert.equal(Object.getOwnPropertyNames(sourceParent).length, 2);
assert.equal(Object.getOwnPropertySymbols(sourceParent).length, 2);
assert.equal(Object.getOwnPropertyNames(source).length, 2);
assert.equal(Object.getOwnPropertySymbols(source).length, 2);

function multiTests(source, func, checkResult, checkTraps) {
  checkResult(_ => func(source), "for source");

  var output = [];
  checkResult(_ => func(new Proxy(source, spyTraps(output))), "for proxy");
  checkTraps(_ => output);
}

// own

multiTests(
  source,
  Object.getOwnPropertyDescriptors,
  checkDeep({
    ca:{value:21, writable:true, configurable:true, enumerable: true},
    cb:{value:22, writable:true, configurable:true, enumerable: false},
    [Symbol.for('cc')]:{value:23, writable:true, configurable:true, enumerable: true},
    [Symbol.for('cd')]:{value:24, writable:true, configurable:true, enumerable: false},
  }),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:ca',
    'getOwnPropertyDescriptor:cb',
    'getOwnPropertyDescriptor:Symbol(cc)',
    'getOwnPropertyDescriptor:Symbol(cd)']));

multiTests(
  sourceArray,
  Object.getOwnPropertyDescriptors,
  checkDeep({
    0:{value:10, writable:true, configurable:true, enumerable: true},
    1:{value:20, writable:true, configurable:true, enumerable: true},
    2:{value:30, writable:true, configurable:true, enumerable: true},
    length:{value:3, writable:true, configurable:false, enumerable: false},
  }),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:0',
    'getOwnPropertyDescriptor:1',
    'getOwnPropertyDescriptor:2',
    'getOwnPropertyDescriptor:length']));

// enumerable own
for (let func of [_ => Object.assign({}, _),
                  _ => ({..._})]) {
  multiTests(
    source,
    func,
    checkDeep({ca:21, [Symbol.for('cc')]:23}),
    checkArray(['ownKeys',
                'getOwnPropertyDescriptor:ca',
                'get:ca',
                'getOwnPropertyDescriptor:cb',
                'getOwnPropertyDescriptor:Symbol(cc)',
                'get:Symbol(cc)',
                'getOwnPropertyDescriptor:Symbol(cd)']));

  multiTests(
    sourceArray,
    func,
    checkDeep({0:10, 1:20, 2:30}),
    checkArray(['ownKeys',
                'getOwnPropertyDescriptor:0',
                'get:0',
                'getOwnPropertyDescriptor:1',
                'get:1',
                'getOwnPropertyDescriptor:2',
                'get:2',
                'getOwnPropertyDescriptor:length']));
}

var descriptors = Object.defineProperties({}, {
  pa: {
    value: Object.getOwnPropertyDescriptor(sourceParent, 'pa'),
    writable: true,
    configurable: true,
    enumerable: true,
  },
  pb: {
    value: Object.getOwnPropertyDescriptor(sourceParent, 'pb'),
    writable: true,
    configurable: true,
    enumerable: false,
  },
  ca: {
    value: Object.getOwnPropertyDescriptor(source, 'ca'),
    writable: true,
    configurable: true,
    enumerable: false,
  },
  cb: {
    value: Object.getOwnPropertyDescriptor(source, 'cb'),
    writable: true,
    configurable: true,
    enumerable: true,
  },
  x: {
    value: "i-am-not-a-descriptor",
    writable: true,
    configurable: true,
    enumerable: false, // and non-enumerable props aren't used descriptors
  },
});

multiTests(
  descriptors,
  descs => Object.defineProperties({}, descs),
  checkDeep({pa: 11, cb: 22}),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:pa',
    'get:pa',
    'getOwnPropertyDescriptor:pb',
    'getOwnPropertyDescriptor:ca',
    'getOwnPropertyDescriptor:cb',
    'get:cb',
    'getOwnPropertyDescriptor:x']));

// enumerable strings
multiTests(
  source,
  function(obj) {
    let keys = [];
    for (let key in obj) {
      keys.push(key);
    }
    return keys;
  },
  checkArray(['ca', 'pa']),
  checkArray(['ownKeys',
              'getOwnPropertyDescriptor:ca',
              'getOwnPropertyDescriptor:cb'],
             function(actual) {
               // d8 does this.  It's weird.  I don't know why.
               if (typeof HermesInternal !== 'object') {
                 if (actual[actual.length - 1] === 'getOwnPropertyDescriptor:pa') {
                   --actual.length;
                 }
               }
               return actual;
             }));

multiTests(
  sourceArray,
  function(obj) {
    let keys = [];
    for (let key in obj) {
      keys.push(key);
    }
    return keys;
  },
  checkArray(["0", '1', '2']),
  checkArray(['ownKeys',
              'getOwnPropertyDescriptor:0',
              'getOwnPropertyDescriptor:1',
              'getOwnPropertyDescriptor:2',
              'getOwnPropertyDescriptor:length']));

// iteration
multiTests(
  sourceArray,
  function(iterable) {
    let values = [];
    for (let value of iterable) {
      values.push(value);
    }
    return values;
  },
  checkArray([10,20,30]),
  checkArray([
    'get:Symbol(Symbol.iterator)',
    'get:length',
    'get:0',
    'get:length',
    'get:1',
    'get:length',
    'get:2',
    'get:length']));

// enumerable own strings
multiTests(
  source,
  Object.keys,
  checkArray(['ca']),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:ca',
    'getOwnPropertyDescriptor:cb']));

multiTests(
  source,
  Object.values,
  checkArray([21]),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:ca',
    'get:ca',
    'getOwnPropertyDescriptor:cb']));

multiTests(
  source,
  Object.entries,
  checkDeep([['ca', 21]]),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:ca',
    'get:ca',
    'getOwnPropertyDescriptor:cb']));

multiTests(
  source,
  JSON.stringify,
  checkValue('{"ca":21}'),
  checkArray([
    'get:toJSON',
    'ownKeys',
    'getOwnPropertyDescriptor:ca',
    'getOwnPropertyDescriptor:cb',
    'get:ca']));

multiTests(
  sourceArray,
  Object.keys,
  checkArray(['0', '1', '2']),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:0',
    'getOwnPropertyDescriptor:1',
    'getOwnPropertyDescriptor:2',
    'getOwnPropertyDescriptor:length']));

multiTests(
  sourceArray,
  Object.values,
  checkArray([10, 20, 30]),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:0',
    'get:0',
    'getOwnPropertyDescriptor:1',
    'get:1',
    'getOwnPropertyDescriptor:2',
    'get:2',
    'getOwnPropertyDescriptor:length']));

multiTests(
  sourceArray,
  Object.entries,
  checkDeep([
    ['0', 10],
    ['1', 20],
    ['2', 30],
  ]),
  checkArray([
    'ownKeys',
    'getOwnPropertyDescriptor:0',
    'get:0',
    'getOwnPropertyDescriptor:1',
    'get:1',
    'getOwnPropertyDescriptor:2',
    'get:2',
    'getOwnPropertyDescriptor:length']));

multiTests(
  sourceArray,
  JSON.stringify,
  checkValue('[10,20,30]'),
  checkArray([
    'get:toJSON',
    'get:length',
    'get:0',
    'get:1',
    'get:2']));

multiTests(
  sourceArray,
  source => [...source],
  checkArray([10,20,30]),
  checkArray([
    "get:Symbol(Symbol.iterator)",
    "get:length",
    "get:0",
    "get:length",
    "get:1",
    "get:length",
    "get:2",
    "get:length",
  ]));

// Spreading when [Symbol.iterator] is a proxy.
var spreadSource = [10,20,30];
multiTests(
  {},
  proto => {
    spreadSource.__proto__ = proto;
    try {
      return [...spreadSource];
    } catch (e) {
      return e.name;
    }
  },
  checkValue("TypeError"),
  checkArray(["get:Symbol(Symbol.iterator)"])
);

print('Array.prototype');
// CHECK-LABEL: Array.prototype

var arrayOne = [11,12];
// mind the gap
arrayOne[3] = 13;
var arrayTwo = [24,25,26,27];

function alterArrayTraps(actual) {
  // Hermes does not yet implement ArraySpeciesCreate, so we ignore
  // those traps in d8.
  if (typeof HermesInternal !== 'object') {
    return actual.filter(_ => _ !== 'get:constructor');
  }
  // We want to return a new array, so later calls don't append to the
  // traps we test.
  return actual.concat();
}

function arrayTests(func, checkResult, checkTraps) {
  checkResult(_ => func(arrayOne.concat(), arrayTwo.concat()),
              "for arrays");

  var output = [];
  var actual;
  try {
    actual = func(new Proxy(arrayOne.concat(), new spyTraps(output)),
                  new Proxy(arrayTwo.concat(), new spyTraps(output)));
  } catch (e) {
    checkResult(function() { throw e }, "for proxy exception");
    checkTraps(_ => output);
    return;
  }

  // If actual is an Array Iterator, expand it here, before copying output
  var copyOutput = alterArrayTraps(output);
  checkResult(_ => actual,
              "for proxies");
  checkTraps(_ => copyOutput);
}

function hasGetTraps(begin, end /* inclusive */) {
  var ret = [];
  for (var i = begin; i <= end; ++i) {
    ret.push("has:" + i);
    ret.push("get:" + i);
  }
  return ret;
}

var oneTraps = [
  'get:length',
  ...hasGetTraps(0, 1),
  'has:2',
  // get:2 is not called
  'has:3',
  'get:3',
];

arrayTests(
  (one, two) => Array.prototype.concat.call(one, two),
  checkArray([11,12,,13,24,25,26,27]),
  checkArray([
    'get:Symbol(Symbol.isConcatSpreadable)',
    ...oneTraps,
    'get:Symbol(Symbol.isConcatSpreadable)',
    'get:length',
    ...hasGetTraps(0, 3),
]));

arrayTests(
  one => Array.prototype.copyWithin.call(one, 2, 1, 3),
  checkArray([11,12,12,,]),
  checkArray([
    'get:length',
    'has:2',
    'delete:3',
    'has:1',
    'get:1',
    'set:2']));

arrayTests(
  one => Array.from(Array.prototype.entries.call(one)),
  checkDeep([[0,11], [1,12], [2, undefined], [3,13]]),
  checkArray([
    'get:length',
    'get:0',
    'get:length',
    'get:1',
    'get:length',
    'get:2',
    'get:length',
    'get:3',
    'get:length']));

arrayTests(
  one => Array.prototype.every.call(one, _ => _ % 2 == 1),
  checkValue(false),
  checkArray([
    'get:length',
    ...hasGetTraps(0, 1)]));

arrayTests(
  one => Array.prototype.fill.call(one, 99, 2),
  checkArray([11,12,99,99]),
  checkArray([
    'get:length',
    'set:2',
    'set:3']));

arrayTests(
  one => Array.prototype.filter.call(one, _ => _ % 2 == 1),
  checkArray([11,13]),
  checkArray(oneTraps));

arrayTests(
  one => Array.prototype.find.call(one, _ => _ % 2 == 0),
  checkValue(12),
  checkArray([
    'get:length',
    'get:0',
    'get:1']));

arrayTests(
  one => Array.prototype.findIndex.call(one, _ => _ % 2 == 0),
  checkValue(1),
  checkArray([
    'get:length',
    'get:0',
    'get:1']));

arrayTests(
  one => Array.prototype.findIndex.call(one, _ => _ > 99),
  checkValue(-1),
  checkArray([
    'get:length',
    'get:0',
    'get:1',
    'get:2',
    'get:3']));

arrayTests(
  one => Array.prototype.flatMap.call(one, _ => _ * 10),
  checkArray([110,120,130]),
  checkArray(oneTraps));

arrayTests(
  one => Array.prototype.forEach.call(one, _ => _ * 10),
  checkValue(undefined),
  checkArray(oneTraps));

arrayTests(
  one => Array.prototype.includes.call(one, 12),
  checkValue(true),
  checkArray([
    'get:length',
    'get:0',
    'get:1']));

arrayTests(
  one => Array.prototype.includes.call(one, 99),
  checkValue(false),
  checkArray([
    'get:length',
    'get:0',
    'get:1',
    'get:2',
    'get:3']));

arrayTests(
  one => Array.prototype.indexOf.call(one, 12),
  checkValue(1),
  checkArray([
    'get:length',
    'has:0',
    'get:0',
    'has:1',
    'get:1']));

arrayTests(
  one => Array.prototype.indexOf.call(one, 99),
  checkValue(-1),
  checkArray(oneTraps));

arrayTests(
  one => Array.prototype.join.call(one, ","),
  checkValue("11,12,,13"),
  checkArray([
    'get:length',
    'get:0',
    'get:1',
    'get:2',
    'get:3']));

arrayTests(
  one => Array.from(Array.prototype.keys.call(one)),
  checkArray([0,1,2,3]),
  checkArray([
    'get:length',
    'get:length',
    'get:length',
    'get:length',
    'get:length']));

arrayTests(
  one => Array.prototype.lastIndexOf.call(one, 12),
  checkValue(1),
  checkArray([
    'get:length',
    'has:3',
    'get:3',
    'has:2',
    'has:1',
    'get:1']));

arrayTests(
  one => Array.prototype.lastIndexOf.call(one, 99),
  checkValue(-1),
  checkArray([
    'get:length',
    'has:3',
    'get:3',
    'has:2',
    'has:1',
    'get:1',
    'has:0',
    'get:0']));

arrayTests(
  one => Array.prototype.map.call(one, _ => _ * 10),
  checkArray([110,120,,130]),
  checkArray(oneTraps));

arrayTests(
  function(one) {
    var ret = [];
    ret.push(Array.prototype.pop.call(one));
    ret.push(Array.prototype.pop.call(one));
    ret.push(one.length);
    return ret;
  },
  checkArray([13,undefined,2]),
  checkArray([
    'get:length',
    'get:3',
    'delete:3',
    'set:length',
    'get:length',
    'get:2',
    'delete:2',
    'set:length',
    'get:length']));

arrayTests(
  function(one) {
    var length = Array.prototype.push.call(one, 99);
    return [length, one];
  },
  checkDeep([5,[11,12,,13,99]]),
  checkArray([
    'get:length',
    'set:4',
    'set:length']));

arrayTests(
  one => Array.prototype.reduce.call(one, (a, b) => a + b),
  checkValue(36),
  checkArray(oneTraps));

arrayTests(
  one => Array.prototype.reduceRight.call(one, (a, b) => a + b),
  checkValue(36),
  checkArray([
    'get:length',
    'has:3',
    'get:3',
    'has:2',
    'has:1',
    'get:1',
    'has:0',
    'get:0']));

arrayTests(
  one => Array.prototype.reverse.call(one),
  checkArray([13,,12,11]),
  checkArray([
    'get:length',
    'has:0',
    'get:0',
    'has:3',
    'get:3',
    'set:0',
    'set:3',
    'has:1',
    'get:1',
    'has:2',
    'delete:1',
    'set:2']));

arrayTests(
  function(one) {
    var ret = [];
    ret.push(Array.prototype.shift.call(one));
    ret.push(Array.prototype.shift.call(one));
    ret.push(Array.prototype.shift.call(one));
    ret.push(one.length);
    return ret;
  },
  checkArray([11,12,undefined,1]),
  checkArray([
    'get:length',
    'get:0',
    'has:1',
    'get:1',
    'set:0',
    'has:2',
    'delete:1',
    'has:3',
    'get:3',
    'set:2',
    'delete:3',
    'set:length',

    'get:length',
    'get:0',
    'has:1',
    'delete:0',
    'has:2',
    'get:2',
    'set:1',
    'delete:2',
    'set:length',

    'get:length',
    'get:0',
    'has:1',
    'get:1',
    'set:0',
    'delete:1',
    'set:length',

    'get:length']));

arrayTests(
  one => Array.prototype.slice.call(one, 1, 3),
  checkArray([12,,]),
  checkArray([
    'get:length',
    'has:1',
    'get:1',
    'has:2']));

arrayTests(
  one => Array.prototype.some.call(one, _ => _ % 2 == 1),
  checkValue(true),
  checkArray([
    'get:length',
    'has:0',
    'get:0']));

arrayTests(
  one => Array.prototype.sort.call(one.reverse()),
  checkArray([11,12,13,,]),
  // sort's behavior is implementation-defined, so we just check
  // that any traps at all are called.
  checkIf(_ => _.length > 0));

arrayTests(
  function(one) {
    var ret = Array.prototype.splice.call(one, 1, 2, 98, 99);
    return [ret, one];
  },
  checkDeep([[12,,],[11,98,99,13]]),
  checkArray([
    'get:length',
    'has:1',
    'get:1',
    'has:2',
    'set:1',
    'set:2',
    'set:length']));

arrayTests(
  one => Array.prototype.toLocaleString.call(one),
  checkValue("11,12,,13"),
  checkArray([
    'get:length',
    'get:0',
    'get:1',
    'get:2',
    'get:3']));

arrayTests(
  one => Array.prototype.toString.call(one),
  checkValue("11,12,,13"),
  checkArray([
    'get:join',
    'get:length',
    'get:0',
    'get:1',
    'get:2',
    'get:3']));

arrayTests(
  function(one) {
    var ret = Array.prototype.unshift.call(one, 98, 99);
    return [ret, one];
  },
  checkDeep([6, [98,99,11,12,,13]]),
  checkArray([
    'get:length',
    'has:3',
    'get:3',
    'set:5',
    'has:2',
    'delete:4',
    'has:1',
    'get:1',
    'set:3',
    'has:0',
    'get:0',
    'set:2',
    'set:0',
    'set:1',
    'set:length']));

arrayTests(
  one => Array.from(Array.prototype.values.call(one)),
  checkArray([11,12,,13]),
  checkArray([
    'get:length',
    'get:0',
    'get:length',
    'get:1',
    'get:length',
    'get:2',
    'get:length',
    'get:3',
    'get:length']));

print('misc');
// CHECK-LABEL: misc

// Do a deep target recursion. This needs to be within the smallest
// (ASAN) limit for tests to pass, but larger numbers will work in
// production builds.
var p = {a:1};
for (var i = 0; i < 20; ++i) {
  p = new Proxy(p, {});
}
assert.equal(p.a, 1);

// Do a really deep target recursion to test for stack overflow
var p = {a:1};
for (var i = 0; i < 10000; ++i) {
  p = new Proxy({}, p);
}
checkThrows(RangeError)(_ => p.a);

// Do a really deep handler recursion to test for stack overflow
var p = {a:1};
for (var i = 0; i < 10000; ++i) {
  p = new Proxy(p, {});
}
checkThrows(RangeError)(_ => p.a);

// Do an infinite recursion to test for stack overflow
var p1 = [];
var p2 = new Proxy(p1, {});
p1.__proto__ = p2;
checkThrows(RangeError)(_ => p1.a);

// Test HermesInternal
assert.equal(
  typeof HermesInternal !== 'object' ||
    HermesInternal.isProxy(new Proxy({}, {})),
  true);

// spread of a callable
var f = function() { return 1; }
f.a = 1;
f.b = 2;
checkDeep({...f})(_ => ({a:1, b:2}));

// spread passes string not numeric arguments to traps
var output = [];
var p = new Proxy({1:""}, {
        getOwnPropertyDescriptor(t, k) {
            output.push(typeof k)
            return Object.getOwnPropertyDescriptor(t, k);
        },
        get(t, k) {
            output.push(typeof k)
            return t[k];
        }});
({...p});
assert.arrayEqual(output, ["string", "string"]);

// newTarget.prototype for Proxy ctor is !== Object.prototype does not throw
Reflect.construct(Proxy, [{}, {}], WeakSet);

// Check that defining a property in a Proxy target which is an array
// uses fast array access (this will trip an assert otherwise)
new Proxy([], {}).unshift(0);

// If putComputed is called on a proxy whose target's prototype is an
// array with a propname of 'length', then internalSetter will be
// true, and the receiver will be a proxy.  In that case, proxy needs
// to win; the behavior may assert or be UB otherwise.
var p = new Proxy(Object.create([]), {});
// using String() forces putComputed
p[String('length')] = 0x123;
p[0xABC] = 1111;

// Regression test for heavy handle allocation path
for (var b in new Proxy([], {
  ownKeys: Reflect.ownKeys,
  getOwnPropertyDescriptor: Reflect.getOwnPropertyDescriptor,
})) {}

// Test when the trap get revokes the proxy.
var {
  proxy,
  revoke
} = Proxy.revocable({}, new Proxy({}, {
  get(t, k, r) {
    revoke();
  }
}));
Object.getPrototypeOf(proxy);

var targetRan = false;

var {
  proxy,
  revoke
} = Proxy.revocable(
  function() { targetRan = true; },
  new Proxy({}, {
    get: function(t, k, r) {
      revoke();
    }
  }
));
proxy();

assert.ok(targetRan);

// This returns the proxy below as a descriptor.
var p1 = new Proxy({}, {
    getOwnPropertyDescriptor(t, p) { return p2; },
});

// Calling get on the Proxy p2 requires calling
// getOwnPropertyDescriptor on the target, p1 above, to check
// invariants.  This constructs a descriptor by getting
// properties of the trap return (p2 below).  Calling get
// on the proxy p2 is how we started, resulting in an infinite
// recursion.

var p2 = new Proxy(p1, {
    has(t, p) { return true; },
    get(t, p, r) { return {}; },
});

assert.throws(() => p2.foo, RangeError, "Maximum call stack size exceeded");

print('done');
// CHECK-LABEL: done
