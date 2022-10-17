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

// CHECK:function global#0()#1
// CHECK-NEXT:frame = [], globals = [simple_try_catch_test, simple_try_catch_finally_test, simple_try_finally_test, try_catch_finally_with_return_test, nested_try_test, nested_catch_test, finally_with_break_continue_test]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{global#0()#1}
// CHECK-NEXT:  %1 = CreateFunctionInst %simple_try_catch_test#0#1()#2, %0
// CHECK-NEXT:  %2 = StorePropertyInst %1 : closure, globalObject : object, "simple_try_catch_test" : string
// CHECK-NEXT:  %3 = CreateFunctionInst %simple_try_catch_finally_test#0#1()#3, %0
// CHECK-NEXT:  %4 = StorePropertyInst %3 : closure, globalObject : object, "simple_try_catch_finally_test" : string
// CHECK-NEXT:  %5 = CreateFunctionInst %simple_try_finally_test#0#1()#4, %0
// CHECK-NEXT:  %6 = StorePropertyInst %5 : closure, globalObject : object, "simple_try_finally_test" : string
// CHECK-NEXT:  %7 = CreateFunctionInst %try_catch_finally_with_return_test#0#1()#5, %0
// CHECK-NEXT:  %8 = StorePropertyInst %7 : closure, globalObject : object, "try_catch_finally_with_return_test" : string
// CHECK-NEXT:  %9 = CreateFunctionInst %nested_try_test#0#1()#6, %0
// CHECK-NEXT:  %10 = StorePropertyInst %9 : closure, globalObject : object, "nested_try_test" : string
// CHECK-NEXT:  %11 = CreateFunctionInst %nested_catch_test#0#1()#7, %0
// CHECK-NEXT:  %12 = StorePropertyInst %11 : closure, globalObject : object, "nested_catch_test" : string
// CHECK-NEXT:  %13 = CreateFunctionInst %finally_with_break_continue_test#0#1()#8, %0
// CHECK-NEXT:  %14 = StorePropertyInst %13 : closure, globalObject : object, "finally_with_break_continue_test" : string
// CHECK-NEXT:  %15 = AllocStackInst $?anon_0_ret
// CHECK-NEXT:  %16 = StoreStackInst undefined : undefined, %15
// CHECK-NEXT:  %17 = LoadStackInst %15
// CHECK-NEXT:  %18 = ReturnInst %17
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_test#0#1()#2
// CHECK-NEXT:frame = [i#2, ?anon_0_e#2]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_try_catch_test#0#1()#2}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#2], %0
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst
// CHECK-NEXT:  %4 = StoreFrameInst %3, [?anon_0_e#2], %0
// CHECK-NEXT:  %5 = LoadFrameInst [?anon_0_e#2], %0
// CHECK-NEXT:  %6 = AsNumericInst %5
// CHECK-NEXT:  %7 = UnaryOperatorInst '++', %6 : number|bigint
// CHECK-NEXT:  %8 = StoreFrameInst %7, [?anon_0_e#2], %0
// CHECK-NEXT:  %9 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '-', %9, 3 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#2], %0
// CHECK-NEXT:  %12 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %14 = ReturnInst %13
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %15 = StoreFrameInst 0 : number, [i#2], %0
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %17 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %18 = BinaryOperatorInst '+', %17, 2 : number
// CHECK-NEXT:  %19 = StoreFrameInst %18, [i#2], %0
// CHECK-NEXT:  %20 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %21 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %22 = ThrowInst %21
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %23 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %24 = BinaryOperatorInst '<', %23, 10 : number
// CHECK-NEXT:  %25 = CondBranchInst %24, %BB5, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %26 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %27 = BinaryOperatorInst '<', %26, 10 : number
// CHECK-NEXT:  %28 = CondBranchInst %27, %BB5, %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %29 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %30 = AsNumericInst %29
// CHECK-NEXT:  %31 = UnaryOperatorInst '++', %30 : number|bigint
// CHECK-NEXT:  %32 = StoreFrameInst %31, [i#2], %0
// CHECK-NEXT:  %33 = BranchInst %BB8
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = LoadFrameInst [i#2], %0
// CHECK-NEXT:  %35 = AsNumericInst %34
// CHECK-NEXT:  %36 = UnaryOperatorInst '--', %35 : number|bigint
// CHECK-NEXT:  %37 = StoreFrameInst %36, [i#2], %0
// CHECK-NEXT:  %38 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = TryEndInst
// CHECK-NEXT:  %40 = BranchInst %BB3
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %41 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_finally_test#0#1()#3
// CHECK-NEXT:frame = [i#3, ?anon_0_e#3]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_try_catch_finally_test#0#1()#3}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#3], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#3], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 3 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#3], %0
// CHECK-NEXT:  %8 = ThrowInst %4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 4 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#3], %0
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CatchInst
// CHECK-NEXT:  %15 = StoreFrameInst %14, [?anon_0_e#3], %0
// CHECK-NEXT:  %16 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 2 : number
// CHECK-NEXT:  %18 = StoreFrameInst %17, [i#3], %0
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %22 = AsNumericInst %21
// CHECK-NEXT:  %23 = UnaryOperatorInst '++', %22 : number|bigint
// CHECK-NEXT:  %24 = StoreFrameInst %23, [i#3], %0
// CHECK-NEXT:  %25 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %28 = TryEndInst
// CHECK-NEXT:  %29 = LoadFrameInst [i#3], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %29, 3 : number
// CHECK-NEXT:  %31 = StoreFrameInst %30, [i#3], %0
// CHECK-NEXT:  %32 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_try_finally_test#0#1()#4
// CHECK-NEXT:frame = [i#4]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{simple_try_finally_test#0#1()#4}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#4], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#4], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 2 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#4], %0
// CHECK-NEXT:  %8 = ThrowInst %4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 3 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#4], %0
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %14 = AsNumericInst %13
// CHECK-NEXT:  %15 = UnaryOperatorInst '++', %14 : number|bigint
// CHECK-NEXT:  %16 = StoreFrameInst %15, [i#4], %0
// CHECK-NEXT:  %17 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = TryEndInst
// CHECK-NEXT:  %19 = LoadFrameInst [i#4], %0
// CHECK-NEXT:  %20 = BinaryOperatorInst '+', %19, 2 : number
// CHECK-NEXT:  %21 = StoreFrameInst %20, [i#4], %0
// CHECK-NEXT:  %22 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function try_catch_finally_with_return_test#0#1()#5
// CHECK-NEXT:frame = [i#5, ?anon_0_e#5]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{try_catch_finally_with_return_test#0#1()#5}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#5], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#5], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 3 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#5], %0
// CHECK-NEXT:  %8 = ReturnInst "c" : string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 4 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#5], %0
// CHECK-NEXT:  %12 = ReturnInst "d" : string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CatchInst
// CHECK-NEXT:  %15 = StoreFrameInst %14, [?anon_0_e#5], %0
// CHECK-NEXT:  %16 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 2 : number
// CHECK-NEXT:  %18 = StoreFrameInst %17, [i#5], %0
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %20 = BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %22 = AsNumericInst %21
// CHECK-NEXT:  %23 = UnaryOperatorInst '++', %22 : number|bigint
// CHECK-NEXT:  %24 = StoreFrameInst %23, [i#5], %0
// CHECK-NEXT:  %25 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %28 = TryEndInst
// CHECK-NEXT:  %29 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %29, 3 : number
// CHECK-NEXT:  %31 = StoreFrameInst %30, [i#5], %0
// CHECK-NEXT:  %32 = ReturnInst "c" : string
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %33 = ReturnInst "a" : string
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %34 = BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %35 = TryEndInst
// CHECK-NEXT:  %36 = BranchInst %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %39 = BinaryOperatorInst '+', %38, 3 : number
// CHECK-NEXT:  %40 = StoreFrameInst %39, [i#5], %0
// CHECK-NEXT:  %41 = ReturnInst "c" : string
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %42 = ReturnInst "b" : string
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %43 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %44 = TryEndInst
// CHECK-NEXT:  %45 = LoadFrameInst [i#5], %0
// CHECK-NEXT:  %46 = BinaryOperatorInst '+', %45, 3 : number
// CHECK-NEXT:  %47 = StoreFrameInst %46, [i#5], %0
// CHECK-NEXT:  %48 = ReturnInst "c" : string
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %49 = BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %50 = ThrowInst %4
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %51 = ReturnInst undefined : undefined
// CHECK-NEXT:function_end

// CHECK:function nested_try_test#0#1()#6
// CHECK-NEXT:frame = [i#6, ?anon_0_e#6, ?anon_1_e#6, ?anon_2_e#6]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nested_try_test#0#1()#6}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#6], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#6], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 3 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#6], %0
// CHECK-NEXT:  %8 = ThrowInst %4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 4 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#6], %0
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CatchInst
// CHECK-NEXT:  %15 = StoreFrameInst %14, [?anon_2_e#6], %0
// CHECK-NEXT:  %16 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 2 : number
// CHECK-NEXT:  %18 = StoreFrameInst %17, [i#6], %0
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %20 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %22 = AsNumericInst %21
// CHECK-NEXT:  %23 = UnaryOperatorInst '++', %22 : number|bigint
// CHECK-NEXT:  %24 = StoreFrameInst %23, [i#6], %0
// CHECK-NEXT:  %25 = TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %26 = CatchInst
// CHECK-NEXT:  %27 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %28 = BinaryOperatorInst '+', %27, 7 : number
// CHECK-NEXT:  %29 = StoreFrameInst %28, [i#6], %0
// CHECK-NEXT:  %30 = ThrowInst %26
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %31 = BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %32 = TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %33 = CatchInst
// CHECK-NEXT:  %34 = StoreFrameInst %33, [?anon_1_e#6], %0
// CHECK-NEXT:  %35 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %36 = BinaryOperatorInst '+', %35, 6 : number
// CHECK-NEXT:  %37 = StoreFrameInst %36, [i#6], %0
// CHECK-NEXT:  %38 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %39 = BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %40 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %41 = BinaryOperatorInst '+', %40, 5 : number
// CHECK-NEXT:  %42 = StoreFrameInst %41, [i#6], %0
// CHECK-NEXT:  %43 = TryStartInst %BB16, %BB17
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %44 = CatchInst
// CHECK-NEXT:  %45 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %46 = BinaryOperatorInst '+', %45, 10 : number
// CHECK-NEXT:  %47 = StoreFrameInst %46, [i#6], %0
// CHECK-NEXT:  %48 = ThrowInst %44
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %49 = BranchInst %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %50 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %51 = CatchInst
// CHECK-NEXT:  %52 = StoreFrameInst %51, [?anon_0_e#6], %0
// CHECK-NEXT:  %53 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %54 = BinaryOperatorInst '+', %53, 9 : number
// CHECK-NEXT:  %55 = StoreFrameInst %54, [i#6], %0
// CHECK-NEXT:  %56 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %57 = BranchInst %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %58 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %59 = BinaryOperatorInst '+', %58, 8 : number
// CHECK-NEXT:  %60 = StoreFrameInst %59, [i#6], %0
// CHECK-NEXT:  %61 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %62 = TryEndInst
// CHECK-NEXT:  %63 = BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %64 = TryEndInst
// CHECK-NEXT:  %65 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %66 = BinaryOperatorInst '+', %65, 10 : number
// CHECK-NEXT:  %67 = StoreFrameInst %66, [i#6], %0
// CHECK-NEXT:  %68 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %69 = TryEndInst
// CHECK-NEXT:  %70 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %71 = TryEndInst
// CHECK-NEXT:  %72 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %73 = BinaryOperatorInst '+', %72, 7 : number
// CHECK-NEXT:  %74 = StoreFrameInst %73, [i#6], %0
// CHECK-NEXT:  %75 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %76 = TryEndInst
// CHECK-NEXT:  %77 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %78 = TryEndInst
// CHECK-NEXT:  %79 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %80 = BinaryOperatorInst '+', %79, 3 : number
// CHECK-NEXT:  %81 = StoreFrameInst %80, [i#6], %0
// CHECK-NEXT:  %82 = ReturnInst "a" : string
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %83 = BranchInst %BB31
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %84 = TryEndInst
// CHECK-NEXT:  %85 = BranchInst %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %86 = TryEndInst
// CHECK-NEXT:  %87 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %88 = BinaryOperatorInst '+', %87, 10 : number
// CHECK-NEXT:  %89 = StoreFrameInst %88, [i#6], %0
// CHECK-NEXT:  %90 = BranchInst %BB18
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %91 = TryEndInst
// CHECK-NEXT:  %92 = BranchInst %BB14
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %93 = TryEndInst
// CHECK-NEXT:  %94 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %95 = BinaryOperatorInst '+', %94, 7 : number
// CHECK-NEXT:  %96 = StoreFrameInst %95, [i#6], %0
// CHECK-NEXT:  %97 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %98 = TryEndInst
// CHECK-NEXT:  %99 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %100 = TryEndInst
// CHECK-NEXT:  %101 = LoadFrameInst [i#6], %0
// CHECK-NEXT:  %102 = BinaryOperatorInst '+', %101, 3 : number
// CHECK-NEXT:  %103 = StoreFrameInst %102, [i#6], %0
// CHECK-NEXT:  %104 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function nested_catch_test#0#1()#7
// CHECK-NEXT:frame = [i#7, ?anon_0_e#7, ?anon_1_e#7, ?anon_2_e#7, ?anon_3_e#7]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{nested_catch_test#0#1()#7}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#7], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#7], %0
// CHECK-NEXT:  %3 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst
// CHECK-NEXT:  %5 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 8 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#7], %0
// CHECK-NEXT:  %8 = ThrowInst %4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '+', %9, 10 : number
// CHECK-NEXT:  %11 = StoreFrameInst %10, [i#7], %0
// CHECK-NEXT:  %12 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %13 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %14 = CatchInst
// CHECK-NEXT:  %15 = StoreFrameInst %14, [?anon_0_e#7], %0
// CHECK-NEXT:  %16 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %17 = BinaryOperatorInst '+', %16, 9 : number
// CHECK-NEXT:  %18 = StoreFrameInst %17, [i#7], %0
// CHECK-NEXT:  %19 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %20 = BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %21 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %22 = AsNumericInst %21
// CHECK-NEXT:  %23 = UnaryOperatorInst '++', %22 : number|bigint
// CHECK-NEXT:  %24 = StoreFrameInst %23, [i#7], %0
// CHECK-NEXT:  %25 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %26 = TryEndInst
// CHECK-NEXT:  %27 = BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %28 = CatchInst
// CHECK-NEXT:  %29 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %29, 4 : number
// CHECK-NEXT:  %31 = StoreFrameInst %30, [i#7], %0
// CHECK-NEXT:  %32 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %33 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %34 = TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %35 = CatchInst
// CHECK-NEXT:  %36 = StoreFrameInst %35, [?anon_1_e#7], %0
// CHECK-NEXT:  %37 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %38 = BinaryOperatorInst '+', %37, 3 : number
// CHECK-NEXT:  %39 = StoreFrameInst %38, [i#7], %0
// CHECK-NEXT:  %40 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %41 = BranchInst %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %42 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %43 = BinaryOperatorInst '+', %42, 2 : number
// CHECK-NEXT:  %44 = StoreFrameInst %43, [i#7], %0
// CHECK-NEXT:  %45 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %46 = TryEndInst
// CHECK-NEXT:  %47 = BranchInst %BB16
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %48 = TryEndInst
// CHECK-NEXT:  %49 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %50 = BinaryOperatorInst '+', %49, 4 : number
// CHECK-NEXT:  %51 = StoreFrameInst %50, [i#7], %0
// CHECK-NEXT:  %52 = TryStartInst %BB19, %BB20
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %53 = CatchInst
// CHECK-NEXT:  %54 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %55 = BinaryOperatorInst '+', %54, 7 : number
// CHECK-NEXT:  %56 = StoreFrameInst %55, [i#7], %0
// CHECK-NEXT:  %57 = ThrowInst %53
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %58 = BranchInst %BB13
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %59 = TryStartInst %BB22, %BB23
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %60 = CatchInst
// CHECK-NEXT:  %61 = StoreFrameInst %60, [?anon_2_e#7], %0
// CHECK-NEXT:  %62 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %63 = BinaryOperatorInst '+', %62, 6 : number
// CHECK-NEXT:  %64 = StoreFrameInst %63, [i#7], %0
// CHECK-NEXT:  %65 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %66 = BranchInst %BB25
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %67 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %68 = BinaryOperatorInst '+', %67, 5 : number
// CHECK-NEXT:  %69 = StoreFrameInst %68, [i#7], %0
// CHECK-NEXT:  %70 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %71 = TryEndInst
// CHECK-NEXT:  %72 = BranchInst %BB24
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %73 = TryEndInst
// CHECK-NEXT:  %74 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %75 = BinaryOperatorInst '+', %74, 7 : number
// CHECK-NEXT:  %76 = StoreFrameInst %75, [i#7], %0
// CHECK-NEXT:  %77 = BranchInst %BB21
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %78 = CatchInst
// CHECK-NEXT:  %79 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %80 = BinaryOperatorInst '+', %79, 7 : number
// CHECK-NEXT:  %81 = StoreFrameInst %80, [i#7], %0
// CHECK-NEXT:  %82 = ThrowInst %78
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %83 = ThrowInst %28
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %84 = TryStartInst %BB28, %BB29
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %85 = CatchInst
// CHECK-NEXT:  %86 = StoreFrameInst %85, [?anon_3_e#7], %0
// CHECK-NEXT:  %87 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %88 = BinaryOperatorInst '+', %87, 6 : number
// CHECK-NEXT:  %89 = StoreFrameInst %88, [i#7], %0
// CHECK-NEXT:  %90 = BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %91 = BranchInst %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %92 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %93 = BinaryOperatorInst '+', %92, 5 : number
// CHECK-NEXT:  %94 = StoreFrameInst %93, [i#7], %0
// CHECK-NEXT:  %95 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %96 = TryEndInst
// CHECK-NEXT:  %97 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %98 = TryEndInst
// CHECK-NEXT:  %99 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %100 = BinaryOperatorInst '+', %99, 7 : number
// CHECK-NEXT:  %101 = StoreFrameInst %100, [i#7], %0
// CHECK-NEXT:  %102 = BranchInst %BB27
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %103 = TryEndInst
// CHECK-NEXT:  %104 = LoadFrameInst [i#7], %0
// CHECK-NEXT:  %105 = BinaryOperatorInst '+', %104, 8 : number
// CHECK-NEXT:  %106 = StoreFrameInst %105, [i#7], %0
// CHECK-NEXT:  %107 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function finally_with_break_continue_test#0#1()#8
// CHECK-NEXT:frame = [i#8, ?anon_0_e#8]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst %S{finally_with_break_continue_test#0#1()#8}
// CHECK-NEXT:  %1 = StoreFrameInst undefined : undefined, [i#8], %0
// CHECK-NEXT:  %2 = StoreFrameInst 0 : number, [i#8], %0
// CHECK-NEXT:  %3 = BranchInst %BB1
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %4 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %5 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %6 = BinaryOperatorInst '+', %5, 4 : number
// CHECK-NEXT:  %7 = StoreFrameInst %6, [i#8], %0
// CHECK-NEXT:  %8 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %9 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %10 = BinaryOperatorInst '<', %9, 10 : number
// CHECK-NEXT:  %11 = CondBranchInst %10, %BB2, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %12 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %13 = BinaryOperatorInst '<', %12, 10 : number
// CHECK-NEXT:  %14 = CondBranchInst %13, %BB2, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %15 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %16 = AsNumericInst %15
// CHECK-NEXT:  %17 = UnaryOperatorInst '++', %16 : number|bigint
// CHECK-NEXT:  %18 = StoreFrameInst %17, [i#8], %0
// CHECK-NEXT:  %19 = BranchInst %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %20 = CatchInst
// CHECK-NEXT:  %21 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %22 = BinaryOperatorInst '+', %21, 3 : number
// CHECK-NEXT:  %23 = StoreFrameInst %22, [i#8], %0
// CHECK-NEXT:  %24 = ThrowInst %20
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %26 = TryStartInst %BB9, %BB10
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %27 = CatchInst
// CHECK-NEXT:  %28 = StoreFrameInst %27, [?anon_0_e#8], %0
// CHECK-NEXT:  %29 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %30 = BinaryOperatorInst '+', %29, 2 : number
// CHECK-NEXT:  %31 = StoreFrameInst %30, [i#8], %0
// CHECK-NEXT:  %32 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %33 = BinaryOperatorInst '==', %32, 3 : number
// CHECK-NEXT:  %34 = CondBranchInst %33, %BB11, %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %35 = BranchInst %BB14
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %36 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %37 = AsNumericInst %36
// CHECK-NEXT:  %38 = UnaryOperatorInst '++', %37 : number|bigint
// CHECK-NEXT:  %39 = StoreFrameInst %38, [i#8], %0
// CHECK-NEXT:  %40 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = TryEndInst
// CHECK-NEXT:  %42 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %43 = TryEndInst
// CHECK-NEXT:  %44 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %45 = BinaryOperatorInst '+', %44, 3 : number
// CHECK-NEXT:  %46 = StoreFrameInst %45, [i#8], %0
// CHECK-NEXT:  %47 = BranchInst %BB5
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %48 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %49 = TryEndInst
// CHECK-NEXT:  %50 = BranchInst %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %51 = BranchInst %BB19
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %52 = BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %53 = BranchInst %BB21
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %54 = TryEndInst
// CHECK-NEXT:  %55 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %56 = BinaryOperatorInst '+', %55, 3 : number
// CHECK-NEXT:  %57 = StoreFrameInst %56, [i#8], %0
// CHECK-NEXT:  %58 = ReturnInst undefined : undefined
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %59 = BranchInst %BB20
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %60 = TryEndInst
// CHECK-NEXT:  %61 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %62 = BinaryOperatorInst '+', %61, 3 : number
// CHECK-NEXT:  %63 = StoreFrameInst %62, [i#8], %0
// CHECK-NEXT:  %64 = BranchInst %BB7
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %65 = BranchInst %BB13
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %66 = TryEndInst
// CHECK-NEXT:  %67 = LoadFrameInst [i#8], %0
// CHECK-NEXT:  %68 = BinaryOperatorInst '+', %67, 3 : number
// CHECK-NEXT:  %69 = StoreFrameInst %68, [i#8], %0
// CHECK-NEXT:  %70 = BranchInst %BB8
// CHECK-NEXT:function_end
