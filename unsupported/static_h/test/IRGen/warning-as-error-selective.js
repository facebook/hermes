/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Treat all warnings as errors
// RUN: ( ! %hermes -dump-ir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,ERROR-EVAL

// Treat no warnings as errors
// RUN: ( %hermes -dump-ir -Wno-error %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,WARN-EVAL

// Treat specific warnings as errors
// RUN: ( ! %hermes -dump-ir -Werror=undefined-variable %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,WARN-EVAL
// RUN: ( ! %hermes -dump-ir -Werror=direct-eval %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,ERROR-EVAL
// RUN: ( ! %hermes -dump-ir -Werror=undefined-variable -Werror=direct-eval %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,ERROR-EVAL

// Treat all except specific warnings as errors
// RUN: ( ! %hermes -dump-ir -Werror -Wno-error=undefined-variable %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,ERROR-EVAL
// RUN: ( ! %hermes -dump-ir -Werror -Wno-error=direct-eval %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,WARN-EVAL
// RUN: ( %hermes -dump-ir -Werror -Wno-error=undefined-variable -Wno-error=direct-eval %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,WARN-EVAL

// Rightmost flag takes precedence
// RUN: ( %hermes -dump-ir -Werror -Wno-error %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,WARN-EVAL
// RUN: ( ! %hermes -dump-ir -Wno-error -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,ERROR-EVAL
// RUN: ( ! %hermes -dump-ir -Wno-error -Werror=undefined-variable %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,WARN-EVAL
// RUN: ( %hermes -dump-ir -Werror=undefined-variable -Wno-error %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=WARN-UNDEFINED,WARN-EVAL
// RUN: ( ! %hermes -dump-ir -Werror -Wno-error=undefined-variable -Werror=undefined-variable %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,ERROR-EVAL
// RUN: ( ! %hermes -dump-ir -Wno-error=undefined-variable -Wno-error=direct-eval -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines \
// RUN:     --check-prefixes=ERROR-UNDEFINED,ERROR-EVAL

"use strict";
print(missing_global);
//ERROR-UNDEFINED: {{.*}}/warning-as-error-selective.js:{{.*}}: error: the variable "missing_global" was not declared in function "global"
//WARN-UNDEFINED: {{.*}}/warning-as-error-selective.js:{{.*}}: warning: the variable "missing_global" was not declared in function "global"

eval("1 + 1");
//ERROR-EVAL: {{.*}}warning-as-error-selective.js:{{.*}}: error: Direct call to eval(), but lexical scope is not supported.
//WARN-EVAL: {{.*}}warning-as-error-selective.js:{{.*}}: warning: Direct call to eval(), but lexical scope is not supported.
