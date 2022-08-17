/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ast %s
// Make sure we parse trailing commas

var a1 = [1,];
var a2 = [,];
var a3 = [1,,2,];
var a4 = [];

var b1 = {x:0,}
var b2 = {}

foo(1, 2, );

function bar(a, b,) {}
var baz = function baz(a,) {}
