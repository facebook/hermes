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
