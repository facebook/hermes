// RUN: ( ! %hermes -dump-ir -Werror %s 2>&1 ) | %FileCheck %s --match-full-lines

"use strict";
print(missing_global);

//CHECK: {{.*}}/warning-as-error.js:4:7: error: the variable "missing_global" was not declared in function "global"
