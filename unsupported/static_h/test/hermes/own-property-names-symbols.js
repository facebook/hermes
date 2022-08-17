/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -non-strict -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

function getOwnPropertySymbolsAsStrings(obj) {
  return Object.getOwnPropertySymbols(obj).map(function (sym) {
    return sym.toString();
  });
}

bar = Symbol.for("bar");
qux = Symbol.for("qux");

proto = { foo: 0 };
proto[bar] = 1;

obj = { __proto__: proto, baz: 2};
obj[qux] = 3;

print(Object.getOwnPropertyNames(obj));
//CHECK: baz

print(getOwnPropertySymbolsAsStrings(obj));
//CHECK: Symbol(qux)

// Strings have internal property slots which shouldn't be treated as property
// names.
print(Object.getOwnPropertyNames(""));
//Check: length
