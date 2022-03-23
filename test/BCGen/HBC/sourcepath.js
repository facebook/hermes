/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -emit-binary -target=HBC -out=%t %s && %hbcdump %t -c "disassemble;quit" | %FileCheck --match-full-lines %s

function func() {
  print("func");
}

//CHECK:   {{.*}}/sourcepath.js[10:1]
//CHECK:   {{.*}}/sourcepath.js[10:1]
