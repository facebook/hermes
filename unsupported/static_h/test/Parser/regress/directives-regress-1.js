/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -non-strict %s

// Assertion failure because parser incorrectly recognized this as a directive
// but the AST validator did not.
"use strict"+
1;
"use strict";

x = 1;
