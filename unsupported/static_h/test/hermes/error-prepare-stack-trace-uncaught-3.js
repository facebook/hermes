/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -w -O -target=HBC %s 2>&1 || true) | %FileCheck --match-full-lines %s
"use strict";

Error.prepareStackTrace = (e, callSites) => {return 'prepared';};

// Duplicate some functions to ensure we are robust against function dedup.
function throws6() {
  throw new Error("Catch me if you can!");
}

try {
  [throws6].forEach(function(arg) { arg(); });
} catch (err) {
  print(err.stack);
}
// CHECK: prepared
