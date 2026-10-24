/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Accessor bodies are checked against the accessor's own function type, not
// against the type of the property it implements.

class A {
  _n: number;
  constructor() { this._n = 0; }

  @Hermes.final
  get wrongReturn(): number {
    return 'a';
  }

  @Hermes.final
  get missingReturn(): number {
  }

  // The property type is itself a function type: the body must be checked
  // against the getter's signature, not against the property's.
  @Hermes.final
  get fnValued(): (x: string) => void {
    return (y: number): void => {};
  }

  // The setter's parameter has its declared type inside the body.
  @Hermes.final
  set param(v: string) {
    this._n = v;
  }

  // A setter returns void.
  @Hermes.final
  set returnsValue(v: number) {
    return v;
  }

  // Even an unannotated setter may not return a value; a bare return is fine.
  @Hermes.final
  set untypedReturn(v) {
    if (v) return;
    return v;
  }

  @Hermes.final
  static get staticWrongReturn(): number {
    return 'b';
  }

  @Hermes.final
  static set staticParam(v: string) {
    let q: number = v;
  }
}

// Both halves of a getter/setter pair are checked.
class B {
  _n: number;
  constructor() { this._n = 0; }

  @Hermes.final
  get pair(): number {
    return 'c';
  }

  @Hermes.final
  set pair(v: number): void {
    let q: string = v;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}accessor-body-error.js:21:5: error: ft: return value incompatible with return type: cannot return "a" as number
// CHECK-NEXT:    return 'a';
// CHECK-NEXT:    ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:25:24: error: ft: implicitly-returned 'undefined' incompatible with return type: number
// CHECK-NEXT:  get missingReturn(): number {
// CHECK-NEXT:                       ^~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:32:5: error: ft: return value incompatible with return type: cannot return function as function
// CHECK-NEXT:    return (y: number): void => {};
// CHECK-NEXT:    ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:38:5: error: ft: incompatible assignment type: cannot implicitly cast from string to number
// CHECK-NEXT:    this._n = v;
// CHECK-NEXT:    ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:44:5: error: ft: setter cannot return a value
// CHECK-NEXT:    return v;
// CHECK-NEXT:    ^~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:51:5: error: ft: setter cannot return a value
// CHECK-NEXT:    return v;
// CHECK-NEXT:    ^~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:56:5: error: ft: return value incompatible with return type: cannot return "b" as number
// CHECK-NEXT:    return 'b';
// CHECK-NEXT:    ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:61:9: error: ft: incompatible initialization type: cannot assign string to number
// CHECK-NEXT:    let q: number = v;
// CHECK-NEXT:        ^~~~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:72:5: error: ft: return value incompatible with return type: cannot return "c" as number
// CHECK-NEXT:    return 'c';
// CHECK-NEXT:    ^~~~~~~~~~~
// CHECK-NEXT:{{.*}}accessor-body-error.js:77:9: error: ft: incompatible initialization type: cannot assign number to string
// CHECK-NEXT:    let q: string = v;
// CHECK-NEXT:        ^~~~~~~~~~~~~
// CHECK-NEXT:Emitted 10 errors. exiting.
