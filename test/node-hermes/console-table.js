/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

// RUN: %node-hermes %s 2>&1 | %FileCheck --match-full-lines %s
// REQUIRES: node-hermes

print('Console table functionality');
// CHECK-LABEL: Console table functionality

console.table([{ a: 'X', b: 'Y' }, { a: 'Z', b: 2 }]);
// CHECK:      ┌─────────┬─────┬─────┐
// CHECK-NEXT: │ (index) │  a  │  b  │
// CHECK-NEXT: ├─────────┼─────┼─────┤
// CHECK-NEXT: │    0    │ 'X' │ 'Y' │
// CHECK-NEXT: │    1    │ 'Z' │  2  │
// CHECK-NEXT: └─────────┴─────┴─────┘

console.table(["apples", "oranges", "bananas"]);
// CHECK-NEXT: ┌─────────┬───────────┐
// CHECK-NEXT: │ (index) │  Values   │
// CHECK-NEXT: ├─────────┼───────────┤
// CHECK-NEXT: │    0    │ 'apples'  │
// CHECK-NEXT: │    1    │ 'oranges' │
// CHECK-NEXT: │    2    │ 'bananas' │
// CHECK-NEXT: └─────────┴───────────┘

// an object whose properties are strings
function Person(firstName, lastName) {
  this.firstName = firstName;
  this.lastName = lastName;
}
var me = new Person("John", "Smith");
console.table(me);
// CHECK-NEXT: ┌───────────┬─────────┐
// CHECK-NEXT: │  (index)  │ Values  │
// CHECK-NEXT: ├───────────┼─────────┤
// CHECK-NEXT: │ firstName │ 'John'  │
// CHECK-NEXT: │ lastName  │ 'Smith' │
// CHECK-NEXT: └───────────┴─────────┘

// an array of objects
function Person(firstName, lastName) {
  this.firstName = firstName;
  this.lastName = lastName;
}
var john = new Person("John", "Smith");
var jane = new Person("Jane", "Doe");
var emily = new Person("Emily", "Jones");
console.table([john, jane, emily]);
// CHECK-NEXT: ┌─────────┬───────────┬──────────┐
// CHECK-NEXT: │ (index) │ firstName │ lastName │
// CHECK-NEXT: ├─────────┼───────────┼──────────┤
// CHECK-NEXT: │    0    │  'John'   │ 'Smith'  │
// CHECK-NEXT: │    1    │  'Jane'   │  'Doe'   │
// CHECK-NEXT: │    2    │  'Emily'  │ 'Jones'  │
// CHECK-NEXT: └─────────┴───────────┴──────────┘

// an array of arrays
var people = [["John", "Smith"], ["Jane", "Doe"], ["Emily", "Jones"]]
console.table(people);
// CHECK-NEXT: ┌─────────┬─────────┬─────────┐
// CHECK-NEXT: │ (index) │    0    │    1    │
// CHECK-NEXT: ├─────────┼─────────┼─────────┤
// CHECK-NEXT: │    0    │ 'John'  │ 'Smith' │
// CHECK-NEXT: │    1    │ 'Jane'  │  'Doe'  │
// CHECK-NEXT: │    2    │ 'Emily' │ 'Jones' │
// CHECK-NEXT: └─────────┴─────────┴─────────┘

// an array of objects, logging only firstName
function Person(firstName, lastName) {
  this.firstName = firstName;
  this.lastName = lastName;
}
var john = new Person("John", "Smith");
var jane = new Person("Jane", "Doe");
var emily = new Person("Emily", "Jones");
console.table([john, jane, emily], ["firstName"]);
// CHECK-NEXT: ┌─────────┬───────────┐
// CHECK-NEXT: │ (index) │ firstName │
// CHECK-NEXT: ├─────────┼───────────┤
// CHECK-NEXT: │    0    │  'John'   │
// CHECK-NEXT: │    1    │  'Jane'   │
// CHECK-NEXT: │    2    │  'Emily'  │
// CHECK-NEXT: └─────────┴───────────┘


// All of the below functionalities are not supported currently.
var myMap = new Map();
myMap.set('0', 'foo');
myMap.set(1, 'bar');
myMap.set({}, 'baz');
console.table(myMap);
// CHECK-NEXT: This data type is currently not supported for the console table functionality of node-hermes
var mapIter = myMap[Symbol.iterator]();
console.table(mapIter);
// CHECK-NEXT: This data type is currently not supported for the console table functionality of node-hermes

const mySet = new Set();
mySet.add(1);
console.table(mySet);
// CHECK-NEXT: This data type is currently not supported for the console table functionality of node-hermes
var setIter = mySet[Symbol.iterator]();
console.table(setIter);
// CHECK-NEXT: This data type is currently not supported for the console table functionality of node-hermes
