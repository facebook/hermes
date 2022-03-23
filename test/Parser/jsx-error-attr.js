/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-jsx -dump-ast -pretty-json %s 2>&1 ) | %FileCheck %s --match-full-lines

<foo x=y></foo>
// CHECK:  {{.*}}:10:8: error: '{' expected in JSX attribute
// CHECK-NEXT: <foo x=y></foo>
// CHECK-NEXT:      ~~^
