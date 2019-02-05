// RUN: %hermes -O -Wno-direct-eval %s | %FileCheck --match-full-lines %s
"use strict";

var a = 'a';
for (var i = 0; i < 28; ++i) {
  a = a + a;
}

var evil = {toString: function() { return a; }};

try {
new Function("hi", evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil, evil);
  print("Unexpected success");
} catch (e) {
  print("caught", e.name);
}
// CHECK: caught RangeError
