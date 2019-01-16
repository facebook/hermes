// RUN: %hermesc -target=HBC -O -dump-ir %s | %FileCheck --match-full-lines %s
// Perform stack promotion after inlining

function f1(num) {
    function bar() {
        return num;
    }
    return bar();
}

//CHECK-LABEL:function f1(num)
//CHECK-NEXT:frame = [num]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = ReturnInst %num
//CHECK-NEXT:function_end
