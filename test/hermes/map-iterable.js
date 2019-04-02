// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

var iterable = {};
iterable[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: [1, 2], done: false};
    },
    return: function() {
      print('returning');
    },
  };
};
var oldSet = Map.prototype.set;
Map.prototype.set = function() {
  throw new Error('add error');
}
try { new Map(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
Map.prototype.set = oldSet;
