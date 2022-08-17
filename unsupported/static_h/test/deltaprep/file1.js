/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

print('toString');
print(Array(1,2,3).toString());
print('empty', Array().toString());

// Overwrite join and make sure it fails over to Object.prototype.toString().
var a = Array(1,2,3);
a.join = null;
print(a.toString());
var ots = Object.prototype.toString;
Object.prototype.toString = null;
print(a.toString());
Object.prototype.toString = ots;
var a = [1,2];
a.push(a);
print(a.toString());
var a = [1,2];
var b = [3,4,a];
a.push(b);
print(a.toString());
var a = [1,2];
var b = [3,4];
var c = [5,6,b];
b.push(c);
a.push(b);
print(a.toString());

print('toLocaleString');
print([1,2,3].toLocaleString());
var n = 0;
var obj = {toLocaleString: function() {++n; return 1337;}};
print([1,2,obj,3,obj].toLocaleString());
print(n);
var a = [1,2];
a.push(a);
print(a.toLocaleString());
var a = [1,2];
var b = [3,4,a];
a.push(b);
print(a.toLocaleString());
var a = [1,2];
var b = [3,4];
var c = [5,6,b];
b.push(c);
a.push(b);
print(a.toLocaleString());
try {[{toLocaleString:1}].toLocaleString()}
catch(e) {print('caught', e.name, e.message);}

print('concat');
print([1,2].concat([3,4]));
print([].concat([3,4]));
print([1,2].concat([]));
print('empty', [].concat([]));
print(['a','b'].concat(['c','d'], 'efg', 123, {}));
var a = [1,2].concat([[3,4],[5,6]]);
print(a[0], a[1], a[2][0], a[2][1], a[3][0], a[3][1]);
print([1,2].concat(3, {0: 4, 1: 5, length: 2}));
var a = [1,,];
a.__proto__[1] = 2;
print(a.concat(3));
delete a.__proto__[1];
var a = [1,2,3]
a = [];
print(typeof a.concat(100)[0]);
Array.prototype[1] = 1;
var x = [];
x.length = 2;
var a = x.concat();
print(a.hasOwnProperty('1'));
delete Array.prototype[1];

print('join');
print(Array(1,2,3).join());
print(Array(1,2,3).join(','));
print(Array(1,2,3).join(':'));
print(Array(1,2,Array(3,4),5).join(':'));
print(Array(1,2,3).join(' SEPARATOR '));
print('empty', Array().join());
print('empty', Array().join('::::'));
print(Array(null,2,undefined,3,null).join('-'));

print('pop');
var a = Array(1,2,3);
print(a.pop(), a.length, a[0], a[1], a[2]);
print(a.pop(), a.length, a[0], a[1], a[2]);
print(a.pop(), a.length, a[0], a[1], a[2]);
print(a.pop(), a.length, a[0], a[1], a[2]);
print(a.pop(), a.length, a[0], a[1], a[2]);

print('push');
var a = [1,2,3];
print(a.push(1834), a.toString());
print(a.push('abcd', 746, 133), a.toString());
print(a.push(), a.toString());
print(a.pop(), a.toString());
print(a.push('last'), a.toString());
var a = [,,,'x','y','z'];
print(a)
print(a.push(1,2,3), a);
var a = {3: 'x', 4: 'y', length: 5};
Array.prototype.push.call(a, 'z', 'last');
print(a.length, a[5], a[6]);
var a = Array(0xfffffffe);
print(a.length);
a.push('arrEnd');
print(a.length, a[4294967294]);
try { a.push('a','b','c'); } catch (e) { print('caught', e.name) }
print(a[4294967295], a[4294967296], a[4294967297]);
Array.prototype[1] = 'asdf';
Object.defineProperty(Array.prototype, '1', {
  value: 'asdf',
  writable: false,
  configurable: true,
});
var a = [];
try { a.push('x','y','z'); } catch (e) { print('caught', e.name) }
delete Array.prototype[1];
Object.defineProperty(Array.prototype, '1', {
  get: function() { return 'asdf'; },
  set: function() { print('setter'); },
  configurable: true,
});
var a = [];
print(a.push(1,2), a);
delete Array.prototype[1];
var a = [];
a[0] = 'x1';
a[10000] = 'x2';
print(a.push('last'), a[0], a[10000], a[10001]);
a = null;

print('reverse');
var a = [1,2,3];
print(a.reverse(), a);
print([1,2].reverse());
print('empty', [].reverse());
print([12,13,14,15,16,17,18].reverse());
print([,,,1].reverse());
print([,,1,,5,7,,].reverse());
var a = {};
a[0] = 'a';
a[1] = 'b';
a[5] = 'c';
a.length = 6;
Array.prototype.reverse.call(a);
print(a[0], a[4], a[5]);

print('shift');
var a = [1,2,3];
print(a.shift(), a, a.length);
print(a.shift(), a, a.length);
print(a.shift(), a, a.length);
print(a.shift(), a, a.length);
print(a.shift(), a, a.length);

print('slice');
var a = ['a','b','c','d','e'];
print(a.slice(1, 3), a.slice(1,3).length, a);
print(a.slice(1));
print(a.slice(1, 1000));
print(a.slice());
print(a.slice(-1000));
print(a.slice(-1000, 1000));
print(a.slice(4, 1000));
print('empty', a.slice(4, -1000));
var a = [];
print('empty', a.slice(4, 1000));
print('empty', a.slice(-10, 10));
print('empty', a.slice());

print('sort');
print('empty', [].sort());
print([1,2].sort());
print([2,1].sort());
print([2,13].sort());
print([2,13].sort(function(x,y) {return x - y}));
print(['a', 'aaa', 'aa'].sort(function(x,y) {return x.length - y.length}));
print([4,3,5,1,8,6,7,2].sort());
print([8,7,6,5,4,3,2,1].sort());
print([,undefined,,8,,,7,6,5,4,3,2,1].sort());
print([8,32,86,25,71,92,66,68,97,52].sort(function(x,y) {return y - x;}));
print([
  93,62,92,70,2,64,61,16,85,18,3,18,59,35,54,9,31,50,67,37,82,6,36
].sort(function(x,y) {return x - y;}));
var a = {0: 'c', 1: 'b', 2: 'a', 3: 'f', 4: 'd', 5: 'g', 6: 'e'};
a.length = 7;
print(Array.prototype.join.call(Array.prototype.sort.call(a), ''));
var a = {0: 2, 1: 13};
a.length = 2;
print(Array.prototype.join.call(Array.prototype.sort.call(a)));
var a = {0: 2, 1: 13};
a.length = 2;
print(Array.prototype.join.call(Array.prototype.sort.call(a, function(x,y) {
  return x - y;
})));
try { [1,2,3].sort(null); } catch(e) { print('caught', e.name); }
try { [1].sort(12); } catch(e) { print('caught', e.name); }
try { [].sort(true); } catch(e) { print('caught', e.name); }
try { [1,2,3].sort({}); } catch(e) { print('caught', e.name); }

print('splice');
var a = ['a','b','c','d'];
print(a.splice(), a);
var a = ['a','b','c','d'];
print(a.splice(1), a);
var a = ['a','b','c','d'];
print(a.splice(1, 2), a);
var a = ['a','b','c','d'];
print(a.splice(1, 2, 'x'), a);
var a = ['a','b','c','d'];
print(a.splice(1, 2, 'x', 'y', 'z'), a);
var a = ['a','b','c','d'];
print(a.splice(1, 1000, 'x', 'y', 'z'), a);
var a = ['a','b','c','d'];
print(a.splice(-1, 1, 'x', 'y', 'z'), a);
var a = ['a','b','c','d'];
print(a.splice(-1, 1, 'x', 'y', 'z'), a);
var a = ['a','b','c','d'];
print(a.splice(2, 0, 'x', 'y', 'z'), a);
var a = ['a','b','c','d'];
print(a.splice(2, -100, 'x', 'y', 'z'), a);
var a = ['a',,'b',,,'c'];
print(a.splice(0, 3, 'x', 'y'), a);

print('unshift');
var a = [1,2,3];
print(a.unshift('a', 'b'), a);
print(a.unshift(), a);
print(a.unshift('c'), a);
var a = [];
print(a.unshift(1,2,3), a);
var a = [];
print(a.unshift(0,0,0,0,0,0,0,0,0,0), a);

print('indexOf');
var a = ['a','b','c','b','x'];
print(a.indexOf('b'));
print(a.indexOf('b', 0));
print(a.indexOf('b', -2));
print(a.indexOf('b', 2));
print(a.indexOf('b', -1));
print(a.indexOf('b', 4));
print(a.indexOf('d'));
var a = [];
print(a.indexOf('a', -1));
print(a.indexOf('a', 4));
print(a.indexOf('a'));
print(1 / [true].indexOf(true, -0));
//CHECK-NEXT: Infinity

print('lastIndexOf');
var a = ['a','b','c','b','x'];
print(a.lastIndexOf('b'));
print(a.lastIndexOf('b', 0));
print(a.lastIndexOf('b', -2));
print(a.lastIndexOf('b', 2));
print(a.lastIndexOf('b', -1));
print(a.lastIndexOf('b', 4));
print(a.lastIndexOf('d'));
var a = [];
print(a.lastIndexOf('a', -1));
print(a.lastIndexOf('a', 4));
print(a.lastIndexOf('a'));

print('every');
print([1,2,3].every(function(x) {return x < 4}));
print([1,2,3].every(function(x) {return x < 2}));
print([].every(function(x) {return false}));
print(['!', '@', '#'].every(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}, 'thisval'));
try {[1].every(1)} catch(e) {print('caught', e.name, e.message);}

print('some');
print([1,2,3].some(function(x) {return x < 4}));
print([1,2,3].some(function(x) {return x < 2}));
print([1,2,3].some(function(x) {return x < 0}));
print([].some(function(x) {return false}));
print(['!', '@', '#'].some(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}, 'thisval'));

print('foreach');
print(['!', '@', '#'].forEach(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}));
print(['!', '@', '#'].forEach(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}, 'thisval'));
try {[1].forEach(1)} catch(e) {print('caught', e.name, e.message);}

print('map');
print(['!', '@', '#'].map(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}));
print(['!', '@', '#'].map(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}, 'thisval'));
try {[1].map(1)} catch(e) {print('caught', e.name, e.message);}

print('filter');
var a = [1,2,3];
print(a.filter(function(x) {return x === 2}));
print(a.filter(function(x) {return x !== 2}));
print(a.filter(function() {return true}));
print('empty', a.filter(function() {return false}));
var a = [];
print('empty', a.filter(function() {return true}));
print(['!', '@', '#'].filter(function(x, i, arr) {
  print(this, x, i, arr);
  return x === '!';
}, 'thisval'));
var a = [null,2,3];
var acc = {get: function() {return 1000}};
Object.defineProperty(a, 0, acc);
print(a.filter(function(x) {return x === 1000}));
var a = {};
a[0] = 1239;
a[3] = 18583;
a.length = 6;
print(Array.prototype.filter.call(a, function(x) {return x === 1239}));
try {[1].filter(1)} catch(e) {print('caught', e.name, e.message);}

print('fill');
var a = Array(3);
a.fill();
print(a[0], a[1], a[2]);
a.fill(1);
print(a[0], a[1], a[2]);
a.fill(2, 0, a.length);
print(a[0], a[1], a[2]);
a.fill(3, -1);
print(a[0], a[1], a[2]);
a.fill(4, 1, -1);
print(a[0], a[1], a[2]);
var a = {};
a[0] = 1;
a[3] = 2;
a.length = 6;
Array.prototype.fill.call(a, 0);
print(a[0], a[1], a[2], a[3], a[4], a[5]);

print('Pop from large array');
// This caused a crash with SegmentedArray used as the backing storage.
// It was due to an off-by-one error in SegmentedArray::decreaseSize.
var a = Array(4097).fill();
// This pop causes the backing storage to shrink to only be inline storage.
a.pop();
print(a[a.length - 1]);
a = Array(5121).fill();
// This pop causes the backing storage to shrink to one segment instead of 2.
a.pop();
print(a[a.length - 1]);

print('find');
print(Array.prototype.find.length);
print([].find(function(){}));
print([1,2,3].find(function(v){ return v === 4 }));
print([0,1,2].find(function(v){ return v }));
print(['a','b','c'].find(function(v, k, obj) {
  print(this, v, k, obj);
  return v === 'b';
}, 'thisarg'));
print(Array.prototype.find.call(
  {0:'a',1:'b',length:3},
  function(v) { return v === 'b'; }
));

print('findIndex');
print(Array.prototype.findIndex.length);
print([].findIndex(function(){}));
print([1,2,3].findIndex(function(v){ return v === 4 }));
print([0,1,2].findIndex(function(v){ return v }));
print(['a','b','c'].findIndex(function(v, k, obj) {
  print(this, v, k, obj);
  return v === 'b';
}, 'thisarg'));
print(Array.prototype.findIndex.call(
  {0:'a',1:'b',length:3},
  function(v) { return v === 'b'; }
));

print('reduce');
var a = [1,2,3,4];
print(a.reduce(function(x,y) {return x + y}));
print(a.reduce(function(x,y) {return x + y}, 100));
print(a.reduce(function(x,y) {return x * y}));
var a = [];
print(a.reduce(function() {}, 10));
var a = [,,,,1,2,,,,3,,4,,,];
print(a.reduce(function(x,y) {return x + y}));
print(a.reduce(function(x,y) {return x + y}, 100));
print(['!', '@', '#'].reduce(function(x, y, i, arr) {
  print(this, x, y, i, arr);
  return x + y;
}, '**'));
try {[1].reduce(1)} catch(e) {print('caught', e.name, e.message);}
var a = Array(10);
try {print(a.reduce(function(){}));} catch(e) {print('caught', e.name, e.message)}

print('reduceRight');
var a = [1,2,3,4];
print(a.reduceRight(function(x,y) {return x + y}));
print(a.reduceRight(function(x,y) {return x + y}, 100));
print(a.reduceRight(function(x,y) {return x * y}));
var a = [];
print(a.reduceRight(function() {}, 10));
var a = [,,,,1,2,,,,3,,4,,,];
print(a.reduceRight(function(x,y) {return x + y}));
print(a.reduceRight(function(x,y) {return x + y}, 100));
print(['!', '@', '#'].reduceRight(function(x, y, i, arr) {
  print(this, x, y, i, arr);
  return x + y;
}, '**'));
var a = Array(10);
try {print(a.reduceRight(function(){}));} catch(e) {print('caught', e.name, e.message)}
try {[1].reduceRight(1)} catch(e) {print('caught', e.name, e.message);}
try {print(a.reduce(function(){}));} catch(e) {print('caught', e.name, e.message)}
