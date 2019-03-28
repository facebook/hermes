// RUN: %hermes -target=HBC -dump-bytecode --pretty-disassemble -fno-calln -O %s | %FileCheck --match-full-lines %s

function foo (a) {
    var sum = 0;
    while (--a)
        sum += a;
    print("This\nis един long Unicode string=", sum);
}

//CHECK-LABEL:Function<foo>(2 params, {{[0-9]+}} registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    LoadConstUInt8    r2, 1
//CHECK-NEXT:    LoadParam         r0, 1
//CHECK-NEXT:    ToNumber          r0, r0
//CHECK-NEXT:    SubN              r1, r0, r2
//CHECK-NEXT:    LoadConstZero     r0
//CHECK-NEXT:    LoadConstZero     r3
//CHECK-NEXT:    JmpFalse          L1, r1
//CHECK-NEXT:L2:
//CHECK-NEXT:    Add               r0, r0, r1
//CHECK-NEXT:    SubN              r1, r1, r2
//CHECK-NEXT:    Mov               r3, r0
//CHECK-NEXT:    JmpTrue           L2, r1
//CHECK-NEXT:L1:
//CHECK-NEXT:    GetGlobalObject   r0
//CHECK-NEXT:    TryGetById        r2, r0, 1, "print"
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    LoadConstString   r6, "This\x0ais \u0435"...
//CHECK-NEXT:    LoadConstUndefined r7
//CHECK-NEXT:    Mov               r5, r3
//CHECK-NEXT:    Call              r1, r2, 3
//CHECK-NEXT:    Ret               r0
