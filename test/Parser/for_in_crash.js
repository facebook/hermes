// RUN: (! %hermes %s 2>&1 ) | %FileCheck %s

//CHECK: error: invalid expression
function foo() {
  for (var x in );
}
