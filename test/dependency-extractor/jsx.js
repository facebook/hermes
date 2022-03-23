/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %dependency-extractor %s | %FileCheck --match-full-lines %s

<foo></foo>;
<bar></bar>;

// CHECK: ESM | react/jsx-runtime
// CHECK: ESM | react/jsx-dev-runtime
// CHECK-NOT: ESM | react/jsx-runtime
