/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -Werror %s
"use strict";

var x;
x = Error;
x = EvalError;
x = RangeError;
x = ReferenceError;
x = SyntaxError;
x = TypeError;
x = URIError;
