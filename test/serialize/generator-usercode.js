// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes -O -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer

function show(iterResult) {
  print(iterResult.value, '|', iterResult.done);
}


function* simple() {
  yield 1;
}

var it = simple();

function *useArgs(x, y) {
  yield x;
  yield x + 1;
  ++x;
  yield x + y;
}

var it2 = useArgs(100, 10);
show(it2.next());
show(it2.next());

function *locals(x,y) {
  var a,b,c;
  a = 4;
  yield a;
  b = a + 5;
  yield b;
  c = b + 6;
  yield c;
}

var it3 = locals(10, 42);
show(it3.next());

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

var it4 = thrower(10, 20);

function *returning(x, y) {
  try {
    print('try');
    yield x;
  } finally {
    print('finally');
    yield y;
  }
}

var it5 = returning(10, 20);

serializeVM(function() {
  print('generators');;
  // CHECK-LABEL: generators

  show(it.next());
  // CHECK-NEXT: 1 | false
  show(it.next());
  // CHECK-NEXT: undefined | true

  show(it2.next());
  // CHECK-NEXT: 111 | false
  show(it2.next());
  // CHECK-NEXT: undefined | true

  show(it3.next());
  // CHECK-NEXT: 9 | false
  show(it3.next());
  // CHECK-NEXT: 15 | false
  show(it3.next());
  // CHECK-NEXT: undefined | true

  show(it4.next(15));
  // CHECK-NEXT: in try
  // CHECK-NEXT: 1 | false
  show(it4.throw('MY ERROR'));
  // CHECK-NEXT: in catch
  // CHECK-NEXT: MY ERROR | false
  show(it4.throw());
  // CHECK-NEXT: in finally
  // CHECK-NEXT: 20 | true
  show(it4.next());
  // CHECK-NEXT: undefined | true

  show(it5.next(15));
  // CHECK-NEXT: try
  // CHECK-NEXT: 10 | false
  show(it5.return('MY RETVAL'));
  // CHECK-NEXT: finally
  // CHECK-NEXT: 20 | false
  show(it5.next());
  // CHECK-NEXT: MY RETVAL | true
})
