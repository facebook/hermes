// RUN: (echo "START" && %hermes -dump-ir -w %s 2>&1 >/dev/null && echo "END" ) | %FileCheck %s --match-full-lines

// CHECK: START
"use strict";
print(missing_global);

// CHECK-NEXT: END
