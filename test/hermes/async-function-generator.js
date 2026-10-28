/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -lazy %s | %FileCheck --match-full-lines %s
// RUN: %hermes -g %s | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

print("async function generator");
// CHECK-LABEL: async function generator

// A "prototype" property on an async function belongs to user code and must
// not affect the generator that runs its body.
async function hijacked() {
  await 0;
  return "not hijacked";
}
hijacked.prototype = {
  next() {
    return {done: true, value: "hijacked"};
  },
};

function thrower(msg) {
  throw new Error(msg);
}
async function badParam(a = thrower("function param")) {}
var badArrowParam = async (a = thrower("arrow param")) => {};

var order = [];
async function ordered(a = order.push("param")) {
  order.push("body");
  await 0;
  order.push("resumed");
}

var obj = {
  v: 3,
  m() {
    return (async (q = this.v + arguments[0]) => q * 2)();
  },
};
async function usesArgs(a, {b} = {b: arguments.length}) {
  return this.tag + " " + a + " " + b + " " + arguments.length;
}

(async function main() {
  print(await hijacked());
// CHECK-NEXT: not hijacked

  // Errors from parameter initialization reject the promise instead of
  // throwing.
  var p1 = badParam();
  var p2 = badArrowParam();
  print("no sync throw");
  await p1.catch(e => print(e.message));
  await p2.catch(e => print(e.message));
// CHECK-NEXT: no sync throw
// CHECK-NEXT: function param
// CHECK-NEXT: arrow param

  // Parameters are initialized and the body runs synchronously up to the
  // first await.
  var p3 = ordered();
  order.push("caller");
  await p3;
  print(order.join(","));
// CHECK-NEXT: param,body,caller,resumed

  // `this`, `arguments` and parameter defaults referring to them.
  print(await obj.m(4));
  print(await usesArgs.call({tag: "T"}, 1));
// CHECK-NEXT: 14
// CHECK-NEXT: T 1 1 1
})();
