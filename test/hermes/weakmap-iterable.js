// RUN: %hermes -O -Xes6-symbol %s | %FileCheck --match-full-lines %s

var iterable = {};
iterable[Symbol.iterator] = function() {
  return {
    next: function() {
      return {value: [{}, 2], done: false};
    },
    return: function() {
      print('returning');
    },
  };
};
var oldSet = WeakMap.prototype.set;
WeakMap.prototype.set = function() {
  throw new Error('add error');
}
try { new WeakMap(iterable); } catch (e) { print('caught', e.message); }
// CHECK: returning
// CHECK: caught add error
WeakMap.prototype.set = oldSet;
