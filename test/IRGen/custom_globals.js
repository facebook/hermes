// RUN: %hermes -hermes-parser -dump-ir -include-globals %s.d %s | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir -include-globals %s.d %s -O

var x = CustomGlobalProperty;

//CHECK: function global()
//CHECK: frame = [], globals = [x]
//CHECK:   %BB0:
//CHECK:     %0 = AllocStackInst $?anon_0_ret
//CHECK:     %1 = StoreStackInst undefined : undefined, %0
//CHECK:     %2 = TryLoadGlobalPropertyInst globalObject : object, "CustomGlobalProperty" : string
//CHECK:     %3 = StorePropertyInst %2, globalObject : object, "x" : string
//CHECK:     %4 = LoadStackInst %0
//CHECK:     %5 = ReturnInst %4
//CHECK: function_end

