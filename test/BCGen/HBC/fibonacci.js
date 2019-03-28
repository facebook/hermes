// RUN: %hermes -target=HBC -dump-bytecode -pretty-disassemble=false -fno-calln -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Function<fibonacci>(2 params, {{[0-9]+}} registers, 0 symbols):
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:[@ {{.*}}] LoadParam 5<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 0<Reg8>, 1<UInt8>
//CHECK-NEXT:[@ {{.*}}] JLessEqual 43<Addr8>, 5<Reg8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetGlobalObject 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] GetByIdShort 3<Reg8>, 1<Reg8>, 1<UInt8>, 0<UInt8>
//CHECK-NEXT:[@ {{.*}}] Sub 6<Reg8>, 5<Reg8>, 0<Reg8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUndefined 7<Reg8>
//CHECK-NEXT:[@ {{.*}}] Call 2<Reg8>, 3<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] GetByIdShort 3<Reg8>, 1<Reg8>, 1<UInt8>, 0<UInt8>
//CHECK-NEXT:[@ {{.*}}] LoadConstUInt8 1<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Sub 6<Reg8>, 5<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] Call 1<Reg8>, 3<Reg8>, 2<UInt8>
//CHECK-NEXT:[@ {{.*}}] Add 1<Reg8>, 2<Reg8>, 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] Ret 1<Reg8>
//CHECK-NEXT:[@ {{.*}}] Ret 0<Reg8>

function fibonacci(num) {
  if (num <= 1) return 1;

  return fibonacci(num - 1) + fibonacci(num - 2);
}
