/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "function f(o){return o.p0" "+o.p"{1..200} ';} var o={}; for(var i=0;i<=200;++i) o["p"+i]=i; print(f(o));' | %hermes - -O -Xjit=force -Xjit-crash-on-error | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The above echo generates:
//   function f(o){ return o.p0 + o.p1 + ... + o.p200; }
//   var o={}; for (var i=0;i<=200;++i) o["p"+i]=i; print(f(o));
//
// Property cache indices are allocated per property name, so 201 distinct
// names give indices up to 200. Cache entries are 24 bytes, and the byte
// offset of entry 171 is 4104, one step past the 4095 ceiling of an ADD
// immediate. Every site that indexes the cache array has to cope with that.
//
// Note that repeating a single site 200 times proves nothing here: those all
// share one index. The names have to differ.

// CHECK: 20100
