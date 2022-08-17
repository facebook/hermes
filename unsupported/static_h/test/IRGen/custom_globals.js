/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir -include-globals %s.d %s -O0 | %FileCheck %s --match-full-lines
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

