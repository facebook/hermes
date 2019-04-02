// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

print("HermesInternal");
// CHECK-LABEL: HermesInternal

var desc = Object.getOwnPropertyDescriptor(this, "HermesInternal");
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false

var desc = Object.getOwnPropertyDescriptor(HermesInternal, "detachArrayBuffer");
print(desc.enumerable, desc.writable, desc.configurable);
// CHECK-NEXT: false false false

try { HermesInternal.asdf = 'asdf'; } catch (e) { print('caught', e.name); }
// CHECK-NEXT: caught TypeError
try {
  delete HermesInternal.detachArrayBuffer;
} catch (e) {
  print('caught', e.name);
}
// CHECK-NEXT: caught TypeError
