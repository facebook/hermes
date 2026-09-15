/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-jsx -dump-ast -pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

// Regression test: a member expression is not a valid JSX attribute name, but
// it used to be accepted because the check tested for MemberExpressionNode
// instead of JSXMemberExpressionNode.

<foo a.b="1"></foo>
// CHECK:  {{.*}}:14:6: error: unexpected member expression
// CHECK-NEXT: <foo a.b="1"></foo>
// CHECK-NEXT:      ^~~
// CHECK-NEXT: Emitted 1 errors. exiting.
