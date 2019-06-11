// RUN: %hermes -Xflow-parser -commonjs -dump-ir %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

export default function() {
  return 400;
}

// CHECK-LABEL: function cjs_module(exports, require, module)
// CHECK-NEXT: frame = [exports, require, module]
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = StoreFrameInst %exports, [exports]
// CHECK-NEXT:   %1 = StoreFrameInst %require, [require]
// CHECK-NEXT:   %2 = StoreFrameInst %module, [module]
// CHECK-NEXT:   %3 = CreateFunctionInst %""()
// CHECK-NEXT:   %4 = StorePropertyInst %3 : closure, %exports, "?default" : string
// CHECK-NEXT:   %5 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end

// CHECK-LABEL: function ""()
// CHECK-NEXT: frame = []
// CHECK-NEXT: %BB0:
// CHECK-NEXT:   %0 = ReturnInst 400 : number
// CHECK-NEXT: %BB1:
// CHECK-NEXT:   %1 = ReturnInst undefined : undefined
// CHECK-NEXT: function_end
