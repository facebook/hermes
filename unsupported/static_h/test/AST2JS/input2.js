/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-js %s | %FileCheck --match-full-lines %s
// RUN: %hermesc -dump-js --pretty=0 %s | %FileCheck --match-full-lines --check-prefix=UGLY %s

-(-x);
-(--x);
-(-5);
+(+x);
+(++x);
a-(-x);
a-(--x);
a-(-5);
a+(+x);
a+(++x);

//CHECK:      -(-x);
//CHECK-NEXT: -(--x);
//CHECK-NEXT: -(-5);
//CHECK-NEXT: +(+x);
//CHECK-NEXT: +(++x);
//CHECK-NEXT: a - (-x);
//CHECK-NEXT: a - (--x);
//CHECK-NEXT: a - (-5);
//CHECK-NEXT: a + (+x);
//CHECK-NEXT: a + (++x);

//UGLY: - -x;- --x;- -5;+ +x;+ ++x;a- -x;a- --x;a- -5;a+ +x;a+ ++x;
