/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: echo "var obj ={p0" ":\"s\",p"{1..4200} ":\"s\"}; print(obj.p4200);" | %hermes - -target=HBC -O -gc-sanitize-handles=0 -Xjit=force -Xjit-crash-on-error | %FileCheck --match-full-lines %s
// REQUIRES: jit

// The above echo generates an object literal with string values:
//   var obj = {p0:"s", p1:"s", ... p4200:"s"}; print(obj.p4200);
//
// Past indirect slot ~4090 the store offset no longer encodes as a scaled
// immediate. storeVal, used for number/bool/null/undefined literals, has
// always fallen back to a register offset; the string path did not, so a
// literal this size failed to JIT with InvalidDisplacement. The number
// equivalent is covered by large_literal_obj.js, which is why this only
// exercises string values.

// CHECK: s
