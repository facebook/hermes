/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// `delete` removes a property and so violates any typed object's declared
// shape. The check looks at the type of the object (the `_object` of the
// MemberExpression), not the type of the property being deleted.

function delExact(o: {x: number}): void {
  delete o.x;
}

function delExactComputed(o: {x: number}): void {
  delete o['x'];
}

class C {
  x: number = 0;
}
function delClassInstance(c: C): void {
  delete c.x;
}

// `delete` on an `any` object is still fine — there is no declared shape
// to violate.
function delAnyOk(o: any): boolean {
  return delete o.x;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}delete-typed-error.js:15:3: error: ft: cannot delete property of typed object
// CHECK-NEXT:  delete o.x;
// CHECK-NEXT:  ^~~~~~~~~~
// CHECK-NEXT:{{.*}}delete-typed-error.js:19:3: error: ft: cannot delete property of typed object
// CHECK-NEXT:  delete o['x'];
// CHECK-NEXT:  ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}delete-typed-error.js:19:12: error: ft: computed access to exact object types not supported
// CHECK-NEXT:  delete o['x'];
// CHECK-NEXT:           ^~~
// CHECK-NEXT:{{.*}}delete-typed-error.js:26:3: error: ft: cannot delete property of typed object
// CHECK-NEXT:  delete c.x;
// CHECK-NEXT:  ^~~~~~~~~~
// CHECK-NEXT:Emitted 4 errors. exiting.
