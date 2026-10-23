/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s
// RUN: %hermes -O %s | %FileCheck %s
// RUN: %hermes -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck %s
// RUN: %shermes -exec -O %s | %FileCheck %s

function equal(actual, expected) {
  if (actual !== expected) throw new Error(String(actual) + ' !== ' + expected);
}
var builtinExec = RegExp.prototype.exec;
function observe(source, flags, input, replace, wrapped) {
  /seed(c)/.exec('seedc');
  var re = new RegExp(source, flags);
  if (wrapped) re.exec = function(s) { return builtinExec.call(this, s); };
  var results = [];
  for (var i = 0; i < 3; i++) {
    var result = replace ? input.replace(re, '') : re.test(input);
    results.push([result, re.lastIndex, RegExp.$1, RegExp.$2, RegExp.input,
      RegExp.lastMatch, RegExp.lastParen, RegExp.leftContext, RegExp.rightContext]);
  }
  return JSON.stringify(results);
}
var cases = [
  ['(?<word>a)(b)?', 'dg', 'aba'],
  ['(a)|(b)', 'g', 'axb'],
  ['(a*)', 'g', 'baac'],
  ['(?:)', 'gu', '\ud83d\ude00x'],
  ['(?:)', 'g', '\ud83d\ude00x'],
  ['a', 'gy', 'aab'],
  ['x', 'g', 'abc'],
  ['(a)', '', 'ab'],
  ['(a)', 'y', 'ab'],
];
cases.forEach(function(c) {
  [false, true].forEach(function(replace) {
    equal(observe(c[0], c[1], c[2], replace, false),
      observe(c[0], c[1], c[2], replace, true));
  });
});
var re = /a/g, gets = 0;
Object.defineProperty(re, 'exec', {get: function() { gets++; return builtinExec; }});
equal('aba'.replace(re, ''), 'b');
equal(gets, 3);
equal(re.test('a'), true);
equal(gets, 4);
var events = [];
re = /a/g;
Object.defineProperty(re, 'global', {get: function() { events.push('global'); return true; }});
Object.defineProperty(re, 'unicode', {get: function() { events.push('unicode'); return false; }});
equal(RegExp.prototype[Symbol.replace].call(re,
  {toString: function() { events.push('input'); return 'aba'; }},
  {toString: function() { events.push('replacement'); return ''; }}), 'b');
equal(events.join(','), 'input,replacement,global,unicode');
equal(re.lastIndex, 0);
re = /(?:)/g;
Object.defineProperty(re, 'unicode', {value: true});
equal('\ud83d\ude00'.replace(re, ''), '\ud83d\ude00');
re = /a/g;
Object.defineProperty(re, 'global', {value: false});
equal('aba'.replace(re, ''), 'ba');
equal(re.lastIndex, 1);
[false, true].forEach(function(replace) {
  var re = /a/g;
  Object.defineProperty(re, 'lastIndex', {writable: false});
  try {
    if (replace) 'a'.replace(re, ''); else re.test('a');
  } catch (e) { equal(e instanceof TypeError, true); return; }
  throw new Error('Expected lastIndex write failure');
});
print('regexp unused results passed');
// CHECK: regexp unused results passed
