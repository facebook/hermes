// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the LICENSE
// file in the root directory of this source tree.
//
// RUN: %hermes --target=HBC -dump-lir -O %s | %FileCheck %s --match-full-lines --check-prefix=IRGEN
// RUN: %hermes --target=HBC -dump-bytecode -O %s | %FileCheck %s --match-full-lines --check-prefix=BCGEN

var obj1 = {'a': 'hello', 'b': 1, 'c': null, 'd': undefined, 'e': true, 'f': function() {}, 'g': 2};

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

//IRGEN-LABEL:function global() : undefined
//IRGEN-NEXT:frame = [], globals = [obj1, obj2, obj3, obj4]
//IRGEN-NEXT:%BB0:
//IRGEN-NEXT:  %0 = HBCLoadConstInst 1 : number
//IRGEN-NEXT:  %1 = HBCAllocObjectFromBufferInst 7 : number, "a" : string, "hello" : string, "b" : string, 1 : number, "c" : string, null : null, "d" : string, null : null, "e" : string, true : boolean, "f" : string, null : null, "g" : string, 2 : number
//IRGEN-NEXT:  %2 = HBCLoadConstInst undefined : undefined
//IRGEN-NEXT:  %3 = StorePropertyInst %2 : undefined, %1 : object, "d" : string
//IRGEN-NEXT:  %4 = HBCCreateEnvironmentInst
//IRGEN-NEXT:  %5 = HBCCreateFunctionInst %f() : undefined, %4
//IRGEN-NEXT:  %6 = StorePropertyInst %5 : closure, %1 : object, "f" : string
//IRGEN-NEXT:  %7 = HBCGetGlobalObjectInst
//IRGEN-NEXT:  %8 = StorePropertyInst %1 : object, %7 : object, "obj1" : string
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
//IRGEN-NEXT:  %29 = AllocObjectInst 13 : number, empty
//IRGEN-NEXT:  %30 = StoreOwnPropertyInst %2 : undefined, %29 : object, 1 : number, true : boolean
//IRGEN-NEXT:  %31 = StoreNewOwnPropertyInst %0 : number, %29 : object, "f" : string, true : boolean
//IRGEN-NEXT:  %32 = StoreNewOwnPropertyInst %0 : number, %29 : object, "g" : string, true : boolean
//IRGEN-NEXT:  %33 = StoreNewOwnPropertyInst %0 : number, %29 : object, "h" : string, true : boolean
//IRGEN-NEXT:  %34 = StoreNewOwnPropertyInst %0 : number, %29 : object, "i" : string, true : boolean
//IRGEN-NEXT:  %35 = StoreNewOwnPropertyInst %0 : number, %29 : object, "j" : string, true : boolean
//IRGEN-NEXT:  %36 = StoreNewOwnPropertyInst %0 : number, %29 : object, "k" : string, true : boolean
//IRGEN-NEXT:  %37 = StoreNewOwnPropertyInst %0 : number, %29 : object, "l" : string, true : boolean
//IRGEN-NEXT:  %38 = StoreNewOwnPropertyInst %0 : number, %29 : object, "m" : string, true : boolean
//IRGEN-NEXT:  %39 = StoreNewOwnPropertyInst %0 : number, %29 : object, "n" : string, true : boolean
//IRGEN-NEXT:  %40 = StoreNewOwnPropertyInst %0 : number, %29 : object, "o" : string, true : boolean
//IRGEN-NEXT:  %41 = StoreNewOwnPropertyInst %0 : number, %29 : object, "p" : string, true : boolean
//IRGEN-NEXT:  %42 = StoreNewOwnPropertyInst %0 : number, %29 : object, "q" : string, true : boolean
//IRGEN-NEXT:  %43 = StorePropertyInst %29 : object, %7 : object, "obj3" : string
//IRGEN-NEXT:  %44 = AllocObjectInst 13 : number, empty
//IRGEN-NEXT:  %45 = StoreOwnPropertyInst %2 : undefined, %44 : object, 1 : number, true : boolean
//IRGEN-NEXT:  %46 = StoreNewOwnPropertyInst %0 : number, %44 : object, "f" : string, true : boolean
//IRGEN-NEXT:  %47 = StoreNewOwnPropertyInst %0 : number, %44 : object, "g" : string, true : boolean
//IRGEN-NEXT:  %48 = StoreNewOwnPropertyInst %0 : number, %44 : object, "h" : string, true : boolean
//IRGEN-NEXT:  %49 = StoreNewOwnPropertyInst %0 : number, %44 : object, "i" : string, true : boolean
//IRGEN-NEXT:  %50 = StoreNewOwnPropertyInst %0 : number, %44 : object, "j" : string, true : boolean
//IRGEN-NEXT:  %51 = StoreNewOwnPropertyInst %0 : number, %44 : object, "k" : string, true : boolean
//IRGEN-NEXT:  %52 = StoreNewOwnPropertyInst %0 : number, %44 : object, "l" : string, true : boolean
//IRGEN-NEXT:  %53 = StoreNewOwnPropertyInst %0 : number, %44 : object, "m" : string, true : boolean
//IRGEN-NEXT:  %54 = StoreNewOwnPropertyInst %0 : number, %44 : object, "n" : string, true : boolean
//IRGEN-NEXT:  %55 = StoreNewOwnPropertyInst %0 : number, %44 : object, "o" : string, true : boolean
//IRGEN-NEXT:  %56 = StoreNewOwnPropertyInst %0 : number, %44 : object, "p" : string, true : boolean
//IRGEN-NEXT:  %57 = StoreNewOwnPropertyInst %0 : number, %44 : object, "q" : string, true : boolean
//IRGEN-NEXT:  %58 = StorePropertyInst %44 : object, %7 : object, "obj4" : string
//IRGEN-NEXT:  %59 = ReturnInst %2 : undefined
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
//BCGEN-NEXT:Object Value Buffer:
//BCGEN-NEXT:[String 1]
//BCGEN-NEXT:[int 1]
//BCGEN-NEXT:null
//BCGEN-NEXT:null
//BCGEN-NEXT:true
//BCGEN-NEXT:null
//BCGEN-NEXT:[int 2]
