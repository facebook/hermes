/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -target=HBC %s 2>&1 || true) | %FileCheck --match-full-lines %s
"use strict";

Error.prepareStackTrace = (e, callSites) => callSites;

// Uncaught errors are formatted with prepareStackTrace
throw new Error('foo');
// CHECK: Uncaught [object CallSite]
// CHECK-EMPTY
