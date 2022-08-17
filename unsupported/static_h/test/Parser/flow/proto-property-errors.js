/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermes -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

// Proto not allowed outside declare class
interface I {
  proto foo: string
}
// CHECK: {{.*}}:12:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto foo: string
// CHECK-NEXT:   ^~~~~

declare class C {
  proto foo(): void
}
// CHECK-NEXT: {{.*}}:19:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto foo(): void
// CHECK-NEXT:   ^~~~~

declare class C {
  proto (): void
}
// CHECK-NEXT: {{.*}}:26:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto (): void
// CHECK-NEXT:   ^~~~~

declare class C {
  proto [string]: string
}
// CHECK-NEXT: {{.*}}:33:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto [string]: string
// CHECK-NEXT:   ^~~~~

declare class C {
  proto [[foo]]: string
}
// CHECK-NEXT: {{.*}}:40:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto {{\[\[}}foo{{\]\]}}: string
// CHECK-NEXT:   ^~~~~

declare class C {
  proto get foo(): string
}
// CHECK-NEXT: {{.*}}:47:3: error: invalid 'proto' modifier
// CHECK-NEXT:   proto get foo(): string
// CHECK-NEXT:   ^~~~~

declare class C {
  proto+: string
}
// CHECK-NEXT: {{.*}}:54:8: error: Unexpected variance sigil
// CHECK-NEXT:   proto+: string
// CHECK-NEXT:        ^
