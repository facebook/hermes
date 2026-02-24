/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -lazy %s 2>&1 ) | %FileCheck %s --match-full-lines

class A {
  static {
    let arr1 = () => {
      let arr2 = () => {
        let arr3 = () => {
          print(arguments);
// CHECK:Uncaught SyntaxError: 15:17:invalid use of 'arguments' as an identifier
        }
        arr3();
      }
      arr2();
    };
    arr1();
  }
}

