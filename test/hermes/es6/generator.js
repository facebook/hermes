// RUN: %hermes -Xflow-parser %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Xflow-parser -O %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

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
