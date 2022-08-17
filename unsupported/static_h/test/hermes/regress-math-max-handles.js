/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O -gc-sanitize-handles=0 %s
// Make sure that we don't get a handle count overflow
"use strict";

var a = [];
for(var i = 0; i < 100; ++i)
  a.push({val: i, valueOf : function() {return i;}});

Math.max.apply(null, a);
print.apply(null, a);
