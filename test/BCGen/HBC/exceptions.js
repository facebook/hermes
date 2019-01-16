// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -O %s | %FileCheck --match-full-lines %s

function foo(a) {
  try {
    a();
  } catch (e) {
    try {
      e();
    } catch (e) {
      e();
    }
    finally {
      e();
    }
  }
  finally {
    a();
  }
}

//CHECK-LABEL:Function<foo>(2 params, {{[0-9]+}} registers, 0 symbols):
//CHECK-NEXT: Offset in debug table: {{.*}}
//CHECK-NEXT: {{.*}} LoadParam 2<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 1<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 0<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 3<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 4<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 4<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Jmp 47<Addr8>
//CHECK-NEXT: {{.*}} Catch 3<Reg8>
//CHECK-NEXT: {{.*}} Mov 4<Reg8>, 3<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 4<Reg8>, 4<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Mov 4<Reg8>, 3<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 4<Reg8>, 4<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Jmp 19<Addr8>
//CHECK-NEXT: {{.*}} Catch 0<Reg8>
//CHECK-NEXT: {{.*}} Mov 4<Reg8>, 0<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 4<Reg8>, 4<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 0<Reg8>, 0<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 0<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Ret 1<Reg8>
//CHECK-NEXT: {{.*}} Catch 0<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 3<Reg8>, 3<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Throw 0<Reg8>
//CHECK-NEXT: {{.*}} Catch 0<Reg8>
//CHECK-NEXT: {{.*}} LoadConstUndefined 5<Reg8>
//CHECK-NEXT: {{.*}} Call 1<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT: {{.*}} Throw 0<Reg8>

//CHECK-LABEL: Exception Handlers:
//CHECK-NEXT: 0: start = 25, end = 34, target = 45
//CHECK-NEXT: 1: start = 47, end = 56, target = 70
//CHECK-NEXT: 2: start = 9, end = 15, target = 23
//CHECK-NEXT: 3: start = 25, end = 62, target = 80
//CHECK-NEXT: 4: start = 70, end = 80, target = 80
