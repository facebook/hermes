/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC -fno-inline %s | %FileCheck --match-full-lines %s

function foo() {
  function bar() {
    var anonVar = function() {
      throw new Error("helloworld");
    }
    anonVar();
  }
  bar();
}

try {
  try {
    foo();
  } finally {
    print("rethrowing");
  }
} catch (e) {
  print(e.stack);
}
//CHECK-LABEL: Error: helloworld
//CHECK-NEXT:     at anonVar ({{.*}}stacktrace.js:13:22)
//CHECK-NEXT:     at bar ({{.*}}stacktrace.js:15:12)
//CHECK-NEXT:     at foo ({{.*}}stacktrace.js:17:6)
//CHECK-NEXT:     at global ({{.*}}stacktrace.js:22:8)

try {
  Object.a();
} catch (e) {
  print(e.stack);
}
//CHECK-LABEL: TypeError: undefined is not a function
//CHECK-NEXT:     at global ({{.*}}stacktrace.js:36:11)

function throwit() { throw new Error("EvalTest"); }
try {
  eval("throwit()");
} catch (e) {
  print(e.stack);
}
//CHECK-LABEL: Error: EvalTest
//CHECK-NEXT:    at throwit ({{.*}}stacktrace.js:43:37)
//CHECK-NEXT:    at eval (JavaScript:1:8)
//CHECK-NEXT:    at global ({{.*}}stacktrace.js:45:7)

function runAndPrint(f) {
  try {
    f();
  } catch (e) {
    print(e.stack);
  }
}

// Try some native scenarios.
print("Native Scenarios");
function thrower(x) { throw new Error(x); }
runAndPrint(function() {
  [1, 2, 3].forEach(thrower);
})
//CHECK-LABEL: Native Scenarios
//CHECK-NEXT: Error: 1
//CHECK-NEXT:     at thrower ({{.*}})
//CHECK-NEXT:     at forEach (native)
//CHECK-NEXT:     at anonymous ({{.*}})

runAndPrint(function() {
  "amatchmec".replace("matchme", thrower);
})
//CHECK-LABEL: Error: matchme
//CHECK-NEXT:     at thrower ({{.*}})
//CHECK-NEXT:     at replace ({{.*}})
//CHECK-NEXT:     at anonymous ({{.*}})

runAndPrint(function() {
  Math.abs({valueOf: thrower});
})
//CHECK-LABEL: Error
//CHECK-NEXT:     at thrower ({{.*}})
//CHECK-NEXT:     at abs (native)
//CHECK-NEXT:     at anonymous ({{.*}})


// Ensure that the 'name' property is respected correctly.
var func = function original() {
  throw new Error(arguments.callee.name);
}
runAndPrint(func)
//CHECK-LABEL: Error: original
//CHECK-NEXT:    at original ({{.*}})

Object.defineProperty(func, 'name', {writable:true, value:'newname'})
print("Name is " + func.name)
runAndPrint(func)
//CHECK-LABEL: Name is newname
//CHECK-NEXT: Error: newname
//CHECK-NEXT:    at newname ({{.*}})

// Empty names are not reported.
Object.defineProperty(func, 'name', {writable:true, value:''})
runAndPrint(func)
//CHECK-LABEL: Error
//CHECK-NEXT:    at original ({{.*}})

// Only report names that are strings, not numbers or others.
Object.defineProperty(func, 'name', {writable:true, value:1234})
runAndPrint(func)
//CHECK-LABEL: Error: 1234
//CHECK-NEXT:    at original ({{.*}})

Object.defineProperty(func, 'name', {writable:true, value:undefined})
runAndPrint(func)
//CHECK-LABEL: Error
//CHECK-NEXT:    at original ({{.*}})

// Native functions can be renamed. Currently native functions with invalid
// names are reported as anonymous, similar to JS functions. What we would
// like to do is preserve the original name  and report that if the set name
// is invalid.
function cosFactory(name, errorText) {
  func = Math.cos;
  Object.defineProperty(func, 'name', {writable:true, value:name})
  obj = {valueOf: function() { throw new Error(errorText); } }
  return function cosWrapper() { func(obj); }
}

runAndPrint(cosFactory(123, "CosTest1"));
//CHECK-LABEL: Error: CosTest1
//CHECK-NEXT:    at valueOf ({{.*}})
//CHECK-NEXT:    at anonymous (native)
//CHECK-NEXT:    at cosWrapper ({{.*}})
//CHECK-NEXT:    at runAndPrint ({{.*}})
//CHECK-NEXT:    at global ({{.*}})

runAndPrint(cosFactory(null, "CosTest2"));
//CHECK-LABEL: Error: CosTest2
//CHECK-NEXT:    at valueOf ({{.*}})
//CHECK-NEXT:    at anonymous (native)
//CHECK-NEXT:    at cosWrapper ({{.*}})
//CHECK-NEXT:    at runAndPrint ({{.*}})
//CHECK-NEXT:    at global ({{.*}})

runAndPrint(cosFactory('', "CosTest3"));
//CHECK-LABEL: Error: CosTest3
//CHECK-NEXT:    at valueOf ({{.*}})
//CHECK-NEXT:    at anonymous (native)
//CHECK-NEXT:    at cosWrapper ({{.*}})
//CHECK-NEXT:    at runAndPrint ({{.*}})
//CHECK-NEXT:    at global ({{.*}})

runAndPrint(cosFactory('totallycosine', "CosTest4"));
//CHECK-LABEL: Error: CosTest4
//CHECK-NEXT:    at valueOf ({{.*}})
//CHECK-NEXT:    at totallycosine (native)
//CHECK-NEXT:    at cosWrapper ({{.*}})
//CHECK-NEXT:    at runAndPrint ({{.*}})
//CHECK-NEXT:    at global ({{.*}})

// Accessors should be ignored.
func = function original2() { throw new Error("original2"); }
Object.defineProperty(func, 'name',
   {get:function() { throw new Error("Don't call me!")} })
runAndPrint(func)
//CHECK-LABEL: Error: original2
//CHECK-NEXT:    at original2 ({{.*}})

function baz() {
  throw new Error("oops");
}
baz.displayName = "MyComponent";

try { baz(); } catch (e) { print(e.stack); }
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at MyComponent ({{.*}}stacktrace.js:175:18)

// Empty strings should be ignored.
function qux() {
  throw new Error("oops");
}
qux.displayName = "";

try { qux(); } catch (e) { print(e.stack); }
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at qux ({{.*}}stacktrace.js:185:18)

// Non-strings should be ignored.
function bop() {
  throw new Error("oops");
}
bop.displayName = 123;

try { bop(); } catch (e) { print(e.stack); }
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at bop ({{.*}}stacktrace.js:195:18)

try {
  Array.prototype.map = function() {
    throw new Error("oops");
  }
  Array.prototype.map.displayName = "map2";
  [].map();
} catch (e) {
  print(e.stack);
}
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at map2 ({{.*}}stacktrace.js:205:20)

let object = {
  someMethod: function() {
    throw new Error("oops");
  }
};

object.someMethod.displayName = 'anotherMethod';

try { object.someMethod(); } catch (e) { print(e.stack); }
//CHECK-LABEL: Error: oops
//CHECK-NEXT:     at anotherMethod ({{.*}}stacktrace.js:217:20)
