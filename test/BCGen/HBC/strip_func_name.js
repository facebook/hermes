// RUN: %hermes -strict -target=HBC -dump-bytecode -fstrip-function-names -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:Global String Table:
//CHECK-NEXT: s0[ASCII, {{[0-9]+\.\.[0-9]+}}]: Done
//CHECK-NEXT: s1[ASCII, {{[0-9]+\.\.[0-9]+}}]: abc
//CHECK-NEXT: i2[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: length
//CHECK-NEXT: i3[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: print
//CHECK-NEXT: i4[ASCII, {{[0-9]+\.\.[0-9]+}}] #{{[0-9A-Z]+}}: substring
//CHECK-NEXT: s5[ASCII, {{[0-9]+\.\.[0-9]+}}]: function-name-stripped

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}global{{.*}}
(function() {

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}entryPoint{{.*}}
function entryPoint() {
  helper();
}

//CHECK-LABEL:Function<function-name-stripped>{{.*}}:
//CHECK-NOT:{{.*}}helper{{.*}}
function helper() {
  var s = "abc";
  var x = 1;
  var y;
  var z = false;
  for (var i = 0; i < 1000000; ++i) {
    x = s.length;
    y = s[i % 3];
    z = s.substring(0, 1);
  }
}
entryPoint();
print("Done");
})();
