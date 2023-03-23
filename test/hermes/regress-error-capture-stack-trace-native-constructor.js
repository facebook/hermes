/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

"use strict";

function BaseError(optErrorConstructor=undefined) {
    Error.apply(this, ["Error"]);
    Error.captureStackTrace(this, optErrorConstructor);
}

function DerivedError(optErrorConstructor=undefined) {
    BaseError.apply(this, [optErrorConstructor]);
}

print("BaseError()");
print((new BaseError()).stack);
// CHECK-LABEL: BaseError()
// CHECK-NEXT: Error
// CHECK-NEXT:     at BaseError ({{.*}}.js:14:28)
// CHECK-NEXT:     at global ({{.*}}.js:22:21)

print("BaseError(Error)");
print((new BaseError(Error)).stack);
// CHECK-LABEL: BaseError(Error)
// CHECK-NEXT: Error

print("BaseError(Error.constructor)");
print((new BaseError(Error.constructor)).stack);
// CHECK-LABEL: BaseError(Error.constructor)
// CHECK-NEXT: Error

print("BaseError(BaseError)");
print((new BaseError(BaseError)).stack);
// CHECK-LABEL: BaseError(BaseError)
// CHECK-NEXT: Error
// CHECK-NEXT:     at global ({{.*}}.js:39:21)

print("BaseError(BaseError.constructor)");
print((new BaseError(BaseError.constructor)).stack);
// CHECK-LABEL: BaseError(BaseError.constructor)
// CHECK-NEXT: Error

print("DerivedError()");
print((new DerivedError()).stack);
// CHECK-LABEL: DerivedError()
// CHECK-NEXT: Error
// CHECK-NEXT:     at BaseError ({{.*}}.js:14:28)
// CHECK-NEXT:     at apply (native)
// CHECK-NEXT:     at DerivedError ({{.*}}.js:18:20)
// CHECK-NEXT:     at global ({{.*}}.js:50:24)

print("DerivedError(Error)");
print((new DerivedError(Error)).stack);
// CHECK-LABEL: DerivedError(Error)
// CHECK-NEXT: Error

print("DerivedError(Error.constructor)");
print((new DerivedError(Error.constructor)).stack);
// CHECK-LABEL: DerivedError(Error.constructor)
// CHECK-NEXT: Error

print("DerivedError(BaseError)");
print((new DerivedError(BaseError)).stack);
// CHECK-LABEL: DerivedError(BaseError)
// CHECK-NEXT: Error
// CHECK-NEXT:     at apply (native)
// CHECK-NEXT:     at DerivedError ({{.*}}.js:18:20)
// CHECK-NEXT:     at global ({{.*}}.js:69:24)

print("DerivedError(BaseError.constructor)");
print((new DerivedError(BaseError.constructor)).stack);
// CHECK-LABEL: DerivedError(BaseError.constructor)
// CHECK-NEXT: Error

print("DerivedError(DerivedError)");
print((new DerivedError(DerivedError)).stack);
// CHECK-LABEL: DerivedError(DerivedError)
// CHECK-NEXT: Error
// CHECK-NEXT:     at global ({{.*}}.js:82:24)

print("DerivedError(DerivedError.constructor)");
print((new DerivedError(DerivedError.constructor)).stack);
// CHECK-LABEL: DerivedError(DerivedError.constructor)
// CHECK-NEXT: Error
