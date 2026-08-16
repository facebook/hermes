/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s
// REQUIRES: intl

function assert(pred, str) {
  if (!pred) {
    throw new Error('assertion failed' + (str === undefined ? '' : (': ' + str)));
  }
}

function assertThrows(fn, ctor) {
  try {
    fn();
  } catch (e) {
    assert(e instanceof ctor, 'expected ' + ctor.name + ', got ' + e);
    return;
  }
  throw new Error('expected ' + ctor.name);
}

function isSortedUnique(arr) {
  for (var i = 1; i < arr.length; i++) {
    if (arr[i] <= arr[i - 1])
      return false;
  }
  return true;
}

function assertStringArray(arr) {
  assert(Array.isArray(arr));
  assert(arr.length > 0);
  for (var i = 0; i < arr.length; i++)
    assert(typeof arr[i] === 'string');
  assert(isSortedUnique(arr), 'values must be sorted and unique');
}

assert(typeof Intl.supportedValuesOf === 'function');
assert(Intl.supportedValuesOf.length === 1);
assert(Intl.supportedValuesOf.name === 'supportedValuesOf');

var desc = Object.getOwnPropertyDescriptor(Intl, 'supportedValuesOf');
assert(desc.writable === true);
assert(desc.enumerable === false);
assert(desc.configurable === true);

assertThrows(function() { Intl.supportedValuesOf(); }, RangeError);
assertThrows(function() { Intl.supportedValuesOf(''); }, RangeError);
assertThrows(function() { Intl.supportedValuesOf('timezone'); }, RangeError);
assertThrows(function() { Intl.supportedValuesOf('invalid'); }, RangeError);

var keyObj = {toString: function() { return 'unit'; }};
assert(Array.isArray(Intl.supportedValuesOf(keyObj)));

var timeZones = Intl.supportedValuesOf('timeZone');
assertStringArray(timeZones);
assert(timeZones.indexOf('UTC') !== -1, 'UTC must be supported');
assert(timeZones.indexOf('Etc/UTC') === -1, 'Etc/UTC is not primary');
assert(timeZones.indexOf('Etc/GMT') === -1, 'Etc/GMT is not primary');
assert(
    new Intl.DateTimeFormat('en', {timeZone: 'UTC'})
        .resolvedOptions()
        .timeZone === 'UTC');

var calendars = Intl.supportedValuesOf('calendar');
assertStringArray(calendars);
assert(calendars.indexOf('iso8601') !== -1);
assert(calendars.indexOf('gregory') !== -1);

var collations = Intl.supportedValuesOf('collation');
assertStringArray(collations);
assert(collations.indexOf('standard') === -1);
assert(collations.indexOf('search') === -1);

var currencies = Intl.supportedValuesOf('currency');
assertStringArray(currencies);
assert(currencies.indexOf('USD') !== -1);
for (var i = 0; i < currencies.length; i++) {
  assert(currencies[i].length === 3);
  assert(currencies[i] === currencies[i].toUpperCase());
}

var numberingSystems = Intl.supportedValuesOf('numberingSystem');
assertStringArray(numberingSystems);
assert(numberingSystems.indexOf('latn') !== -1);

var units = Intl.supportedValuesOf('unit');
assertStringArray(units);
assert(units.indexOf('meter') !== -1);
assert(units.indexOf('celsius') !== -1);
assert(units.indexOf('year') !== -1);
