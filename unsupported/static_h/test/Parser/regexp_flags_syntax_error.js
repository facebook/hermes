/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes %s -dump-ast 2>&1 ) | %FileCheck %s

// ES6 11.8.5.1: "It is a Syntax Error if IdentifierPart contains a Unicode escape sequence"
/abc/\u1234;

//CHECK: {{.*}}regexp_flags_syntax_error.js:11:7: error: Unicode escape sequences are not allowed in regular expression flags
//CHECK-NEXT: /abc/\u1234;
//CHECK-NEXT:       ^
