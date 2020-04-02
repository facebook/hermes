/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Xes6-proxy -target=HBC -serializevm-path=%t %s
// RUN: %hermes -O -Xes6-proxy -deserialize-file=%t -target=HBC %s | %FileCheck --match-full-lines %s
// REQUIRES: serializer
"use strict"

var proxy = new Proxy({a:1}, {get(t, k) { return k == "a" ? t[k] : 11; }});
var revocable = Proxy.revocable({b:2}, {get(t, k) { return k == "b" ? t[k] : 22; }});
var revoked = Proxy.revocable({c:3}, {get(t, k) { return k == "c" ? t[k] : 33; }});
revoked.revoke();
var callable = new Proxy(function(){ return 4; }, {get(t, k) { return 44; }});

serializeVM(function() {
  print('Proxy');
  // CHECK-LABEL: Proxy
  print(proxy.a, proxy.aa);
  // CHECK-NEXT: 1 11
  print(revocable.proxy.b, revocable.proxy.bb);
  // CHECK-NEXT: 2 22
  try {
    revocable.revoked.c;
  } catch (e) {
    print("caught " + e.name);
  }
  // CHECK-NEXT: caught TypeError
  print(callable(), callable.prop);
  // CHECK-NEXT: 4 44
})
