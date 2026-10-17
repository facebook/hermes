
/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
// RUN: %shermes -typed -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -O -exec %s | %FileCheck --match-full-lines %s
class MonsterManger {
  static name: string = "MonsterManager";
  objectName: string;
  constructor(objName: string) {
    this.objectName = objName;
  }

  static getManagerName(): string {
    return this.name;
  }
  @Hermes.final
  static concatAndReturnName<T>(val: T, count: number): T {
    let output: string = ""
    for (let i = 0; i < count; i++) {
      output += val + this.name;
    }
    return output;

  }
  static getNameFromArrow() {
    const getIt = () => this.name;
    return getIt();
  }
  static callGetName(): string {
    return this.getManagerName();
  }

  getName(): string {
    return this.objectName;
  }
  static untypedReturnName() {
    return this.name;
  }
}

const manager = new MonsterManger("Wolfman's manager");

print(MonsterManger.getManagerName());
// CHECK-LABEL: MonsterManager

print(MonsterManger.concatAndReturnName < string > ("a ", 1));
// CHECK-NEXT: a MonsterManager

print(MonsterManger.callGetName());
// CHECK-NEXT: MonsterManager

print(manager.getName());
// CHECK-NEXT: Wolfman's manager

print(MonsterManger.getNameFromArrow());
// CHECK-NEXT: MonsterManager

print(MonsterManger.untypedReturnName());
// CHECK-NEXT: MonsterManager


