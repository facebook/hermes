// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -target=HBC -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

var x = 0
if (typeof arguments == 'undefined') {
	x = 1;
}

print(x)
//CHECK-LABEL: 0
