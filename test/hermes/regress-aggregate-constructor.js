/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s

// Previously this failed in debug mode,
// due to allocating too many handles.
var err = AggregateError("abcdefgh");
print(err.errors[0]);
// CHECK: a
