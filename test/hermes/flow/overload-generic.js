/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -typed %s | %FileCheck %s --match-full-lines
// RUN: %shermes -typed -exec %s | %FileCheck %s --match-full-lines

// Overloaded methods mixing generic and non-generic overloads.
class Converter {
  @Hermes.final @Hermes.overload
  convert(x: number): number { return x * 2; }
  @Hermes.final @Hermes.overload
  convert<T>(x: T): T { return x; }
}

// Explicit type arguments select the generic overload.
print("explicit type args");
// CHECK-LABEL: explicit type args
let conv = new Converter();
print(conv.convert<string>("hello"));
// CHECK-NEXT: hello
print(conv.convert<boolean>(true));
// CHECK-NEXT: true
