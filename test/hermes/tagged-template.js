/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

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

print(String.raw`hello${1} world${2}\n${3}`);
//CHECK: hello1 world2\n3
print(String.raw`hello
world`);
//CHECK: hello
//CHECK: world
var animal = "dog";
print(String.raw`hello ${animal}!`);
//CHECK: hello dog!
print(String.raw`Hi\u000A there!`);
//CHECK: Hi\u000A there!
print(String.raw`\u548C${1 + 1}\u5E73`);
//CHECK: \u548C2\u5E73
