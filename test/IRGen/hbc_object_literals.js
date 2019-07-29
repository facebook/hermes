// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes --target=HBC -dump-lir -O %s | %FileCheck %s --match-full-lines --check-prefix=IRGEN
// RUN: %hermes --target=HBC -dump-bytecode -O %s | %FileCheck %s --match-full-lines --check-prefix=BCGEN

//IRGEN-LABEL:function global() : undefined
//IRGEN-NEXT:frame = [], globals = [obj1, obj2, obj3, obj4]
//IRGEN-NEXT:%BB0:

var obj1 = {'a': 'hello', 'b': 1, 'c': null, 'd': undefined, 'e': true, 'f': function() {}, 'g': 2};
//IRGEN-NEXT:  %0 = HBCLoadConstInst 1 : number
//IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst 7 : number, "a" : string, "hello" : string, "b" : string, 1 : number, "c" : string, null : null, "d" : string, null : null, "e" : string, true : boolean, "f" : string, null : null, "g" : string, 2 : number
//IRGEN-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//IRGEN-NEXT:  %3 = StorePropertyInst %2 : undefined, %1 : object, "d" : string
//IRGEN-NEXT:  %4 = HBCCreateEnvironmentInst
//IRGEN-NEXT:  %5 = HBCCreateFunctionInst %f() : undefined, %4
//IRGEN-NEXT:  %6 = StorePropertyInst %5 : closure, %1 : object, "f" : string
//IRGEN-NEXT:  %7 = HBCGetGlobalObjectInst
//IRGEN-NEXT:  %8 = StorePropertyInst %1 : object, %7 : object, "obj1" : string

// Test the case when even we will save bytecode size if we serialize the object
// into the buffer, we choose not to, because it will add too many placeholders.
var obj2 = {
  a : undefined,
  b : undefined,
  c : undefined,
  d : undefined,
  e : undefined,
  r : undefined,
  f : 1,
  g : 1,
  h : 1,
  i : 1,
  j : 1,
  k : 1,
  l : 1,
  m : 1,
  n : 1,
  o : 1,
  p : 1,
  q : 1,
};
//IRGEN-NEXT:  %9 = AllocObjectInst 18 : number, empty
//IRGEN-NEXT:  %10 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "a" : string, true : boolean
//IRGEN-NEXT:  %11 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "b" : string, true : boolean
//IRGEN-NEXT:  %12 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "c" : string, true : boolean
//IRGEN-NEXT:  %13 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "d" : string, true : boolean
//IRGEN-NEXT:  %14 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "e" : string, true : boolean
//IRGEN-NEXT:  %15 = StoreNewOwnPropertyInst %2 : undefined, %9 : object, "r" : string, true : boolean
//IRGEN-NEXT:  %16 = StoreNewOwnPropertyInst %0 : number, %9 : object, "f" : string, true : boolean
//IRGEN-NEXT:  %17 = StoreNewOwnPropertyInst %0 : number, %9 : object, "g" : string, true : boolean
//IRGEN-NEXT:  %18 = StoreNewOwnPropertyInst %0 : number, %9 : object, "h" : string, true : boolean
//IRGEN-NEXT:  %19 = StoreNewOwnPropertyInst %0 : number, %9 : object, "i" : string, true : boolean
//IRGEN-NEXT:  %20 = StoreNewOwnPropertyInst %0 : number, %9 : object, "j" : string, true : boolean
//IRGEN-NEXT:  %21 = StoreNewOwnPropertyInst %0 : number, %9 : object, "k" : string, true : boolean
//IRGEN-NEXT:  %22 = StoreNewOwnPropertyInst %0 : number, %9 : object, "l" : string, true : boolean
//IRGEN-NEXT:  %23 = StoreNewOwnPropertyInst %0 : number, %9 : object, "m" : string, true : boolean
//IRGEN-NEXT:  %24 = StoreNewOwnPropertyInst %0 : number, %9 : object, "n" : string, true : boolean
//IRGEN-NEXT:  %25 = StoreNewOwnPropertyInst %0 : number, %9 : object, "o" : string, true : boolean
//IRGEN-NEXT:  %26 = StoreNewOwnPropertyInst %0 : number, %9 : object, "p" : string, true : boolean
//IRGEN-NEXT:  %27 = StoreNewOwnPropertyInst %0 : number, %9 : object, "q" : string, true : boolean
//IRGEN-NEXT:  %28 = StorePropertyInst %9 : object, %7 : object, "obj2" : string

// Cannot serialize undefined as placeholder when the property is a number.
var obj3 = {
  1 : undefined,
  f : 1,
  g : 1,
  h : 1,
  i : 1,
  j : 1,
  k : 1,
  l : 1,
  m : 1,
  n : 1,
  o : 1,
  p : 1,
  q : 1,
}
//IRGEN-NEXT:  %29 = HBCAllocObjectFromBufferInst 13 : number, "f" : string, 1 : number, "g" : string, 1 : number, "h" : string, 1 : number, "i" : string, 1 : number, "j" : string, 1 : number, "k" : string, 1 : number, "l" : string, 1 : number, "m" : string, 1 : number, "n" : string, 1 : number, "o" : string, 1 : number, "p" : string, 1 : number, "q" : string, 1 : number
//IRGEN-NEXT:  %30 = StoreOwnPropertyInst %2 : undefined, %29 : object, 1 : number, true : boolean
//IRGEN-NEXT:  %31 = StorePropertyInst %29 : object, %7 : object, "obj3" : string

var obj4 = {
  '1' : undefined,
  f : 1,
  g : 1,
  h : 1,
  i : 1,
  j : 1,
  k : 1,
  l : 1,
  m : 1,
  n : 1,
  o : 1,
  p : 1,
  q : 1,
}
//IRGEN-NEXT:  %32 = HBCAllocObjectFromBufferInst 13 : number, "f" : string, 1 : number, "g" : string, 1 : number, "h" : string, 1 : number, "i" : string, 1 : number, "j" : string, 1 : number, "k" : string, 1 : number, "l" : string, 1 : number, "m" : string, 1 : number, "n" : string, 1 : number, "o" : string, 1 : number, "p" : string, 1 : number, "q" : string, 1 : number
//IRGEN-NEXT:  %33 = StoreOwnPropertyInst %2 : undefined, %32 : object, 1 : number, true : boolean
//IRGEN-NEXT:  %34 = StorePropertyInst %32 : object, %7 : object, "obj4" : string

//IRGEN-NEXT:  %35 = ReturnInst %2 : undefined
//IRGEN-NEXT:function_end

//BCGEN-LABEL:Global String Table:
//BCGEN:  s1[ASCII, {{.*}}]: hello
//BCGEN:  i2[ASCII, {{.*}}] #{{.*}}: g
//BCGEN:  i3[ASCII, {{.*}}] #{{.*}}: a
//BCGEN:  i9[ASCII, {{.*}}] #{{.*}}: b
//BCGEN:  i10[ASCII, {{.*}}] #{{.*}}: c
//BCGEN:  i11[ASCII, {{.*}}] #{{.*}}: d
//BCGEN:  i12[ASCII, {{.*}}] #{{.*}}: e
//BCGEN:  i13[ASCII, {{.*}}] #{{.*}}: f

//BCGEN-LABEL:Object Key Buffer:
//BCGEN-NEXT:[String 3]
//BCGEN-NEXT:[String 9]
//BCGEN-NEXT:[String 10]
//BCGEN-NEXT:[String 11]
//BCGEN-NEXT:[String 12]
//BCGEN-NEXT:[String 13]
//BCGEN-NEXT:[String 2]
//BCGEN-NEXT:[String 13]
//BCGEN-NEXT:[String 2]
//BCGEN-NEXT:[String 5]
//BCGEN-NEXT:[String 14]
//BCGEN-NEXT:[String 8]
//BCGEN-NEXT:[String 15]
//BCGEN-NEXT:[String 4]
//BCGEN-NEXT:[String 16]
//BCGEN-NEXT:[String 17]
//BCGEN-NEXT:[String 6]
//BCGEN-NEXT:[String 21]
//BCGEN-NEXT:[String 22]
//BCGEN-LABEL:Object Value Buffer:
//BCGEN-LABEL:[String 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:null
//BCGEN-LABEL:null
//BCGEN-LABEL:true
//BCGEN-LABEL:null
//BCGEN-LABEL:[int 2]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
//BCGEN-LABEL:[int 1]
