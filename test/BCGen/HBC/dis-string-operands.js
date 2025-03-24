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

//CHECK:    DeclareGlobalVar  "glob"
//CHECK:    DeclareGlobalVar  "re"
//CHECK:    NewObjectWithBuffer {{r[0-9]+}}, 0, 0
//CHECK:    GetGlobalObject   {{r[0-9]+}}
//CHECK:    TryGetById        {{r[0-9]+}}, {{r[0-9]+}}, 1, "bar"
//CHECK:    LoadConstUndefined {{r[0-9]+}}
//CHECK:    Call1             {{r[0-9]+}}, {{r[0-9]+}}, {{r[0-9]+}}
//CHECK:    PutOwnBySlotIdx   {{r[0-9]+}}, {{r[0-9]+}}, 0
//CHECK:    PutByIdStrict     {{r[0-9]+}}, {{r[0-9]+}}, 1, "glob"
//CHECK:    CreateRegExp      {{r[0-9]+}}, "foo", "i", 0
//CHECK:    PutByIdStrict     {{r[0-9]+}}, {{r[0-9]+}}, 2, "re"
//CHECK:    GetByIdShort      {{r[0-9]+}}, {{r[0-9]+}}, 2, "glob"
//CHECK:    GetByIdShort      {{r[0-9]+}}, {{r[0-9]+}}, 3, "baz"
//CHECK:    LoadConstString   {{r[0-9]+}}, "const-string"
//CHECK:    TryPutByIdStrict  {{r[0-9]+}}, {{r[0-9]+}}, 3, "bazz"
//CHECK:    Ret               {{r[0-9]+}}
