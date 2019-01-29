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

//CHECK-LABEL:Function<foo>(2 params, 11 registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:{{.*}} LoadParam 2<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 1<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 3<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 0<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Jmp 32<Addr8>
//CHECK-NEXT:{{.*}} Catch 3<Reg8>
//CHECK-NEXT:{{.*}} Mov 0<Reg8>, 3<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 0<Reg8>, 0<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Jmp 10<Addr8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 0<Reg8>, 0<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Mov 0<Reg8>, 3<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 0<Reg8>, 0<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 0<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Ret 1<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 3<Reg8>, 3<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>
//CHECK-NEXT:{{.*}} Catch 0<Reg8>
//CHECK-NEXT:{{.*}} LoadConstUndefined 4<Reg8>
//CHECK-NEXT:{{.*}} Call 1<Reg8>, 2<Reg8>, 1<UInt8>
//CHECK-NEXT:{{.*}} Throw 0<Reg8>

//CHECK-LABEL: Exception Handlers:
//CHECK-NEXT: 0: start = 17, end = 26, target = 28
//CHECK-NEXT: 1: start = 7, end = 13, target = 15
//CHECK-NEXT: 2: start = 17, end = 36, target = 53
//CHECK-NEXT: 3: start = 7, end = 45, target = 63
//CHECK-NEXT: 4: start = 53, end = 63, target = 63
