/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %hermes -typed %s | %FileCheck --match-full-lines %s
// RUN: %shermes -typed -exec %s | %FileCheck --match-full-lines %s

/// Test that contextual typing for arrow function parameters indexes the
/// constraint function type per parameter, so that the Nth arrow parameter
/// receives the Nth parameter type from the expected callback signature.
/// Previously every parameter was inferred as the type of the first
/// constraint parameter.

'use strict';

(function () {

type Entry = {name: string};
type Counter = {value: number};

function combine(
  callback: (left: Entry, right: Counter) => string,
): string {
  return callback(({name: 'left'}: any), ({value: 7}: any));
}

print(
  combine((left, right) => {
    // 'right' must be inferred as Counter, not Entry.
    let v: number = right.value;
    return left.name + ':' + String(v);
  }),
);
// CHECK: left:7

})();
