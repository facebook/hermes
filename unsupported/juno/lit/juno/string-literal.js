/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %juno --gen-js --double-quote-strings --pretty=1 %s | %FileCheck %s --match-full-lines

"abc";

// CHECK: "abc";
