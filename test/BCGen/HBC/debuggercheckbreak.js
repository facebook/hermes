/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-bytecode -target=HBC %s -O -g | %FileCheckOrRegen %s --match-full-lines

function test1() {
  var count = 0;
  for(var count=0; ; count++) {
    if (Math.random() === 3)
      break;
    if (Math.random() === 5)
      for(;;){} // infinite loop
  }
  while (count > 10)
    count--;
  print(count);
}
