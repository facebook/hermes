/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -Xparse-flow-match -ferror-limit=0 -dump-sema %s 2>&1) \
// RUN:   | %FileCheck %s --match-full-lines

// IRGen cannot lower a match, so compiling one is rejected in sema. Without
// this the failure surfaced much later as IRGen's generic "invalid statement
// encountered", pointing at the enclosing statement rather than the match.
//
// Resolving on behalf of a parser is unaffected; that is covered by
// match-implicit-return.js.

function stmt(x) {
  match (x) { _ => { g(); } }
}
// CHECK:{{.*}}match-unsupported.js:19:3: error: match statements are unsupported
// CHECK-NEXT:  match (x) { _ => { g(); } }
// CHECK-NEXT:  ^~~~~~~~~~~~~~~~~~~~~~~~~~~

var e = match (g()) { _ => 1 };
// CHECK:{{.*}}match-unsupported.js:25:9: error: match expressions are unsupported
// CHECK-NEXT:var e = match (g()) { _ => 1 };
// CHECK-NEXT:        ^~~~~~~~~~~~~~~~~~~~~~
