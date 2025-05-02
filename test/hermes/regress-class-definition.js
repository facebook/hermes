/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

(function() {
  const handler = {
    get(target, prop, receiver) {
      print(k);
      return {};
    }
  };
  const constructibleProxy = new Proxy(function() {}, handler);
  // This store should not be eliminated because the creation of the class can
  // read it.
  let k = 1;
  class B extends constructibleProxy {}
  k = 2;
})();
// CHECK: 1

