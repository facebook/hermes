// RUN: %hermes -target=HBC -dump-ir -O %s | %FileCheck --match-full-lines %s

function thisUndefined () {
    function inner() {
        return this;
    }
    return inner();
}

//CHECK-LABEL:function thisUndefined() : object
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst globalObject : object
//CHECK-NEXT:function_end
