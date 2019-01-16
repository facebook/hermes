  // RUN: %hermes --target=HBC -dump-ra -O %s | %FileCheck %s --match-full-lines --check-prefix=IRGEN
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
//IRGEN-NEXT:  {{.*}} %0 = HBCLoadConstInst 1 : number
//IRGEN-NEXT:  {{.*}} %1 = HBCAllocObjectFromBufferInst 7 : number, "a" : string, "hello" : string, "b" : string, 1 : number, "c" : string, null : null, "d" : string, null : null, "e" : string, true : boolean, "f" : string, null : null, "g" : string, 2 : number
//IRGEN-NEXT:  {{.*}} %2 = HBCLoadConstInst undefined : undefined
//IRGEN-NEXT:  {{.*}} %3 = StorePropertyInst %2 : undefined, %1 : object, "d" : string
//IRGEN-NEXT:  {{.*}} %4 = HBCCreateEnvironmentInst
//IRGEN-NEXT:  {{.*}} %5 = HBCCreateFunctionInst %f() : undefined, %4
//IRGEN-NEXT:  {{.*}} %6 = StorePropertyInst %5 : closure, %1 : object, "f" : string
//IRGEN-NEXT:  {{.*}} %7 = HBCGetGlobalObjectInst
//IRGEN-NEXT:  {{.*}} %8 = StorePropertyInst %1 : object, %7 : object, "obj1" : string
//IRGEN-NEXT:  {{.*}} %9 = AllocObjectInst 18 : number
//IRGEN-NEXT:  {{.*}} %10 = StoreOwnPropertyInst %2 : undefined, %9 : object, "a" : string
//IRGEN-NEXT:  {{.*}} %11 = StoreOwnPropertyInst %2 : undefined, %9 : object, "b" : string
//IRGEN-NEXT:  {{.*}} %12 = StoreOwnPropertyInst %2 : undefined, %9 : object, "c" : string
//IRGEN-NEXT:  {{.*}} %13 = StoreOwnPropertyInst %2 : undefined, %9 : object, "d" : string
//IRGEN-NEXT:  {{.*}} %14 = StoreOwnPropertyInst %2 : undefined, %9 : object, "e" : string
//IRGEN-NEXT:  {{.*}} %15 = StoreOwnPropertyInst %2 : undefined, %9 : object, "r" : string
//IRGEN-NEXT:  {{.*}} %16 = StoreOwnPropertyInst %0 : number, %9 : object, "f" : string
//IRGEN-NEXT:  {{.*}} %17 = StoreOwnPropertyInst %0 : number, %9 : object, "g" : string
//IRGEN-NEXT:  {{.*}} %18 = StoreOwnPropertyInst %0 : number, %9 : object, "h" : string
//IRGEN-NEXT:  {{.*}} %19 = StoreOwnPropertyInst %0 : number, %9 : object, "i" : string
//IRGEN-NEXT:  {{.*}} %20 = StoreOwnPropertyInst %0 : number, %9 : object, "j" : string
//IRGEN-NEXT:  {{.*}} %21 = StoreOwnPropertyInst %0 : number, %9 : object, "k" : string
//IRGEN-NEXT:  {{.*}} %22 = StoreOwnPropertyInst %0 : number, %9 : object, "l" : string
//IRGEN-NEXT:  {{.*}} %23 = StoreOwnPropertyInst %0 : number, %9 : object, "m" : string
//IRGEN-NEXT:  {{.*}} %24 = StoreOwnPropertyInst %0 : number, %9 : object, "n" : string
//IRGEN-NEXT:  {{.*}} %25 = StoreOwnPropertyInst %0 : number, %9 : object, "o" : string
//IRGEN-NEXT:  {{.*}} %26 = StoreOwnPropertyInst %0 : number, %9 : object, "p" : string
//IRGEN-NEXT:  {{.*}} %27 = StoreOwnPropertyInst %0 : number, %9 : object, "q" : string
//IRGEN-NEXT:  {{.*}} %28 = StorePropertyInst %9 : object, %7 : object, "obj2" : string
//IRGEN-NEXT:  {{.*}} %29 = AllocObjectInst 13 : number
//IRGEN-NEXT:  {{.*}} %30 = StoreOwnPropertyInst %2 : undefined, %29 : object, 1 : number
//IRGEN-NEXT:  {{.*}} %31 = StoreOwnPropertyInst %0 : number, %29 : object, "f" : string
//IRGEN-NEXT:  {{.*}} %32 = StoreOwnPropertyInst %0 : number, %29 : object, "g" : string
//IRGEN-NEXT:  {{.*}} %33 = StoreOwnPropertyInst %0 : number, %29 : object, "h" : string
//IRGEN-NEXT:  {{.*}} %34 = StoreOwnPropertyInst %0 : number, %29 : object, "i" : string
//IRGEN-NEXT:  {{.*}} %35 = StoreOwnPropertyInst %0 : number, %29 : object, "j" : string
//IRGEN-NEXT:  {{.*}} %36 = StoreOwnPropertyInst %0 : number, %29 : object, "k" : string
//IRGEN-NEXT:  {{.*}} %37 = StoreOwnPropertyInst %0 : number, %29 : object, "l" : string
//IRGEN-NEXT:  {{.*}} %38 = StoreOwnPropertyInst %0 : number, %29 : object, "m" : string
//IRGEN-NEXT:  {{.*}} %39 = StoreOwnPropertyInst %0 : number, %29 : object, "n" : string
//IRGEN-NEXT:  {{.*}} %40 = StoreOwnPropertyInst %0 : number, %29 : object, "o" : string
//IRGEN-NEXT:  {{.*}} %41 = StoreOwnPropertyInst %0 : number, %29 : object, "p" : string
//IRGEN-NEXT:  {{.*}} %42 = StoreOwnPropertyInst %0 : number, %29 : object, "q" : string
//IRGEN-NEXT:  {{.*}} %43 = StorePropertyInst %29 : object, %7 : object, "obj3" : string
//IRGEN-NEXT:  {{.*}} %44 = AllocObjectInst 13 : number
//IRGEN-NEXT:  {{.*}} %45 = StoreOwnPropertyInst %2 : undefined, %44 : object, 1 : number
//IRGEN-NEXT:  {{.*}} %46 = StoreOwnPropertyInst %0 : number, %44 : object, "f" : string
//IRGEN-NEXT:  {{.*}} %47 = StoreOwnPropertyInst %0 : number, %44 : object, "g" : string
//IRGEN-NEXT:  {{.*}} %48 = StoreOwnPropertyInst %0 : number, %44 : object, "h" : string
//IRGEN-NEXT:  {{.*}} %49 = StoreOwnPropertyInst %0 : number, %44 : object, "i" : string
//IRGEN-NEXT:  {{.*}} %50 = StoreOwnPropertyInst %0 : number, %44 : object, "j" : string
//IRGEN-NEXT:  {{.*}} %51 = StoreOwnPropertyInst %0 : number, %44 : object, "k" : string
//IRGEN-NEXT:  {{.*}} %52 = StoreOwnPropertyInst %0 : number, %44 : object, "l" : string
//IRGEN-NEXT:  {{.*}} %53 = StoreOwnPropertyInst %0 : number, %44 : object, "m" : string
//IRGEN-NEXT:  {{.*}} %54 = StoreOwnPropertyInst %0 : number, %44 : object, "n" : string
//IRGEN-NEXT:  {{.*}} %55 = StoreOwnPropertyInst %0 : number, %44 : object, "o" : string
//IRGEN-NEXT:  {{.*}} %56 = StoreOwnPropertyInst %0 : number, %44 : object, "p" : string
//IRGEN-NEXT:  {{.*}} %57 = StoreOwnPropertyInst %0 : number, %44 : object, "q" : string
//IRGEN-NEXT:  {{.*}} %58 = StorePropertyInst %44 : object, %7 : object, "obj4" : string
//IRGEN-NEXT:  {{.*}} %59 = ReturnInst %2 : undefined
//IRGEN-NEXT:function_end

//BCGEN-LABEL:Object Key Buffer:
//BCGEN-NEXT:[String 3]
//BCGEN-NEXT:[String 17]
//BCGEN-NEXT:[String 18]
//BCGEN-NEXT:[String 5]
//BCGEN-NEXT:[String 19]
//BCGEN-NEXT:[String 0]
//BCGEN-NEXT:[String 1]
//BCGEN-NEXT:Object Value Buffer:
//BCGEN-NEXT:[String 7]
//BCGEN-NEXT:[int 1]
//BCGEN-NEXT:null
//BCGEN-NEXT:null
//BCGEN-NEXT:true
//BCGEN-NEXT:null
//BCGEN-NEXT:[int 2]
