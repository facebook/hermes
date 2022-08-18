/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -dump-ast %s | %FileCheck %s --check-prefix=CHKAST
// RUN: %hermes -O0 -dump-ir %s | %FileCheck %s --check-prefix=CHKIR

// This test ensures that a very large numeric literal can be parsed as
// Infinity.
// It is only testing the parser, so it should use dump-ast; however, Infinity
// is not serializable in JSON, so this test needs to happen at the IR level.

55e55555555555555555555555555555555555;

//CHKAST:{
//CHKAST-NEXT:  "type": "Program",
//CHKAST-NEXT:  "body": [
//CHKAST-NEXT:    {
//CHKAST-NEXT:      "type": "ExpressionStatement",
//CHKAST-NEXT:      "expression": {
//CHKAST-NEXT:        "type": "NumericLiteral",
//CHKAST-NEXT:        "value": null,
//CHKAST-NEXT:        "raw": "55e55555555555555555555555555555555555"
//CHKAST-NEXT:      },
//CHKAST-NEXT:      "directive": null
//CHKAST-NEXT:    }
//CHKAST-NEXT:  ]
//CHKAST-NEXT:}

// CHKIR-LABEL: %BB0:
// CHKIR:   %[[RETVAL:[0-9]+]] = AllocStackInst $?anon_0_ret
// CHKIR:   %{{[0-9]+}} = StoreStackInst Infinity : number, %[[RETVAL]]
