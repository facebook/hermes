// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

var x = 0
if (typeof arguments == 'undefined') {
	x = 1;
}

print(x)
//CHECK-LABEL: 0
