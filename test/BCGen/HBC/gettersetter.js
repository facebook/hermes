// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s

var obj = {
  get b() {},
  set b(x) {},
  get c() {},
  set d(x) {},
};

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: src 0x0, vars 0x0
//CHECK-NEXT:    DeclareGlobalVar  "obj"
//CHECK-NEXT:    CreateEnvironment r1
//CHECK-NEXT:    NewObject         r2
//CHECK-NEXT:    CreateClosure     r3, r1, 1
//CHECK-NEXT:    CreateClosure     r0, r1, 2
//CHECK-NEXT:    PutGetterSetter   r2, "b", r3, r0
//CHECK-NEXT:    CreateClosure     r3, r1, 3
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    PutGetterSetter   r2, "c", r3, r0
//CHECK-NEXT:    CreateClosure     r1, r1, 4
//CHECK-NEXT:    PutGetterSetter   r2, "d", r0, r1
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r2, 1, "obj"
//CHECK-NEXT:    Ret               r0
