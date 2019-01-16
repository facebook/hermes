// RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

function printCallee() {
  print(arguments.callee);
}
function printCaller() {
  print(arguments.caller);
}
function leak() {
  return arguments;
}

print('callee/caller');
// CHECK-LABEL: callee/caller
try { printCallee() } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
try { printCaller() } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
try { print(leak().callee); } catch(e) { print(e.name) }
// CHECK-NEXT: TypeError
