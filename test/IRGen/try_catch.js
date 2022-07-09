/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheck %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O


//CHECK-LABEL:function simple_try_catch_test()
//CHECK-NEXT:frame = [i, ?anon_0_e]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %2 = CatchInst
//CHECK-NEXT:    %3 = StoreFrameInst %2, [?anon_0_e]
//CHECK-NEXT:    %4 = LoadFrameInst [?anon_0_e]
//CHECK-NEXT:    %5 = AsNumericInst %4
//CHECK-NEXT:    %6 = UnaryOperatorInst '++', %5 : number|bigint
//CHECK-NEXT:    %7 = StoreFrameInst %6, [?anon_0_e]
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '-', %8, 3 : number
//CHECK-NEXT:    %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:    %11 = BranchInst %BB3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %12 = LoadFrameInst [i]
//CHECK-NEXT:    %13 = ReturnInst %12
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %14 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %15 = BranchInst %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %16 = LoadFrameInst [i]
//CHECK-NEXT:    %17 = BinaryOperatorInst '+', %16, 2 : number
//CHECK-NEXT:    %18 = StoreFrameInst %17, [i]
//CHECK-NEXT:    %19 = BranchInst %BB6
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %20 = LoadFrameInst [i]
//CHECK-NEXT:    %21 = ThrowInst %20
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %22 = LoadFrameInst [i]
//CHECK-NEXT:    %23 = BinaryOperatorInst '<', %22, 10 : number
//CHECK-NEXT:    %24 = CondBranchInst %23, %BB5, %BB7
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %25 = LoadFrameInst [i]
//CHECK-NEXT:    %26 = BinaryOperatorInst '<', %25, 10 : number
//CHECK-NEXT:    %27 = CondBranchInst %26, %BB5, %BB7
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %28 = LoadFrameInst [i]
//CHECK-NEXT:    %29 = AsNumericInst %28
//CHECK-NEXT:    %30 = UnaryOperatorInst '++', %29 : number|bigint
//CHECK-NEXT:    %31 = StoreFrameInst %30, [i]
//CHECK-NEXT:    %32 = BranchInst %BB8
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %33 = LoadFrameInst [i]
//CHECK-NEXT:    %34 = AsNumericInst %33
//CHECK-NEXT:    %35 = UnaryOperatorInst '--', %34 : number|bigint
//CHECK-NEXT:    %36 = StoreFrameInst %35, [i]
//CHECK-NEXT:    %37 = BranchInst %BB10
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %38 = TryEndInst
//CHECK-NEXT:    %39 = BranchInst %BB3
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %40 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function simple_try_catch_test() {
  try {
    for (var i = 0; i < 10; i++) {
      i += 2;
    }
    throw i;
    i--;
  }
  catch (e) {
    e++;
    i -= 3;
  }
  return i;
}

//CHECK-LABEL:function simple_try_catch_finally_test()
//CHECK-NEXT:frame = [i, ?anon_0_e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = LoadFrameInst [i]
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
//CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:  %7 = ThrowInst %3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadFrameInst [i]
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
//CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %13 = CatchInst
//CHECK-NEXT:  %14 = StoreFrameInst %13, [?anon_0_e]
//CHECK-NEXT:  %15 = LoadFrameInst [i]
//CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
//CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %18 = BranchInst %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %19 = BranchInst %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %20 = LoadFrameInst [i]
//CHECK-NEXT:  %21 = AsNumericInst %20
//CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
//CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:  %24 = BranchInst %BB8
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %25 = TryEndInst
//CHECK-NEXT:  %26 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %27 = TryEndInst
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 3 : number
//CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
//CHECK-NEXT:  %31 = BranchInst %BB3
//CHECK-NEXT:function_end
function simple_try_catch_finally_test() {
  var i = 0;
  try {
    i++;
  }
  catch (e) {
    i += 2;
  }
  finally {
    i += 3;
  }
  i += 4;
}


//CHECK-LABEL:function simple_try_finally_test()
//CHECK-NEXT:frame = [i]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = CatchInst
//CHECK-NEXT:    %4 = LoadFrameInst [i]
//CHECK-NEXT:    %5 = BinaryOperatorInst '+', %4, 2 : number
//CHECK-NEXT:    %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:    %7 = ThrowInst %3
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '+', %8, 3 : number
//CHECK-NEXT:    %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:    %11 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %12 = LoadFrameInst [i]
//CHECK-NEXT:    %13 = AsNumericInst %12
//CHECK-NEXT:    %14 = UnaryOperatorInst '++', %13 : number|bigint
//CHECK-NEXT:    %15 = StoreFrameInst %14, [i]
//CHECK-NEXT:    %16 = BranchInst %BB4
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %17 = TryEndInst
//CHECK-NEXT:    %18 = LoadFrameInst [i]
//CHECK-NEXT:    %19 = BinaryOperatorInst '+', %18, 2 : number
//CHECK-NEXT:    %20 = StoreFrameInst %19, [i]
//CHECK-NEXT:    %21 = BranchInst %BB3
//CHECK-NEXT:function_end

function simple_try_finally_test() {
  var i = 0;
  try {
    i++;
  }
  finally {
    i += 2;
  }
  i += 3;
}


//CHECK-LABEL:function try_catch_finally_with_return_test()
//CHECK-NEXT:frame = [i, ?anon_0_e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = LoadFrameInst [i]
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
//CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:  %7 = ReturnInst "c" : string
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadFrameInst [i]
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
//CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:  %11 = ReturnInst "d" : string
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %13 = CatchInst
//CHECK-NEXT:  %14 = StoreFrameInst %13, [?anon_0_e]
//CHECK-NEXT:  %15 = LoadFrameInst [i]
//CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
//CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %18 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %19 = BranchInst %BB8
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %20 = LoadFrameInst [i]
//CHECK-NEXT:  %21 = AsNumericInst %20
//CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
//CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:  %24 = BranchInst %BB9
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %25 = TryEndInst
//CHECK-NEXT:  %26 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %27 = TryEndInst
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 3 : number
//CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
//CHECK-NEXT:  %31 = ReturnInst "c" : string
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %32 = ReturnInst "a" : string
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %33 = BranchInst %BB13
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %34 = TryEndInst
//CHECK-NEXT:  %35 = BranchInst %BB7
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %36 = TryEndInst
//CHECK-NEXT:  %37 = LoadFrameInst [i]
//CHECK-NEXT:  %38 = BinaryOperatorInst '+', %37, 3 : number
//CHECK-NEXT:  %39 = StoreFrameInst %38, [i]
//CHECK-NEXT:  %40 = ReturnInst "c" : string
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %41 = ReturnInst "b" : string
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %42 = BranchInst %BB7
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %43 = TryEndInst
//CHECK-NEXT:  %44 = LoadFrameInst [i]
//CHECK-NEXT:  %45 = BinaryOperatorInst '+', %44, 3 : number
//CHECK-NEXT:  %46 = StoreFrameInst %45, [i]
//CHECK-NEXT:  %47 = ReturnInst "c" : string
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %48 = BranchInst %BB3
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %49 = ThrowInst %3
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %50 = ReturnInst undefined : undefined
//CHECK-NEXT:function_end

function try_catch_finally_with_return_test() {
  var i = 0;
  try {
    i++;
    return "a";
  }
  catch (e) {
    i += 2;
    return "b";
  }
  finally {
    i += 3;
    return "c";
  }
  i += 4;
  return "d";
}


//CHECK-LABEL:function nested_try_test()
//CHECK-NEXT:frame = [i, ?anon_0_e, ?anon_1_e, ?anon_2_e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = LoadFrameInst [i]
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
//CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:  %7 = ThrowInst %3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadFrameInst [i]
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
//CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %13 = CatchInst
//CHECK-NEXT:  %14 = StoreFrameInst %13, [?anon_2_e]
//CHECK-NEXT:  %15 = LoadFrameInst [i]
//CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
//CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %18 = BranchInst %BB6
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %19 = BranchInst %BB7
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %20 = LoadFrameInst [i]
//CHECK-NEXT:  %21 = AsNumericInst %20
//CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
//CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:  %24 = TryStartInst %BB8, %BB9
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %25 = CatchInst
//CHECK-NEXT:  %26 = LoadFrameInst [i]
//CHECK-NEXT:  %27 = BinaryOperatorInst '+', %26, 7 : number
//CHECK-NEXT:  %28 = StoreFrameInst %27, [i]
//CHECK-NEXT:  %29 = ThrowInst %25
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %30 = BranchInst %BB11
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %31 = TryStartInst %BB12, %BB13
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %32 = CatchInst
//CHECK-NEXT:  %33 = StoreFrameInst %32, [?anon_1_e]
//CHECK-NEXT:  %34 = LoadFrameInst [i]
//CHECK-NEXT:  %35 = BinaryOperatorInst '+', %34, 6 : number
//CHECK-NEXT:  %36 = StoreFrameInst %35, [i]
//CHECK-NEXT:  %37 = BranchInst %BB14
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %38 = BranchInst %BB15
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %39 = LoadFrameInst [i]
//CHECK-NEXT:  %40 = BinaryOperatorInst '+', %39, 5 : number
//CHECK-NEXT:  %41 = StoreFrameInst %40, [i]
//CHECK-NEXT:  %42 = TryStartInst %BB16, %BB17
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %43 = CatchInst
//CHECK-NEXT:  %44 = LoadFrameInst [i]
//CHECK-NEXT:  %45 = BinaryOperatorInst '+', %44, 10 : number
//CHECK-NEXT:  %46 = StoreFrameInst %45, [i]
//CHECK-NEXT:  %47 = ThrowInst %43
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %48 = BranchInst %BB19
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %49 = TryStartInst %BB20, %BB21
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %50 = CatchInst
//CHECK-NEXT:  %51 = StoreFrameInst %50, [?anon_0_e]
//CHECK-NEXT:  %52 = LoadFrameInst [i]
//CHECK-NEXT:  %53 = BinaryOperatorInst '+', %52, 9 : number
//CHECK-NEXT:  %54 = StoreFrameInst %53, [i]
//CHECK-NEXT:  %55 = BranchInst %BB22
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %56 = BranchInst %BB23
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %57 = LoadFrameInst [i]
//CHECK-NEXT:  %58 = BinaryOperatorInst '+', %57, 8 : number
//CHECK-NEXT:  %59 = StoreFrameInst %58, [i]
//CHECK-NEXT:  %60 = BranchInst %BB24
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %61 = TryEndInst
//CHECK-NEXT:  %62 = BranchInst %BB25
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %63 = TryEndInst
//CHECK-NEXT:  %64 = LoadFrameInst [i]
//CHECK-NEXT:  %65 = BinaryOperatorInst '+', %64, 10 : number
//CHECK-NEXT:  %66 = StoreFrameInst %65, [i]
//CHECK-NEXT:  %67 = BranchInst %BB26
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %68 = TryEndInst
//CHECK-NEXT:  %69 = BranchInst %BB27
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %70 = TryEndInst
//CHECK-NEXT:  %71 = LoadFrameInst [i]
//CHECK-NEXT:  %72 = BinaryOperatorInst '+', %71, 7 : number
//CHECK-NEXT:  %73 = StoreFrameInst %72, [i]
//CHECK-NEXT:  %74 = BranchInst %BB28
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %75 = TryEndInst
//CHECK-NEXT:  %76 = BranchInst %BB29
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %77 = TryEndInst
//CHECK-NEXT:  %78 = LoadFrameInst [i]
//CHECK-NEXT:  %79 = BinaryOperatorInst '+', %78, 3 : number
//CHECK-NEXT:  %80 = StoreFrameInst %79, [i]
//CHECK-NEXT:  %81 = ReturnInst "a" : string
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %82 = BranchInst %BB31
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %83 = TryEndInst
//CHECK-NEXT:  %84 = BranchInst %BB22
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %85 = TryEndInst
//CHECK-NEXT:  %86 = LoadFrameInst [i]
//CHECK-NEXT:  %87 = BinaryOperatorInst '+', %86, 10 : number
//CHECK-NEXT:  %88 = StoreFrameInst %87, [i]
//CHECK-NEXT:  %89 = BranchInst %BB18
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %90 = TryEndInst
//CHECK-NEXT:  %91 = BranchInst %BB14
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %92 = TryEndInst
//CHECK-NEXT:  %93 = LoadFrameInst [i]
//CHECK-NEXT:  %94 = BinaryOperatorInst '+', %93, 7 : number
//CHECK-NEXT:  %95 = StoreFrameInst %94, [i]
//CHECK-NEXT:  %96 = BranchInst %BB10
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %97 = TryEndInst
//CHECK-NEXT:  %98 = BranchInst %BB6
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %99 = TryEndInst
//CHECK-NEXT:  %100 = LoadFrameInst [i]
//CHECK-NEXT:  %101 = BinaryOperatorInst '+', %100, 3 : number
//CHECK-NEXT:  %102 = StoreFrameInst %101, [i]
//CHECK-NEXT:  %103 = BranchInst %BB3
//CHECK-NEXT:function_end

function nested_try_test() {
  var i = 0;
  try {
    i++;
    try {
      i += 5;
      try {
        i += 8;
        return "a";
      }
      catch (e) {
        i+= 9;
      }
      finally {
        i += 10;
      }
    }
    catch (e) {
      i += 6;
    }
    finally {
      i += 7;
    }
  }
  catch (e) {
    i += 2;
  }
  finally {
    i += 3;
  }
  i += 4;
}


//CHECK-LABEL:function nested_catch_test()
//CHECK-NEXT:frame = [i, ?anon_0_e, ?anon_1_e, ?anon_2_e, ?anon_3_e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %3 = CatchInst
//CHECK-NEXT:  %4 = LoadFrameInst [i]
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 8 : number
//CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:  %7 = ThrowInst %3
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %8 = LoadFrameInst [i]
//CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 10 : number
//CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
//CHECK-NEXT:  %11 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %13 = CatchInst
//CHECK-NEXT:  %14 = StoreFrameInst %13, [?anon_0_e]
//CHECK-NEXT:  %15 = LoadFrameInst [i]
//CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 9 : number
//CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %18 = TryStartInst %BB6, %BB7
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %19 = BranchInst %BB9
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %20 = LoadFrameInst [i]
//CHECK-NEXT:  %21 = AsNumericInst %20
//CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
//CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:  %24 = BranchInst %BB10
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %25 = TryEndInst
//CHECK-NEXT:  %26 = BranchInst %BB8
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %27 = CatchInst
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 4 : number
//CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
//CHECK-NEXT:  %31 = TryStartInst %BB11, %BB12
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %32 = BranchInst %BB8
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %33 = TryStartInst %BB14, %BB15
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %34 = CatchInst
//CHECK-NEXT:  %35 = StoreFrameInst %34, [?anon_1_e]
//CHECK-NEXT:  %36 = LoadFrameInst [i]
//CHECK-NEXT:  %37 = BinaryOperatorInst '+', %36, 3 : number
//CHECK-NEXT:  %38 = StoreFrameInst %37, [i]
//CHECK-NEXT:  %39 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %40 = BranchInst %BB17
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %41 = LoadFrameInst [i]
//CHECK-NEXT:  %42 = BinaryOperatorInst '+', %41, 2 : number
//CHECK-NEXT:  %43 = StoreFrameInst %42, [i]
//CHECK-NEXT:  %44 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %45 = TryEndInst
//CHECK-NEXT:  %46 = BranchInst %BB16
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %47 = TryEndInst
//CHECK-NEXT:  %48 = LoadFrameInst [i]
//CHECK-NEXT:  %49 = BinaryOperatorInst '+', %48, 4 : number
//CHECK-NEXT:  %50 = StoreFrameInst %49, [i]
//CHECK-NEXT:  %51 = TryStartInst %BB19, %BB20
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %52 = CatchInst
//CHECK-NEXT:  %53 = LoadFrameInst [i]
//CHECK-NEXT:  %54 = BinaryOperatorInst '+', %53, 7 : number
//CHECK-NEXT:  %55 = StoreFrameInst %54, [i]
//CHECK-NEXT:  %56 = ThrowInst %52
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %57 = BranchInst %BB13
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %58 = TryStartInst %BB22, %BB23
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %59 = CatchInst
//CHECK-NEXT:  %60 = StoreFrameInst %59, [?anon_2_e]
//CHECK-NEXT:  %61 = LoadFrameInst [i]
//CHECK-NEXT:  %62 = BinaryOperatorInst '+', %61, 6 : number
//CHECK-NEXT:  %63 = StoreFrameInst %62, [i]
//CHECK-NEXT:  %64 = BranchInst %BB24
//CHECK-NEXT:%BB24:
//CHECK-NEXT:  %65 = BranchInst %BB25
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %66 = LoadFrameInst [i]
//CHECK-NEXT:  %67 = BinaryOperatorInst '+', %66, 5 : number
//CHECK-NEXT:  %68 = StoreFrameInst %67, [i]
//CHECK-NEXT:  %69 = BranchInst %BB26
//CHECK-NEXT:%BB26:
//CHECK-NEXT:  %70 = TryEndInst
//CHECK-NEXT:  %71 = BranchInst %BB24
//CHECK-NEXT:%BB25:
//CHECK-NEXT:  %72 = TryEndInst
//CHECK-NEXT:  %73 = LoadFrameInst [i]
//CHECK-NEXT:  %74 = BinaryOperatorInst '+', %73, 7 : number
//CHECK-NEXT:  %75 = StoreFrameInst %74, [i]
//CHECK-NEXT:  %76 = BranchInst %BB21
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %77 = CatchInst
//CHECK-NEXT:  %78 = LoadFrameInst [i]
//CHECK-NEXT:  %79 = BinaryOperatorInst '+', %78, 7 : number
//CHECK-NEXT:  %80 = StoreFrameInst %79, [i]
//CHECK-NEXT:  %81 = ThrowInst %77
//CHECK-NEXT:%BB27:
//CHECK-NEXT:  %82 = ThrowInst %27
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %83 = TryStartInst %BB28, %BB29
//CHECK-NEXT:%BB28:
//CHECK-NEXT:  %84 = CatchInst
//CHECK-NEXT:  %85 = StoreFrameInst %84, [?anon_3_e]
//CHECK-NEXT:  %86 = LoadFrameInst [i]
//CHECK-NEXT:  %87 = BinaryOperatorInst '+', %86, 6 : number
//CHECK-NEXT:  %88 = StoreFrameInst %87, [i]
//CHECK-NEXT:  %89 = BranchInst %BB30
//CHECK-NEXT:%BB30:
//CHECK-NEXT:  %90 = BranchInst %BB31
//CHECK-NEXT:%BB29:
//CHECK-NEXT:  %91 = LoadFrameInst [i]
//CHECK-NEXT:  %92 = BinaryOperatorInst '+', %91, 5 : number
//CHECK-NEXT:  %93 = StoreFrameInst %92, [i]
//CHECK-NEXT:  %94 = BranchInst %BB32
//CHECK-NEXT:%BB32:
//CHECK-NEXT:  %95 = TryEndInst
//CHECK-NEXT:  %96 = BranchInst %BB30
//CHECK-NEXT:%BB31:
//CHECK-NEXT:  %97 = TryEndInst
//CHECK-NEXT:  %98 = LoadFrameInst [i]
//CHECK-NEXT:  %99 = BinaryOperatorInst '+', %98, 7 : number
//CHECK-NEXT:  %100 = StoreFrameInst %99, [i]
//CHECK-NEXT:  %101 = BranchInst %BB27
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %102 = TryEndInst
//CHECK-NEXT:  %103 = LoadFrameInst [i]
//CHECK-NEXT:  %104 = BinaryOperatorInst '+', %103, 8 : number
//CHECK-NEXT:  %105 = StoreFrameInst %104, [i]
//CHECK-NEXT:  %106 = BranchInst %BB3
//CHECK-NEXT:function_end

function nested_catch_test() {
  var i = 0;
  try {
    i++;
  }
  catch (e) {
    i += 9;
    try {
      i += 2;
    }
    catch (e) {
      i += 3;
    }
    finally {
      i += 4;
      try {
        i += 5;
      }
      catch (e) {
        i += 6;
      }
      finally {
        i += 7;
      }
    }
  }
  finally {
    i += 8;
  }
  i += 10;
}


//CHECK-LABEL:function finally_with_break_continue_test()
//CHECK-NEXT:frame = [i, ?anon_0_e]
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:  %2 = BranchInst %BB1
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  %3 = TryStartInst %BB3, %BB4
//CHECK-NEXT:%BB5:
//CHECK-NEXT:  %4 = LoadFrameInst [i]
//CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 4 : number
//CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:  %7 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  %8 = LoadFrameInst [i]
//CHECK-NEXT:  %9 = BinaryOperatorInst '<', %8, 10 : number
//CHECK-NEXT:  %10 = CondBranchInst %9, %BB2, %BB5
//CHECK-NEXT:%BB6:
//CHECK-NEXT:  %11 = LoadFrameInst [i]
//CHECK-NEXT:  %12 = BinaryOperatorInst '<', %11, 10 : number
//CHECK-NEXT:  %13 = CondBranchInst %12, %BB2, %BB5
//CHECK-NEXT:%BB7:
//CHECK-NEXT:  %14 = LoadFrameInst [i]
//CHECK-NEXT:  %15 = AsNumericInst %14
//CHECK-NEXT:  %16 = UnaryOperatorInst '++', %15 : number|bigint
//CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:  %18 = BranchInst %BB6
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  %19 = CatchInst
//CHECK-NEXT:  %20 = LoadFrameInst [i]
//CHECK-NEXT:  %21 = BinaryOperatorInst '+', %20, 3 : number
//CHECK-NEXT:  %22 = StoreFrameInst %21, [i]
//CHECK-NEXT:  %23 = ThrowInst %19
//CHECK-NEXT:%BB8:
//CHECK-NEXT:  %24 = BranchInst %BB7
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  %25 = TryStartInst %BB9, %BB10
//CHECK-NEXT:%BB9:
//CHECK-NEXT:  %26 = CatchInst
//CHECK-NEXT:  %27 = StoreFrameInst %26, [?anon_0_e]
//CHECK-NEXT:  %28 = LoadFrameInst [i]
//CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 2 : number
//CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
//CHECK-NEXT:  %31 = LoadFrameInst [i]
//CHECK-NEXT:  %32 = BinaryOperatorInst '==', %31, 3 : number
//CHECK-NEXT:  %33 = CondBranchInst %32, %BB11, %BB12
//CHECK-NEXT:%BB13:
//CHECK-NEXT:  %34 = BranchInst %BB14
//CHECK-NEXT:%BB10:
//CHECK-NEXT:  %35 = LoadFrameInst [i]
//CHECK-NEXT:  %36 = AsNumericInst %35
//CHECK-NEXT:  %37 = UnaryOperatorInst '++', %36 : number|bigint
//CHECK-NEXT:  %38 = StoreFrameInst %37, [i]
//CHECK-NEXT:  %39 = BranchInst %BB15
//CHECK-NEXT:%BB15:
//CHECK-NEXT:  %40 = TryEndInst
//CHECK-NEXT:  %41 = BranchInst %BB16
//CHECK-NEXT:%BB16:
//CHECK-NEXT:  %42 = TryEndInst
//CHECK-NEXT:  %43 = LoadFrameInst [i]
//CHECK-NEXT:  %44 = BinaryOperatorInst '+', %43, 3 : number
//CHECK-NEXT:  %45 = StoreFrameInst %44, [i]
//CHECK-NEXT:  %46 = BranchInst %BB5
//CHECK-NEXT:%BB17:
//CHECK-NEXT:  %47 = BranchInst %BB18
//CHECK-NEXT:%BB18:
//CHECK-NEXT:  %48 = TryEndInst
//CHECK-NEXT:  %49 = BranchInst %BB13
//CHECK-NEXT:%BB11:
//CHECK-NEXT:  %50 = BranchInst %BB19
//CHECK-NEXT:%BB12:
//CHECK-NEXT:  %51 = BranchInst %BB20
//CHECK-NEXT:%BB20:
//CHECK-NEXT:  %52 = BranchInst %BB21
//CHECK-NEXT:%BB19:
//CHECK-NEXT:  %53 = TryEndInst
//CHECK-NEXT:  %54 = LoadFrameInst [i]
//CHECK-NEXT:  %55 = BinaryOperatorInst '+', %54, 3 : number
//CHECK-NEXT:  %56 = StoreFrameInst %55, [i]
//CHECK-NEXT:  %57 = ReturnInst undefined : undefined
//CHECK-NEXT:%BB22:
//CHECK-NEXT:  %58 = BranchInst %BB20
//CHECK-NEXT:%BB21:
//CHECK-NEXT:  %59 = TryEndInst
//CHECK-NEXT:  %60 = LoadFrameInst [i]
//CHECK-NEXT:  %61 = BinaryOperatorInst '+', %60, 3 : number
//CHECK-NEXT:  %62 = StoreFrameInst %61, [i]
//CHECK-NEXT:  %63 = BranchInst %BB7
//CHECK-NEXT:%BB23:
//CHECK-NEXT:  %64 = BranchInst %BB13
//CHECK-NEXT:%BB14:
//CHECK-NEXT:  %65 = TryEndInst
//CHECK-NEXT:  %66 = LoadFrameInst [i]
//CHECK-NEXT:  %67 = BinaryOperatorInst '+', %66, 3 : number
//CHECK-NEXT:  %68 = StoreFrameInst %67, [i]
//CHECK-NEXT:  %69 = BranchInst %BB8
//CHECK-NEXT:function_end

function finally_with_break_continue_test() {
  for (var i = 0; i < 10; i++) {
    try {
      i++;
      break;
    }
    catch (e) {
      i += 2;
      if (i == 3) {
        return;
      }
      continue;
    }
    finally {
      i += 3;
    }
  }
  i += 4;
}
