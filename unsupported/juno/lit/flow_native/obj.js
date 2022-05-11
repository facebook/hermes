/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %fnc < %s

var a = {x: 1, ["y"]: 2, z: 3, f: function (){return 4;}}
a.b = 5;
a["c"] = 6;
var b = "z";
var res = a.c + a.b + a["x"] + a.y + a.z + a.f() + a[b];
