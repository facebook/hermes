// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

"use strict";
if (typeof Math === "undefined") var Math = 10;
print(Math);
// CHECK: [object Math]
