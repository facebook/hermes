/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc --transform-ts --dump-transformed-ast %s 2>&1) | %FileCheck %s

// CHECK: {{.*}}:12:1: error: TypeScript enums are not supported with --transform-ts
// CHECK: {{.*}}:14:1: error: TypeScript namespaces/modules are not supported with --transform-ts
enum Color { Red, Green, Blue }

namespace Foo { const x = 1; }
