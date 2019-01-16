// RUN: %hermesc -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=1 -outline-min-params=0 -outline-max-params=0 -dump-ir %s  | %FileCheck --match-full-lines %s --check-prefix=CHKIR
// RUN: %hermes -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=1 -outline-min-params=0 -outline-max-params=0 %s | %FileCheck --match-full-lines %s

//CHKIR-LABEL:function basic1() : undefined
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR-NEXT:  %1 = ReturnInst undefined : undefined
//CHKIR-NEXT:function_end
function basic1() {
  print('this');
  print('block');
  print('is');
  print('most');
  print('definitely');
  print('worth');
  print('outlining');
  print('!');
}

//CHKIR-LABEL:function basic2() : undefined
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR-NEXT:  %1 = ReturnInst undefined : undefined
//CHKIR-NEXT:function_end
function basic2() {
  print('this');
  print('block');
  print('is');
  print('most');
  print('definitely');
  print('worth');
  print('outlining');
  print('!');
}

//CHKIR-LABEL:function OUTLINED_FUNCTION() : undefined
//CHKIR-NEXT:frame = []
//CHKIR-NEXT:%BB0:
//CHKIR-NEXT:  %0 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %1 = CallInst %0, undefined : undefined, "this" : string
//CHKIR-NEXT:  %2 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %3 = CallInst %2, undefined : undefined, "block" : string
//CHKIR-NEXT:  %4 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %5 = CallInst %4, undefined : undefined, "is" : string
//CHKIR-NEXT:  %6 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %7 = CallInst %6, undefined : undefined, "most" : string
//CHKIR-NEXT:  %8 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %9 = CallInst %8, undefined : undefined, "definitely" : string
//CHKIR-NEXT:  %10 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %11 = CallInst %10, undefined : undefined, "worth" : string
//CHKIR-NEXT:  %12 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %13 = CallInst %12, undefined : undefined, "outlining" : string
//CHKIR-NEXT:  %14 = TryLoadGlobalPropertyInst globalObject : object, "print" : string
//CHKIR-NEXT:  %15 = CallInst %14, undefined : undefined, "!" : string
//CHKIR-NEXT:  %16 = ReturnInst undefined : undefined
//CHKIR-NEXT:function_end

//CHECK-LABEL:this
//CHECK-NEXT:block
//CHECK-NEXT:is
//CHECK-NEXT:most
//CHECK-NEXT:definitely
//CHECK-NEXT:worth
//CHECK-NEXT:outlining
//CHECK-NEXT:!
basic1();

//CHECK-LABEL:this
//CHECK-NEXT:block
//CHECK-NEXT:is
//CHECK-NEXT:most
//CHECK-NEXT:definitely
//CHECK-NEXT:worth
//CHECK-NEXT:outlining
//CHECK-NEXT:!
basic2();
