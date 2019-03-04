// RUN: %hermes -target=HBC -O -fno-static-builtins %s | %FileCheck --match-full-lines %s

var saveProto = Object.prototype;
var singleton = new Object();

Object = function Foo() {
    print("Foo");
    return singleton;
}

print("start of test");
//CHECK: start of test
var x = {};
print(x.toString());
//CHECK-NEXT: [object Object]
print(x !== singleton);
//CHECK-NEXT: true
print(x.__proto__ === saveProto);
//CHECK-NEXT: true
