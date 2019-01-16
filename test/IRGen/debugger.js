// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo () {
    debugger;
}
//CHECK-LABEL: function foo()
//CHECK-NEXT: frame = []
//CHECK-NEXT:   %BB0:
//CHECK-NEXT:     %0 = DebuggerInst
//CHECK-NEXT:     %1 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
