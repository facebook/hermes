// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

debugger;
print("hello");

//CHECK: Break on 'debugger' statement in global: {{.*}}:4:1
//CHECK: hello
