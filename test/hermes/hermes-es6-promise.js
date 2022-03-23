/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck --match-full-lines --check-prefix=ON %s
// RUN: %hermes -Xes6-promise=0 %s | %FileCheck --match-full-lines --check-prefix=OFF %s

print(HermesInternal.hasPromise());
// ON: true
// OFF: false

print(typeof Promise);
// ON: function
// OFF: undefined
