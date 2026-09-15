/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheck %s --match-full-lines

// Function variance must check unmatched fixed params against the other
// side's rest element type when only one side has a rest param.

// Case A: source has rest, target has more fixed params. Calls through
// the target type would route values into the rest array, so the target's
// extra fixed params must be assignable to the source's rest element.
type FA1 = (x: number, ...rest: Array<string>) => void;
type FB1 = (x: number, y: number) => void;
function ga(x: number, ...rest: Array<string>): void {}
const fa1: FA1 = ga;
// CHECK: {{.*}}:[[@LINE+1]]:7: error: ft: incompatible initialization type: cannot assign function FA1 to function FB1
const fb1: FB1 = fa1;

// Case B: target has rest, source has additional fixed (optional) params.
// The target's rest element type must flow into each of the source's
// extra optional fixed params.
type FA2 = (x: number, y?: number) => void;
type FB2 = (x: number, ...rest: Array<string>) => void;
function gb(x: number, y?: number): void {}
const fa2: FA2 = gb;
// CHECK: {{.*}}:[[@LINE+1]]:7: error: ft: incompatible initialization type: cannot assign function FA2 to function FB2
const fb2: FB2 = fa2;

// Case C: both have rest, but source has fewer fixed params. The target's
// extra fixed params must be assignable to the source's rest element.
type FA3 = (x: number, ...rest: Array<string>) => void;
type FB3 = (x: number, y: number, ...rest: Array<string>) => void;
function gc(x: number, ...rest: Array<string>): void {}
const fa3: FA3 = gc;
// CHECK: {{.*}}:[[@LINE+1]]:7: error: ft: incompatible initialization type: cannot assign function FA3 to function FB3
const fb3: FB3 = fa3;
