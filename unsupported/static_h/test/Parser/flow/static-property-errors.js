/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

interface I {
  static foo: string
}
// CHECK: {{.*}}:11:3: error: invalid 'static' modifier
// CHECK-NEXT:   static foo: string
// CHECK-NEXT:   ^~~~~~

interface I {
  static foo(): void
}
// CHECK-NEXT: {{.*}}:18:3: error: invalid 'static' modifier
// CHECK-NEXT:   static foo(): void
// CHECK-NEXT:   ^~~~~~

interface I {
  static [string]: string
}
// CHECK-NEXT: {{.*}}:25:3: error: invalid 'static' modifier
// CHECK-NEXT:   static [string]: string
// CHECK-NEXT:   ^~~~~~

interface I {
  static [[foo]]: string
}
// CHECK-NEXT: {{.*}}:32:3: error: invalid 'static' modifier
// CHECK-NEXT:   static {{\[\[}}foo{{\]\]}}: string
// CHECK-NEXT:   ^~~~~~

interface I {
  static get foo(): string
}
// CHECK-NEXT: {{.*}}:39:3: error: invalid 'static' modifier
// CHECK-NEXT:   static get foo(): string
// CHECK-NEXT:   ^~~~~~

declare class C {
  static+: string
}
// CHECK-NEXT: {{.*}}:46:9: error: Unexpected variance sigil
// CHECK-NEXT:   static+: string
// CHECK-NEXT:         ^
