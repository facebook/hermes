/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -lazy %s | %FileCheck %s --match-full-lines

// Make sure we can correctly resolve scopes through lazily compiled
// functions in lazily compiled generators.
function f() {
  var f_var = 10;
  function* g() {
    var g_var = 32;
    function h() {
      /* Some text to pad out the function so that it won't be eagerly compiled
       * for being too short. Lorem ipsum dolor sit amet, consectetur adipiscing
       * elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
       */
      return f_var + g_var;
    }
    yield h();
  }
  return g().next().value;
}
// CHECK: 42
print(f());
