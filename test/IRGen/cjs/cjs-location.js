// RUN: %hermesc -commonjs -dump-ir -dump-source-location < %s | %FileCheck --match-full-lines %s

x = 10;

//CHECK-LABEL: function cjs_module(exports, require, module)
//CHECK-NEXT: frame = [exports, require, module]
//CHECK-NEXT: source location: [<stdin>:3:1 ... <stdin>:3:8)
//CHECK-NEXT: %BB0:
//CHECK-NEXT: ; <stdin>:3:1
//CHECK-NEXT:   %0 = StoreFrameInst %exports, [exports]
//CHECK-NEXT: ; <stdin>:3:1
//CHECK-NEXT:   %1 = StoreFrameInst %require, [require]
//CHECK-NEXT: ; <stdin>:3:1
//CHECK-NEXT:   %2 = StoreFrameInst %module, [module]
//CHECK-NEXT: ; <stdin>:3:3
//CHECK-NEXT:   %3 = StorePropertyInst 10 : number, globalObject : object, "x" : string
//CHECK-NEXT: ; <stdin>:3:7
//CHECK-NEXT:   %4 = ReturnInst undefined : undefined
//CHECK-NEXT: function_end
