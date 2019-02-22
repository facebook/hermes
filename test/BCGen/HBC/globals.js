// RUN: %hermes -target=HBC -dump-bytecode -O -strict %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -dump-bytecode -O -non-strict %s | %FileCheck --match-full-lines --check-prefix=CHKNONSTRICT %s

var x = 5;
foo(x);
y = x;

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: src 0x0, vars 0x0
//CHECK-NEXT:    DeclareGlobalVar  "x"
//CHECK-NEXT:    LoadConstUInt8    r0, 5
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r0, 1, "x"
//CHECK-NEXT:    TryGetById        r3, r1, 1, "foo"
//CHECK-NEXT:    GetByIdShort      r4, r1, 2, "x"
//CHECK-NEXT:    LoadConstUndefined r5
//CHECK-NEXT:    Call              r0, r3, 2
//CHECK-NEXT:    GetByIdShort      r0, r1, 2, "x"
//CHECK-NEXT:    TryPutById        r1, r0, 2, "y"
//CHECK-NEXT:    Ret               r0

//CHKNONSTRICT-LABEL:Function<global>{{.*}}:
//CHKNONSTRICT-NEXT:Offset in debug table: src 0x0, vars 0x0
//CHKNONSTRICT-NEXT:    DeclareGlobalVar  "x"
//CHKNONSTRICT-NEXT:    LoadConstUInt8    r0, 5
//CHKNONSTRICT-NEXT:    GetGlobalObject   r1
//CHKNONSTRICT-NEXT:    PutById           r1, r0, 1, "x"
//CHKNONSTRICT-NEXT:    TryGetById        r3, r1, 1, "foo"
//CHKNONSTRICT-NEXT:    GetByIdShort      r4, r1, 2, "x"
//CHKNONSTRICT-NEXT:    LoadConstUndefined r5
//CHKNONSTRICT-NEXT:    Call              r0, r3, 2
//CHKNONSTRICT-NEXT:    GetByIdShort      r0, r1, 2, "x"
//CHKNONSTRICT-NEXT:    PutById           r1, r0, 2, "y"
//CHKNONSTRICT-NEXT:    Ret               r0
