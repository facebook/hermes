// RUN: %hermes -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s

"use strict";

print('errors');
// CHECK-LABEL: errors

try { undefined() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError undefined is not a function
try { null() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError null is not a function
try { 3.14() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError 3.14 is not a function
try { true() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError true is not a function
try { false() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError false is not a function
try { Object()() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError Object is not a function
try { "asdf"() } catch(e) { print(e.name, e.message) }
// CHECK-NEXT: TypeError "asdf" is not a function
