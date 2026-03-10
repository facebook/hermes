/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Xhermes-internal-test-methods %s -- %s arg1 arg2 | %FileCheck --match-full-lines %s

// hermescli is only available with -Xhermes-internal-test-methods.
print("hermescli type:", typeof hermescli);
// CHECK-LABEL: hermescli type: object

print("loadFile type:", typeof hermescli.loadFile);
// CHECK-NEXT: loadFile type: function

print("loadHBC type:", typeof hermescli.loadHBC);
// CHECK-NEXT: loadHBC type: function

print("getScriptArgs type:", typeof hermescli.getScriptArgs);
// CHECK-NEXT: getScriptArgs type: function

// --- loadFile ---
print("loadFile");
// CHECK-LABEL: loadFile

// loadFile returns a Uint8Array with the file contents.
// Use the test file's own path, passed as the first script arg.
var args = hermescli.getScriptArgs();
var data = hermescli.loadFile(args[0]);
print("loadFile instanceof Uint8Array:", data instanceof Uint8Array);
// CHECK-NEXT: loadFile instanceof Uint8Array: true
print("loadFile length > 0:", data.length > 0);
// CHECK-NEXT: loadFile length > 0: true

// loadFile throws on missing file.
try {
  hermescli.loadFile("/nonexistent/path/file.txt");
  print("ERROR: should have thrown");
} catch(e) {
  print("loadFile missing file:", e.constructor.name);
}
// CHECK-NEXT: loadFile missing file: TypeError

// loadFile throws with no argument.
try {
  hermescli.loadFile();
  print("ERROR: should have thrown");
} catch(e) {
  print("loadFile no arg:", e.constructor.name);
}
// CHECK-NEXT: loadFile no arg: TypeError

// loadFile throws with non-string argument.
try {
  hermescli.loadFile(123);
  print("ERROR: should have thrown");
} catch(e) {
  print("loadFile non-string:", e.constructor.name);
}
// CHECK-NEXT: loadFile non-string: TypeError

// --- getScriptArgs ---
print("getScriptArgs");
// CHECK-LABEL: getScriptArgs

print("args length:", args.length);
// CHECK-NEXT: args length: 3
print("args[1]:", args[1]);
// CHECK-NEXT: args[1]: arg1
print("args[2]:", args[2]);
// CHECK-NEXT: args[2]: arg2
