/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

declare module Foo {
  import Bar from "baz";
}


// CHECK: {{.*}}:11:3: error: imports within a `declare module` body must always be `import type` or `import typeof`
// CHECK-NEXT:   import Bar from "baz";
// CHECK-NEXT:   ^~~~~~~~~~~~~~~~~~~~~~
