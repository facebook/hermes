/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "function outer(x){ var a0=x++;" "var a"{1..4200}"=x++;" "function cap(){return a0" "+a"{1..4200} "; } function inner(){return a4200;} if(x<0) return cap(); return inner(); } print(outer(7));" | %hermes - -O -fno-inline -Xjit=force -Xjit-crash-on-error | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The above echo generates an environment with 4201 slots:
//   function outer(x){
//     var a0=x++; ... var a4200=x++;
//     function cap(){ return a0 + ... + a4200; }   // captures everything
//     function inner(){ return a4200; }            // reads the highest slot
//     if (x<0) return cap();
//     return inner();
//   }
//
// An environment slot sits at offsetof(SHEnvironment, slots) + 8 * slot, so
// past slot ~4092 the LDR displacement no longer encodes.
//
// Three details are load-bearing. cap is never called, so it is never JIT'ed
// and its own huge frame does not come into it; it exists only to force every
// variable into the environment. inner stays tiny, so the function under test
// does not also need thousands of frame registers, which is a separate limit
// (see large-frame.js). And -fno-inline is required: otherwise inner is
// inlined into outer and the environment load disappears entirely.

// CHECK: 4207
