/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1) | %FileCheckOrRegen --match-full-lines %s

'use strict';

class A {
  'foo'(): void {}
  1(): void {}
  1n(): void {}
  static 'bar'(): void {}
  @Hermes.final 'f'(): void {}
  @Hermes.final get 'x'(): number { return 1; }
  @Hermes.final 'g'<T>(): void {}
}

class B extends A {
  'baz'(): void {}
}

class C {
  f(): void {}
}
class D extends C {
  'f'(): void {}
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}class-literal-method-error.js:13:3: error: ft: method name must be an identifier
// CHECK-NEXT:  'foo'(): void {}
// CHECK-NEXT:  ^~~~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:14:3: error: ft: method name must be an identifier
// CHECK-NEXT:  1(): void {}
// CHECK-NEXT:  ^
// CHECK-NEXT:{{.*}}class-literal-method-error.js:15:3: error: ft: method name must be an identifier
// CHECK-NEXT:  1n(): void {}
// CHECK-NEXT:  ^~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:16:10: error: ft: method name must be an identifier
// CHECK-NEXT:  static 'bar'(): void {}
// CHECK-NEXT:         ^~~~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:17:17: error: ft: method name must be an identifier
// CHECK-NEXT:  @Hermes.final 'f'(): void {}
// CHECK-NEXT:                ^~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:18:21: error: ft: method name must be an identifier
// CHECK-NEXT:  @Hermes.final get 'x'(): number { return 1; }
// CHECK-NEXT:                    ^~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:19:17: error: ft: method name must be an identifier
// CHECK-NEXT:  @Hermes.final 'g'<T>(): void {}
// CHECK-NEXT:                ^~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:23:3: error: ft: method name must be an identifier
// CHECK-NEXT:  'baz'(): void {}
// CHECK-NEXT:  ^~~~~
// CHECK-NEXT:{{.*}}class-literal-method-error.js:30:3: error: ft: method name must be an identifier
// CHECK-NEXT:  'f'(): void {}
// CHECK-NEXT:  ^~~
// CHECK-NEXT:Emitted 9 errors. exiting.
