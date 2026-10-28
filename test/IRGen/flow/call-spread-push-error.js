/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -Werror -typed -dump-ir %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Array.push() is typed as 'any', so the FlowChecker does not check its
// arguments and IRGen has to reject an unspreadable one itself. Only an array
// can be spread into push(); a tuple will work once push() is a real function
// instead of a special case.

'use strict';

const a: number[] = [1];
const t: [number] = [2];
const x: any = 0;
a.push(...t);
a.push(...x);

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}call-spread-push-error.js:20:8: error: spread argument must be an array
// CHECK-NEXT:a.push(...t);
// CHECK-NEXT:       ^~~~
// CHECK-NEXT:{{.*}}call-spread-push-error.js:21:8: error: spread argument must be an array
// CHECK-NEXT:a.push(...x);
// CHECK-NEXT:       ^~~~
// CHECK-NEXT:Emitted 2 errors. exiting.
