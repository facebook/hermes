/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LANG=en_US.UTF-8 %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

print('String');
// CHECK-LABEL: String
print('empty', String());
// CHECK-NEXT: empty
print(String('asdf'), String('asdf').length);
// CHECK-NEXT: asdf 4
print(String(133));
// CHECK-NEXT: 133
print(String(undefined));
// CHECK-NEXT: undefined
var s = new String('asdf');
print(s, s.toString(), s.valueOf(), s.length, s.__proto__ === String.prototype);
// CHECK-NEXT: asdf asdf asdf 4 true
var desc = Object.getOwnPropertyDescriptor(s, 'length');
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false
print(String('a') === String('a'), new String('a') === new String('a'));
// CHECK-NEXT: true false
print('aaaa'.length, String('aaaa').length, (new String('aaaa')).length);
// CHECK-NEXT: 4 4 4

print('fromCharCode');
// CHECK-LABEL: fromCharCode
print('empty', String.fromCharCode());
// CHECK-NEXT: empty
print(String.fromCharCode(97));
// CHECK-NEXT: a
print(String.fromCharCode(0xff0041));
// CHECK-NEXT: A
print(String.fromCharCode(0xff0048, 0xe8, 114, 109, 101, 115));
// CHECK-NEXT: Hèrmes

print('fromCodePoint');
// CHECK-LABEL: fromCodePoint
print('empty', String.fromCodePoint());
// CHECK-NEXT: empty
print(String.fromCodePoint(65));
// CHECK-NEXT: A
print(String.fromCodePoint(65, 66));
// CHECK-NEXT: AB
print(String.fromCodePoint(0x00A9));
// CHECK-NEXT: ©
print(String.fromCodePoint(0x00A9, 0x00A9));
// CHECK-NEXT: ©©
print(String.fromCodePoint(65, 0x1f4d8, 66, 66, 66, 66));
// CHECK-NEXT: A📘BBBB
print(String.fromCodePoint(0).charCodeAt(0));
// CHECK-NEXT: 0
print(String.fromCodePoint(false).charCodeAt(0));
// CHECK-NEXT: 0
print(String.fromCodePoint(true).charCodeAt(0));
// CHECK-NEXT: 1
print(String.fromCodePoint(true).length);
// CHECK-NEXT: 1
try {String.fromCodePoint(1.1);} catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught RangeError
try {String.fromCodePoint(-1);} catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught RangeError
try {String.fromCodePoint(0xffffff);} catch (e) {print('caught', e.name);}
// CHECK-NEXT: caught RangeError

print('charAt');
// CHECK-LABEL: charAt
print('abc'.charAt(0))
// CHECK-NEXT: a
print('abc'.charAt(1))
// CHECK-NEXT: b
print('empty', 'abc'.charAt(-1))
// CHECK-NEXT: empty
print('empty', 'abc'.charAt(100))
// CHECK-NEXT: empty
print('abc'.charAt(1.1))
// CHECK-NEXT: b

print('charCodeAt');
// CHECK-LABEL: charCodeAt
print('abc'.charCodeAt(0))
// CHECK-NEXT: 97
print('abc'.charCodeAt(1))
// CHECK-NEXT: 98
print('abc'.charCodeAt(-1))
// CHECK-NEXT: NaN
print('abc'.charCodeAt(100))
// CHECK-NEXT: NaN
print('abc'.charCodeAt(1.1))
// CHECK-NEXT: 98
print("Ж".charCodeAt(0))
// CHECK-NEXT: 1046
print("Ж".charCodeAt(1))
// CHECK-NEXT: NaN
print("💩".charCodeAt(0))
// CHECK-NEXT: 55357
print("💩".charCodeAt(1))
// CHECK-NEXT: 56489

print('codePointAt');
// CHECK-LABEL: codePointAt
print('asdf'.codePointAt(-Infinity));
// CHECK-NEXT: undefined
print('asdf'.codePointAt(Infinity));
// CHECK-NEXT: undefined
print('asdf'.codePointAt(-1));
// CHECK-NEXT: undefined
print('A'.codePointAt(-1));
// CHECK-NEXT: undefined
print('A'.codePointAt(0));
// CHECK-NEXT: 65
print('A'.codePointAt(3));
// CHECK-NEXT: undefined
print("Ж".codePointAt(0))
// CHECK-NEXT: 1046
print("ЖA".codePointAt(1))
// CHECK-NEXT: 65
print("💩".codePointAt(0))
// CHECK-NEXT: 128169
print("💩".codePointAt(1))
// CHECK-NEXT: 56489
print("A💩B".codePointAt(3))
// CHECK-NEXT: 66

print('concat');
// CHECK-LABEL: concat
print('asdf'.concat());
// CHECK-NEXT: asdf
print('empty', ''.concat());
// CHECK-NEXT: empty
print(''.concat('ghjkl'));
// CHECK-NEXT: ghjkl
print('asdf'.concat('ghjkl'));
// CHECK-NEXT: asdfghjkl
print('ab'.concat('cd', 'ef', 'gh'));
// CHECK-NEXT: abcdefgh
print('abc'.concat(123));
// CHECK-NEXT: abc123
var x = 'asdf';
print(x.concat(x));
// CHECK-NEXT: asdfasdf
var a = 'a';
for (i = 0; i < 28; ++i) {
  a = a + a;
}
try {
  a.concat(a);
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: RangeError

print('slice');
// CHECK-LABEL: slice
print('abcdcba'.slice());
// CHECK-NEXT: abcdcba
print('abcdcba'.slice(1));
// CHECK-NEXT: bcdcba
print('abcdcba'.slice(1, 4));
// CHECK-NEXT: bcd
print('abcdcba'.slice(-1));
// CHECK-NEXT: a
print('abcdcba'.slice(1, -1));
// CHECK-NEXT: bcdcb
print('empty', 'abcdcba'.slice(1000, 123892));
// CHECK-NEXT: empty
print('empty', 'abcdcba'.slice(-1000, -123892));
// CHECK-NEXT: empty
print('empty', ''.slice(-1000, -123892));
// CHECK-NEXT: empty
print('abcdcba'.slice(-1000, 123892));
// CHECK-NEXT: abcdcba
var s = ''
for (var i = 0; i < 10; ++i) {
  s += 'asdf';
}
print(s.slice(1, 5));
// CHECK-NEXT: sdfa
print('empty', 'asfdf'.slice(Infinity, NaN));
// CHECK-NEXT: empty

print('split');
// CHECK-LABEL: split
print('abc,def,ghi'.split(','));
// CHECK-NEXT: abc,def,ghi
print('abc,def,ghi'.split(',', 2));
// CHECK-NEXT: abc,def
print('abc,def,ghi'.split(',', 0), 'empty');
// CHECK-NEXT: empty
print('abc::ghi'.split(':'));
// CHECK-NEXT: abc,,ghi
print('abc'.split(''));
// CHECK-NEXT: a,b,c
print('ab'.split(/a*?/));
// CHECK-NEXT: a,b
var re = /,/y;
re.lastIndex=6;
print('abc,def,ghi'.split(re));
// CHECK-NEXT: abc,def,ghi
print('helloworld'.split(/(hello)(world)/));
// CHECK-NEXT: ,hello,world,
print('AhelloworldB'.split(/(hello)(world)/));
// CHECK-NEXT: A,hello,world,B
print(''.split('').length);
// CHECK-NEXT: 0
print(''.split(/(?:)/).length);
// CHECK-NEXT: 0
print(''.split('a').length);
// CHECK-NEXT: 1
print(''.split(/a/).length);
// CHECK-NEXT: 1
var result = 'ab'.split(/a*/);
print(result, result.length, result[0].length);
// CHECK-NEXT: ,b 2 0
var result = "A<B>bold</B>and<CODE>coded</CODE>".split(/<(\/)?([^<>]+)>/);
print(JSON.stringify(result));
print(result[1], result[7]);
// CHECK-NEXT: ["A",null,"B","bold","/","B","and",null,"CODE","coded","/","CODE",""]
// CHECK-NEXT: undefined undefined
print('  '.split(/()()()()()()()()()()/).length);
// CHECK-NEXT: 12

// Ensure we step over surrogate pairs iff Unicode is set.
print("\u{12345}".split(/(?:)/u).length);
// CHECK-NEXT: 1
print("\u{12345}".split(/(?:)/).length);
// CHECK-NEXT: 2

// test ES6 specific implementation
// borrowed from mjsunit/es6/string-split.js
var patternSplit = {};
var limit = { value: 3 };
patternSplit[Symbol.split] = function(string, limit) {
  return string.length * limit.value;
};
// Check object coercible fails.
try {
  String.prototype.split.call(null, patternSplit, limit);
} catch(e) {
  print(e.name);
}
// CHECK-NEXT: TypeError
// Override is called.
print("abcde".split(patternSplit, limit));
// CHECK-NEXT: 15
// Non-callable override.
patternSplit[Symbol.split] = "dumdidum";
try { "abcde".split(patternSplit, limit); } catch(e) { print(e.name); }
// CHECK-NEXT: TypeError

// Check that you cannot match at the end of a string
print("aXbXcX".split(/(?<=X)/))
// CHECK-NEXT: aX,bX,cX
print("test".split(/$/))
// CHECK-NEXT: test
print("abc".split(""))
// CHECK-NEXT: a,b,c

// Check splitting with sticky/global flag set
var pattern = /X/y
print("aXbXcX".split(pattern))
// CHECK-NEXT: a,b,c,
print(pattern.lastIndex)
// CHECK-NEXT: 0
// We temporarily strip the sticky flag when splitting, ensure it is restored
print(pattern)
// CHECK-NEXT: /X/y
var pattern = /X/g
print("aXbXcX".split(pattern))
// CHECK-NEXT: a,b,c,
print(pattern.lastIndex)
// CHECK-NEXT: 0

// Check that split uses the "flags" property
var pattern = /abc/u
Object.defineProperty(pattern, 'flags', {value: 'test'})
print(pattern.flags)
// CHECK-NEXT: test
try { "abcabc".split(pattern); } catch(e) { print(e.name); }
// CHECK-NEXT: SyntaxError
var pattern = /abc/
Object.defineProperty(pattern, 'flags', {value: 'i'})
print(pattern.flags)
// CHECK-NEXT: i
print("zAbCzAbCz".split(pattern))
// CHECK-NEXT: z,z,z

print('substring');
// CHECK-LABEL: substring
print('abcdcba'.substring());
// CHECK-NEXT: abcdcba
print('abcdcba'.substring(1));
// CHECK-NEXT: bcdcba
print('abcdcba'.substring(1, 4));
// CHECK-NEXT: bcd
print('abcdcba'.substring(4, 1));
// CHECK-NEXT: bcd
print('abcdcba'.substring(-1));
// CHECK-NEXT: abcdcba
print('abcdcba'.substring(1, -1));
// CHECK-NEXT: a
print('empty', 'abcdcba'.substring(1000, 123892));
// CHECK-NEXT: empty
print('empty', 'abcdcba'.substring(-1000, -123892));
// CHECK-NEXT: empty
print('abcdcba'.substring(-1000, 123892));
// CHECK-NEXT: abcdcba
print('abcdcba'.substring(23892, -1000));
// CHECK-NEXT: abcdcba
var s = ''
for (var i = 0; i < 10; ++i) {
  s += 'asdf';
}
print(s.substring(1, 5));
// CHECK-NEXT: sdfa

print('toLowerCase');
// CHECK-LABEL: toLowerCase
print('q'.toLowerCase());
// CHECK-NEXT: q
print('Q'.toLowerCase());
// CHECK-NEXT: q
print('|' + ' '.toLowerCase() + '|');
// CHECK-NEXT: | |
print('ABC'.toLowerCase());
// CHECK-NEXT: abc
print('abc'.toLowerCase());
// CHECK-NEXT: abc
print('empty', ''.toLowerCase());
// CHECK-NEXT: empty
print('a/B.c'.toLowerCase());
// CHECK-NEXT: a/b.c
print('ß'.toLowerCase());
// CHECK-NEXT: ß
print('A\u180e\u03a3\u180e'.toLowerCase() === 'a\u180e\u03c2\u180e');
// CHECK-NEXT: true
print('A\u180e\u03a3\u180eB'.toLowerCase() === 'a\u180e\u03c3\u180eb');
// CHECK-NEXT: true

print('toLocaleLowerCase');
// CHECK-LABEL: toLocaleLowerCase
print('ABC'.toLocaleLowerCase());
// CHECK-NEXT: abc
print('abc'.toLocaleLowerCase());
// CHECK-NEXT: abc
print('empty', ''.toLocaleLowerCase());
// CHECK-NEXT: empty
print('a/B.c'.toLocaleLowerCase());
// CHECK-NEXT: a/b.c
print('ß'.toLocaleLowerCase());
// CHECK-NEXT: ß
print('A\u180e\u03a3\u180e'.toLocaleLowerCase() === 'a\u180e\u03c2\u180e');
// CHECK-NEXT: true
print('A\u180e\u03a3\u180eB'.toLocaleLowerCase() === 'a\u180e\u03c3\u180eb');
// CHECK-NEXT: true

print('toUpperCase');
// CHECK-LABEL: toUpperCase
print('Q'.toUpperCase());
// CHECK-NEXT: Q
print('q'.toUpperCase());
// CHECK-NEXT: Q
print('|' + ' '.toUpperCase() + '|');
// CHECK-NEXT: | |
print('abc'.toUpperCase());
// CHECK-NEXT: ABC
print('à'.toUpperCase());
// CHECK-NEXT: À
print('xßy'.toUpperCase());
// CHECK-NEXT: XSSY
var s = ''
for (var i = 0; i < 100; ++i) {
  s += 'ß';
}
var result = s.toUpperCase();
print(result.length);
// CHECK-NEXT: 200
print(Array.prototype.every.call(result, function(c) {return c === 'S';}));
// CHECK-NEXT: true

print('toLocaleUpperCase');
// CHECK-LABEL: toLocaleUpperCase
print('abc'.toLocaleUpperCase());
// CHECK-NEXT: ABC
print('à'.toLocaleUpperCase());
// CHECK-NEXT: À
print('xßy'.toLocaleUpperCase());
// CHECK-NEXT: XSSY
var s = ''
for (var i = 0; i < 100; ++i) {
  s += 'ß';
}
var result = s.toLocaleUpperCase();
print(result.length);
// CHECK-NEXT: 200
print(Array.prototype.every.call(result, function(c) {return c === 'S';}));
// CHECK-NEXT: true

print('substr');
// CHECK-LABEL: substr
print('abcdcba'.substr());
// CHECK-NEXT: abcdcba
print('abcdcba'.substr(1));
// CHECK-NEXT: bcdcba
print('abcdcba'.substr(1, 3));
// CHECK-NEXT: bcd
print('abcdcba'.substr(-1));
// CHECK-NEXT: a
print('abcdcba'.substr(-3, 2));
// CHECK-NEXT: cb
print('empty', 'abcdcba'.substr(1, -2));
// CHECK-NEXT: empty
print('empty', 'abcdcba'.substr(100));
// CHECK-NEXT: empty
print('empty', 'abcdcba'.substr(100, 2));
// CHECK-NEXT: empty
print('abcdcba'.substr(-100));
// CHECK-NEXT: abcdcba
print('abcdcba'.substr(-100, 2));
// CHECK-NEXT: ab
var s = ''
for (var i = 0; i < 10; ++i) {
  s += 'asdf';
}
print(s.substr(1, 5));
// CHECK-NEXT: sdfas

print('trim');
// CHECK-LABEL: trim
print('abc'.trim());
// CHECK-NEXT: abc
print(' abc '.trim());
// CHECK-NEXT: abc
print('abc '.trim());
// CHECK-NEXT: abc
print(' abc '.trim());
// CHECK-NEXT: abc
print(' \t\t \n a bc \t \n '.trim());
// CHECK-NEXT: a bc
print('empty', '  '.trim());
// CHECK-NEXT: empty
print('empty', ''.trim());
// CHECK-NEXT: empty
print('empty', '\n\ufeff\u2028'.trim());
// CHECK-NEXT: empty
var s = '   ';
for (var i = 0; i < 10; ++i) {
  s += 'a';
}
print(s.trim());
// CHECK-NEXT: aaaaaaaaaa

print('trimStart');
// CHECK-LABEL: trimStart
print(String.prototype.trimStart.length);
// CHECK-NEXT: 0
print('abc'.trimStart());
// CHECK-NEXT: abc
var res = ' abc '.trimStart();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = '\ufeff\n abc '.trimStart();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = '  '.trimStart();
print('empty', res, res.length);
// CHECK-NEXT: empty  0
print(String.prototype.trimStart === String.prototype.trimLeft);
// CHECK-NEXT: true

print('trimEnd');
// CHECK-LABEL: trimEnd
print(String.prototype.trimEnd.length);
// CHECK-NEXT: 0
print('abc'.trimEnd());
// CHECK-NEXT: abc
var res = ' abc '.trimEnd();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = ' abc \n\ufeff'.trimEnd();
print(res, res.length);
// CHECK-NEXT:  abc 4
var res = '  '.trimEnd();
print('empty', res, res.length);
// CHECK-NEXT: empty  0
print(String.prototype.trimEnd === String.prototype.trimRight);
// CHECK-NEXT: true

print('indexOf');
// CHECK-LABEL: indexOf
print('abc'.indexOf('a'))
// CHECK-NEXT: 0
print('abc'.indexOf('b'))
// CHECK-NEXT: 1
print('abc'.indexOf('c'))
// CHECK-NEXT: 2
print('abc'.indexOf('d'))
// CHECK-NEXT: -1
print('abc'.indexOf('abc'))
// CHECK-NEXT: 0
print('abc'.indexOf('abcd'))
// CHECK-NEXT: -1
print(''.indexOf('abc'))
// CHECK-NEXT: -1
print('abc'.indexOf(''))
// CHECK-NEXT: 0
print(''.indexOf(''))
// CHECK-NEXT: 0
print('x1212x'.indexOf('12'))
// CHECK-NEXT: 1
print('abc'.indexOf('a', 1))
// CHECK-NEXT: -1
print('abc'.indexOf('b', 1))
// CHECK-NEXT: 1
print('abc'.indexOf('b', -1000))
// CHECK-NEXT: 1
print('abc'.indexOf('b', 1000))
// CHECK-NEXT: -1
print('abc'.indexOf('', 1000))
// CHECK-NEXT: 3
print('abc'.indexOf('', -1000))
// CHECK-NEXT: 0
var selfStr = '12345';
print(selfStr.indexOf(selfStr))
// CHECK-NEXT: 0

print('lastIndexOf');
// CHECK-LABEL: lastIndexOf
print('abc'.lastIndexOf('a'))
// CHECK-NEXT: 0
print('abc'.lastIndexOf('b'))
// CHECK-NEXT: 1
print('abc'.lastIndexOf('c'))
// CHECK-NEXT: 2
print('abc'.lastIndexOf('d'))
// CHECK-NEXT: -1
print('abc'.lastIndexOf('abc'))
// CHECK-NEXT: 0
print('abcabc'.lastIndexOf('abc'))
// CHECK-NEXT: 3
print(''.lastIndexOf('abc'))
// CHECK-NEXT: -1
print('xxxxx'.lastIndexOf('x'))
// CHECK-NEXT: 4
print(''.lastIndexOf(''))
// CHECK-NEXT: 0
print('x1212x'.lastIndexOf('12'))
// CHECK-NEXT: 3
print('abc'.lastIndexOf('a', 1))
// CHECK-NEXT: 0
print('abc'.lastIndexOf('a', 0))
// CHECK-NEXT: 0
print('abc'.lastIndexOf('b', 1))
// CHECK-NEXT: 1
print('abc'.lastIndexOf('b', 0))
// CHECK-NEXT: -1
print('abc'.lastIndexOf('a', -1000))
// CHECK-NEXT: 0
print('abc'.lastIndexOf('b', -1000))
// CHECK-NEXT: -1
print('abc'.lastIndexOf('b', 1000))
// CHECK-NEXT: 1
print('abc'.lastIndexOf('', 1000))
// CHECK-NEXT: 3
print('abc'.lastIndexOf('', 2))
// CHECK-NEXT: 2
print('abc'.lastIndexOf('', -1000))
// CHECK-NEXT: 0
print('abc'.lastIndexOf('bc', 1))
// CHECK-NEXT: 1
var selfStr2 = '12345';
print(selfStr2.lastIndexOf(selfStr2))
// CHECK-NEXT: 0

print('localeCompare');
// CHECK-LABEL: localeCompare
print('abc'.localeCompare('abc'));
// CHECK-NEXT: 0
print('abc'.localeCompare('def'));
// CHECK-NEXT: -1
print('a'.localeCompare('Z'));
// CHECK-NEXT: -1
print('ö'.localeCompare('ö'));
// CHECK-NEXT: 0
print('o\u0308'.localeCompare('ö'));
// CHECK-NEXT: 0
print('ạ\u0308'.localeCompare('a\u0323\u0308'));
// CHECK-NEXT: 0
print('\u212B'.localeCompare('A\u030A')); // Angstrom sign vs A + O-ring
// CHECK-NEXT: 0
print('S\u0323\u0307'.localeCompare('S\u0307\u0323')); // diacritic ordering
// CHECK-NEXT: 0

print('match');
// CHECK-LABEL: match
var result = 'abcabc'.match(/a/);
print(result, result.index, result.input);
// CHECK-NEXT: a 0 abcabc
print('abcabc'.match(/[bc]/));
// CHECK-NEXT: b
print('abcabc'.match(/ca/));
// CHECK-NEXT: ca
print('abcabc'.match('(a)b'));
// CHECK-NEXT: ab,a
print('abcabc'.match(/(a)b/));
// CHECK-NEXT: ab,a
print('abcabc'.match(/a/g));
// CHECK-NEXT: a,a
print(''.match().length);
// CHECK-NEXT: 1
var result = 'undefined'.match(undefined);
print(result.length, result.index, result.input === 'undefined');
// CHECK-NEXT: 1 0 true
var result = 'abcabc'.match(/[ac]/g);
print(result, result.length);
// CHECK-NEXT: a,c,a,c 4
print('abcd'.match(/(?:)/g));
// CHECK-NEXT: ,,,,
print('abcd'.match(/[ac]?/g));
// CHECK-NEXT: a,,c,,
print('ab\u00a0cd'.match(/\s/)[0] === '\u00a0');
// CHECK-NEXT: true
print('ab\u00a0cd'.match(/\S/g));
// CHECK-NEXT: a,b,c,d
print('\u2604b\u00a0cd'.match(/\s/)[0] === '\u00a0');
// CHECK-NEXT: true
print('\u2604b\u00a0cd'.match(/\S/g));
// CHECK-NEXT: ☄,b,c,d
var obj = {toString:function(){return {};},valueOf:function(){throw "intostr";}}
var str = new String("ABB\u0041BABAB");
try {str.match(obj);} catch(e) {print('caught', e);}
// CHECK-NEXT: caught intostr

// test ES6 specific implementation
// borrowed from mjsunit/es6/string-match.js
print('match (Symbol.match)');
// CHECK-LABEL: match (Symbol.match)
var pattern = {};
pattern[Symbol.match] = function(string) {
  return string.length;
};
// Check object coercible fails.
try {
  String.prototype.match.call(null, pattern);
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError
// Override is called.
print("abcde".match(pattern));
// CHECK-NEXT: 5
// Non-callable override.
pattern[Symbol.match] = "dumdidum";
try {
  "abcde".match(pattern)
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError

print('matchAll');
// CHECK-LABEL: matchAll
var it = "foo bar".matchAll(/\w+/g)
var foo = it.next()
var bar = it.next()
var nil = it.next()
print(it)
// CHECK-NEXT: [object RegExp String Iterator]
print(foo.value, foo.done)
// CHECK-NEXT: foo false
print(foo.value[0], foo.value.index, foo.value.input, foo.value.groups);
// CHECK-NEXT: foo 0 foo bar undefined
print(bar.value, bar.done)
// CHECK-NEXT: bar false
print(bar.value[0], bar.value.index, bar.value.input, bar.value.groups);
// CHECK-NEXT: bar 4 foo bar undefined
print(nil.value, nil.done)
// CHECK-NEXT: undefined true
try {
  "foo bar".matchAll(/\w+/);
} catch(e) {
  print(e)
}
// CHECK-NEXT: TypeError: String.prototype.matchAll called with a non-global RegExp argument
try {
  RegExp.prototype[Symbol.matchAll].call(1);
} catch (e) {
  print(e)
}
// CHECK-NEXT: TypeError: RegExp.prototype[@@matchAll] should be called on a js object
try {
  var it = "foo bar".matchAll(/\w+/g)
  it.next.call(1)
} catch (e) {
  print(e)
}
// CHECK-NEXT: TypeError: RegExpStringIteratorPrototype.next requires 'this' is a RegExp String Iterator
print(Array.from("foo bar".matchAll(/\w+/g)));
// CHECK-NEXT: foo,bar
print([..."foo bar".matchAll(/\w+/g)]);
// CHECK-NEXT: foo,bar

// matchAll should have no side effects to original regExp.
print('matchAll (lastIndex)');
// CHECK-LABEL: matchAll (lastIndex)
var re = /\w+/g;
re.lastIndex = 3;
var it = 'foo bar'.matchAll(re)
print(it.next().value, re.lastIndex)
// CHECK-NEXT: bar 3
print(it.next().value, re.lastIndex)
// CHECK-NEXT: undefined 3

print('matchAll (flags)');
// CHECK-LABEL: matchAll (flags)
var re = /\w+/;
Object.defineProperty(re, 'flags', { value: 'g' })
var it = 'foo bar'.matchAll(re)
print(it.next().value)
// CHECK-NEXT: foo
print(it.next().value)
// CHECK-NEXT: bar
print(re.global)
// CHECK-NEXT: false

print('matchAll (Symbol.matchAll)');
// CHECK-LABEL: matchAll (Symbol.matchAll)
var re = /\w+/g;
re[Symbol.matchAll] = function(str) { return str }
print("abc cde".matchAll(re));
// CHECK-NEXT: abc cde
try {
  var re = /\w+/;
  re[Symbol.matchAll] = function(str) { return str }
  "abc cde".matchAll(re)
} catch(e) { print(e) }
// CHECK-NEXT: TypeError: String.prototype.matchAll called with a non-global RegExp argument
var re = {
  [Symbol.matchAll]: function(str) { return str },
  flags: "contain_g"
}
print("abc cde".matchAll(re));
// CHECK-NEXT: abc cde
var re = {
  [Symbol.matchAll]: function(str) { return str },
  flags: "blah"
}
// re had no [Symbol.match] made it !isRegExp, so this should not throw.
print("abc cde".matchAll(re));
// CHECK-NEXT: abc cde
var re = {
  [Symbol.match]: function(str) { return str },
  [Symbol.matchAll]: function(str) { return str },
  flags: "blah"
}
// re had [Symbol.match] made it isRegExp and now this throws!
try { "abc cde".matchAll(re) } catch(e) { print(e) }
// CHECK-NEXT: TypeError: String.prototype.matchAll called with a non-global RegExp argument

print('normalize');
// CHECK-LABEL: normalize
// latin small letter long S with dot above, combining dot below
var s = '\u1E9B\u0323'
var r = s.normalize('NFC');
print(r.codePointAt(0), r.codePointAt(1), r.codePointAt(2));
// CHECK-NEXT: 7835 803 undefined
var r = s.normalize('NFD');
print(r.codePointAt(0), r.codePointAt(1), r.codePointAt(2), r.codePointAt(3));
// CHECK-NEXT: 383 803 775 undefined
var r = s.normalize('NFKC');
print(r.codePointAt(0), r.codePointAt(1));
// CHECK-NEXT: 7785 undefined
var r = s.normalize('NFKD');
print(r.codePointAt(0), r.codePointAt(1), r.codePointAt(2), r.codePointAt(3));
// CHECK-NEXT: 115 803 775 undefined

print('padEnd');
// CHECK-LABEL: padEnd
print('abc'.padEnd(5, 'X'));
// CHECK-NEXT: abcXX
print('abc'.padEnd(2, 'X'));
// CHECK-NEXT: abc
print('abc'.padEnd(5) + 'X');
// CHECK-NEXT: abc  X
print('abc'.padEnd() + 'X');
// CHECK-NEXT: abcX
print('abc'.padEnd(-1) + 'X');
// CHECK-NEXT: abcX
print('abc'.padEnd(10, 'ABCD'));
// CHECK-NEXT: abcABCDABC
print('abc'.padEnd(14, 'ABCD'));
// CHECK-NEXT: abcABCDABCDABC
print('abc'.padEnd(4, 'ABCD'));
// CHECK-NEXT: abcA
print('abc'.padEnd(7, 'ABCD'));
// CHECK-NEXT: abcABCD
print('X' + ''.padEnd(5) + 'X');
// CHECK-NEXT: X     X
try { print('abc'.padEnd(1e100, 'ABC')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError

print('padStart');
// CHECK-LABEL: padStart
print('abc'.padStart(5, 'X'));
// CHECK-NEXT: XXabc
print('abc'.padStart(2, 'X'));
// CHECK-NEXT: abc
print('X' + 'abc'.padStart(5));
// CHECK-NEXT: X  abc
print('X' + 'abc'.padStart());
// CHECK-NEXT: Xabc
print('X' + 'abc'.padStart(-1));
// CHECK-NEXT: Xabc
print('abc'.padStart(10, 'ABCD'));
// CHECK-NEXT: ABCDABCabc
print('abc'.padStart(4, 'ABCD'));
// CHECK-NEXT: Aabc
print('abc'.padStart(7, 'ABCD'));
// CHECK-NEXT: ABCDabc
print('X' + ''.padStart(5) + 'X');
// CHECK-NEXT: X     X
try { print('abc'.padStart(1e100, 'ABC')); } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError

print('repeat');
// CHECK-LABEL: repeat
print('a'.repeat(3));
// CHECK-NEXT: aaa
print('ab'.repeat(3));
// CHECK-NEXT: ababab
print('ab'.repeat(1));
// CHECK-NEXT: ab
print(''.repeat(10000), "EMPTY");
// CHECK-NEXT: EMPTY
print('a'.repeat(), "EMPTY");
// CHECK-NEXT: EMPTY
print('a'.repeat(0), "EMPTY");
// CHECK-NEXT: EMPTY
try { 'a'.repeat(Infinity) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError
try { 'a'.repeat(-19) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError
try { 'asdfasdf'.repeat(1000000000000) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught RangeError
try { String.prototype.repeat.call(null) } catch (e) { print('caught', e.name) }
// CHECK-NEXT: caught TypeError

print('endsWith');
// CHECK-LABEL: endsWith
print('abcd'.endsWith('d'));
// CHECK-NEXT: true
print('abcd'.endsWith('cd'));
// CHECK-NEXT: true
print('abcd'.endsWith(''));
// CHECK-NEXT: true
print('abcd'.endsWith('D'));
// CHECK-NEXT: false
print('abcd'.endsWith('bc', 3));
// CHECK-NEXT: true
print('abcd'.endsWith('bc', -20));
// CHECK-NEXT: false
print('abcd'.endsWith('d', Infinity));
// CHECK-NEXT: true
print(''.endsWith(''));
// CHECK-NEXT: true
try { print('abcd'.endsWith(/d/)); } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print('startsWith');
// CHECK-LABEL: startsWith
print('abcd'.startsWith('a'));
// CHECK-NEXT: true
print('abcd'.startsWith('ab'));
// CHECK-NEXT: true
print('abcd'.startsWith(''));
// CHECK-NEXT: true
print('abcd'.startsWith('AB'));
// CHECK-NEXT: false
print('abcd'.startsWith('bc', 1));
// CHECK-NEXT: true
print('abcd'.startsWith('ab', -20));
// CHECK-NEXT: true
print('abcd'.startsWith('d', Infinity));
// CHECK-NEXT: false
print(''.startsWith(''));
// CHECK-NEXT: true
try { print('abcd'.startsWith(/d/)); } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print('includes');
// CHECK-LABEL: includes
print('abcd'.includes('a'));
// CHECK-NEXT: true
print('abcd'.includes('ab'));
// CHECK-NEXT: true
print('abcd'.includes('bc'));
// CHECK-NEXT: true
print('abcd'.includes('bd'));
// CHECK-NEXT: false
print('abcd'.includes('abcd'));
// CHECK-NEXT: true
print('abcd'.includes(''));
// CHECK-NEXT: true
print('abcd'.includes('AB'));
// CHECK-NEXT: false
print('abcd'.includes('bc', 1));
// CHECK-NEXT: true
print('abcd'.includes('bc', -20));
// CHECK-NEXT: true
print('abcd'.includes('d', Infinity));
// CHECK-NEXT: false
print(''.includes(''));
// CHECK-NEXT: true
try { print('abcd'.includes(/d/)); } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError

print('raw');
// CHECK-LABEL: raw
print(String.raw({raw: ["first ", " second"]}, 'ABCD'));
// CHECK-NEXT: first ABCD second
print(String.raw({raw: ["first", "second", "third"]}, 'ABCD', 'EFG'));
// CHECK-NEXT: firstABCDsecondEFGthird
print(String.raw({raw: ["first", "second"]}, 'ABCD', 'EFG'));
// CHECK-NEXT: firstABCDsecond
print(String.raw({raw: ["first", "second", "third"]}, 'ABCD', 'EFG', "JK", "HL"));
// CHECK-NEXT: firstABCDsecondEFGthird
print(String.raw({raw: []}, 'ABCD', 'EFG', 'asdf'), "EMPTY");
// CHECK-NEXT: EMPTY
print(String.raw({raw: {length: -10}}), "EMPTY");
// CHECK-NEXT: EMPTY

print('replace');
// CHECK-LABEL: replace
print('x' + ''.replace('a', 'b') + 'y');
// CHECK-NEXT: xy
print('x' + 'abc'.replace('', 'b') + 'y');
// CHECK-NEXT: xbabcy
print('x' + ''.replace('', 'b') + 'y');
// CHECK-NEXT: xby
print('abcabc'.replace('b', 'xy'));
// CHECK-NEXT: axycabc
print('abcabc'.replace('bc', 'x'));
// CHECK-NEXT: axabc
print('abcabc'.replace('', 'XYZ'));
// CHECK-NEXT: XYZabcabc
print('abcdef'.replace('ab', '$'));
// CHECK-NEXT: $cdef
print('abcdef'.replace('ab', '12$'));
// CHECK-NEXT: 12$cdef
print('abcdefg'.replace('de', "($`)($')($$)($a)"));
// CHECK-NEXT: abc(abc)(fg)($)($a)fg
print('abcdefg'.replace('de', "($&)($&)($&)"));
// CHECK-NEXT: abc(de)(de)(de)fg
print('abc'.replace(/(x)?b/, '$1'));
// CHECK-NEXT: ac
print('axbc'.replace(/(x)?b/, '$1'));
// CHECK-NEXT: axc
print('aybc'.replace(/(xxx)?b/, '$1'));
// CHECK-NEXT: ayc
print('$1,$2'.replace(/(\$(\d))/, '$$1-$1$2'));
// CHECK-NEXT: $1-$11,$2
print('$1,$2'.replace(/(\$(\d))/g, '$$1-$1$2'));
// CHECK-NEXT: $1-$11,$1-$22
print('buoy'.replace(/([abc]).*([xyz])/, '$2$99$1'));
// CHECK-NEXT: y$99b
print('x=3'.replace(/(x=)(\d+)/, '$1115'));
// CHECK-NEXT: x=115
print('x=3'.replace(/(x=)(\d+)/, '$01115'));
// CHECK-NEXT: x=115
print('abcdefghijkl'.replace(/(a)(b)(c)(d)(e)(f)(g)(h)(i)(j)(k)(l)/, 'X$12Y'));
// CHECK-NEXT: XlY
print('abcabc'.replace('bc', function(matched, offset, string) {
  print(matched, offset, string);
  return 'XYZ';
}));
// CHECK-NEXT: bc 1 abcabc
// CHECK-NEXT: aXYZabc
print('x' + ''.replace('', function(matched, offset, string) {
  return 'XYZ';
}));
// CHECK-NEXT: xXYZ
print('abCdefgh'.replace(
  /([cC]).*([gG])/,
  function(matched, c, g, offset, string) {
    print(matched, c, g, offset, string);
    return 123;
  }
));
// CHECK-NEXT: Cdefg C g 2 abCdefgh
// CHECK-NEXT: ab123h
print('abCdefgh'.replace(
  /([cCgG])/g,
  function(matched, c, offset, string) {
    print(matched, c, offset, string);
    return 123;
  }
));
// CHECK-NEXT: C C 2 abCdefgh
// CHECK-NEXT: g g 6 abCdefgh
// CHECK-NEXT: ab123def123h
print('aaa'.replace(/aa/g, 'b'));
// CHECK-NEXT: ba
print('aaaa'.replace(/aa/g, 'b'));
// CHECK-NEXT: bb
print('aaaa'.replace(/(?:)/g, 'b'));
// CHECK-NEXT: babababab
var re = /ab/g;
re.lastIndex = 10;
print('ababc'.replace(re, '__'));
// CHECK-NEXT: ____c
print(re.lastIndex);
// CHECK-NEXT: 0

// test ES6 specific implementation
// borrowed from mjsunit/es6/string-replace.js
var patternReplace = {};
patternReplace[Symbol.replace] = function (string, newValue) {
  return string + newValue;
};

// Check object coercible fails.
try {
  String.prototype.replace.call(null, patternReplace, "x");
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError
// Override is called.
print("abcde".replace(patternReplace, "x"));
// CHECK-NEXT: abcdex
// Non-callable override.
patternReplace[Symbol.replace] = "dumdidum";
try {"abcde".replace(patternReplace, "x");} catch(e) {print(e.name);}
// CHECK-NEXT: TypeError

print('replaceAll');
// CHECK-LABEL: replaceAll
print('abcabc'.replaceAll('a', 'aa'));
// CHECK-NEXT: aabcaabc
print('abcabc'.replaceAll(/a/gi, 'aa'));
// CHECK-NEXT: aabcaabc
print('abcabc'.replaceAll('a', m => m + m));
// CHECK-NEXT: aabcaabc
print('abcabc'.replaceAll(/a/gi, m => m + m));
// CHECK-NEXT: aabcaabc
print('aa'.replaceAll('', '_'))
// CHECK-NEXT: _a_a_

print('abc-abc'.replaceAll('a', '$$'));
// CHECK-NEXT: $bc-$bc
print('abc-abc'.replaceAll('a', '$'));
// CHECK-NEXT: $bc-$bc
print('abc-abc'.replaceAll(/a/gi, '$&'));
// CHECK-NEXT: abc-abc
print('abc-abc'.replaceAll(/a/gi, "$'"));
// CHECK-NEXT: bc-abcbc-bcbc
print('abc-abc'.replaceAll('a', '$1'));
// CHECK-NEXT: $1bc-$1bc
print('foo bar'.replaceAll(/(\w+)/g, "$1"));
// CHECK-NEXT: foo bar
var regex = /(\w+)-(\w+)/g;
print('foo-foo bar-bar'.replaceAll(regex, "$1-$2"));
// CHECK-NEXT: foo-foo bar-bar
print('foo-foo bar-bar'.replaceAll(regex, function (m, p1, p2, offset) {
  return m + p1 + p2 + offset;
}));
// CHECK-NEXT: foo-foofoofoo0 bar-barbarbar8

print('search');
// CHECK-LABEL: search
print('abcabc'.search('ca'));
// CHECK-NEXT: 2
print('abcabc'.search(/ca/));
// CHECK-NEXT: 2
print('abcabc'.search('d'));
// CHECK-NEXT: -1
print('abcabc'.search(/d/));
// CHECK-NEXT: -1
print('abcabc'.search('.'));
// CHECK-NEXT: 0
print('abcabc'.search(/./));
// CHECK-NEXT: 0
print('54321'.search(1));
// CHECK-NEXT: 4
var rx = /a/;
rx.lastIndex = 2;
print('abcabc'.search(rx), rx.lastIndex);
// CHECK-NEXT: 0 2
print(''.search());
// CHECK-NEXT: 0
print('--undefined--'.search());
// CHECK-NEXT: 0

// test ES6 specific implementation
// borrowed from mjsunit/es6/string-search.js
var pattern = {};
pattern[Symbol.search] = function(string) {
  return string.length;
};
// Check object coercible fails.
try {
  String.prototype.search.call(null, pattern);
} catch (e) {
  print(e.name);
}
// CHECK-NEXT: TypeError

// Override is called.
print("abcde".search(pattern));
// CHECK-NEXT: 5

// Non-callable override.
pattern[Symbol.search] = "dumdidum";
try { "abcde".search(pattern); } catch (e) { print(e.name); }
// CHECK-NEXT: TypeError
