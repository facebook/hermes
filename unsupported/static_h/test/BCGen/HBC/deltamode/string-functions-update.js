/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: LANG=en_US.UTF-8 %hermes -O -emit-binary -target=HBC -out=%T/base.hbc %S/Inputs/string-functions-base.js.in && LANG=en_US.UTF-8 %hermes -emit-binary -target=HBC -O -base-bytecode %T/base.hbc -out %T/update.hbc %s && LANG=en_US.UTF-8 %hermes -b %T/update.hbc | %FileCheck --match-full-lines %s

// This test is used for verifying the correct behavior of the bytecode compiled with the delta optimizing mode.
// First generate the base bytecode, then generate the new bytecode, finally run the new bytecode.
// Adopted from the test string-function.js.

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
print(String.fromCharCode(0xff0048, 0xe8, 114, 109, 101, 115));
// CHECK-NEXT: Hèrmes

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

print('trimLeft');
// CHECK-LABEL: trimLeft
print(String.prototype.trimLeft.length);
// CHECK-NEXT: 0
print('abc'.trimLeft());
// CHECK-NEXT: abc
var res = ' abc '.trimLeft();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = '\ufeff\n abc '.trimLeft();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = '  '.trimLeft();
print('empty', res, res.length);
// CHECK-NEXT: empty  0

print('trimRight');
// CHECK-LABEL: trimRight
print(String.prototype.trimRight.length);
// CHECK-NEXT: 0
print('abc'.trimRight());
// CHECK-NEXT: abc
var res = ' abc '.trimRight();
print(res, res.length);
// CHECK-NEXT: abc  4
var res = ' abc \n\ufeff'.trimRight();
print(res, res.length);
// CHECK-NEXT:  abc 4
var res = '  '.trimRight();
print('empty', res, res.length);
// CHECK-NEXT: empty  0

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

print('replace');
// CHECK-LABEL: replace
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
