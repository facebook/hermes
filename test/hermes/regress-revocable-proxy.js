/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck %s

function testTrap(trap, callback){
  var handler = {
    x: 5,
    // Make this a getter to revoke the proxy once the JSProxy method has started.
    // The handler should be read before the trap is called.
    get [trap]() {
      // Print to make sure the trap actually is executing.
      print(trap);
      revoke();
      return function(){
        // This should receive the correct handler, even though the proxy has been revoked.
        print(this.x);
      };
    }
  };

  // We give a function as the target to the proxy so
  // that the [[Call]] and [[Construct]] handlers are setup.
  let { proxy, revoke } = Proxy.revocable(function(){}, handler);
  // There are some cases where more operations are done on the proxy after
  // it is revoked, which throws an error. That's fine, we are just checking
  // to make sure the process doesn't crash.
  try {
    callback(proxy);
  } catch(e){
  }
}


let base = {};
testTrap("setPrototypeOf", proxy => Object.setPrototypeOf(proxy, base));
// CHECK-LABEL: setPrototypeOf
// CHECK-NEXT: 5
testTrap("getPrototypeOf", Object.getPrototypeOf);
// CHECK-LABEL: getPrototypeOf
// CHECK-NEXT: 5
testTrap("isExtensible", Object.isExtensible);
// CHECK-LABEL: isExtensible
// CHECK-NEXT: 5
testTrap("preventExtensions", Object.preventExtensions);
// CHECK-LABEL: preventExtensions
// CHECK-NEXT: 5
testTrap("getOwnPropertyDescriptor", proxy => Object.getOwnPropertyDescriptor(proxy, 'prop'));
// CHECK-LABEL: getOwnPropertyDescriptor
// CHECK-NEXT: 5
testTrap("defineProperty", proxy => proxy.hi = 1);
// CHECK-LABEL: defineProperty
// CHECK-NEXT: 5
testTrap("has", proxy => 'prop' in proxy);
// CHECK-LABEL: has
// CHECK-NEXT: 5
testTrap("get", proxy => proxy.prop);
// CHECK-LABEL: get
// CHECK-NEXT: 5
testTrap("set", proxy => proxy.prop = 1);
// CHECK-LABEL: set
// CHECK-NEXT: 5
testTrap("deleteProperty", proxy => delete proxy.prop);
// CHECK-LABEL: deleteProperty
// CHECK-NEXT: 5
testTrap("ownKeys", Object.getOwnPropertyNames);
// CHECK-LABEL: ownKeys
// CHECK-NEXT: 5
testTrap("apply", proxy => proxy(1));
// CHECK-LABEL: apply
// CHECK-NEXT: 5
testTrap("construct", proxy => new proxy(1));
// CHECK-LABEL: construct
// CHECK-NEXT: 5
