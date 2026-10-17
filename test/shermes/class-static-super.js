
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s 
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s 

'use strict';

class SuperBase {
  static parentValue: number = 19;
  @Hermes.final
  static identity<T>(value: T): T {
    return value;
  }
  static receiverIsChild(): boolean {
    return this === SuperChild;
  }
  instanceValue: number = 43;
}

class SuperChild extends SuperBase {
  static readSuperField(): number {
    return super.parentValue;
  }
  static callSuperMethod(): boolean {
    return super.receiverIsChild();
  }
  static readSuperArrow(): number {
    const read = (): number => super.parentValue;
    return read();
  }
  static callSuperGeneric(): number {
    return super.identity < number > (23);
  }
  @Hermes.final
  static readSuperGeneric<T>(value: T): number {
    return super.parentValue;
  }
  readInstanceSuper(): number {
    return super.instanceValue;
  }
}

print(SuperChild.readSuperField());
// CHECK: 19
print(SuperChild.callSuperMethod());
// CHECK-NEXT: true
print(SuperChild.readSuperArrow());
// CHECK-NEXT: 19
print(SuperChild.callSuperGeneric());
// CHECK-NEXT: 23
print(SuperChild.readSuperGeneric < number > (1));
// CHECK-NEXT: 19
print(new SuperChild().readInstanceSuper());
// CHECK-NEXT: 43

class GetterBase {
  static calls: number = 0;

  @Hermes.final
  static get inherited(): number {
    GetterBase.calls += 1;
    // Test the receiver without depending on inherited this.field accesses.
    return this === GetterChild ? 29 : 31;
  }
}

class GetterChild extends GetterBase {
  static read(): number {
    return super.inherited;
  }

  static readArrow(): number {
    const read = (): number => super.inherited;
    return read();
  }

  @Hermes.final
  static readGeneric<T>(value: T): number {
    return super.inherited;
  }
}

// Each access must invoke the getter with GetterChild as its receiver.
print(GetterChild.read());
// CHECK: 29
print(GetterChild.read());
// CHECK-NEXT: 29
print(GetterChild.readArrow());
// CHECK-NEXT: 29
print(GetterChild.readGeneric < number > (1));
// CHECK-NEXT: 29
print(GetterChild.readGeneric < string > ('ignored'));
// CHECK-NEXT: 29
print(GetterBase.calls);
// CHECK-NEXT: 5

class GetterGrandparent {
  @Hermes.final
  static get ancestor(): number {
    return this === GetterLeaf ? 47 : 53;
  }
}

class GetterMiddle extends GetterGrandparent { }

class GetterLeaf extends GetterMiddle {
  static readAncestor(): number {
    // Lookup must continue past GetterMiddle, which has no own getter.
    return super.ancestor;
  }
}

print(GetterLeaf.readAncestor());
// CHECK-NEXT: 47

class SetterBase {
  static calls: number = 0;
  static field: number = 0;
  @Hermes.final
  static set inherited(value: number) {
    SetterBase.calls += 1;
    SetterBase.field = value;
  }
}

class SetterChild extends SetterBase {

  static writeToChildField(val: number): void {
    super.inherited = val;
  }
}

SetterChild.writeToChildField(5);
print(SetterBase.field);
// CHECK-NEXT: 5

print(SetterBase.calls);
// CHECK-NEXT: 1


class SetterGrandparent {
  static _field: number = 0;
  static calls: number = 0;
  @Hermes.final
  static set field(val: number): void {
    SetterGrandparent.calls += 1;
    SetterGrandparent._field = val;

  }
}

class SetterMiddle extends SetterGrandparent { }

class SetterLeaf extends SetterMiddle {
  static writeAncestor(val: number): void {
    // Lookup must continue past SetterMiddle, which has no own setter.
    super.field = val;
  }
}

print(SetterLeaf._field);
// CHECK-NEXT: 0
SetterLeaf.writeAncestor(888);

print(SetterLeaf.calls);
//CHECK-NEXT: 1
print(SetterLeaf._field);
// CHECK-NEXT: 888




class GenericGrand {
  static value: number = 5;
}
class GenericBase extends GenericGrand {
  static value: number = 7;
  @Hermes.final
  static read<T>(argument: T): number { return super.value; }
  static nonGenericRead(): number { return super.value; }
}
class GenericChild extends GenericBase {
  static value: number = 9;
}
print(GenericChild.read < number > (1)); // Expected: 5.
print(GenericChild.nonGenericRead()); // Expected: 5.
// CHECK-NEXT: 5
// CHECK-NEXT: 5

// checks to make sure the correct 'this' is passed when invoking generic methods through super
class BaseSuperThis {
  static id: number = 10;

  @Hermes.final
  static addRead<T>(param: T): number {
    return this.id + param;
  }
}

class DerivedSuperThis extends BaseSuperThis {
  static id: number = 20;
  @Hermes.final
  static callAddRead<T>(param: T): number {
    return super.addRead < T > (param);
  }
}

print(DerivedSuperThis.callAddRead < number > (6));
// CHECK-NEXT: 26
//
// checks to make sure the 'this' correctly retrieves the base class id field when the derived class id is missing
class BaseFallbackSuperThis {
  static id: number = 10;

  @Hermes.final
  static readAdd<T>(param: T): number {
    return this.id + param;
  }
}

class DerivedFallbackSuperThis extends BaseFallbackSuperThis {
  @Hermes.final
  static callReadAdd<T>(param: T): number {
    return super.readAdd < T > (param);
  }
}
print(DerivedFallbackSuperThis.callReadAdd < number > (6));
// CHECK-NEXT: 16

class DiffReceiverBase {
  static id: number = 10;

  @Hermes.final
  static baseFirst<T>(unused: T): number {
    return this.id;
  }

  @Hermes.final
  static derivedFirst<T>(unused: T): number {
    return this.id;
  }
}

class DiffReceiverChild extends DiffReceiverBase {
  static id: number = 20;

  @Hermes.final
  static readBaseFirst<T>(value: T): number {
    return super.baseFirst < T > (value);
  }

  @Hermes.final
  static readDerivedFirst<T>(value: T): number {
    return super.derivedFirst < T > (value);
  }
}
print(DiffReceiverBase.baseFirst < number > (0));
// CHECK-NEXT: 10
print(DiffReceiverChild.readBaseFirst < number > (0));
// CHECK-NEXT: 20

print(DiffReceiverChild.readDerivedFirst < number > (0));
// CHECK-NEXT: 20
print(DiffReceiverBase.derivedFirst < number > (0));
// CHECK-NEXT: 10


class InferredTypeBase {
  static id: number = 10;

  @Hermes.final
  static baseFirst<T>(unused: T): number {
    return this.id;
  }

  @Hermes.final
  static derivedFirst<T>(unused: T): number {
    return this.id;
  }
}

class InferredTypeChild extends InferredTypeBase {
  static id: number = 20;

  static readBaseFirst(value: number): number {
    return super.baseFirst(value);
  }
  static readDerivedFirst(value: number): number {
    return super.derivedFirst(value);
  }
}
print(InferredTypeBase.baseFirst < number > (0));
// CHECK-NEXT: 10
print(InferredTypeChild.readBaseFirst(0));
// CHECK-NEXT: 20

print(InferredTypeChild.readDerivedFirst(0));
// CHECK-NEXT: 20
print(InferredTypeBase.derivedFirst < number > (0));
// CHECK-NEXT: 10

