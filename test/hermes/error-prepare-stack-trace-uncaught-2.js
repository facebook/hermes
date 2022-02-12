/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermes -target=HBC %s 2>&1 || true) | %FileCheck --match-full-lines -D=SRC=%s %s
"use strict";

// When printing an uncaught error, if prepareStackTrace throws, fall back to
// the plain stack getter.
Error.prepareStackTrace = (e, callSites) => {throw new Error("inner")};
throw new Error("outer");
// CHECK: Uncaught Error: outer
// CHECK-NEXT: at global ([[SRC]]:14:16)
// CHECK-EMPTY
