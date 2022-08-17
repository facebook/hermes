/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s

function show(iterResult) {
  print(iterResult.value, '|', iterResult.done);
}

print('generators');;
// CHECK-LABEL: generators

function* simple() {
  yield 1;
}

var it = simple();
show(it.next());
// CHECK-NEXT: 1 | false
show(it.next());
// CHECK-NEXT: undefined | true

try {
  new simple();
  print('must throw');
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError

function *useArgs(x, y) {
  yield x;
  yield x + 1;
  ++x;
  yield x + y;
}

var it = useArgs(100, 10);
show(it.next());
// CHECK-NEXT: 100 | false
show(it.next());
// CHECK-NEXT: 101 | false
show(it.next());
// CHECK-NEXT: 111 | false
show(it.next());
// CHECK-NEXT: undefined | true

(function() {
function *useArgsLocal(x, y) {
  yield x;
  yield x + 1;
  ++x;
  yield x + y;
}

var it = useArgsLocal(100, 10);
show(it.next());
// CHECK-NEXT: 100 | false
show(it.next());
// CHECK-NEXT: 101 | false
show(it.next());
// CHECK-NEXT: 111 | false
show(it.next());
// CHECK-NEXT: undefined | true
})();

function *locals(x,y) {
  var a,b,c;
  a = 4;
  yield a;
  b = a + 5;
  yield b;
  c = b + 6;
  yield c;
}

var it = locals(10, 42);
show(it.next());
// CHECK-NEXT: 4 | false
show(it.next());
// CHECK-NEXT: 9 | false
show(it.next());
// CHECK-NEXT: 15 | false
show(it.next());
// CHECK-NEXT: undefined | true

function *args() {
  yield arguments[0];
  yield arguments[1];
}

var it = args(184, 457);
show(it.next());
// CHECK-NEXT: 184 | false
show(it.next());
// CHECK-NEXT: 457 | false
show(it.next());
// CHECK-NEXT: undefined | true

function* thrower(x, y) {
  try {
    print('in try');
    yield 1;
  } catch (e) {
    print('in catch');
    yield e;
    return x;
  } finally {
    print('in finally');
    return y;
  }
}

var it = thrower(10, 20);
show(it.next(15));
// CHECK-NEXT: in try
// CHECK-NEXT: 1 | false
show(it.throw('MY ERROR'));
// CHECK-NEXT: in catch
// CHECK-NEXT: MY ERROR | false
show(it.throw());
// CHECK-NEXT: in finally
// CHECK-NEXT: 20 | true
show(it.next());
// CHECK-NEXT: undefined | true

function *returning(x, y) {
  try {
    print('try');
    yield x;
  } finally {
    print('finally');
    yield y;
  }
}

var it = returning(10, 20);
show(it.next(15));
// CHECK-NEXT: try
// CHECK-NEXT: 10 | false
show(it.return('MY RETVAL'));
// CHECK-NEXT: finally
// CHECK-NEXT: 20 | false
show(it.next());
// CHECK-NEXT: MY RETVAL | true

// Ensures that the StartGenerator instruction is moved to the start
// of the function after optimizations.
function *localsTry() {
  var x = 0;
  try {
    yield x;
  } catch (e) {
  }
}

var it = localsTry();
show(it.next());
// CHECK-NEXT: 0 | false
show(it.next());
// CHECK-NEXT: undefined | true

function *simpleDelegate() {
  yield* [1,2,3];
}

var it = simpleDelegate();
show(it.next());
// CHECK-NEXT: 1 | false
show(it.next());
// CHECK-NEXT: 2 | false
show(it.next());
// CHECK-NEXT: 3 | false
show(it.next());
// CHECK-NEXT: undefined | true

function *tryCatchDelegate() {
  function *gen() {
    try {
      yield 1;
      yield 2;
      yield 3;
    } catch (e) {
      print('gen caught', e);
      return 292;
    } finally {
      print('gen finally');
    }
  }

  try {
    var x = yield* gen();
  } catch (e) {
    print('outer caught', e);
    return 193;
  } finally {
    print('outer finally');
  }
  print('out of the try', x);
  return 1092;
}

var it = tryCatchDelegate()
show(it.next('a'));
// CHECK-NEXT: 1 | false
show(it.next('b'));
// CHECK-NEXT: 2 | false
show(it.return('c'));
// CHECK-NEXT: gen finally
// CHECK-NEXT: outer finally
// CHECK-NEXT: c | true

var it = tryCatchDelegate()
show(it.next('a'));
// CHECK-NEXT: 1 | false
show(it.next('b'));
// CHECK-NEXT: 2 | false
show(it.throw('c'));
// CHECK-NEXT: gen caught c
// CHECK-NEXT: gen finally
// CHECK-NEXT: outer finally
// CHECK-NEXT: out of the try 292
// CHECK-NEXT: 1092 | true

function *complexDelegate() {
  function *gen() {
    try {
      yield 1;
      yield 2;
      yield 3;
    } catch (e) {
      print('gen caught', e);
      return 292;
    } finally {
      print('gen finally');
      yield 919;
      return 100;
    }
  }

  try {
    var x = yield* gen();
  } catch (e) {
    print('outer caught', e);
    return 193;
  } finally {
    print('outer finally');
    return 101;
  }
  print(x);
}

var it = complexDelegate();
show(it.next('a'));
// CHECK-NEXT: 1 | false
show(it.next('b'));
// CHECK-NEXT: 2 | false
show(it.return('c'));
// CHECK-NEXT: gen finally
// CHECK-NEXT: 919 | false
show(it.next('d'));
// CHECK-NEXT: outer finally
// CHECK-NEXT: 101 | true
show(it.next('e'));
// CHECK-NEXT: undefined | true

var it = complexDelegate();
show(it.next('a'));
// CHECK-NEXT: 1 | false
show(it.next('b'));
// CHECK-NEXT: 2 | false
show(it.throw('c'));
// CHECK-NEXT: gen caught c
// CHECK-NEXT: gen finally
// CHECK-NEXT: 919 | false
show(it.next('d'));
// CHECK-NEXT: outer finally
// CHECK-NEXT: 101 | true
show(it.next('e'));
// CHECK-NEXT: undefined | true

// Ensure that we don't unwrap/rewrap the value/done properties
// of the results of .next(), and make sure that abrupt .return() works.
function *iterDelegateWithSecret() {
  var count = 0;
  var iterable = {};
  iterable[Symbol.iterator] = function() {
    return {
      next(x) {
        print('from inside:', x, arguments.length);
        count++;
        return { value: count, done: count > 1, SECRET: 42 };
      },
      return() {
        print('closing iterator');
        return {};
      }
    }
  }
  yield* iterable;
}

var it = iterDelegateWithSecret();
var result = it.next(1234);
// CHECK-NEXT: from inside: undefined 1
show(result);
// CHECK-NEXT: 1 | false
print(result.SECRET);
// CHECK-NEXT: 42
show(it.next(1234));
// CHECK-NEXT: from inside: 1234 1
// CHECK-NEXT: undefined | true
show(it.next());
// CHECK-NEXT: undefined | true

var it = iterDelegateWithSecret();
show(it.next());
// CHECK-NEXT: from inside: undefined 1
// CHECK-NEXT: 1 | false
try { it.throw() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: closing iterator
// CHECK-NEXT: TypeError yield* delegate must have a .throw() method

var iter = {
  [Symbol.iterator]: function() {
    print('OPEN')
    return {
      next: function() {
        return { value: 42, done: false };
      }
    };
  }
};

var f = function*([x]) {
  print('START', x)
  return 5;
};
print(f.length);
// CHECK-NEXT: 1
var it = f(iter);
// CHECK-NEXT: OPEN
show(it.next())
// CHECK-NEXT: START 42
// CHECK-NEXT: 5 | true

var iterable = {
  next() {
    return { value: 1, done: false };
  },
  get return() {
    print('get return');
    return null;
  },
  [Symbol.iterator]() {
    return iterable;
  },
};

function* generator() {
  yield* iterable;
}

// GetMethod returns undefined, so there shouldn't be an attempt to call.
var iterator = generator();
print(iterator.next().value);
iterator.return(123);
// CHECK-NEXT: 1
// CHECK-NEXT: get return

var gen = function* genAlias() { yield 1 };
show(gen().next())
// CHECK-NEXT: 1 | false

var gen = function* genAlias() {
  print(genAlias() !== undefined)
};
show(gen().next())
// CHECK-NEXT: true
// CHECK-NEXT: undefined | true

// Make sure using SaveGeneratorLong works.
function* saveGeneratorLong() {
    yield* [1];
    // Waste some registers, to change SaveGenerator to SaveGeneratorLong.
    [].push(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
}
print(saveGeneratorLong().next().value);
// CHECK-NEXT: 1
