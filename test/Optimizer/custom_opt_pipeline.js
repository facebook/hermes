// Use the custom-optimization to eliminate dead code.
//
// RUN: %hermes -hermes-parser -dump-ir %s  -custom-opt="stackpromotion" -custom-opt="mem2reg" -custom-opt="dce" | %FileCheck %s --match-full-lines

//CHECK-LABEL:function test_two(x, y, z)
//CHECK-NEXT:frame = []
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end
function test_two(x,y,z) {
  function test00() {}
  var test01 = function() {}
}

