/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

'use strict';

const pathSymbol: any = Symbol.for('path');

function createPath(pathParts: Array<string> = []): any {
  const handler: any = {
    get(_target: any, property: any): any {
      if (typeof property === 'string') {
        return createPath([...pathParts, property]);
      }
      if (property === pathSymbol) {
        return pathParts.join('.');
      }
      return undefined;
    },
  };
  return new Proxy({}, handler);
}

function resolvePath(): string {
  return createPath().first.second[pathSymbol];
}

print(resolvePath());
// CHECK: first.second
print(resolvePath());
// CHECK-NEXT: first.second

function joinDynamic(values: any): string {
  let result = '';
  for (const value of values) {
    result += value;
  }
  return result;
}

const values: Array<string> = ['third', '.', 'fourth'];
print(joinDynamic(values));
// CHECK-NEXT: third.fourth

function collectRest(...values: Array<string>): Array<string> {
  return values;
}

print(joinDynamic(collectRest('fifth', '.', 'sixth')));
// CHECK-NEXT: fifth.sixth
