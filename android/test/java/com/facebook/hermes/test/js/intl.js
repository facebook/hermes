/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

function assert(pred, str) {
  if (!pred) {
    throw new Error('assertion failed' + (str === undefined ? '' : (': ' + str)));
  }
}

// This is all required by the standard

function testServiceTypes(service) {
  assert(service !== undefined);
  assert(service.prototype.constructor === service);
  assert(service.__proto__ === Function.prototype);
  assert(service.prototype[Symbol.toStringTag] === 'Object');
  assert(service.supportedLocalesOf.__proto__ === Function.prototype);

  var s = service();
  assert(s.constructor === service);
  assert(s.__proto__ === service.prototype);

  var snew = new service();
  assert(s.constructor === service);
  assert(s.__proto__ === service.prototype);
}

function testServiceGetterTypes(service, getter) {
  var desc = Object.getOwnPropertyDescriptor(service.prototype, getter);

  assert(desc.get.__proto__ === Function.prototype);
  assert(desc.put === undefined);
}

function testServiceMethodTypes(service) {
  assert(service.prototype['resolvedOptions'].__proto__ === Function.prototype);
}

function testServiceCommon(service) {
  // Once the implementations are more correct, this could get richer.
  // For now, we mostly test that methods are callable and return the
  // right types.
  var locales = service.supportedLocalesOf();
  assert(Array.isArray(locales));

  var instance = service();
  var options = instance.resolvedOptions();
  assert(typeof options === 'object' && options !== null);
}

function testParts(parts) {
  assert(Array.isArray(parts));
  for (p of parts) {
    assert(typeof p === 'object');
    for ([k, v] of Object.entries(p)) {
      assert(typeof k === 'string');
      assert(typeof v === 'string');
    }
  }
}

assert(Intl !== undefined);

assert(Array.isArray(Intl.getCanonicalLocales('en-US')));

testServiceTypes(Intl.Collator);
testServiceGetterTypes(Intl.Collator, 'compare');
testServiceMethodTypes(Intl.Collator, 'resolvedOptions');
testServiceCommon(Intl.Collator);
var collator = Intl.Collator();
assert(typeof Intl.Collator().compare('foo', 'bar') === 'number');

testServiceTypes(Intl.DateTimeFormat);
testServiceGetterTypes(Intl.DateTimeFormat, 'format');
testServiceMethodTypes(Intl.DateTimeFormat, 'formatToParts');
testServiceMethodTypes(Intl.DateTimeFormat, 'resolvedOptions');
assert(typeof Intl.DateTimeFormat().format() === 'string');
testParts(Intl.DateTimeFormat().formatToParts());

testServiceTypes(Intl.NumberFormat);
testServiceGetterTypes(Intl.NumberFormat, 'format');
testServiceMethodTypes(Intl.NumberFormat, 'formatToParts');
testServiceMethodTypes(Intl.NumberFormat, 'resolvedOptions');
assert(typeof Intl.NumberFormat(12345.67).format() === 'string');
testParts(Intl.NumberFormat(12345.67).formatToParts());

// Verify the shared logic around style.

Intl.NumberFormat(undefined, {style:'currency', currency:'USD'});
Intl.NumberFormat(undefined, {style:'unit', unit:'meter'});
Intl.NumberFormat(undefined, {style:'percent'});

try {
  Intl.NumberFormat(undefined, {style:'currency'});
  throw new Error();
} catch (e) {}
try {
  Intl.NumberFormat(undefined, {style:'unit'});
  throw new Error();
} catch (e) {}

var d = new Date();
assert(d.toLocaleString() === Intl.DateTimeFormat().format(d));

// These tests are especially weak, as there's no way to verify that
// the ECMA 402 replacements are being used.  We'll need better tests
// for that which use actual data.  But if they are used, this will
// verify the calls are not fundementally busted.

assert(typeof new Date().toLocaleDateString() === 'string');
assert(typeof new Date().toLocaleString() === 'string');
assert(typeof new Date().toLocaleTimeString() === 'string');

assert(typeof new Number().toLocaleString() === 'string');

assert(typeof 'a'.localeCompare('b') === 'number');
assert(typeof 'A'.toLocaleLowerCase() === 'string');
assert(typeof 'a'.toLocaleUpperCase() === 'string');

// The rest tests that the stubs can be called, but the results are
// not spec-compliant (or even close).  It should be replaced with
// real tests once the implementation is complete.

var locales = Intl.Collator.supportedLocalesOf();
assert(locales.length === 2);
assert(locales[0] === 'en-CA');
assert(locales[1] === 'de-DE');

var collator = Intl.Collator();
var options = collator.resolvedOptions();
assert(options.numeric === false);
assert(options.locale === 'en-US');

for ([left, right, expected] of [
       ['a', 'b', -1],
       ['a', 'a', 0],
       ['b', 'a', 1],
       ['a', 'aa', -1],
       ['aa', 'a', 1],
]) {
  assert(collator.compare(left, right) === expected);
  assert(left.localeCompare(right) === expected);
}

var nf = Intl.NumberFormat();
assert(nf.format(123.45).startsWith('123.45'));
var parts = nf.formatToParts(123.45);
assert(parts.length === 1);
assert(parts[0].type === 'integer');
assert(parts[0].value.startsWith('123.45'));

assert('A'.toLocaleLowerCase() === 'lowered');
assert('a'.toLocaleUpperCase() === 'uppered');
