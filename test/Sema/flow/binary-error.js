/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror --typed --dump-sema -ferror-limit=0 %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s

function main() {
  'a' + 1;
  'a' - 1;
  'a' * 1;
  'a' / 1;

  1 + 'a';
  1 - 'a';
  1 * 'a';
  1 / 'a';

  'a' - 'a';
  'a' * 'a';
  'a' / 'a';

  1 < 'a';
  1 <= 'a';
  1 > 'a';
  1 >= 'a';

  'a' < 1;
  'a' <= 1;
  'a' > 1;
  'a' >= 1;

  1 | 1n;
  1 ^ 1n;
  1 & 1n;

  1n | 1;
  1n ^ 1;
  1n & 1;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}binary-error.js:11:3: error: ft: incompatible binary operation: + cannot be applied to string and number
// CHECK-NEXT:  'a' + 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:12:3: error: ft: incompatible binary operation: - cannot be applied to string and number
// CHECK-NEXT:  'a' - 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:13:3: error: ft: incompatible binary operation: * cannot be applied to string and number
// CHECK-NEXT:  'a' * 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:14:3: error: ft: incompatible binary operation: / cannot be applied to string and number
// CHECK-NEXT:  'a' / 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:16:3: error: ft: incompatible binary operation: + cannot be applied to number and string
// CHECK-NEXT:  1 + 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:17:3: error: ft: incompatible binary operation: - cannot be applied to number and string
// CHECK-NEXT:  1 - 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:18:3: error: ft: incompatible binary operation: * cannot be applied to number and string
// CHECK-NEXT:  1 * 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:19:3: error: ft: incompatible binary operation: / cannot be applied to number and string
// CHECK-NEXT:  1 / 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:21:3: error: ft: incompatible binary operation: - cannot be applied to string and string
// CHECK-NEXT:  'a' - 'a';
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:22:3: error: ft: incompatible binary operation: * cannot be applied to string and string
// CHECK-NEXT:  'a' * 'a';
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:23:3: error: ft: incompatible binary operation: / cannot be applied to string and string
// CHECK-NEXT:  'a' / 'a';
// CHECK-NEXT:  ^~~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:25:3: error: ft: incompatible binary operation: < cannot be applied to number and string
// CHECK-NEXT:  1 < 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:26:3: error: ft: incompatible binary operation: <= cannot be applied to number and string
// CHECK-NEXT:  1 <= 'a';
// CHECK-NEXT:  ^~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:27:3: error: ft: incompatible binary operation: > cannot be applied to number and string
// CHECK-NEXT:  1 > 'a';
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:28:3: error: ft: incompatible binary operation: >= cannot be applied to number and string
// CHECK-NEXT:  1 >= 'a';
// CHECK-NEXT:  ^~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:30:3: error: ft: incompatible binary operation: < cannot be applied to string and number
// CHECK-NEXT:  'a' < 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:31:3: error: ft: incompatible binary operation: <= cannot be applied to string and number
// CHECK-NEXT:  'a' <= 1;
// CHECK-NEXT:  ^~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:32:3: error: ft: incompatible binary operation: > cannot be applied to string and number
// CHECK-NEXT:  'a' > 1;
// CHECK-NEXT:  ^~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:33:3: error: ft: incompatible binary operation: >= cannot be applied to string and number
// CHECK-NEXT:  'a' >= 1;
// CHECK-NEXT:  ^~~~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:35:3: error: ft: incompatible binary operation: | cannot be applied to number and bigint
// CHECK-NEXT:  1 | 1n;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:36:3: error: ft: incompatible binary operation: ^ cannot be applied to number and bigint
// CHECK-NEXT:  1 ^ 1n;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:37:3: error: ft: incompatible binary operation: & cannot be applied to number and bigint
// CHECK-NEXT:  1 & 1n;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:39:3: error: ft: incompatible binary operation: | cannot be applied to bigint and number
// CHECK-NEXT:  1n | 1;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:40:3: error: ft: incompatible binary operation: ^ cannot be applied to bigint and number
// CHECK-NEXT:  1n ^ 1;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:{{.*}}binary-error.js:41:3: error: ft: incompatible binary operation: & cannot be applied to bigint and number
// CHECK-NEXT:  1n & 1;
// CHECK-NEXT:  ^~~~~~
// CHECK-NEXT:Emitted 25 errors. exiting.
