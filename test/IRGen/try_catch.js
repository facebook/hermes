/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

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

// CHECK:function global(): any
// CHECK-NEXT:frame = []
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = DeclareGlobalVarInst "simple_try_catch_test": string
// CHECK-NEXT:  %1 = DeclareGlobalVarInst "simple_try_catch_finally_test": string
// CHECK-NEXT:  %2 = DeclareGlobalVarInst "simple_try_finally_test": string
// CHECK-NEXT:  %3 = DeclareGlobalVarInst "try_catch_finally_with_return_test": string
// CHECK-NEXT:  %4 = DeclareGlobalVarInst "nested_try_test": string
// CHECK-NEXT:  %5 = DeclareGlobalVarInst "nested_catch_test": string
// CHECK-NEXT:  %6 = DeclareGlobalVarInst "finally_with_break_continue_test": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:closure) %simple_try_catch_test(): any
// CHECK-NEXT:  %8 = StorePropertyLooseInst %7: closure, globalObject: object, "simple_try_catch_test": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:closure) %simple_try_catch_finally_test(): any
// CHECK-NEXT:  %10 = StorePropertyLooseInst %9: closure, globalObject: object, "simple_try_catch_finally_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:closure) %simple_try_finally_test(): any
// CHECK-NEXT:  %12 = StorePropertyLooseInst %11: closure, globalObject: object, "simple_try_finally_test": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:closure) %try_catch_finally_with_return_test(): any
// CHECK-NEXT:  %14 = StorePropertyLooseInst %13: closure, globalObject: object, "try_catch_finally_with_return_test": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:closure) %nested_try_test(): any
// CHECK-NEXT:  %16 = StorePropertyLooseInst %15: closure, globalObject: object, "nested_try_test": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:closure) %nested_catch_test(): any
// CHECK-NEXT:  %18 = StorePropertyLooseInst %17: closure, globalObject: object, "nested_catch_test": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:closure) %finally_with_break_continue_test(): any
// CHECK-NEXT:  %20 = StorePropertyLooseInst %19: closure, globalObject: object, "finally_with_break_continue_test": string
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %22 = StoreStackInst undefined: undefined, %21: any
// CHECK-NEXT:  %23 = LoadStackInst (:any) %21: any
// CHECK-NEXT:  %24 = ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = CatchInst (:any)
// CHECK-NEXT:  %3 = StoreFrameInst %2: any, [e]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [e]: any
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = UnaryIncInst (:any) %5: number|bigint
// CHECK-NEXT:  %7 = StoreFrameInst %6: any, [e]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinarySubtractInst (:any) %8: any, 3: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = ReturnInst %12: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryLessThanInst (:any) %15: any, 10: number
// CHECK-NEXT:  %17 = CondBranchInst %16: any, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 2: number
// CHECK-NEXT:  %20 = StoreFrameInst %19: any, [i]: any
// CHECK-NEXT:  %21 = BranchInst %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %23 = ThrowInst %22: any
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %25 = BinaryLessThanInst (:any) %24: any, 10: number
// CHECK-NEXT:  %26 = CondBranchInst %25: any, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %28 = AsNumericInst (:number|bigint) %27: any
// CHECK-NEXT:  %29 = UnaryIncInst (:any) %28: number|bigint
// CHECK-NEXT:  %30 = StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:  %31 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %33 = AsNumericInst (:number|bigint) %32: any
// CHECK-NEXT:  %34 = UnaryDecInst (:any) %33: number|bigint
// CHECK-NEXT:  %35 = StoreFrameInst %34: any, [i]: any
// CHECK-NEXT:  %36 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %37 = TryEndInst
// CHECK-NEXT:  %38 = BranchInst %BB3
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %39 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_finally_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:  %7 = ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:  %14 = StoreFrameInst %13: any, [e]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 2: number
// CHECK-NEXT:  %17 = StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:any) %21: number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22: any, [i]: any
// CHECK-NEXT:  %24 = BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 3: number
// CHECK-NEXT:  %30 = StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:  %31 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_try_finally_test(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 2: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:  %7 = ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 3: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = AsNumericInst (:number|bigint) %12: any
// CHECK-NEXT:  %14 = UnaryIncInst (:any) %13: number|bigint
// CHECK-NEXT:  %15 = StoreFrameInst %14: any, [i]: any
// CHECK-NEXT:  %16 = BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = TryEndInst
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 2: number
// CHECK-NEXT:  %20 = StoreFrameInst %19: any, [i]: any
// CHECK-NEXT:  %21 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function try_catch_finally_with_return_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:  %7 = ReturnInst "c": string
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = ReturnInst "d": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:  %14 = StoreFrameInst %13: any, [e]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 2: number
// CHECK-NEXT:  %17 = StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %19 = BranchInst %BB8
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:any) %21: number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22: any, [i]: any
// CHECK-NEXT:  %24 = BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %27 = TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 3: number
// CHECK-NEXT:  %30 = StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:  %31 = ReturnInst "c": string
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %32 = ReturnInst "a": string
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %33 = BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %34 = TryEndInst
// CHECK-NEXT:  %35 = BranchInst %BB7
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %36 = TryEndInst
// CHECK-NEXT:  %37 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %38 = BinaryAddInst (:any) %37: any, 3: number
// CHECK-NEXT:  %39 = StoreFrameInst %38: any, [i]: any
// CHECK-NEXT:  %40 = ReturnInst "c": string
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %41 = ReturnInst "b": string
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %42 = BranchInst %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %43 = TryEndInst
// CHECK-NEXT:  %44 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %45 = BinaryAddInst (:any) %44: any, 3: number
// CHECK-NEXT:  %46 = StoreFrameInst %45: any, [i]: any
// CHECK-NEXT:  %47 = ReturnInst "c": string
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %48 = BranchInst %BB3
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = ThrowInst %3: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %50 = ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function nested_try_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:  %7 = ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:  %14 = StoreFrameInst %13: any, [e#2]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 2: number
// CHECK-NEXT:  %17 = StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:  %18 = BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %19 = BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:any) %21: number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22: any, [i]: any
// CHECK-NEXT:  %24 = TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = CatchInst (:any)
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %26: any, 7: number
// CHECK-NEXT:  %28 = StoreFrameInst %27: any, [i]: any
// CHECK-NEXT:  %29 = ThrowInst %25: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %30 = BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %31 = TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %32 = CatchInst (:any)
// CHECK-NEXT:  %33 = StoreFrameInst %32: any, [e#1]: any
// CHECK-NEXT:  %34 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %35 = BinaryAddInst (:any) %34: any, 6: number
// CHECK-NEXT:  %36 = StoreFrameInst %35: any, [i]: any
// CHECK-NEXT:  %37 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %38 = BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %39 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %40 = BinaryAddInst (:any) %39: any, 5: number
// CHECK-NEXT:  %41 = StoreFrameInst %40: any, [i]: any
// CHECK-NEXT:  %42 = TryStartInst %BB16, %BB17
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %43 = CatchInst (:any)
// CHECK-NEXT:  %44 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %45 = BinaryAddInst (:any) %44: any, 10: number
// CHECK-NEXT:  %46 = StoreFrameInst %45: any, [i]: any
// CHECK-NEXT:  %47 = ThrowInst %43: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %48 = BranchInst %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %49 = TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %50 = CatchInst (:any)
// CHECK-NEXT:  %51 = StoreFrameInst %50: any, [e]: any
// CHECK-NEXT:  %52 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %53 = BinaryAddInst (:any) %52: any, 9: number
// CHECK-NEXT:  %54 = StoreFrameInst %53: any, [i]: any
// CHECK-NEXT:  %55 = BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %56 = BranchInst %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %58 = BinaryAddInst (:any) %57: any, 8: number
// CHECK-NEXT:  %59 = StoreFrameInst %58: any, [i]: any
// CHECK-NEXT:  %60 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %61 = TryEndInst
// CHECK-NEXT:  %62 = BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %63 = TryEndInst
// CHECK-NEXT:  %64 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %65 = BinaryAddInst (:any) %64: any, 10: number
// CHECK-NEXT:  %66 = StoreFrameInst %65: any, [i]: any
// CHECK-NEXT:  %67 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %68 = TryEndInst
// CHECK-NEXT:  %69 = BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %70 = TryEndInst
// CHECK-NEXT:  %71 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %72 = BinaryAddInst (:any) %71: any, 7: number
// CHECK-NEXT:  %73 = StoreFrameInst %72: any, [i]: any
// CHECK-NEXT:  %74 = BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %75 = TryEndInst
// CHECK-NEXT:  %76 = BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %77 = TryEndInst
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %79 = BinaryAddInst (:any) %78: any, 3: number
// CHECK-NEXT:  %80 = StoreFrameInst %79: any, [i]: any
// CHECK-NEXT:  %81 = ReturnInst "a": string
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %82 = BranchInst %BB31
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %83 = TryEndInst
// CHECK-NEXT:  %84 = BranchInst %BB22
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %85 = TryEndInst
// CHECK-NEXT:  %86 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %87 = BinaryAddInst (:any) %86: any, 10: number
// CHECK-NEXT:  %88 = StoreFrameInst %87: any, [i]: any
// CHECK-NEXT:  %89 = BranchInst %BB18
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %90 = TryEndInst
// CHECK-NEXT:  %91 = BranchInst %BB14
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %92 = TryEndInst
// CHECK-NEXT:  %93 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %94 = BinaryAddInst (:any) %93: any, 7: number
// CHECK-NEXT:  %95 = StoreFrameInst %94: any, [i]: any
// CHECK-NEXT:  %96 = BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %97 = TryEndInst
// CHECK-NEXT:  %98 = BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %99 = TryEndInst
// CHECK-NEXT:  %100 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %101 = BinaryAddInst (:any) %100: any, 3: number
// CHECK-NEXT:  %102 = StoreFrameInst %101: any, [i]: any
// CHECK-NEXT:  %103 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function nested_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 8: number
// CHECK-NEXT:  %6 = StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:  %7 = ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 10: number
// CHECK-NEXT:  %10 = StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:  %11 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:  %14 = StoreFrameInst %13: any, [e]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 9: number
// CHECK-NEXT:  %17 = StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:  %18 = TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %19 = BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:any) %21: number|bigint
// CHECK-NEXT:  %23 = StoreFrameInst %22: any, [i]: any
// CHECK-NEXT:  %24 = BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %25 = TryEndInst
// CHECK-NEXT:  %26 = BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 4: number
// CHECK-NEXT:  %30 = StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:  %31 = TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %32 = BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %33 = TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = CatchInst (:any)
// CHECK-NEXT:  %35 = StoreFrameInst %34: any, [e#1]: any
// CHECK-NEXT:  %36 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %37 = BinaryAddInst (:any) %36: any, 3: number
// CHECK-NEXT:  %38 = StoreFrameInst %37: any, [i]: any
// CHECK-NEXT:  %39 = BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %40 = BranchInst %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %42 = BinaryAddInst (:any) %41: any, 2: number
// CHECK-NEXT:  %43 = StoreFrameInst %42: any, [i]: any
// CHECK-NEXT:  %44 = BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %45 = TryEndInst
// CHECK-NEXT:  %46 = BranchInst %BB16
// CHECK-NEXT:%BB17:
// CHECK-NEXT:  %47 = TryEndInst
// CHECK-NEXT:  %48 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %49 = BinaryAddInst (:any) %48: any, 4: number
// CHECK-NEXT:  %50 = StoreFrameInst %49: any, [i]: any
// CHECK-NEXT:  %51 = TryStartInst %BB19, %BB20
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %52 = CatchInst (:any)
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %54 = BinaryAddInst (:any) %53: any, 7: number
// CHECK-NEXT:  %55 = StoreFrameInst %54: any, [i]: any
// CHECK-NEXT:  %56 = ThrowInst %52: any
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = BranchInst %BB13
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %58 = TryStartInst %BB22, %BB23
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %59 = CatchInst (:any)
// CHECK-NEXT:  %60 = StoreFrameInst %59: any, [e#2]: any
// CHECK-NEXT:  %61 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %62 = BinaryAddInst (:any) %61: any, 6: number
// CHECK-NEXT:  %63 = StoreFrameInst %62: any, [i]: any
// CHECK-NEXT:  %64 = BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %65 = BranchInst %BB25
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %66 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %67 = BinaryAddInst (:any) %66: any, 5: number
// CHECK-NEXT:  %68 = StoreFrameInst %67: any, [i]: any
// CHECK-NEXT:  %69 = BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:  %70 = TryEndInst
// CHECK-NEXT:  %71 = BranchInst %BB24
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %72 = TryEndInst
// CHECK-NEXT:  %73 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %74 = BinaryAddInst (:any) %73: any, 7: number
// CHECK-NEXT:  %75 = StoreFrameInst %74: any, [i]: any
// CHECK-NEXT:  %76 = BranchInst %BB21
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %77 = CatchInst (:any)
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %79 = BinaryAddInst (:any) %78: any, 7: number
// CHECK-NEXT:  %80 = StoreFrameInst %79: any, [i]: any
// CHECK-NEXT:  %81 = ThrowInst %77: any
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %82 = ThrowInst %27: any
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %83 = TryStartInst %BB28, %BB29
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %84 = CatchInst (:any)
// CHECK-NEXT:  %85 = StoreFrameInst %84: any, [e#2]: any
// CHECK-NEXT:  %86 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %87 = BinaryAddInst (:any) %86: any, 6: number
// CHECK-NEXT:  %88 = StoreFrameInst %87: any, [i]: any
// CHECK-NEXT:  %89 = BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:  %90 = BranchInst %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %91 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %92 = BinaryAddInst (:any) %91: any, 5: number
// CHECK-NEXT:  %93 = StoreFrameInst %92: any, [i]: any
// CHECK-NEXT:  %94 = BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %95 = TryEndInst
// CHECK-NEXT:  %96 = BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %97 = TryEndInst
// CHECK-NEXT:  %98 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %99 = BinaryAddInst (:any) %98: any, 7: number
// CHECK-NEXT:  %100 = StoreFrameInst %99: any, [i]: any
// CHECK-NEXT:  %101 = BranchInst %BB27
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %102 = TryEndInst
// CHECK-NEXT:  %103 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %104 = BinaryAddInst (:any) %103: any, 8: number
// CHECK-NEXT:  %105 = StoreFrameInst %104: any, [i]: any
// CHECK-NEXT:  %106 = BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function finally_with_break_continue_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:  %1 = StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:any) %2: any, 10: number
// CHECK-NEXT:  %4 = CondBranchInst %3: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 4: number
// CHECK-NEXT:  %8 = StoreFrameInst %7: any, [i]: any
// CHECK-NEXT:  %9 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:any) %10: any, 10: number
// CHECK-NEXT:  %12 = CondBranchInst %11: any, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = AsNumericInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = UnaryIncInst (:any) %14: number|bigint
// CHECK-NEXT:  %16 = StoreFrameInst %15: any, [i]: any
// CHECK-NEXT:  %17 = BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = CatchInst (:any)
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %20 = BinaryAddInst (:any) %19: any, 3: number
// CHECK-NEXT:  %21 = StoreFrameInst %20: any, [i]: any
// CHECK-NEXT:  %22 = ThrowInst %18: any
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %23 = BranchInst %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %24 = TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = CatchInst (:any)
// CHECK-NEXT:  %26 = StoreFrameInst %25: any, [e]: any
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %28 = BinaryAddInst (:any) %27: any, 2: number
// CHECK-NEXT:  %29 = StoreFrameInst %28: any, [i]: any
// CHECK-NEXT:  %30 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %31 = BinaryEqualInst (:any) %30: any, 3: number
// CHECK-NEXT:  %32 = CondBranchInst %31: any, %BB10, %BB11
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %33 = BranchInst %BB13
// CHECK-NEXT:%BB9:
// CHECK-NEXT:  %34 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %35 = AsNumericInst (:number|bigint) %34: any
// CHECK-NEXT:  %36 = UnaryIncInst (:any) %35: number|bigint
// CHECK-NEXT:  %37 = StoreFrameInst %36: any, [i]: any
// CHECK-NEXT:  %38 = BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %39 = TryEndInst
// CHECK-NEXT:  %40 = BranchInst %BB15
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = TryEndInst
// CHECK-NEXT:  %42 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %43 = BinaryAddInst (:any) %42: any, 3: number
// CHECK-NEXT:  %44 = StoreFrameInst %43: any, [i]: any
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
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %54 = BinaryAddInst (:any) %53: any, 3: number
// CHECK-NEXT:  %55 = StoreFrameInst %54: any, [i]: any
// CHECK-NEXT:  %56 = ReturnInst undefined: undefined
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = BranchInst %BB19
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %58 = TryEndInst
// CHECK-NEXT:  %59 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %60 = BinaryAddInst (:any) %59: any, 3: number
// CHECK-NEXT:  %61 = StoreFrameInst %60: any, [i]: any
// CHECK-NEXT:  %62 = BranchInst %BB6
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %63 = BranchInst %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %64 = TryEndInst
// CHECK-NEXT:  %65 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %66 = BinaryAddInst (:any) %65: any, 3: number
// CHECK-NEXT:  %67 = StoreFrameInst %66: any, [i]: any
// CHECK-NEXT:  %68 = BranchInst %BB7
// CHECK-NEXT:function_end
