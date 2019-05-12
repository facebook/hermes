// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s

var o = {};
Object.defineProperty(o, 'foo', { value: 'bar' });
print(o.foo)

// CHECK-LABEL: Bytecode File Information:
// CHECK:       String Kind Entry count: 3

// CHECK-LABEL: Global String Table:
// CHECK-NEXT:  s0[ASCII, {{.*}}]: bar
// CHECK-NEXT:  s1[ASCII, {{.*}}]: global
// CHECK-NEXT:  i2[ASCII, {{.*}}] #{{.*}}: foo
// CHECK-NEXT:  i3[ASCII, {{.*}}] #{{.*}}: o
// CHECK-NEXT:  p4[ASCII, {{.*}}] @{{.*}}: Object
// CHECK-NEXT:  p5[ASCII, {{.*}}] @{{.*}}: defineProperty
// CHECK-NEXT:  p6[ASCII, {{.*}}] @{{.*}}: value
// CHECK-NEXT:  p7[ASCII, {{.*}}] @{{.*}}: print

