/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -dump-ast -commonjs -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s
'use strict';
import { foo , foo } from 'foo.js';
// CHECK: {{.*}}/import-error.js:10:16: error: Duplicate entry in import declaration list
// CHECK-NEXT: import { foo , foo } from 'foo.js';
// CHECK-NEXT:                ^~~
// CHECK-NEXT: {{.*}}/import-error.js:10:10: note: first usage of name
// CHECK-NEXT: import { foo , foo } from 'foo.js';
// CHECK-NEXT:          ^~~

import { abc , xyz as abc } from 'bar.js';
// CHECK: {{.*}}/import-error.js:18:23: error: Duplicate entry in import declaration list
// CHECK-NEXT: import { abc , xyz as abc } from 'bar.js';
// CHECK-NEXT:                       ^~~
// CHECK-NEXT: {{.*}}/import-error.js:18:10: note: first usage of name
// CHECK-NEXT: import { abc , xyz as abc } from 'bar.js';
// CHECK-NEXT:          ^~~

import { invalid as catch } from 'invalid.js';
// CHECK: {{.*}}/import-error.js:26:21: error: Invalid local name for import
// CHECK-NEXT: import { invalid as catch } from 'invalid.js';
// CHECK-NEXT:                     ^~~~~

import * as protected from 'ns.js';
// CHECK: {{.*}}:31:13: error: 'identifier' expected in namespace import
// CHECK-NEXT: import * as protected from 'ns.js';
// CHECK-NEXT:        ~~~~~^
