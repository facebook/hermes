// RUN: %hermesc -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=0 -outline-min-params=0 -outline-max-params=5 -dump-ir %s  | %FileCheck --match-full-lines %s --check-prefix=CHKIR0
// RUN: %hermesc -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=1 -outline-min-params=0 -outline-max-params=5 -dump-ir %s  | %FileCheck --match-full-lines %s --check-prefix=CHKIR1
// RUN: %hermesc -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=2 -outline-min-params=0 -outline-max-params=5 -dump-ir %s  | %FileCheck --match-full-lines %s --check-prefix=CHKIR2
// RUN: %hermes -target=HBC -O -foutline -outline-near-caller=false -outline-min-length=16 -outline-max-rounds=2 -outline-min-params=0 -outline-max-params=5 %s | %FileCheck --match-full-lines %s

function couldUseTwoRounds() {
  a(); a(); a(); a();
  a(); a(); a(); a();

  separator1();

  a(); a(); a(); a();
  a(); a(); a(); a();

  separator2();

  a(); a(); a(); a();
  a(); a(); a(); a();

  separator1();

  a(); a(); a(); a();
  a(); a(); a(); a();
}

// =============================================================================
// Max rounds = 0. No outlining.
// =============================================================================

//CHKIR0-LABEL:function couldUseTwoRounds() : undefined
//CHKIR0-NOT:{{.*}}OUTLINED_FUNCTION{{.*}}

//CHKIR0-NOT:function OUTLINED_FUNCTION{{.*}}

// =============================================================================
// Max rounds = 1. Outline before and after the separator2() call.
// =============================================================================

//CHKIR1-LABEL:function couldUseTwoRounds() : undefined
//CHKIR1:  {{.*}} = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR1:  {{.*}} = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR1-NEXT:  {{.*}} = ReturnInst undefined : undefined

//CHKIR1-LABEL:function OUTLINED_FUNCTION() : undefined
//CHKIR1-NOT:function {{.*}}OUTLINED_FUNCTION{{.*}}

// =============================================================================
// Max rounds = 2. Outline before and after the separator2() call, and then in
// that outlined function outline before and after the separator1() call.
// =============================================================================

//CHKIR2-LABEL:function couldUseTwoRounds() : undefined
//CHKIR2:  {{.*}} = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR2:  {{.*}} = HBCCallDirectInst %OUTLINED_FUNCTION() : undefined, undefined : undefined
//CHKIR2-NEXT:  {{.*}} = ReturnInst undefined : undefined

//CHKIR2-LABEL:function OUTLINED_FUNCTION() : undefined
//CHKIR2:  {{.*}} = HBCCallDirectInst %"OUTLINED_FUNCTION 1#"() : undefined, undefined : undefined
//CHKIR2:  {{.*}} = HBCCallDirectInst %"OUTLINED_FUNCTION 1#"() : undefined, undefined : undefined
//CHKIR2-NEXT:  {{.*}} = ReturnInst undefined : undefined

//CHKIR2-LABEL:function "OUTLINED_FUNCTION 1#"() : undefined
//CHKIR2-NOT:  {{.*}} = HBCCallDirectInst {{.*}}OUTLINED_FUNCTION{{.*}}
//CHKIR2-NOT:function {{.*}}OUTLINED_FUNCTION{{.*}}

// =============================================================================
// Max rounds = 2. Make sure it executes correctly.
// =============================================================================

count = 0;
a = function() { ++count; }
separator1 = function() { print("separator1: " + count); }
separator2 = function() { print("separator2: " + count); }
couldUseTwoRounds();
print("end: " + count);

//CHECK:separator1: 8
//CHECK-NEXT:separator2: 16
//CHECK-NEXT:separator1: 24
//CHECK-NEXT:end: 32
