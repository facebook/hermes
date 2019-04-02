// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
"use strict";

function sink(x) {}

print('test');
// CHECK-LABEL: test
sink('a');
// Make empty string the final entry in the string storage.
sink('');
