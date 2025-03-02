/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -O %s | %FileCheck --match-full-lines %s
// RUN: %hermes -target=HBC -O -emit-binary -out %t.hbc %s && %hermes %t.hbc | %FileCheck --match-full-lines %s
// RUN: %shermes -exec %s | %FileCheck --match-full-lines %s
"use strict";

// Performs a nested array comparison between the two arrays.
function arrayEquals(a, b) {
  if (a.length !== b.length) {
    return false;
  }
  for (var i = 0; i < a.length; ++i) {
    if (Array.isArray(a[i]) && Array.isArray(b[i])) {
      if (!arrayEquals(a[i], b[i])) {
        return false;
      }
    } else {
      if (a[i] !== b[i]) {
        return false;
      }
    }
  }
  return true;
}

function pr(msg, arr, end) {
    function fmtArray(arr) {
        var res = "[";
        for(var i = 0; i < arr.length; ++i) {
            if (i)
                res += ", ";
            if (i in arr)
                res += arr[i];
            else
                res += "<empty>";
        }
        return res + "]";
    }
    print(msg, fmtArray(arr), end);
}
function arrayClone(a) {
    var res = [];
    res.length = a.length;
    for(var i = 0; i < a.length; ++i)
        if (a.hasOwnProperty(i))
            res[i] = a[i];
    return res;
}
function toWeirdArray(a) {
    a = arrayClone(a);
    a.__proto__ = {__proto__: Array.prototype};
    return a;
}
function toBadArray(a) {
    a = arrayClone(a);

    let prototype = {__proto__: Array.prototype};

    let key = 0;
    for(let i = 1; i < a.length; ++i) {
        if (a.hasOwnProperty(i)) {
            key = i;
            break;
        }
    }
    if (!key)
        return a;

    let elem = a[key];
    delete a[key];
    prototype[key] = elem;
    a.__proto__ = prototype;
    return a;
}
function toObj(a) {
    return {...a, length: a.length};
}

print('reverse');
// CHECK: reverse

function testReverse(arr) {
    pr("input:", arr, "end");
    pr("array:", arrayClone(arr).reverse(), "end");
    pr("obj:", Array.prototype.reverse.call(toObj(arr)));
    pr("weird array:", toWeirdArray(arr).reverse(), "end");
    pr("bad array:", toBadArray(arr).reverse(), "end");
}

testReverse([1,2,3]);
// CHECK-NEXT: input: [1, 2, 3] end
// CHECK-NEXT: array: [3, 2, 1] end
// CHECK-NEXT: obj: [3, 2, 1] undefined
// CHECK-NEXT: weird array: [3, 2, 1] end
// CHECK-NEXT: bad array: [3, 2, 1] end

testReverse([1,2]);
// CHECK-NEXT: input: [1, 2] end
// CHECK-NEXT: array: [2, 1] end
// CHECK-NEXT: obj: [2, 1] undefined
// CHECK-NEXT: weird array: [2, 1] end
// CHECK-NEXT: bad array: [2, 1] end

testReverse([]);
// CHECK-NEXT: input: [] end
// CHECK-NEXT: array: [] end
// CHECK-NEXT: obj: [] undefined
// CHECK-NEXT: weird array: [] end
// CHECK-NEXT: bad array: [] end

testReverse([12,13,14,15,16,17,18]);
// CHECK-NEXT: input: [12, 13, 14, 15, 16, 17, 18] end
// CHECK-NEXT: array: [18, 17, 16, 15, 14, 13, 12] end
// CHECK-NEXT: obj: [18, 17, 16, 15, 14, 13, 12] undefined
// CHECK-NEXT: weird array: [18, 17, 16, 15, 14, 13, 12] end
// CHECK-NEXT: bad array: [18, 17, 16, 15, 14, 13, 12] end

testReverse([,,,1]);
// CHECK-NEXT: input: [<empty>, <empty>, <empty>, 1] end
// CHECK-NEXT: array: [1, <empty>, <empty>, <empty>] end
// CHECK-NEXT: obj: [1, <empty>, <empty>, <empty>] undefined
// CHECK-NEXT: weird array: [1, <empty>, <empty>, <empty>] end
// CHECK-NEXT: bad array: [1, <empty>, <empty>, 1] end

testReverse([,,1,,5,7,,]);
// CHECK-NEXT: input: [<empty>, <empty>, 1, <empty>, 5, 7, <empty>] end
// CHECK-NEXT: array: [<empty>, 7, 5, <empty>, 1, <empty>, <empty>] end
// CHECK-NEXT: obj: [<empty>, 7, 5, <empty>, 1, <empty>, <empty>] undefined
// CHECK-NEXT: weird array: [<empty>, 7, 5, <empty>, 1, <empty>, <empty>] end
// CHECK-NEXT: bad array: [<empty>, 7, 5, <empty>, 1, <empty>, <empty>] end

var a = {};
a[0] = 'a';
a[1] = 'b';
a[5] = 'c';
a.length = 6;
Array.prototype.reverse.call(a);
print(a[0], a[4], a[5]);
// CHECK-NEXT: c b a
var a = [0, 1, 2, 3];
Object.defineProperty(a, 3, {
  get: function() {
    delete a[1];
    return -1;
  },
  set: function() { print('setter'); }
});
a.reverse();
print(a);
// CHECK-NEXT: setter
// CHECK-NEXT: -1,2,,-1
var a = [0, 1];
Object.defineProperty(a, 0, {
  get: function() {
    a.pop();
    return -1;
  }
});
a.reverse();
print(a);
// CHECK-NEXT: ,-1
var a = [];
Object.defineProperties(a, {
  '0': {
    get: function() {
      print('getter 0');
      return a.val_0;
    },
    set: function(v) { a.val_0 = v; }
  },
  '1': {
    get: function() {
      print('getter 1');
      return a.val_1;
    },
    set: function(v) { a.val_1 = v; }
  },
});
a[0] = 0;
a[1] = 1;
a.reverse();
print(a);
// CHECK-NEXT: getter 0
// CHECK-NEXT: getter 1
// CHECK-NEXT: getter 0
// CHECK-NEXT: getter 1
// CHECK-NEXT: 1,0
var a = [0, 1, 2, 3, 4, 5, 6];
Object.defineProperties(a, {
  '0': {
    get: function() {
      a.pop();
      return -1;
    }
  },
  '1': {
    set: function() { a.push('a'); }
  },
  '2': {
    get: function() {
      a.push('b');
      return -2;
    },
    set: function() { a.pop(); }
  }
});
a.reverse();
print(a);
// CHECK-NEXT: ,,-2,3,-2,,-1,a

print('shift');
// CHECK-LABEL: shift
var a = [1,2,3];
print(a.shift(), a, a.length);
// CHECK-NEXT: 1 2,3 2
print(a.shift(), a, a.length);
// CHECK-NEXT: 2 3 1
print(a.shift(), a, a.length);
// CHECK-NEXT: 3  0
print(a.shift(), a, a.length);
// CHECK-NEXT: undefined  0
print(a.shift(), a, a.length);
// CHECK-NEXT: undefined  0

print('toReversed');
// CHECK-LABEL: toReversed

print(Array.prototype.toReversed.length);
// CHECK-NEXT: 0
var a = [1,2,3,4];
print(a.toReversed().toString())
// CHECK-NEXT: 4,3,2,1
print(a.toString())
// CHECK-NEXT: 1,2,3,4
print(arrayEquals([ 1, 2, 3 ].toReversed(), [ 3, 2, 1 ]));
// CHECK-NEXT: true
print(Array.prototype.toReversed.call({length : 3, 0 : 'a', 1 : 'b', 2 : 'c'})
          .toString())
// CHECK-NEXT: c,b,a

print('toReversed');
// CHECK-LABEL: toReversed

function testToReversed(arr) {
    pr("input:", arr, "end");
    pr("array:", arr.toReversed(), "end");
    pr("obj:", Array.prototype.toReversed.call(toObj(arr)));
    pr("weird array:", toWeirdArray(arr).toReversed(), "end");
    pr("bad array:", toBadArray(arr).toReversed(), "end");
}

print(Array.prototype.toReversed.length);
// CHECK-NEXT: 0

testToReversed(["a", "b", "c", "d"]);
// CHECK-NEXT: input: [a, b, c, d] end
// CHECK-NEXT: array: [d, c, b, a] end
// CHECK-NEXT: obj: [d, c, b, a] undefined
// CHECK-NEXT: weird array: [d, c, b, a] end
// CHECK-NEXT: bad array: [d, c, b, a] end

testToReversed(["a", "b",,,]);
// CHECK-NEXT: input: [a, b, <empty>, <empty>] end
// CHECK-NEXT: array: [undefined, undefined, b, a] end
// CHECK-NEXT: obj: [undefined, undefined, b, a] undefined
// CHECK-NEXT: weird array: [undefined, undefined, b, a] end
// CHECK-NEXT: bad array: [undefined, undefined, b, a] end
