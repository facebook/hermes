/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s

// class Hello
function Hello(text) {
    this.hello = text;
}

Hello.prototype.getName = function () {
    return "Hermes";
}
Hello.prototype.sayIt = function() {
    print(this.hello + ", " + this.getName() + "!");
}

// class Greeting extends Hello
function Greeting(name) {
    this.name = name;
    return 10;
}
Greeting.prototype = new Hello("Hello");
Greeting.prototype.getName = function () {
    return this.name;
}

// main
var gr1 = new Hello("Good day");
gr1.sayIt();
//CHECK: Good day, Hermes!
var gr2 = new Greeting("world");
gr2.sayIt();
//CHECK-NEXT: Hello, world!

// Test returning an object from the constructor

var singleton = new Object();
function Singleton() {
    return singleton;
}

print("singleton", new Singleton() === singleton);
//CHECK-NEXT: singleton true
print("singleton", new Singleton() === singleton);
//CHECK-NEXT: singleton true

try {
  new (5);
} catch (e) {
  print(e);
//CHECK-NEXT: TypeError: 5 cannot be used as a constructor.
}

try {
  new ({});
} catch (e) {
  print(e);
//CHECK-NEXT: TypeError: Object cannot be used as a constructor.
}

(function () {
  function foo() {}
  print(Object.getPrototypeOf(new foo()).constructor === foo);
//CHECK-NEXT: true
  print(Object.getPrototypeOf(new Array()).constructor === Array);
//CHECK-NEXT: true
  let bound = Array.bind();
  print(Object.getPrototypeOf(new bound()).constructor === Array);
//CHECK-NEXT: true
})();
