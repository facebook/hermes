/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Werror -typed -O0 %s | %FileCheck --match-full-lines %s
// RUN: %hermes -Werror -typed -O %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O0 %s | %FileCheck --match-full-lines %s
// RUN: %shermes -Werror -typed -exec -O %s | %FileCheck --match-full-lines %s

'use strict';

// A static field initializer runs with the class constructor as its receiver,
// so 'super' there is the superclass constructor and resolves against its
// static members, not its prototype.
class P {
  static base: number = 4;
  static who(): string {
    return 'P';
  }
  static ident(): any {
    return this;
  }
}

class C extends P {
  static viaSuper: number = super.base;
  static name1: string = super.who();
  // 'this' is preserved as the receiver of a super call, so it is C.
  static recv: any = super.ident();
  // Arrows in the initializer inherit both 'this' and 'super'.
  static arrow: any = () => super.who();
}

print(C.viaSuper, C.name1, C.recv === C, C.arrow(), C.base);
// CHECK: 4 P true P 4

// 'super' in a static initializer reaches through more than one level.
class E extends C {
  static deep: string = super.name1 + super.who();
}

print(E.deep);
// CHECK-NEXT: PP

// A write through 'super' to an inherited instance field goes to the receiver,
// whose layout already contains the superclass's slots.
class F {
  q: number = 3;
}

class G extends F {}

// The field need not be on the immediate superclass: 'q' lives on F.
class H extends G {
  setQ(n: number): number {
    super.q = n;
    return this.q;
  }
}

print(new H().setQ(7));
// CHECK-NEXT: 7

// A static method's receiver is the class, so 'super' there is the superclass
// constructor and resolves against its statics, exactly as in an initializer.
class SP {
  static tag: number = 4;
  static who(): string {
    return 'SP';
  }
}

class SC extends SP {
  static whoSC(): string {
    return 'SC>' + super.who();
  }
  static readField(): number {
    return super.tag;
  }
  // Arrows in a static method body inherit 'super' as well as 'this'.
  static viaArrow(): string {
    const f = (): string => super.who();
    return f();
  }
}

print(SC.whoSC(), SC.readField(), SC.viaArrow());
// CHECK-NEXT: SC>SP 4 SP

// Static accessors carry no Decl to the access site, so 'super' has to find
// them on the superclass's static type and call them with 'this' as receiver.
class AP {
  static _v: number = 1;
  @Hermes.final static set v(n: number) {
    AP._v = n;
  }
  @Hermes.final static get v(): number {
    return this._v;
  }
}

// The immediate superclass need not declare the accessor: lookup walks the
// static type chain. Declaring the setter first also verifies that each half's
// Decl is resolved independently.
class APMid extends AP {}

class AC extends APMid {
  static readViaSuper(): number {
    return super.v;
  }
  static writeViaSuper(n: number): void {
    super.v = n;
  }
  static initViaSuper: number = super.v;
  // 'prototype' is the immediate superclass's home object, not an ordinary
  // static.
  static proto(): any {
    return super.prototype;
  }
}

print(AP.v, AC.initViaSuper, AC.readViaSuper());
// CHECK-NEXT: 1 1 1
AP.v = 5;
print(AP.v, AC.readViaSuper());
// CHECK-NEXT: 5 5
AC.writeViaSuper(9);
print(AP.v, AC.readViaSuper());
// CHECK-NEXT: 9 9
print(AC.proto() === APMid.prototype);
// CHECK-NEXT: true
