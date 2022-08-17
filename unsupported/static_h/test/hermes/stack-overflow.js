/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//RUN: %hermes -O -gc-sanitize-handles=0 -target=HBC %s | %FileCheck --match-full-lines %s

var cnt = 0;
function func() {
    ++cnt;
    func();
    return cnt;
}

try {
    func();
} catch(e) {
    print("caught:", cnt, e.name, e.message);
    // Ensure we can construct the stack.
    print(e.stack.length);
}
//CHECK: caught: {{.*}} RangeError {{.*}}
//CHECK-NEXT: {{[0-9]+}}

(function(){
  try {
    // This attempts to overflow the register stack.
    (function overflowRegisterStack() {
      overflowRegisterStack();
      print("Unreachable");
    })();
  } catch (err) {
    print("Register Overflow", err.name);
  }
//CHECK: Register Overflow RangeError

  // This attempts to overflow the native stack via toString() calls.
  // The native depth counter should limit it.
  try {
    var a= {toString: String.prototype.lastIndexOf}
    a.toString();
  } catch (err) {
    print("Native Overflow", err.name);
  }
//CHECK: Native Overflow RangeError
})();
