/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-ra -O %s | %FileCheck --match-full-lines --check-prefix=RA   %s
// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines --check-prefix=EXEC %s

// This test failed with an assert because we tried to resolve the global scope
// instead of using GetGlobalScope. However, exception variables have limited
// scope, even in the global function, so a resolve is necessary.

var inner, e="global";

try {
  throw "local";
} catch (e) {
  //RA-LABEL:function local()
  //RA-NEXT: frame = []
  //RA-NEXT: %BB0:
  //RA-NEXT:   {{.*}} %0 = HBCResolveEnvironment %global()
  //RA-NEXT:   {{.*}} %1 = HBCLoadFromEnvironmentInst %0, [?anon_1_e@global]
  //RA-NEXT:   {{.*}} %2 = ReturnInst %1
  //RA-NEXT: function_end
  local = function() { return e; };
}

// EXEC-LABEL: local
print(local());
// EXEC-NEXT: global
print(e);
