// RUN: %hermes -strict -target=HBC -dump-bytecode --basic-block-profiling -O %s | %FileCheck --match-full-lines %s

var condition = false;
try {
  try {
    print(condition? "yes": "no");
  } finally {
    print("rethrowing");
  }
} catch (e) {
  print(e.stack);
}

//CHECK-LABEL: Function<global>{{.*}}:
//CHECK-NEXT: Offset in debug table: src 0x0, vars 0x0
//CHECK-NEXT:     DeclareGlobalVar  "condition"
//CHECK-NEXT:     ProfilePoint      10
//CHECK-NEXT:     CreateEnvironment r1
//CHECK-NEXT:     LoadConstString   r6, "yes"
//CHECK-NEXT:     LoadConstString   r5, "no"
//CHECK-NEXT:     LoadConstUndefined r3
//CHECK-NEXT:     LoadConstUndefined r0
//CHECK-NEXT:     LoadConstFalse    r4
//CHECK-NEXT:     GetGlobalObject   r2
//CHECK-NEXT:     PutById           r2, r4, 1, "condition"
//CHECK-NEXT:     ProfilePoint      7
//CHECK-NEXT:     ProfilePoint      5
//CHECK-NEXT:     TryGetById        r4, r2, 1, "print"
//CHECK-NEXT:     GetByIdShort      r7, r2, 2, "condition"
//CHECK-NEXT:     JmpFalse          L1, r7
//CHECK-NEXT:     ProfilePoint      4
//CHECK-NEXT:     Mov               r5, r6
//CHECK-NEXT: L1:
//CHECK-NEXT:     ProfilePoint      3
//CHECK-NEXT:     Call2             r0, r4, r3, r5
//CHECK-NEXT:     ProfilePoint      2
//CHECK-NEXT:     TryGetById        r5, r2, 1, "print"
//CHECK-NEXT:     LoadConstString   r4, "rethrowing"
//CHECK-NEXT:     Call2             r0, r5, r3, r4
//CHECK-NEXT:     ProfilePoint      1
//CHECK-NEXT:     Jmp               L2
//CHECK-NEXT:     Catch             r4
//CHECK-NEXT:     ProfilePoint      6
//CHECK-NEXT:     TryGetById        r6, r2, 1, "print"
//CHECK-NEXT:     LoadConstString   r5, "rethrowing"
//CHECK-NEXT:     Call2             r0, r6, r3, r5
//CHECK-NEXT:     Throw             r4
//CHECK-NEXT:     Catch             r4
//CHECK-NEXT:     ProfilePoint      9
//CHECK-NEXT:     StoreToEnvironment r1, 0, r4
//CHECK-NEXT:     TryGetById        r2, r2, 1, "print"
//CHECK-NEXT:     LoadFromEnvironment r1, r1, 0
//CHECK-NEXT:     GetByIdShort      r1, r1, 3, "stack"
//CHECK-NEXT:     Call2             r0, r2, r3, r1
//CHECK-NEXT: L2:
//CHECK-NEXT:     ProfilePoint      8
//CHECK-NEXT:     Ret               r0
