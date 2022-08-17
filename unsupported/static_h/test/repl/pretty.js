// Remove this once the REPL can handle block comments.
// @lint-ignore-every LICENSELINT

// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

// RUN: cat %s | %hermes -prompt="" -prompt2="" | %FileCheck --match-full-lines %s

"pretty printer"
// CHECK-LABEL: "pretty printer"
1
// CHECK-NEXT: 1
+0
// CHECK-NEXT: 0
-0
// CHECK-NEXT: -0
true
// CHECK-NEXT: true
undefined
// CHECK-NEXT: undefined
null
// CHECK-NEXT: null
"asdf"
// CHECK-NEXT: "asdf"
(function(){})
// CHECK-NEXT: function () { [bytecode] }
(function*(){})
// CHECK-NEXT: function *() { [bytecode] }
(async function(){})
// CHECK-NEXT: async function () { [bytecode] }

a = [1,2, function(){}, null]
// CHECK-NEXT: [ 1, 2, [Function], null, [length]: 4 ]
a.x = 4
// CHECK-NEXT: 4
a
// CHECK-NEXT: [ 1, 2, [Function], null, [length]: 4, x: 4 ]

a = [1]
// CHECK-NEXT: [ 1, [length]: 1 ]
a[4] = 2
// CHECK-NEXT: 2
a[1] = undefined
// CHECK-NEXT: undefined
a
// CHECK-NEXT: [ 1, undefined, 2 x <empty>, 2, [length]: 5 ]
Object.defineProperty(a, 1, {enumerable: false, value: 3})
// CHECK-NEXT: [ 1, [1]: 3, 2 x <empty>, 2, [length]: 5 ]

a = []
// CHECK-NEXT: [ [length]: 0 ]
a[2] = 1
// CHECK-NEXT: 1
a
// CHECK-NEXT: [ 2 x <empty>, 1, [length]: 3 ]
a.length = 5;
// CHECK-NEXT: 5
a
// CHECK-NEXT: [ 2 x <empty>, 1, 2 x <empty>, [length]: 5 ]

a = {x:1, y:2}
// CHECK-NEXT: { x: 1, y: 2 }
a = {x:1}
// CHECK-NEXT: { x: 1 }
Object.defineProperty(a, 'z', {value: 3, enumerable: false});
// CHECK-NEXT: { x: 1, [z]: 3 }
a.arr = ['foo', 4];
// CHECK-NEXT: [ "foo", 4, [length]: 2 ]
a.arr.cycle = a;
// CHECK-NEXT: { x: 1, [z]: 3, arr: [ "foo", 4, [length]: 2, cycle: [cyclic] ] }

a = function foo() {}
// CHECK-NEXT: function foo() { [bytecode] }
a = [function foo() {}]
// CHECK-NEXT: [ [Function foo], [length]: 1 ]

a = /asdf/
// CHECK-NEXT: /asdf/
a = /.*/
// CHECK-NEXT: /.*/
a = new RegExp()
// CHECK-NEXT: /(?:)/

a = new Set()
// CHECK-NEXT: Set { }
a = new Set([1, 2, 3, 'asdf'])
// CHECK-NEXT: Set { 1, 2, 3, "asdf" }
a = new Set([1, 2, 3, 3, 'asdf', 'asdf'])
// CHECK-NEXT: Set { 1, 2, 3, "asdf" }

a = new Map()
// CHECK-NEXT: Map { }
a = new Map([[1,2], [3, 'asdf']])
// CHECK-NEXT: Map { 1 => 2, 3 => "asdf" }
a = new Map([[1,2], [3, 4], [3, 'asdf']])
// CHECK-NEXT: Map { 1 => 2, 3 => "asdf" }
a = new Map([[-0, -0]]);
// CHECK-NEXT: Map { 0 => -0 }

a = new Date(0)
// CHECK-NEXT: [Date 1970-01-01T00:00:00.000Z]
a = new Date('asdf')
// CHECK-NEXT: [Date Invalid]

a = Symbol()
// CHECK-NEXT: Symbol()
a = Symbol('asdf')
// CHECK-NEXT: Symbol(asdf)
obj = {}
// CHECK-NEXT: {  }
obj[Symbol()] = 3
// CHECK-NEXT: 3
obj
// CHECK-NEXT: { Symbol(): 3 }
obj = {}
// CHECK-NEXT: {  }
obj[Symbol('abc')] = 3
// CHECK-NEXT: 3
obj[Symbol('def')] = 4
// CHECK-NEXT: 4
obj[Symbol('xyz')] = 5
// CHECK-NEXT: 5
obj
// CHECK-NEXT: { Symbol(abc): 3, Symbol(def): 4, Symbol(xyz): 5 }

obj = Object.create(null)
// CHECK-NEXT: {  }

a = {}
// CHECK-NEXT: { }
Object.defineProperty(a, 'x', {
  enumerable: true,
  get: function(){ print('in getter'); return 1; }
});
// CHECK-NEXT: { x: [accessor] }
Object.defineProperty(a, 'y', {
  enumerable: true,
  set: function(){ print('in setter'); }
});
// CHECK-NEXT: { x: [accessor], y: [accessor] }

a = new Int8Array([1,2,3]);
// CHECK-NEXT: Int8Array [ 1, 2, 3 ]
a = new Float64Array([]);
// CHECK-NEXT: Float64Array [  ]
a = new ArrayBuffer();
// CHECK-NEXT: ArrayBuffer {  }

g = (function*(){})();
// CHECK-NEXT: Generator {  }

it = new Map().entries()
// CHECK-NEXT: Map Iterator {  }
it = new Set().entries()
// CHECK-NEXT: Set Iterator {  }
it =  [][Symbol.iterator]();
// CHECK-NEXT: Array Iterator {  }
it = 'abc'[Symbol.iterator]();
// CHECK-NEXT: String Iterator {  }
it = 'abc'.matchAll(/1/g)
// CHECK-NEXT: RegExp String Iterator {  }

Math
// CHECK-NEXT: Math {{{.*}}}
JSON
// CHECK-NEXT: JSON {{{.*}}}
