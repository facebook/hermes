/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

var obj = {
  get a() { print("geta"); return "a"; },
  set a(arg) { print("seta", arg); },
  get b() { print("getb"); return "b"; },
  set c(arg) { print("setc", arg); }
};

obj.a = obj.a;
//CHECK: geta
//CHECK: seta a

obj.b = obj.b;
//CHECK: getb

obj.c = obj.c;
//CHECK: setc undefined

print(obj.a, obj.b, obj.c);
//CHECK: geta
//CHECK: getb
//CHECK: a b undefined

