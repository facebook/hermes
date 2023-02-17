/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -g3 && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=G3 --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -g2 && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=G2 --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -g1 && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=G1 --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -g0 && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=G0 --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -g && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=G --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=DEFAULT --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -output-source-map && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=SOURCEMAP --check-prefix=SHARED
// RUN: %hermes -O0 %s -emit-binary -out %t.hbc -output-source-map -g && %hermes -dump-bytecode %t.hbc | %FileCheck --match-full-lines %s --check-prefix=SOURCEMAP-G --check-prefix=SHARED

doSomething();
function nonThrowingFunction() {var x = {};}
function throwingFunction() {throw new Error();}

// SHARED-LABEL: Function<global>{{.*}}
// G3: AsyncBreakCheck
// G2-NOT: AsyncBreakCheck
// G1-NOT: AsyncBreakCheck
// G0-NOT: AsyncBreakCheck
// G: AsyncBreakCheck
// DEFAULT-NOT: AsyncBreakCheck
// SOURCEMAP-NOT: AsyncBreakCheck
// SOURCEMAP-G: AsyncBreakCheck

// SHARED-LABEL: Debug filename table:
// G3: 0: {{.*}}/debug-levels.js
// G2: 0: {{.*}}/debug-levels.js
// G1: 0: {{.*}}/debug-levels.js
// G0-NOT: 0: {{.*}}/debug-levels.js
// G: 0: {{.*}}/debug-levels.js
// DEFAULT: 0: {{.*}}/debug-levels.js
// SOURCEMAP-NOT: 0: {{.*}}/debug-levels.js
// SOURCEMAP-G-NOT: 0: {{.*}}/debug-levels.js

// SHARED-LABEL: Debug source table:
/* nonThrowingFunction is only mapped at -g2 and up. */
// G3: {{.*}} function idx {{.*}}, starts at line 18 col 1
// G2: {{.*}} function idx {{.*}}, starts at line 18 col 1
// G1-NOT: {{.*}} function idx {{.*}}, starts at line 18 col 1
// G0-NOT: {{.*}} function idx {{.*}}, starts at line 18 col 1
// G: {{.*}} function idx {{.*}}, starts at line 18 col 1
// DEFAULT-NOT: {{.*}} function idx {{.*}}, starts at line 18 col 1
// SOURCEMAP-NOT: {{.*}} function idx {{.*}}, starts at line 18 col 1
// SOURCEMAP-G-NOT: {{.*}} function idx {{.*}}, starts at line 18 col 1

/* throwingFunction is mapped at -g1 and up. */
// G3: {{.*}} function idx {{.*}}, starts at line 19 col 1
// G2: {{.*}} function idx {{.*}}, starts at line 19 col 1
// G1: {{.*}} function idx {{.*}}, starts at line 19 col 1
// G0-NOT: {{.*}} function idx {{.*}}, starts at line 19 col 1
// G: {{.*}} function idx {{.*}}, starts at line 19 col 1
// DEFAULT: {{.*}} function idx {{.*}}, starts at line 19 col 1
// SOURCEMAP-NOT: {{.*}} function idx {{.*}}, starts at line 19 col 1
// SOURCEMAP-G-NOT: {{.*}} function idx {{.*}}, starts at line 19 col 1

// SHARED-LABEL: Debug lexical table:
// G3: {{.*}} variable count: 1
// G2-NOT: {{.*}} variable count: 1
// G1-NOT: {{.*}} variable count: 1
// G0-NOT: {{.*}} variable count: 1
// G: {{.*}} variable count: 1
// DEFAULT-NOT: {{.*}} variable count: 1
// SOURCEMAP-NOT: {{.*}} variable count: 1
// SOURCEMAP-G-NOT: {{.*}} variable count: 1
