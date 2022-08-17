/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheck --match-full-lines %s

var obj = {
  get b() {},
  set b(x) {},
  get c() {},
  set d(x) {},
};

//CHECK-LABEL:Function<global>{{.*}}:
//CHECK-NEXT:Offset in debug table: {{.*}}
//CHECK-NEXT:    DeclareGlobalVar  "obj"
//CHECK-NEXT:    CreateEnvironment r1
//CHECK-NEXT:    NewObject         r2
//CHECK-NEXT:    CreateClosure     r4, r1, Function<get b>
//CHECK-NEXT:    CreateClosure     r3, r1, Function<set b>
//CHECK-NEXT:    LoadConstString   r0, "b"
//CHECK-NEXT:    PutOwnGetterSetterByVal r2, r0, r4, r3, 1
//CHECK-NEXT:    CreateClosure     r4, r1, Function<get c>
//CHECK-NEXT:    LoadConstUndefined r0
//CHECK-NEXT:    LoadConstString   r3, "c"
//CHECK-NEXT:    PutOwnGetterSetterByVal r2, r3, r4, r0, 1
//CHECK-NEXT:    CreateClosure     r3, r1, Function<set d>
//CHECK-NEXT:    LoadConstString   r1, "d"
//CHECK-NEXT:    PutOwnGetterSetterByVal r2, r1, r0, r3, 1
//CHECK-NEXT:    GetGlobalObject   r1
//CHECK-NEXT:    PutById           r1, r2, 1, "obj"
//CHECK-NEXT:    Ret               r0
