/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -Wno-direct-eval -sample-profiling=chrome -profiling-out=%t.cpuprofile %s

// NOTE: This test is here to check that the sampling profiler can play nicely
// with the multiple domains.
var global = this;
print(eval('global.x = function() {}'));
print(eval('global.x = function() {var y = 100}'));
print(eval('global.x = function() {var y = 3100}'));
