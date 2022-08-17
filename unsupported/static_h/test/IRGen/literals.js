/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

function foo() {
  return "hi"
  return 2.312
  return 12
  return 0x12
  return true
  return undefined
  return null
}

foo()


//CHECK: function foo()
//CHECK: frame = []
//CHECK:   %BB0:
//CHECK:     %0 = ReturnInst "hi" : string
//CHECK:   %BB1:
//CHECK:     %1 = ReturnInst 2.312 : number
//CHECK:   %BB2:
//CHECK:     %2 = ReturnInst 12 : number
//CHECK:   %BB3:
//CHECK:     %3 = ReturnInst 18 : number
//CHECK:   %BB4:
//CHECK:     %4 = ReturnInst true : boolean
//CHECK:   %BB5:
//CHECK:     %5 = ReturnInst undefined : undefined
//CHECK:   %BB6:
//CHECK:     %6 = ReturnInst null : null
//CHECK:   %BB7:
//CHECK:     %7 = ReturnInst undefined : undefined

