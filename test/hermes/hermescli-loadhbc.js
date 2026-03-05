/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Compile a helper module to HBC, then load it with hermescli.loadHBC.
// RUN: echo "var x = 40 + 2; x;" > %t.js
// RUN: %hermes -emit-binary -out %t.hbc %t.js
// RUN: %hermes -Xhermes-internal-test-methods %s -- %t.hbc | %FileCheck --match-full-lines %s

print("loadHBC");
// CHECK-LABEL: loadHBC

// Load and execute HBC bytecode from a file path passed as a script arg.
var args = hermescli.getScriptArgs();
var buf = hermescli.loadFile(args[0]);
print("buf instanceof Uint8Array:", buf instanceof Uint8Array);
// CHECK-NEXT: buf instanceof Uint8Array: true

var result = hermescli.loadHBC(buf);
print("result:", result);
// CHECK-NEXT: result: 42

// loadHBC also accepts an ArrayBuffer.
var buf2 = hermescli.loadFile(args[0]);
var abResult = hermescli.loadHBC(buf2.buffer);
print("ArrayBuffer result:", abResult);
// CHECK-NEXT: ArrayBuffer result: 42

// loadHBC throws on invalid bytecode.
try {
  hermescli.loadHBC(new Uint8Array([1, 2, 3]));
  print("ERROR: should have thrown");
} catch(e) {
  print("invalid bytecode:", e.constructor.name);
}
// CHECK-NEXT: invalid bytecode: TypeError

// loadHBC throws with non-buffer argument.
try {
  hermescli.loadHBC("not a buffer");
  print("ERROR: should have thrown");
} catch(e) {
  print("non-buffer arg:", e.constructor.name);
}
// CHECK-NEXT: non-buffer arg: TypeError
