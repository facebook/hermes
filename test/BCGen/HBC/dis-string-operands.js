/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -Wno-undefined-variable -O -dump-bytecode %s | %FileCheck --match-full-lines %s
// Test that we are correctly disassembling string operands.
"use strict";

var glob = {prop: bar()};
var re = /foo/i;
glob.baz;
bazz = "const-string";
delete glob.baz;

//CHECK:    DeclareGlobalVar  "glob"
//CHECK:    DeclareGlobalVar  "re"
//CHECK:    TryGetById        {{r[0-9]+}}, {{r[0-9]+}}, 1, "bar"
//CHECK:    PutNewOwnByIdShort   {{r[0-9]+}}, {{r[0-9]+}}, "prop"
//CHECK:    PutById           {{r[0-9]+}}, {{r[0-9]+}}, 1, "glob"
//CHECK:    CreateRegExp      {{r[0-9]+}}, "foo", "i", 0
//CHECK:    PutById           {{r[0-9]+}}, {{r[0-9]+}}, 2, "re"
//CHECK:    GetByIdShort      {{r[0-9]+}}, {{r[0-9]+}}, 2, "glob"
//CHECK:    GetByIdShort      {{r[0-9]+}}, {{r[0-9]+}}, 3, "baz"
//CHECK:    LoadConstString   {{r[0-9]+}}, "const-string"
//CHECK:    TryPutById        {{r[0-9]+}}, {{r[0-9]+}}, 3, "bazz"
//CHECK:    GetByIdShort      {{r[0-9]+}}, {{r[0-9]+}}, 2, "glob"
//CHECK:    DelById           {{r[0-9]+}}, {{r[0-9]+}}, "baz"
