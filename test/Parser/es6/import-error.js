// RUN: (! %hermesc -commonjs -pretty-json %s 2>&1 ) | %FileCheck --match-full-lines %s

import { foo , foo } from 'foo.js';
// CHECK: {{.*}}/import-error.js:3:16: error: Duplicate entry in import declaration list
// CHECK-NEXT: import { foo , foo } from 'foo.js';
// CHECK-NEXT:                ^~~
// CHECK-NEXT: {{.*}}/import-error.js:3:10: note: first usage of name
// CHECK-NEXT: import { foo , foo } from 'foo.js';
// CHECK-NEXT:          ^~~

import { abc , xyz as abc } from 'bar.js';
// CHECK: {{.*}}/import-error.js:11:23: error: Duplicate entry in import declaration list
// CHECK-NEXT: import { abc , xyz as abc } from 'bar.js';
// CHECK-NEXT:                       ^~~
// CHECK-NEXT: {{.*}}/import-error.js:11:10: note: first usage of name
// CHECK-NEXT: import { abc , xyz as abc } from 'bar.js';
// CHECK-NEXT:          ^~~

import { invalid as catch } from 'invalid.js';
// CHECK: {{.*}}/import-error.js:19:21: error: Invalid local name for import
// CHECK-NEXT: import { invalid as catch } from 'invalid.js';
// CHECK-NEXT:                     ^~~~~
