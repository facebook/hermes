/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -ferror-limit=0 -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

// Generic method without @Hermes.final.
class A {
  foo<T>(x: T): T { return x; }
}

// Generic private method without @Hermes.final.
class A2 {
  #foo<T>(x: T): T { return x; }
}

// Cannot override parent method with generic method.
class B {
  foo(x: number): number { return x; }
}
class C extends B {
  @Hermes.final
  foo<T>(x: T): T { return x; }
}

// Cannot override a final (generic) method.
class D {
  @Hermes.final
  bar<T>(x: T): T { return x; }
}
class E extends D {
  bar(x: number): number { return x; }
}

// Cannot apply @Hermes.final to a constructor.
class F {
  @Hermes.final
  constructor() {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}generic-method-error.js:12:6: error: ft: generic methods must be marked as final with @Hermes.final
// CHECK-NEXT:  foo<T>(x: T): T { return x; }
// CHECK-NEXT:     ^~~
// CHECK-NEXT:{{.*}}generic-method-error.js:17:7: error: ft: generic methods must be marked as final with @Hermes.final
// CHECK-NEXT:  #foo<T>(x: T): T { return x; }
// CHECK-NEXT:      ^~~
// CHECK-NEXT:{{.*}}generic-method-error.js:26:3: error: ft: cannot override with generic method
// CHECK-NEXT:  foo<T>(x: T): T { return x; }
// CHECK-NEXT:  ^~~
// CHECK-NEXT:{{.*}}generic-method-error.js:35:3: error: ft: cannot override final method
// CHECK-NEXT:  bar(x: number): number { return x; }
// CHECK-NEXT:  ^~~
// CHECK-NEXT:{{.*}}generic-method-error.js:40:3: error: ft: @Hermes.final cannot be applied to a constructor
// CHECK-NEXT:  @Hermes.final
// CHECK-NEXT:  ^
// CHECK-NEXT:Emitted 5 errors. exiting.
