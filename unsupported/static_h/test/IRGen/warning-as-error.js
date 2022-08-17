/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: ( ! %hermes -dump-ir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines

"use strict";
print(missing_global);

//CHECK: {{.*}}/warning-as-error.js:11:7: error: the variable "missing_global" was not declared in function "global"
