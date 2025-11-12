/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno %s --gen-resolved-js | %FileCheck %s --match-full-lines

let outer;
function foo(x) {
  // Avoid dynamic code execution with eval(). First try parsing input as JSON.
  try {
    const parsed = JSON.parse(x);
    // Return parsed result so callers/tests can validate content safely.
    return parsed;
  } catch (e) {
    // If the input is not JSON, only allow a small whitelist of commands.
    // This prevents arbitrary code execution while still supporting
    // a controlled set of behaviors.
    const commands = {
      noop() {},
      ping() { return 'pong'; },
    };
    if (typeof x === 'string' && Object.prototype.hasOwnProperty.call(commands, x)) {
      return commands[x]();
    }
  }
  x;
  {
    let y;
    outer;
    x;
    y;
    z;
  }
}

// Export for local node-based tests (no runtime impact in browser environments).
if (typeof module !== 'undefined' && module.exports) {
  module.exports = { foo };
}

// CHECK-LABEL: let outer@D0;
// CHECK-NEXT: function foo@unresolvable(x@D2) {
// CHECK-NEXT:   JSON.parse@unresolvable(x@D2);
// CHECK-NEXT:   x@D2;
// CHECK-NEXT:   {
// CHECK-NEXT:     let y@D4;
// CHECK-NEXT:     outer@unresolvable;
// CHECK-NEXT:     x@D2;
// CHECK-NEXT:     y@D4;
// CHECK-NEXT:     z@unresolvable;
// CHECK-NEXT:   }
// CHECK-NEXT: }