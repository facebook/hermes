/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s

// In this test we generate the IR for interesting cases and make sure we don't crash.

// Check a few interesting combinations of lables, breaks and continues;
function lots_of_lables() {
  foo0:
  foo1:
  foo2:
  foo3:
  foo4:
  foo5:
  foo6:
  while (1) { break;
              break foo0;
              break foo6;
              continue;
              continue foo0;
              continue foo6;
  }
}

// Check a few interesting combinations of if/while control flow.
function nested_cfg() {
  while (1) {
    if (2) {
    while(3) {
        if (4) { continue; }
      }
     }
    while(5) {
      if (6) {
        break;
      }
    }
  }
}

// Check the nesting of while statements.
function high_nest_level(cond) {
label0: while (cond) {
label1: while (cond) {
label2: while (cond) {
label3: while (cond) {
label4: while (cond) {
label5: while (cond) {
          if (cond) {
            break label0;
          }
        } } } } } }
}

