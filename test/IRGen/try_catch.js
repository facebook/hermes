/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermes -hermes-parser -dump-ir %s -O

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

// Auto-generated content below. Please do not modify manually.

// CHECK:function global()
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple_try_catch_test" : string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simple_try_catch_finally_test" : string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "simple_try_finally_test" : string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "try_catch_finally_with_return_test" : string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "nested_try_test" : string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "nested_catch_test" : string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "finally_with_break_continue_test" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %simple_try_catch_test()
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7 : closure, globalObject : object, "simple_try_catch_test" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %simple_try_catch_finally_test()
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9 : closure, globalObject : object, "simple_try_catch_finally_test" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %simple_try_finally_test()
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11 : closure, globalObject : object, "simple_try_finally_test" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %try_catch_finally_with_return_test()
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13 : closure, globalObject : object, "try_catch_finally_with_return_test" : string
// CHECK-NEXT:  %15 = CreateFunctionInst %nested_try_test()
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15 : closure, globalObject : object, "nested_try_test" : string
// CHECK-NEXT:  %17 = CreateFunctionInst %nested_catch_test()
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17 : closure, globalObject : object, "nested_catch_test" : string
// CHECK-NEXT:  %19 = CreateFunctionInst %finally_with_break_continue_test()
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19 : closure, globalObject : object, "finally_with_break_continue_test" : string
// CHECK-NEXT:  %21 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %22 = StoreStackInst undefined : undefined, %21
// CHECK-NEXT:  %23 = LoadStackInst %21
// CHECK-NEXT:  %24 = ReturnInst %23
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_test()
// CHECK-NEXT:frame = [i, e]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = CatchInst
// CHECK-NEXT:  %3 = StoreFrameInst %2, [e]
// CHECK-NEXT:  %4 = LoadFrameInst [e]
// CHECK-NEXT:  %5 = AsNumericInst %4
// CHECK-NEXT:  %6 = UnaryOperatorInst '++', %5 : number|bigint
// CHECK-NEXT:  %7 = StoreFrameInst %6, [e]
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '-', %8, 3 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst [i]
// CHECK-NEXT:  %13 = ReturnInst %12
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %15 = LoadFrameInst [i]
// CHECK-NEXT:  %16 = BinaryOperatorInst '<', %15, 10 : number
// CHECK-NEXT:  %17 = CondBranchInst %16, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadFrameInst [i]
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %18, 2 : number
// CHECK-NEXT:  %20 = StoreFrameInst %19, [i]
// CHECK-NEXT:  %21 = BranchInst %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = LoadFrameInst [i]
// CHECK-NEXT:  %23 = ThrowInst %22
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = LoadFrameInst [i]
// CHECK-NEXT:  %25 = BinaryOperatorInst '<', %24, 10 : number
// CHECK-NEXT:  %26 = CondBranchInst %25, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = LoadFrameInst [i]
// CHECK-NEXT:  %28 = AsNumericInst %27
// CHECK-NEXT:  %29 = UnaryOperatorInst '++', %28 : number|bigint
// CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
// CHECK-NEXT:  %31 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = LoadFrameInst [i]
// CHECK-NEXT:  %33 = AsNumericInst %32
// CHECK-NEXT:  %34 = UnaryOperatorInst '--', %33 : number|bigint
// CHECK-NEXT:  %35 = StoreFrameInst %34, [i]
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB3
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_finally_test()
// CHECK-NEXT:frame = [i, e]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
// CHECK-NEXT:  %7 = ThrowInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst
// CHECK-NEXT:  %14 = StoreFrameInst %13, [e]
// CHECK-NEXT:  %15 = LoadFrameInst [i]
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
// CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst [i]
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
// CHECK-NEXT:  %24 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst [i]
// CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 3 : number
// CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
// CHECK-NEXT:  %31 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_try_finally_test()
// CHECK-NEXT:frame = [i]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 2 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
// CHECK-NEXT:  %7 = ThrowInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 3 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst [i]
// CHECK-NEXT:  %13 = AsNumericInst %12
// CHECK-NEXT:  %14 = UnaryOperatorInst '++', %13 : number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14, [i]
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = TryEndInst
// CHECK-NEXT:  %18 = LoadFrameInst [i]
// CHECK-NEXT:  %19 = BinaryOperatorInst '+', %18, 2 : number
// CHECK-NEXT:  %20 = StoreFrameInst %19, [i]
// CHECK-NEXT:  %21 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function try_catch_finally_with_return_test()
// CHECK-NEXT:frame = [i, e]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
// CHECK-NEXT:  %7 = ReturnInst "c" : string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = ReturnInst "d" : string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst
// CHECK-NEXT:  %14 = StoreFrameInst %13, [e]
// CHECK-NEXT:  %15 = LoadFrameInst [i]
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
// CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %19 = BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst [i]
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
// CHECK-NEXT:  %24 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %27 = TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst [i]
// CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 3 : number
// CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
// CHECK-NEXT:  %31 = ReturnInst "c" : string
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %32 = ReturnInst "a" : string
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %33 = BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %34 = TryEndInst
// CHECK-NEXT:  %35 = BranchInst %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %36 = TryEndInst
// CHECK-NEXT:  %37 = LoadFrameInst [i]
// CHECK-NEXT:  %38 = BinaryOperatorInst '+', %37, 3 : number
// CHECK-NEXT:  %39 = StoreFrameInst %38, [i]
// CHECK-NEXT:  %40 = ReturnInst "c" : string
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %41 = ReturnInst "b" : string
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %42 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %43 = TryEndInst
// CHECK-NEXT:  %44 = LoadFrameInst [i]
// CHECK-NEXT:  %45 = BinaryOperatorInst '+', %44, 3 : number
// CHECK-NEXT:  %46 = StoreFrameInst %45, [i]
// CHECK-NEXT:  %47 = ReturnInst "c" : string
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %48 = BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = ThrowInst %3
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %50 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nested_try_test()
// CHECK-NEXT:frame = [i, e, e#1, e#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 3 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
// CHECK-NEXT:  %7 = ThrowInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 4 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst
// CHECK-NEXT:  %14 = StoreFrameInst %13, [e#2]
// CHECK-NEXT:  %15 = LoadFrameInst [i]
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 2 : number
// CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst [i]
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
// CHECK-NEXT:  %24 = TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = CatchInst
// CHECK-NEXT:  %26 = LoadFrameInst [i]
// CHECK-NEXT:  %27 = BinaryOperatorInst '+', %26, 7 : number
// CHECK-NEXT:  %28 = StoreFrameInst %27, [i]
// CHECK-NEXT:  %29 = ThrowInst %25
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %30 = BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %31 = TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %32 = CatchInst
// CHECK-NEXT:  %33 = StoreFrameInst %32, [e#1]
// CHECK-NEXT:  %34 = LoadFrameInst [i]
// CHECK-NEXT:  %35 = BinaryOperatorInst '+', %34, 6 : number
// CHECK-NEXT:  %36 = StoreFrameInst %35, [i]
// CHECK-NEXT:  %37 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %38 = BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %39 = LoadFrameInst [i]
// CHECK-NEXT:  %40 = BinaryOperatorInst '+', %39, 5 : number
// CHECK-NEXT:  %41 = StoreFrameInst %40, [i]
// CHECK-NEXT:  %42 = TryStartInst %BB16, %BB17
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %43 = CatchInst
// CHECK-NEXT:  %44 = LoadFrameInst [i]
// CHECK-NEXT:  %45 = BinaryOperatorInst '+', %44, 10 : number
// CHECK-NEXT:  %46 = StoreFrameInst %45, [i]
// CHECK-NEXT:  %47 = ThrowInst %43
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %48 = BranchInst %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %50 = CatchInst
// CHECK-NEXT:  %51 = StoreFrameInst %50, [e]
// CHECK-NEXT:  %52 = LoadFrameInst [i]
// CHECK-NEXT:  %53 = BinaryOperatorInst '+', %52, 9 : number
// CHECK-NEXT:  %54 = StoreFrameInst %53, [i]
// CHECK-NEXT:  %55 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %56 = BranchInst %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = LoadFrameInst [i]
// CHECK-NEXT:  %58 = BinaryOperatorInst '+', %57, 8 : number
// CHECK-NEXT:  %59 = StoreFrameInst %58, [i]
// CHECK-NEXT:  %60 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %61 = TryEndInst
// CHECK-NEXT:  %62 = BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %63 = TryEndInst
// CHECK-NEXT:  %64 = LoadFrameInst [i]
// CHECK-NEXT:  %65 = BinaryOperatorInst '+', %64, 10 : number
// CHECK-NEXT:  %66 = StoreFrameInst %65, [i]
// CHECK-NEXT:  %67 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %68 = TryEndInst
// CHECK-NEXT:  %69 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %70 = TryEndInst
// CHECK-NEXT:  %71 = LoadFrameInst [i]
// CHECK-NEXT:  %72 = BinaryOperatorInst '+', %71, 7 : number
// CHECK-NEXT:  %73 = StoreFrameInst %72, [i]
// CHECK-NEXT:  %74 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %75 = TryEndInst
// CHECK-NEXT:  %76 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %77 = TryEndInst
// CHECK-NEXT:  %78 = LoadFrameInst [i]
// CHECK-NEXT:  %79 = BinaryOperatorInst '+', %78, 3 : number
// CHECK-NEXT:  %80 = StoreFrameInst %79, [i]
// CHECK-NEXT:  %81 = ReturnInst "a" : string
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %82 = BranchInst %BB31
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %83 = TryEndInst
// CHECK-NEXT:  %84 = BranchInst %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %85 = TryEndInst
// CHECK-NEXT:  %86 = LoadFrameInst [i]
// CHECK-NEXT:  %87 = BinaryOperatorInst '+', %86, 10 : number
// CHECK-NEXT:  %88 = StoreFrameInst %87, [i]
// CHECK-NEXT:  %89 = BranchInst %BB18
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %90 = TryEndInst
// CHECK-NEXT:  %91 = BranchInst %BB14
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %92 = TryEndInst
// CHECK-NEXT:  %93 = LoadFrameInst [i]
// CHECK-NEXT:  %94 = BinaryOperatorInst '+', %93, 7 : number
// CHECK-NEXT:  %95 = StoreFrameInst %94, [i]
// CHECK-NEXT:  %96 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %97 = TryEndInst
// CHECK-NEXT:  %98 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %99 = TryEndInst
// CHECK-NEXT:  %100 = LoadFrameInst [i]
// CHECK-NEXT:  %101 = BinaryOperatorInst '+', %100, 3 : number
// CHECK-NEXT:  %102 = StoreFrameInst %101, [i]
// CHECK-NEXT:  %103 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function nested_catch_test()
// CHECK-NEXT:frame = [i, e, e#1, e#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = LoadFrameInst [i]
// CHECK-NEXT:  %5 = BinaryOperatorInst '+', %4, 8 : number
// CHECK-NEXT:  %6 = StoreFrameInst %5, [i]
// CHECK-NEXT:  %7 = ThrowInst %3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst [i]
// CHECK-NEXT:  %9 = BinaryOperatorInst '+', %8, 10 : number
// CHECK-NEXT:  %10 = StoreFrameInst %9, [i]
// CHECK-NEXT:  %11 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst
// CHECK-NEXT:  %14 = StoreFrameInst %13, [e]
// CHECK-NEXT:  %15 = LoadFrameInst [i]
// CHECK-NEXT:  %16 = BinaryOperatorInst '+', %15, 9 : number
// CHECK-NEXT:  %17 = StoreFrameInst %16, [i]
// CHECK-NEXT:  %18 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %19 = BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst [i]
// CHECK-NEXT:  %21 = AsNumericInst %20
// CHECK-NEXT:  %22 = UnaryOperatorInst '++', %21 : number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22, [i]
// CHECK-NEXT:  %24 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = CatchInst
// CHECK-NEXT:  %28 = LoadFrameInst [i]
// CHECK-NEXT:  %29 = BinaryOperatorInst '+', %28, 4 : number
// CHECK-NEXT:  %30 = StoreFrameInst %29, [i]
// CHECK-NEXT:  %31 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %32 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %33 = TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = CatchInst
// CHECK-NEXT:  %35 = StoreFrameInst %34, [e#1]
// CHECK-NEXT:  %36 = LoadFrameInst [i]
// CHECK-NEXT:  %37 = BinaryOperatorInst '+', %36, 3 : number
// CHECK-NEXT:  %38 = StoreFrameInst %37, [i]
// CHECK-NEXT:  %39 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %40 = BranchInst %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = LoadFrameInst [i]
// CHECK-NEXT:  %42 = BinaryOperatorInst '+', %41, 2 : number
// CHECK-NEXT:  %43 = StoreFrameInst %42, [i]
// CHECK-NEXT:  %44 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %45 = TryEndInst
// CHECK-NEXT:  %46 = BranchInst %BB16
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %47 = TryEndInst
// CHECK-NEXT:  %48 = LoadFrameInst [i]
// CHECK-NEXT:  %49 = BinaryOperatorInst '+', %48, 4 : number
// CHECK-NEXT:  %50 = StoreFrameInst %49, [i]
// CHECK-NEXT:  %51 = TryStartInst %BB19, %BB20
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %52 = CatchInst
// CHECK-NEXT:  %53 = LoadFrameInst [i]
// CHECK-NEXT:  %54 = BinaryOperatorInst '+', %53, 7 : number
// CHECK-NEXT:  %55 = StoreFrameInst %54, [i]
// CHECK-NEXT:  %56 = ThrowInst %52
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = BranchInst %BB13
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %58 = TryStartInst %BB22, %BB23
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %59 = CatchInst
// CHECK-NEXT:  %60 = StoreFrameInst %59, [e#2]
// CHECK-NEXT:  %61 = LoadFrameInst [i]
// CHECK-NEXT:  %62 = BinaryOperatorInst '+', %61, 6 : number
// CHECK-NEXT:  %63 = StoreFrameInst %62, [i]
// CHECK-NEXT:  %64 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %65 = BranchInst %BB25
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %66 = LoadFrameInst [i]
// CHECK-NEXT:  %67 = BinaryOperatorInst '+', %66, 5 : number
// CHECK-NEXT:  %68 = StoreFrameInst %67, [i]
// CHECK-NEXT:  %69 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %70 = TryEndInst
// CHECK-NEXT:  %71 = BranchInst %BB24
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %72 = TryEndInst
// CHECK-NEXT:  %73 = LoadFrameInst [i]
// CHECK-NEXT:  %74 = BinaryOperatorInst '+', %73, 7 : number
// CHECK-NEXT:  %75 = StoreFrameInst %74, [i]
// CHECK-NEXT:  %76 = BranchInst %BB21
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %77 = CatchInst
// CHECK-NEXT:  %78 = LoadFrameInst [i]
// CHECK-NEXT:  %79 = BinaryOperatorInst '+', %78, 7 : number
// CHECK-NEXT:  %80 = StoreFrameInst %79, [i]
// CHECK-NEXT:  %81 = ThrowInst %77
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %82 = ThrowInst %27
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %83 = TryStartInst %BB28, %BB29
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %84 = CatchInst
// CHECK-NEXT:  %85 = StoreFrameInst %84, [e#2]
// CHECK-NEXT:  %86 = LoadFrameInst [i]
// CHECK-NEXT:  %87 = BinaryOperatorInst '+', %86, 6 : number
// CHECK-NEXT:  %88 = StoreFrameInst %87, [i]
// CHECK-NEXT:  %89 = BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %90 = BranchInst %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %91 = LoadFrameInst [i]
// CHECK-NEXT:  %92 = BinaryOperatorInst '+', %91, 5 : number
// CHECK-NEXT:  %93 = StoreFrameInst %92, [i]
// CHECK-NEXT:  %94 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %95 = TryEndInst
// CHECK-NEXT:  %96 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %97 = TryEndInst
// CHECK-NEXT:  %98 = LoadFrameInst [i]
// CHECK-NEXT:  %99 = BinaryOperatorInst '+', %98, 7 : number
// CHECK-NEXT:  %100 = StoreFrameInst %99, [i]
// CHECK-NEXT:  %101 = BranchInst %BB27
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %102 = TryEndInst
// CHECK-NEXT:  %103 = LoadFrameInst [i]
// CHECK-NEXT:  %104 = BinaryOperatorInst '+', %103, 8 : number
// CHECK-NEXT:  %105 = StoreFrameInst %104, [i]
// CHECK-NEXT:  %106 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function finally_with_break_continue_test()
// CHECK-NEXT:frame = [i, e]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined : undefined, [i]
// CHECK-NEXT:  %1 = StoreFrameInst 0 : number, [i]
// CHECK-NEXT:  %2 = LoadFrameInst [i]
// CHECK-NEXT:  %3 = BinaryOperatorInst '<', %2, 10 : number
// CHECK-NEXT:  %4 = CondBranchInst %3, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst [i]
// CHECK-NEXT:  %7 = BinaryOperatorInst '+', %6, 4 : number
// CHECK-NEXT:  %8 = StoreFrameInst %7, [i]
// CHECK-NEXT:  %9 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst [i]
// CHECK-NEXT:  %11 = BinaryOperatorInst '<', %10, 10 : number
// CHECK-NEXT:  %12 = CondBranchInst %11, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst [i]
// CHECK-NEXT:  %14 = AsNumericInst %13
// CHECK-NEXT:  %15 = UnaryOperatorInst '++', %14 : number|bigint
// CHECK-NEXT:  %16 = StoreFrameInst %15, [i]
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = CatchInst
// CHECK-NEXT:  %19 = LoadFrameInst [i]
// CHECK-NEXT:  %20 = BinaryOperatorInst '+', %19, 3 : number
// CHECK-NEXT:  %21 = StoreFrameInst %20, [i]
// CHECK-NEXT:  %22 = ThrowInst %18
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %23 = BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = CatchInst
// CHECK-NEXT:  %26 = StoreFrameInst %25, [e]
// CHECK-NEXT:  %27 = LoadFrameInst [i]
// CHECK-NEXT:  %28 = BinaryOperatorInst '+', %27, 2 : number
// CHECK-NEXT:  %29 = StoreFrameInst %28, [i]
// CHECK-NEXT:  %30 = LoadFrameInst [i]
// CHECK-NEXT:  %31 = BinaryOperatorInst '==', %30, 3 : number
// CHECK-NEXT:  %32 = CondBranchInst %31, %BB10, %BB11
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %33 = BranchInst %BB13
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = LoadFrameInst [i]
// CHECK-NEXT:  %35 = AsNumericInst %34
// CHECK-NEXT:  %36 = UnaryOperatorInst '++', %35 : number|bigint
// CHECK-NEXT:  %37 = StoreFrameInst %36, [i]
// CHECK-NEXT:  %38 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %39 = TryEndInst
// CHECK-NEXT:  %40 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = TryEndInst
// CHECK-NEXT:  %42 = LoadFrameInst [i]
// CHECK-NEXT:  %43 = BinaryOperatorInst '+', %42, 3 : number
// CHECK-NEXT:  %44 = StoreFrameInst %43, [i]
// CHECK-NEXT:  %45 = BranchInst %BB2
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %46 = BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %47 = TryEndInst
// CHECK-NEXT:  %48 = BranchInst %BB12
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %49 = BranchInst %BB18
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %50 = BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %51 = BranchInst %BB20
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %52 = TryEndInst
// CHECK-NEXT:  %53 = LoadFrameInst [i]
// CHECK-NEXT:  %54 = BinaryOperatorInst '+', %53, 3 : number
// CHECK-NEXT:  %55 = StoreFrameInst %54, [i]
// CHECK-NEXT:  %56 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = BranchInst %BB19
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %58 = TryEndInst
// CHECK-NEXT:  %59 = LoadFrameInst [i]
// CHECK-NEXT:  %60 = BinaryOperatorInst '+', %59, 3 : number
// CHECK-NEXT:  %61 = StoreFrameInst %60, [i]
// CHECK-NEXT:  %62 = BranchInst %BB6
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %63 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %64 = TryEndInst
// CHECK-NEXT:  %65 = LoadFrameInst [i]
// CHECK-NEXT:  %66 = BinaryOperatorInst '+', %65, 3 : number
// CHECK-NEXT:  %67 = StoreFrameInst %66, [i]
// CHECK-NEXT:  %68 = BranchInst %BB7
// CHECK-NEXT:function_end
