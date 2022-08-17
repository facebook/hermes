/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

//RUN: %hermes -O -target=HBC %s -gc-sanitize-handles=0 | %FileCheck --match-full-lines %s

try {
  var proxy = new Proxy(function() {}, {});

  for (var i = 0; i < 100000; i++) {
    proxy = new Proxy(proxy, {});
  }

  Reflect.construct(proxy, {});
} catch (e) {
  print('caught: ', e.name, e.message);
}
//CHECK: caught: RangeError {{.*}}
