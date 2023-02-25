/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes --typed --dump-sema %s

let a1 : number[] = [];
let a2 : number[] = a1;

type A1 = number[][];
type A2 = number[][];
type U1 = A1 | string;
type U2 = string | A2 | A1;

let a3 : U1[] = [];
let a4 : U2[] = [];
