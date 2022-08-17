/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
print("nan ops");
// CHECK-LABEL: nan ops

print((undefined+1) == (undefined+1));
// CHECK-NEXT: false

print((undefined+1) != (undefined+1));
// CHECK-NEXT: true

print((undefined+1) === (undefined+1));
// CHECK-NEXT: false

print((undefined+1) !== (undefined+1));
// CHECK-NEXT: true

print((undefined+1) == 123);
// CHECK-NEXT: false

print((undefined+1) != 123);
// CHECK-NEXT: true

print((undefined+1) === 123);
// CHECK-NEXT: false

print((undefined+1) !== 123);
// CHECK-NEXT: true

print(123 == (undefined+1));
// CHECK-NEXT: false

print(123 != (undefined+1));
// CHECK-NEXT: true

print(123 === (undefined+1));
// CHECK-NEXT: false

print(123 !== (undefined+1));
// CHECK-NEXT: true



print((undefined+1) < (undefined+1));
// CHECK-NEXT: false

print((undefined+1) <= (undefined+1));
// CHECK-NEXT: false

print((undefined+1) > (undefined+1));
// CHECK-NEXT: false

print((undefined+1) >= (undefined+1));
// CHECK-NEXT: false

print((undefined+1) < 123);
// CHECK-NEXT: false

print((undefined+1) <= 123);
// CHECK-NEXT: false

print((undefined+1) > 123);
// CHECK-NEXT: false

print((undefined+1) >= 123);
// CHECK-NEXT: false

print(123 == (undefined+1));
// CHECK-NEXT: false

print(123 != (undefined+1));
// CHECK-NEXT: true

print(123 === (undefined+1));
// CHECK-NEXT: false

print(123 !== (undefined+1));
// CHECK-NEXT: true

print(123 < (undefined+1));
// CHECK-NEXT: false

print(123 <= (undefined+1));
// CHECK-NEXT: false

print(123 > (undefined+1));
// CHECK-NEXT: false

print(123 >= (undefined+1));
// CHECK-NEXT: false



print((undefined+1) + (undefined+1));
// CHECK-NEXT: NaN

print((undefined+1) - (undefined+1));
// CHECK-NEXT: NaN

print((undefined+1) * (undefined+1));
// CHECK-NEXT: NaN

print((undefined+1) / (undefined+1));
// CHECK-NEXT: NaN

print((undefined+1) % (undefined+1));
// CHECK-NEXT: NaN

print(123 + (undefined+1));
// CHECK-NEXT: NaN

print(123 - (undefined+1));
// CHECK-NEXT: NaN

print(123 * (undefined+1));
// CHECK-NEXT: NaN

print(123 / (undefined+1));
// CHECK-NEXT: NaN

print(123 % (undefined+1));
// CHECK-NEXT: NaN

print((undefined+1) + 123);
// CHECK-NEXT: NaN

print((undefined+1) - 123);
// CHECK-NEXT: NaN

print((undefined+1) * 123);
// CHECK-NEXT: NaN

print((undefined+1) / 123);
// CHECK-NEXT: NaN

print((undefined+1) % 123);
// CHECK-NEXT: NaN



print((undefined+1) | (undefined+1));
// CHECK-NEXT: 0

print((undefined+1) ^ (undefined+1));
// CHECK-NEXT: 0

print((undefined+1) & (undefined+1));
// CHECK-NEXT: 0

print(2048 | (undefined+1));
// CHECK-NEXT: 2048

print(2048 ^ (undefined+1));
// CHECK-NEXT: 2048

print(2048 & (undefined+1));
// CHECK-NEXT: 0

print((undefined+1) | 2048);
// CHECK-NEXT: 2048

print((undefined+1) ^ 2048);
// CHECK-NEXT: 2048

print((undefined+1) & 2048);
// CHECK-NEXT: 0



print((undefined+1) << (undefined+1));
// CHECK-NEXT: 0

print((undefined+1) >> (undefined+1));
// CHECK-NEXT: 0

print((undefined+1) >>> (undefined+1));
// CHECK-NEXT: 0

print(20 <<  (undefined+1));
// CHECK-NEXT: 20

print(20 >>  (undefined+1));
// CHECK-NEXT: 20

print(20 >>> (undefined+1));
// CHECK-NEXT: 20

print((undefined+1) <<  20);
// CHECK-NEXT: 0

print((undefined+1) >>  20);
// CHECK-NEXT: 0

print((undefined+1) >>> 20);
// CHECK-NEXT: 0

print("Hello" + (undefined+1));
// CHECK-NEXT: HelloNaN

print((undefined+1) + "Hello");
// CHECK-NEXT: NaNHello

print((undefined+1) + undefined);
// CHECK-NEXT: NaN

print(undefined + (undefined+1));
// CHECK-NEXT: NaN
