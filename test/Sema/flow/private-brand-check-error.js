/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

'use strict';

// Private brand checks are not supported in typed classes, for either
// instance or static private names.
class C {
  #x: number = 1;
  static #s: number = 2;
  hasX(o: any): boolean {
    return #x in o;
  }
  static hasS(o: any): boolean {
    return #s in o;
  }
  // Errors in the right operand are still reported.
  hasXErr(o: any): boolean {
    return #x in (() => { let n: number = 's'; return o; })();
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}private-brand-check-error.js:18:12: error: ft: private brand checks ('#x in obj') are not supported
// CHECK-NEXT:    return #x in o;
// CHECK-NEXT:           ^~
// CHECK-NEXT:{{.*}}private-brand-check-error.js:21:12: error: ft: private brand checks ('#x in obj') are not supported
// CHECK-NEXT:    return #s in o;
// CHECK-NEXT:           ^~
// CHECK-NEXT:{{.*}}private-brand-check-error.js:25:12: error: ft: private brand checks ('#x in obj') are not supported
// CHECK-NEXT:    return #x in (() => { let n: number = 's'; return o; })();
// CHECK-NEXT:           ^~
// CHECK-NEXT:{{.*}}private-brand-check-error.js:25:31: error: ft: incompatible initialization type: cannot assign "s" to number
// CHECK-NEXT:    return #x in (() => { let n: number = 's'; return o; })();
// CHECK-NEXT:                              ^~~~~~~~~~~~~~~
// CHECK-NEXT:Emitted 4 errors. exiting.
