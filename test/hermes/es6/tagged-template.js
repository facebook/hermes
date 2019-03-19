// RUN: %hermes -Xflow-parser -O %s | %FileCheck --match-full-lines %s

// REQUIRES: flowparser

function checkArgs() {
  print(arguments.length);
  // check template object
  print(Object.isFrozen(arguments[0]), JSON.stringify(arguments[0]), arguments[0].length);
  // check raw object
  print(Object.isFrozen(arguments[0].raw), JSON.stringify(arguments[0].raw), arguments[0].raw.length);
  // print substitutions
  print(JSON.stringify(Array.prototype.slice.call(arguments, 1)));
}

checkArgs``;
//CHECK: 1
//CHECK: true [""] 1
//CHECK: true [""] 1
//CHECK: []
checkArgs`${111}hello${222}`;
//CHECK: 3
//CHECK: true ["","hello",""] 3
//CHECK: true ["","hello",""] 3
//CHECK: [111,222]
checkArgs`${111}hello\n${222}`;
//CHECK: 3
//CHECK: true ["","hello\n",""] 3
//CHECK: true ["","hello\\n",""] 3
//CHECK: [111,222]
checkArgs`hello ${666} world!`;
//CHECK: 2
//CHECK: true ["hello "," world!"] 2
//CHECK: true ["hello "," world!"] 2
//CHECK: [666]
(function () {
    return checkArgs;
})()`hello ${666} world!`;
//CHECK: 2
//CHECK: true ["hello "," world!"] 2
//CHECK: true ["hello "," world!"] 2
//CHECK: [666]
var obj1 = {func: checkArgs};
obj1.func`hello${`world${666}!`}!${888}`;
//CHECK: 3
//CHECK: true ["hello","!",""] 3
//CHECK: true ["hello","!",""] 3
//CHECK: ["world666!",888]
