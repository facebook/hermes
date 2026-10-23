/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s
// RUN: %hermes -O %s | %FileCheck %s
// RUN: %hermes -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck %s

function equal(actual, expected) {
  if (actual !== expected) throw new Error(String(actual) + ' !== ' + expected);
}
function throwsTypeError(fn) {
  try { fn(); } catch (e) { equal(e instanceof TypeError, true); return; }
  throw new Error('Expected TypeError');
}
var test = RegExp.prototype.test;
var r = /x/;
r.exec = function(s) { equal(this, r); equal(s, 'abc'); return {}; };
equal(r.test('abc'), true);
r.exec = function() { return null; };
equal(r.test('x'), false);
equal(test.call({exec: function() { return {}; }}, 'x'), true);
equal(test.call({exec: function() { return null; }}, 'x'), false);
[undefined, false, 0, '', Symbol('bad')].forEach(function(result) {
  throwsTypeError(function() { test.call({exec: function() { return result; }}, 'x'); });
});
[undefined, null, false, 0, 'x'].forEach(function(receiver) {
  throwsTypeError(function() { test.call(receiver, {toString: function() {
    throw new Error('Must reject receiver before coercion');
  }}); });
});
throwsTypeError(function() { test.call({exec: 1}, 'x'); });
r.exec = 1;
equal(r.test('x'), true);
equal(r.test('y'), false);
var order = [];
var object = {get exec() {
  order.push('get');
  return function(s) { order.push('call'); equal(this, object); equal(s, 'text'); return {}; };
}};
equal(test.call(object, {toString: function() { order.push('string'); return 'text'; }}), true);
equal(order.join(','), 'string,get,call');
var replacement = /x/;
equal(replacement.test({toString: function() {
  replacement.exec = function() { return null; };
  return 'x';
}}), false);
var sentinel = {};
try { test.call({get exec() { throw sentinel; }}, 'x'); throw new Error('Did not throw'); }
catch (e) { equal(e, sentinel); }
var builtin = RegExp.prototype.exec;
Object.defineProperty(r, 'exec', {get: function() { order.push('builtin'); return builtin; }});
equal(r.test('x'), true);
equal(order[order.length - 1], 'builtin');
print('regexp test dispatch passed');
// CHECK: regexp test dispatch passed
