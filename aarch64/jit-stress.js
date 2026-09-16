/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// A differential-testing workload for the JIT: it must print identical output
// with and without -Xjit=force. It deliberately touches emitters across the
// backend rather than testing any one of them deeply.

var out = [];
function log(x) {
  out.push(String(x));
}

// Arithmetic, bit ops, NaN/-0 corners, the *N fast paths.
function arith(n) {
  var s = 0, f = 1.5;
  for (var i = 0; i < n; ++i) {
    s += i * 3 - (i >> 2) + (i | 1) + (i ^ 7) + (i & 3);
    s -= (i << 1) >>> 3;
    f = f * 1.0000001 + i / 7 - (i % 5);
  }
  return [s, f.toFixed(6)];
}
log(arith(200));
log([0 / 0, 1 / 0, -1 / 0, -0, (0 === -0), Object.is(-0, 0 * -1)]);
log([(2147483647 | 0) + 1, -2147483648 >> 1, -1 >>> 28, 5 % -3, -5 % 3]);
log([Math.pow(2, 53) + 1, 0.1 + 0.2, (1e308 * 10)]);

// Comparisons incl. NaN, and the inverted branch forms.
function cmp(a, b) {
  var r = [];
  r.push(a < b, a <= b, a > b, a >= b, a == b, a === b, a != b, a !== b);
  if (!(a < b)) r.push("!lt");
  if (!(a >= b)) r.push("!ge");
  return r;
}
log(cmp(1, 2));
log(cmp(NaN, 1));
log(cmp(1, NaN));
log(cmp("a", "b"));
log(cmp(null, undefined));

// Strings: concat, AddS, comparison, interning behaviour.
function strs(n) {
  var s = "";
  for (var i = 0; i < n; ++i) s += (i % 10);
  return [s.length, s.charCodeAt(5), s.slice(3, 9), s === s, s > "0"];
}
log(strs(50));
log(["x" + 1, 1 + "x", "a" + null, "" + undefined, "" + {}, "" + [1, 2]]);

// Property access: inline caches, prototype chains, accessors, shape changes.
function Point(x, y) {
  this.x = x;
  this.y = y;
}
Point.prototype.norm = function () {
  return this.x * this.x + this.y * this.y;
};
Object.defineProperty(Point.prototype, "tag", {
  get: function () {
    return "P" + this.x;
  },
});
function props(n) {
  var acc = 0, tags = "";
  for (var i = 0; i < n; ++i) {
    var p = new Point(i, i + 1);
    acc += p.norm();
    if (i % 20 === 0) tags += p.tag;
    if (i === 30) p.extra = 1; // shape transition mid-loop
  }
  return [acc, tags];
}
log(props(100));

// Objects/arrays from literal buffers, then mutation.
log([{ a: 1, b: "two", c: null, d: true, e: 2.5 }, [1, 2, 3, "four", null]]);
var big = {};
for (var i = 0; i < 40; ++i) big["k" + i] = i * i;
log([Object.keys(big).length, big.k39, big.k0]);

// Arrays: dense/sparse, push/pop, holes, iteration.
function arrays(n) {
  var a = [];
  for (var i = 0; i < n; ++i) a.push(i);
  a[n + 5] = "sparse";
  var sum = 0;
  for (var j = 0; j < a.length; ++j) if (a[j] !== undefined) sum += typeof a[j] === "number" ? a[j] : 0;
  return [a.length, sum, a.indexOf(7), a.pop(), a.slice(-3).join(",")];
}
log(arrays(60));

// Builtins that read `this` -- the CallBuiltin ThisArg regression.
function builtinThis() {
  function Ctor() {}
  Ctor.method = function () {
    return 1;
  };
  Ctor.method(); // leaves Ctor in the shared outgoing ThisArg slot
  var r = Array.from([10, 20, 30]);
  return [Array.isArray(r), r instanceof Ctor, r.join("|")];
}
log(builtinThis());

// Closures, environments (incl. deeper nesting), recursion.
function makeCounter(start) {
  var n = start;
  return function () {
    var inner = function () {
      return ++n;
    };
    return inner();
  };
}
var c = makeCounter(10);
log([c(), c(), c()]);
function fib(n) {
  return n < 2 ? n : fib(n - 1) + fib(n - 2);
}
log(fib(20));

// Exceptions: try/catch/finally, throw across frames, catch in a loop.
function thrower(i) {
  if (i % 3 === 0) throw new Error("e" + i);
  return i;
}
function exceptions(n) {
  var ok = 0, bad = 0, fin = 0;
  for (var i = 0; i < n; ++i) {
    try {
      ok += thrower(i);
    } catch (e) {
      bad += e.message.length;
    } finally {
      fin++;
    }
  }
  return [ok, bad, fin];
}
log(exceptions(30));
try {
  null.x;
} catch (e) {
  log(e instanceof TypeError);
}

// Switches (dense uint and string), typeof, instanceof, in.
function sw(x) {
  switch (x) {
    case 0: return "zero";
    case 1: return "one";
    case 2: return "two";
    case 3: return "three";
    default: return "many";
  }
}
log([sw(0), sw(2), sw(9)]);
function ssw(s) {
  switch (s) {
    case "alpha": return 1;
    case "beta": return 2;
    case "gamma": return 3;
    default: return 0;
  }
}
log([ssw("alpha"), ssw("gamma"), ssw("zzz")]);
log([typeof 1, typeof "s", typeof undefined, typeof null, typeof {}, typeof log, typeof true]);
log([[] instanceof Array, "x" in { x: 1 }, 0 in [1], "length" in []]);

// Classes, super, getters, static, private-ish patterns.
class Base {
  constructor(v) {
    this.v = v;
  }
  get double() {
    return this.v * 2;
  }
  describe() {
    return "Base(" + this.v + ")";
  }
}
class Derived extends Base {
  constructor(v) {
    super(v + 1);
  }
  describe() {
    return "Derived>" + super.describe();
  }
}
var d = new Derived(4);
log([d.v, d.double, d.describe(), d instanceof Base]);

// Iterators, spread, destructuring, for-of, generators.
function* gen(n) {
  for (var i = 0; i < n; ++i) yield i * i;
}
log([...gen(5)]);
var [p, q, ...rest] = [1, 2, 3, 4, 5];
log([p, q, rest]);
var { a: aa = 9, z: zz = 42 } = { a: 1 };
log([aa, zz]);
var forOf = 0;
for (var v of [1, 2, 3, 4]) forOf += v;
log(forOf);

// for-in over a mutated object (GetPNameList / GetNextPName).
var fi = { one: 1, two: 2, three: 3 };
var keys = [];
for (var k in fi) keys.push(k);
log(keys.sort().join(","));

// arguments object, reified and not.
function argSum() {
  var s = 0;
  for (var i = 0; i < arguments.length; ++i) s += arguments[i];
  return [s, arguments.length];
}
log(argSum(1, 2, 3, 4));

// Higher-order builtins that call back into JIT'd code.
log([1, 2, 3, 4, 5].map(function (x) { return x * x; }).filter(function (x) { return x % 2; }));
log([3, 1, 2].sort(function (a, b) { return a - b; }).join("-"));
log([1, 2, 3].reduce(function (a, b) { return a + b; }, 0));

// JSON round-trip (exercises the runtime broadly).
log(JSON.parse(JSON.stringify({ n: 1, s: "x", b: [1, null, true] })).b[2]);

print(out.join("\n"));
