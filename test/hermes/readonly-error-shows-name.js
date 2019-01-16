// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s
"use strict";

var a = {}
Object.defineProperty(a, 'mypropname', { writable: false });
try {
  a.mypropname = 42;
} catch(e) {
  print('caught', e.name, e.message);
}
// CHECK: caught TypeError Cannot assign to read-only property 'mypropname'
