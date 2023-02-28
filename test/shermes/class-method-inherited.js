/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

print('method inheritance');
// CHECK-LABEL: method inheritance

class C {
  notOverridden(): number {
    return 100;
  }
  inherited(): number {
    return 1;
  }
}

print(new C().notOverridden());
// CHECK-NEXT: 100
print(new C().inherited());
// CHECK-NEXT: 1

class D extends C {
  inherited(): number {
    return 2;
  }

  onlyDerived(): number {
    return 3;
  }
}

print(new D() instanceof C);
// CHECK-NEXT: true
print(new D() instanceof D);
// CHECK-NEXT: true
print(new D().notOverridden());
// CHECK-NEXT: 100
print(new D().inherited());
// CHECK-NEXT: 2
print(new D().onlyDerived());
// CHECK-NEXT: 3 

class E extends D {
  inherited(): number {
    return 10;
  }

  onlyDerivedE(): number {
    return 11;
  }
}

print(new E().inherited());
// CHECK-NEXT: 10
print(new E().notOverridden());
// CHECK-NEXT: 100
print(new E().onlyDerived());
// CHECK-NEXT: 3
print(new E().onlyDerivedE());
// CHECK-NEXT: 11
