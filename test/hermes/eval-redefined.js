// RUN: %hermes -w %s | %FileCheck --match-full-lines %s

(function (){
  eval("print('real eval')");
//CHECK: real eval
  real_eval = eval
  eval = function() { print("fake eval"); }
  eval("print('will this work?')");
//CHECK-NEXT: fake eval
  eval = real_eval
  eval("print('real eval')");
//CHECK-NEXT: real eval
})()

