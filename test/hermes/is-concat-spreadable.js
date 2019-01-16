// RUN: %hermes -Xes6-symbol -O %s | %FileCheck --match-full-lines %s

print('isConcatSpreadable');
// CHECK-LABEL: isConcatSpreadable

obj = {
  0: 'a',
  1: 'b',
  2: 'c',
  length: 3,
};
print(['first'].concat(obj));
// CHECK-NEXT: first,[object Object]
obj[Symbol.isConcatSpreadable] = true;
print(['first'].concat(obj));
// CHECK-NEXT: first,a,b,c

arr = [3,4,5]
arr[Symbol.isConcatSpreadable] = false;
print([1,2].concat(arr).length);
// CHECK-NEXT: 3
