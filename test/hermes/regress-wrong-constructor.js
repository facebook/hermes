// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s


try {
  print(Uint8Array.from.call(Date, [123]));
} catch (e) {
  print('caught', e.name);
}
// CHECK: caught TypeError
