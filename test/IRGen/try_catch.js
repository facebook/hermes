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
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_catch_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_catch_finally_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_finally_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "try_catch_finally_with_return_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_try_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_catch_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "finally_with_break_continue_test": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %simple_try_catch_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "simple_try_catch_test": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %simple_try_catch_finally_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "simple_try_catch_finally_test": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %simple_try_finally_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "simple_try_finally_test": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %try_catch_finally_with_return_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "try_catch_finally_with_return_test": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %nested_try_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "nested_try_test": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %nested_catch_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "nested_catch_test": string
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %finally_with_break_continue_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %19: object, globalObject: object, "finally_with_break_continue_test": string
// CHECK-NEXT:  %21 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %21: any
// CHECK-NEXT:  %23 = LoadStackInst (:any) %21: any
// CHECK-NEXT:        ReturnInst %23: any
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %2 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %2: any, [e]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [e]: any
// CHECK-NEXT:  %5 = AsNumericInst (:number|bigint) %4: any
// CHECK-NEXT:  %6 = UnaryIncInst (:number|bigint) %5: number|bigint
// CHECK-NEXT:       StoreFrameInst %6: number|bigint, [e]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinarySubtractInst (:any) %8: any, 3: number
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:        ReturnInst %12: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryLessThanInst (:boolean) %15: any, 10: number
// CHECK-NEXT:        CondBranchInst %16: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 2: number
// CHECK-NEXT:        StoreFrameInst %19: any, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:        ThrowInst %22: any
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %25 = BinaryLessThanInst (:boolean) %24: any, 10: number
// CHECK-NEXT:        CondBranchInst %25: boolean, %BB4, %BB5
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %28 = AsNumericInst (:number|bigint) %27: any
// CHECK-NEXT:  %29 = UnaryIncInst (:number|bigint) %28: number|bigint
// CHECK-NEXT:        StoreFrameInst %29: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_finally_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:       StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %13: any, [e]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 2: number
// CHECK-NEXT:        StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:        StoreFrameInst %22: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 3: number
// CHECK-NEXT:        StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function simple_try_finally_test(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 2: number
// CHECK-NEXT:       StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 3: number
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %13 = AsNumericInst (:number|bigint) %12: any
// CHECK-NEXT:  %14 = UnaryIncInst (:number|bigint) %13: number|bigint
// CHECK-NEXT:        StoreFrameInst %14: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %18 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %19 = BinaryAddInst (:any) %18: any, 2: number
// CHECK-NEXT:        StoreFrameInst %19: any, [i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function try_catch_finally_with_return_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:       StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:       ReturnInst "c": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %9: any, [e]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %12 = BinaryAddInst (:any) %11: any, 2: number
// CHECK-NEXT:        StoreFrameInst %12: any, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = AsNumericInst (:number|bigint) %15: any
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: number|bigint
// CHECK-NEXT:        StoreFrameInst %17: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %23 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %24 = BinaryAddInst (:any) %23: any, 3: number
// CHECK-NEXT:        StoreFrameInst %24: any, [i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 3: number
// CHECK-NEXT:        StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:function_end

// CHECK:function nested_try_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 3: number
// CHECK-NEXT:       StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %13: any, [e#2]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 2: number
// CHECK-NEXT:        StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:        StoreFrameInst %22: number|bigint, [i]: any
// CHECK-NEXT:        TryStartInst %BB8, %BB9
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %25 = CatchInst (:any)
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %26: any, 7: number
// CHECK-NEXT:        StoreFrameInst %27: any, [i]: any
// CHECK-NEXT:        ThrowInst %25: any
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryStartInst %BB12, %BB13
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %32 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %32: any, [e#1]: any
// CHECK-NEXT:  %34 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %35 = BinaryAddInst (:any) %34: any, 6: number
// CHECK-NEXT:        StoreFrameInst %35: any, [i]: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %39 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %40 = BinaryAddInst (:any) %39: any, 5: number
// CHECK-NEXT:        StoreFrameInst %40: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB16, %BB17
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %43 = CatchInst (:any)
// CHECK-NEXT:  %44 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %45 = BinaryAddInst (:any) %44: any, 10: number
// CHECK-NEXT:        StoreFrameInst %45: any, [i]: any
// CHECK-NEXT:        ThrowInst %43: any
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryStartInst %BB20, %BB21
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %50 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %50: any, [e]: any
// CHECK-NEXT:  %52 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %53 = BinaryAddInst (:any) %52: any, 9: number
// CHECK-NEXT:        StoreFrameInst %53: any, [i]: any
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %57 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %58 = BinaryAddInst (:any) %57: any, 8: number
// CHECK-NEXT:        StoreFrameInst %58: any, [i]: any
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %64 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %65 = BinaryAddInst (:any) %64: any, 10: number
// CHECK-NEXT:        StoreFrameInst %65: any, [i]: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %71 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %72 = BinaryAddInst (:any) %71: any, 7: number
// CHECK-NEXT:        StoreFrameInst %72: any, [i]: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB29
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %79 = BinaryAddInst (:any) %78: any, 3: number
// CHECK-NEXT:        StoreFrameInst %79: any, [i]: any
// CHECK-NEXT:        ReturnInst "a": string
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %83 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %84 = BinaryAddInst (:any) %83: any, 10: number
// CHECK-NEXT:        StoreFrameInst %84: any, [i]: any
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %90 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %91 = BinaryAddInst (:any) %90: any, 7: number
// CHECK-NEXT:        StoreFrameInst %91: any, [i]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %97 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %98 = BinaryAddInst (:any) %97: any, 3: number
// CHECK-NEXT:        StoreFrameInst %98: any, [i]: any
// CHECK-NEXT:         BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function nested_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %3 = CatchInst (:any)
// CHECK-NEXT:  %4 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: any, 8: number
// CHECK-NEXT:       StoreFrameInst %5: any, [i]: any
// CHECK-NEXT:       ThrowInst %3: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 10: number
// CHECK-NEXT:        StoreFrameInst %9: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %13 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %13: any, [e]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 9: number
// CHECK-NEXT:        StoreFrameInst %16: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB6, %BB7
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %21 = AsNumericInst (:number|bigint) %20: any
// CHECK-NEXT:  %22 = UnaryIncInst (:number|bigint) %21: number|bigint
// CHECK-NEXT:        StoreFrameInst %22: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:  %28 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 4: number
// CHECK-NEXT:        StoreFrameInst %29: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB11, %BB12
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryStartInst %BB14, %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %34 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %34: any, [e#1]: any
// CHECK-NEXT:  %36 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %37 = BinaryAddInst (:any) %36: any, 3: number
// CHECK-NEXT:        StoreFrameInst %37: any, [i]: any
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB16:
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %41 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %42 = BinaryAddInst (:any) %41: any, 2: number
// CHECK-NEXT:        StoreFrameInst %42: any, [i]: any
// CHECK-NEXT:        BranchInst %BB18
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB16
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %48 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %49 = BinaryAddInst (:any) %48: any, 4: number
// CHECK-NEXT:        StoreFrameInst %49: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB19, %BB20
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %52 = CatchInst (:any)
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %54 = BinaryAddInst (:any) %53: any, 7: number
// CHECK-NEXT:        StoreFrameInst %54: any, [i]: any
// CHECK-NEXT:        ThrowInst %52: any
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        TryStartInst %BB22, %BB23
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %59 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %59: any, [e#2]: any
// CHECK-NEXT:  %61 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %62 = BinaryAddInst (:any) %61: any, 6: number
// CHECK-NEXT:        StoreFrameInst %62: any, [i]: any
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %66 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %67 = BinaryAddInst (:any) %66: any, 5: number
// CHECK-NEXT:        StoreFrameInst %67: any, [i]: any
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %73 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %74 = BinaryAddInst (:any) %73: any, 7: number
// CHECK-NEXT:        StoreFrameInst %74: any, [i]: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %77 = CatchInst (:any)
// CHECK-NEXT:  %78 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %79 = BinaryAddInst (:any) %78: any, 7: number
// CHECK-NEXT:        StoreFrameInst %79: any, [i]: any
// CHECK-NEXT:        ThrowInst %77: any
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        ThrowInst %27: any
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryStartInst %BB28, %BB29
// CHECK-NEXT:%BB28:
// CHECK-NEXT:  %84 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %84: any, [e#2]: any
// CHECK-NEXT:  %86 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %87 = BinaryAddInst (:any) %86: any, 6: number
// CHECK-NEXT:        StoreFrameInst %87: any, [i]: any
// CHECK-NEXT:        BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:        BranchInst %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %91 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %92 = BinaryAddInst (:any) %91: any, 5: number
// CHECK-NEXT:        StoreFrameInst %92: any, [i]: any
// CHECK-NEXT:        BranchInst %BB32
// CHECK-NEXT:%BB32:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB30
// CHECK-NEXT:%BB31:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %98 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %99 = BinaryAddInst (:any) %98: any, 7: number
// CHECK-NEXT:         StoreFrameInst %99: any, [i]: any
// CHECK-NEXT:         BranchInst %BB27
// CHECK-NEXT:%BB9:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:  %103 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %104 = BinaryAddInst (:any) %103: any, 8: number
// CHECK-NEXT:         StoreFrameInst %104: any, [i]: any
// CHECK-NEXT:         BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:function finally_with_break_continue_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       StoreFrameInst undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst 0: number, [i]: any
// CHECK-NEXT:  %2 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %3 = BinaryLessThanInst (:boolean) %2: any, 10: number
// CHECK-NEXT:       CondBranchInst %3: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 4: number
// CHECK-NEXT:       StoreFrameInst %7: any, [i]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB1, %BB2
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %14 = AsNumericInst (:number|bigint) %13: any
// CHECK-NEXT:  %15 = UnaryIncInst (:number|bigint) %14: number|bigint
// CHECK-NEXT:        StoreFrameInst %15: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %18 = CatchInst (:any)
// CHECK-NEXT:  %19 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %20 = BinaryAddInst (:any) %19: any, 3: number
// CHECK-NEXT:        StoreFrameInst %20: any, [i]: any
// CHECK-NEXT:        ThrowInst %18: any
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %24 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %24: any, [e]: any
// CHECK-NEXT:  %26 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %27 = BinaryAddInst (:any) %26: any, 2: number
// CHECK-NEXT:        StoreFrameInst %27: any, [i]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %30 = BinaryEqualInst (:boolean) %29: any, 3: number
// CHECK-NEXT:        CondBranchInst %30: boolean, %BB9, %BB10
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %32 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %33 = AsNumericInst (:number|bigint) %32: any
// CHECK-NEXT:  %34 = UnaryIncInst (:number|bigint) %33: number|bigint
// CHECK-NEXT:        StoreFrameInst %34: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %40 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %41 = BinaryAddInst (:any) %40: any, 3: number
// CHECK-NEXT:        StoreFrameInst %41: any, [i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %48 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %49 = BinaryAddInst (:any) %48: any, 3: number
// CHECK-NEXT:        StoreFrameInst %49: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %53 = LoadFrameInst (:any) [i]: any
// CHECK-NEXT:  %54 = BinaryAddInst (:any) %53: any, 3: number
// CHECK-NEXT:        StoreFrameInst %54: any, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end
