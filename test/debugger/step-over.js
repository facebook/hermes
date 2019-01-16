// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// RUN: %hdb --lazy < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

print('step over');
// CHECK-LABEL: step over

function happy() {
  print('happy');
  print('stepping');
  return 'this is the result';
}

debugger;
print('first');
print(happy());
print('second');

// CHECK: Break on 'debugger' statement in global: {{.*}}:14:1
// CHECK-NEXT: Stepped to global: {{.*}}:15:1
// CHECK-NEXT: first
// CHECK-NEXT: Stepped to global: {{.*}}:16:1
// CHECK-NEXT: happy
// CHECK-NEXT: stepping
// CHECK-NEXT: this is the result
// CHECK-NEXT: Stepped to global: {{.*}}:17:1
// CHECK-NEXT: second
