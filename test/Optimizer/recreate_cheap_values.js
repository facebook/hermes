// RUN: %hermes -fno-calln -dump-postra -O %s | %FileCheck %s --match-full-lines

// Positive zero is 'cheap'.
function poszero(f) {
  return f(0.0, 0.0);
}
// CHECK: function poszero(f)
// CHECK:   %2 = HBCLoadConstInst 0 : number
// CHECK-NEXT:   %3 = HBCLoadConstInst 0 : number
// CHECK-NEXT:   %4 = CallInst %0, %1 : undefined, %2 : number, %3 : number

// Negative zero is NOT 'cheap'.
function negzero(f) {
  return f(-0.0, -0.0);
}
// CHECK:function negzero(f)
// CHECK:  %2 = HBCLoadConstInst -0 : number
// CHECK-NEXT:  %3 = MovInst %2 : number
// CHECK-NEXT:  %4 = MovInst %2 : number
// CHECK-NEXT: %5 = CallInst %0, %1 : undefined, %3 : number, %4 : number
