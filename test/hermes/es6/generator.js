// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

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
