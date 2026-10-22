/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

function *iter() {
  for (var i = 0; i < 5; ++i) {
    print(i);
    yield i;
  }
}

print('START');
// CHECK-LABEL: START

var [x,y] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
print(x,y);
// CHECK-NEXT: 0 1

var [a,b,...[]] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
print(a,b);
// CHECK-NEXT: 0 1

var [c,d,...[e,f]] = iter();
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
print(c,d,e,f);
// CHECK-NEXT: 0 1 2 3

function tracked(values) {
  var calls = 0;
  var closes = 0;
  return {
    [Symbol.iterator]() {
      return this;
    },
    next() {
      var index = calls++;
      if (index >= values.length) {
        if (index > values.length) throw 'next after done';
        return {
          done: true,
          get value() { throw 'value after done'; },
        };
      }
      return {done: false, value: values[index]};
    },
    return() {
      ++closes;
      return {};
    },
    counts() {
      return calls + ':' + closes;
    },
  };
}

function takeThree(source) {
  var [a, b, c] = source;
  print(a, b, c);
}

print('exhaustion');
// CHECK-NEXT: exhaustion
takeThree([]);
// CHECK-NEXT: undefined undefined undefined
takeThree([1]);
// CHECK-NEXT: 1 undefined undefined
var empty = tracked([]);
takeThree(empty);
// CHECK-NEXT: undefined undefined undefined
print(empty.counts());
// CHECK-NEXT: 1:0
var short = tracked([1]);
takeThree(short);
// CHECK-NEXT: 1 undefined undefined
print(short.counts());
// CHECK-NEXT: 2:0
var live = tracked([1, 2, 3, 4]);
takeThree(live);
// CHECK-NEXT: 1 2 3
print(live.counts());
// CHECK-NEXT: 3:1

var defaults = tracked([undefined]);
var [d1 = 10, d2 = 20, d3 = 30] = defaults;
print(d1, d2, d3, defaults.counts());
// CHECK-NEXT: 10 20 30 2:0
var elisions = tracked([1]);
var [, , skipped] = elisions;
print(skipped, elisions.counts());
// CHECK-NEXT: undefined 2:0
var restSource = tracked([]);
var [r1, r2, ...remaining] = restSource;
print(r1, r2, remaining.length, restSource.counts());
// CHECK-NEXT: undefined undefined 0 1:0
var fullRestSource = tracked([1, 2, 3]);
var [head, ...tail] = fullRestSource;
print(head, tail.join(','), fullRestSource.counts());
// CHECK-NEXT: 1 2,3 4:0

function fail() { throw 'original'; }
function throwingDefault(source) {
  try {
    var [a = fail(), b] = source;
  } catch (e) {
    print(e, source.counts());
  }
}
var throwingLive = tracked([undefined, 2]);
var throwingCloses = 0;
throwingLive.return = function() { ++throwingCloses; throw 'close'; };
throwingDefault(throwingLive);
// CHECK-NEXT: original 1:0
print(throwingCloses);
// CHECK-NEXT: 1
throwingDefault(tracked([]));
// CHECK-NEXT: original 1:0

function throwingTarget(source) {
  var object = {set value(x) { throw 'target'; }};
  try {
    [object.value] = source;
  } catch (e) {
    print(e, source.counts());
  }
}
throwingTarget(tracked([1]));
// CHECK-NEXT: target 1:1
throwingTarget(tracked([]));
// CHECK-NEXT: target 1:0

var nestedSource = tracked([[], [1]]);
var [[n1 = 10, n2 = 20], [n3, n4], n5 = 30] = nestedSource;
print(n1, n2, n3, n4, n5, nestedSource.counts());
// CHECK-NEXT: 10 20 1 undefined 30 3:0

function *cleanup() {
  try {
    yield undefined;
    yield 2;
  } finally {
    print('cleanup');
  }
}
try {
  var [g = fail()] = cleanup();
} catch (e) {
  print(e);
}
// CHECK-NEXT: cleanup
// CHECK-NEXT: original

var growing = [];
var [grown1 = (growing.push(1), 10), grown2] = growing;
print(grown1, grown2, growing.length);
// CHECK-NEXT: 10 undefined 1

var customArray = [1, 2];
customArray[Symbol.iterator] = function() { return tracked([3]); };
takeThree(customArray);
// CHECK-NEXT: 3 undefined undefined

var nextThrows = tracked([1]);
nextThrows.next = function() { throw 'next'; };
try {
  takeThree(nextThrows);
} catch (e) {
  print(e, nextThrows.counts());
}
// CHECK-NEXT: next 0:0

var exhaustedLookup = tracked([]);
var exhaustedReturnLookups = 0;
Object.defineProperty(exhaustedLookup, 'return', {
  get() { ++exhaustedReturnLookups; throw 'return lookup after done'; },
});
throwingDefault(exhaustedLookup);
// CHECK-NEXT: original 1:0
print(exhaustedReturnLookups);
// CHECK-NEXT: 0

function stepFailure(result) {
  var source = tracked([]);
  source.next = function() { return result; };
  try {
    takeThree(source);
  } catch (e) {
    print(typeof e === 'string' ? e : e.name, source.counts());
  }
}
stepFailure(1);
// CHECK-NEXT: TypeError 0:0
stepFailure({get done() { throw 'done'; }});
// CHECK-NEXT: done 0:0
stepFailure({done: false, get value() { throw 'value'; }});
// CHECK-NEXT: value 0:0

var innerFailure = tracked([]);
innerFailure.next = function() { throw 'inner'; };
var outerFailure = tracked([innerFailure]);
try {
  var [[innerValue]] = outerFailure;
} catch (e) {
  print(e, outerFailure.counts(), innerFailure.counts());
}
// CHECK-NEXT: inner 1:1 0:0

function closeFailure(abrupt, getter, primitive) {
  var source = tracked([undefined, 2]);
  var closes = 0;
  Object.defineProperty(source, 'return', {
    get() {
      if (getter) { ++closes; throw 'return getter'; }
      return function() {
        ++closes;
        if (primitive) return 0;
        throw 'return method';
      };
    },
  });
  try {
    if (abrupt) {
      var [a = fail()] = source;
    } else {
      var [a] = source;
    }
  } catch (e) {
    print(typeof e === 'string' ? e : e.name, closes);
  }
}
closeFailure(false, true, false);
// CHECK-NEXT: return getter 1
closeFailure(false, false, false);
// CHECK-NEXT: return method 1
closeFailure(false, false, true);
// CHECK-NEXT: TypeError 1
closeFailure(true, true, false);
// CHECK-NEXT: original 1
closeFailure(true, false, false);
// CHECK-NEXT: original 1
closeFailure(true, false, true);
// CHECK-NEXT: original 1

function *suspendDefault(source) {
  var [a = yield 'pause', b] = source;
}
var suspendedLive = tracked([undefined, 2]);
var generatorLive = suspendDefault(suspendedLive);
print(generatorLive.next().value);
generatorLive.return();
print(suspendedLive.counts());
// CHECK-NEXT: pause
// CHECK-NEXT: 1:1
var suspendedDone = tracked([]);
var generatorDone = suspendDefault(suspendedDone);
print(generatorDone.next().value);
try {
  generatorDone.throw('resume');
} catch (e) {
  print(e, suspendedDone.counts());
}
// CHECK-NEXT: pause
// CHECK-NEXT: resume 1:0
