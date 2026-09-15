/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -Xcompile=false -dump-sema %s | %FileCheck %s --match-full-lines

// Identifiers inside 'with' are marked unresolvable when resolving on behalf
// of a parser. The dumper used to call getExpressionDecl() on them, violating
// its precondition, which asserted in debug builds while release builds
// already printed ' UNR'. It must print UNR in both.

with(o){x;}

// CHECK:            ExpressionStatement
// CHECK-NEXT:                Id 'x' UNR
