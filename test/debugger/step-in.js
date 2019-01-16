// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step in');
// CHECK-LABEL: step in

function foo() {
  print('first');
  var x = bar();
  print('second');
}

function bar() {
  return 'second';
}

debugger;
foo();
print('returned');

// CHECK: Break on 'debugger' statement in {{.*}}:17:1
// CHECK-NEXT: Stepped to global: {{.*}}:18:1
// CHECK-NEXT: Stepped to foo: {{.*}}:8:3
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to foo: {{.*}}:9:11
// CHECK-NEXT: Stepped to bar: {{.*}}:14:3
// CHECK-NEXT: Stepped to foo: {{.*}}:9:9
// CHECK-NEXT: Stepped to foo: {{.*}}:10:3
// CHECK-NEXT: second
// CHECK-NEXT: Stepped to global: {{.*}}:18:4
// CHECK-NEXT: Stepped to global: {{.*}}:19:1
// CHECK-NEXT: Continuing execution
// CHECK-NEXT: returned

debugger;
var x = [1,2,3];
x.forEach(function(i) {
  return 'myresult';
});

// CHECK: Break on 'debugger' statement in global: {{.*}}:35:1
// CHECK-NEXT: Stepped to global: {{.*}}:36:9
// CHECK-NEXT: Stepped to global: {{.*}}:37:1
// CHECK-NEXT: Stepped to anonymous: {{.*}}:38:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:37:23
// CHECK-NEXT: Stepped to anonymous: {{.*}}:38:3
// CHECK-NEXT: Stepped to anonymous: {{.*}}:37:23
// CHECK-NEXT: Stepped to anonymous: {{.*}}:38:3
// CHECK-NEXT: Continuing execution
