// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s

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
