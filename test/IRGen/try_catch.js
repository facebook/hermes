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

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_catch_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_catch_finally_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_try_finally_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "try_catch_finally_with_return_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_try_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "nested_catch_test": string
// CHECK-NEXT:       DeclareGlobalVarInst "finally_with_break_continue_test": string
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %0: environment, %simple_try_catch_test(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %8: object, globalObject: object, "simple_try_catch_test": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %simple_try_catch_finally_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "simple_try_catch_finally_test": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %simple_try_finally_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "simple_try_finally_test": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %try_catch_finally_with_return_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "try_catch_finally_with_return_test": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %nested_try_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "nested_try_test": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %nested_catch_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "nested_catch_test": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %finally_with_break_continue_test(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "finally_with_break_continue_test": string
// CHECK-NEXT:  %22 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %22: any
// CHECK-NEXT:  %24 = LoadStackInst (:any) %22: any
// CHECK-NEXT:        ReturnInst %24: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [i: any, e: any]

// CHECK:function simple_try_catch_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS1.e]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS1.e]: any
// CHECK-NEXT:  %7 = AsNumericInst (:number|bigint) %6: any
// CHECK-NEXT:  %8 = UnaryIncInst (:number|bigint) %7: number|bigint
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: number|bigint, [%VS1.e]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:  %11 = BinarySubtractInst (:any) %10: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS1.i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %1: environment, 0: number, [%VS1.i]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:  %18 = BinaryLessThanInst (:boolean) %17: any, 10: number
// CHECK-NEXT:        CondBranchInst %18: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %20: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [%VS1.i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:        ThrowInst %24: any, %BB1
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:  %27 = BinaryLessThanInst (:boolean) %26: any, 10: number
// CHECK-NEXT:        CondBranchInst %27: boolean, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [%VS1.i]: any
// CHECK-NEXT:  %30 = AsNumericInst (:number|bigint) %29: any
// CHECK-NEXT:  %31 = UnaryIncInst (:number|bigint) %30: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: number|bigint, [%VS1.i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: any, e: any]

// CHECK:function simple_try_catch_finally_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS2.i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS2.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [%VS2.e]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [%VS2.i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst %BB1, %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [%VS2.i]: any
// CHECK-NEXT:        TryEndInst %BB4, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: any, [%VS2.i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [i: any]

// CHECK:function simple_try_finally_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS3.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS3.i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 2: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS3.i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS3.i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS3.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS3.i]: any
// CHECK-NEXT:  %15 = AsNumericInst (:number|bigint) %14: any
// CHECK-NEXT:  %16 = UnaryIncInst (:number|bigint) %15: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: number|bigint, [%VS3.i]: any
// CHECK-NEXT:        TryEndInst %BB1, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS3.i]: any
// CHECK-NEXT:  %20 = BinaryAddInst (:any) %19: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: any, [%VS3.i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [i: any, e: any]

// CHECK:function try_catch_finally_with_return_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS4.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS4.i]: any
// CHECK-NEXT:       ReturnInst "c": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS4.e]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) %13: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [%VS4.i]: any
// CHECK-NEXT:        TryEndInst %BB1, %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %18 = AsNumericInst (:number|bigint) %17: any
// CHECK-NEXT:  %19 = UnaryIncInst (:number|bigint) %18: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: number|bigint, [%VS4.i]: any
// CHECK-NEXT:        TryEndInst %BB3, %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst %BB1, %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %24 = BinaryAddInst (:any) %23: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: any, [%VS4.i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %28 = BinaryAddInst (:any) %27: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %28: any, [%VS4.i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [i: any, e: any, e#1: any, e#2: any]

// CHECK:function nested_try_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS5.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS5.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS5.i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS5.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [%VS5.e#2]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst %BB1, %BB29
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [%VS5.i]: any
// CHECK-NEXT:        TryStartInst %BB7, %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: any, [%VS5.i]: any
// CHECK-NEXT:        ThrowInst %27: any, %BB4
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst %BB4, %BB28
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryStartInst %BB10, %BB12
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: any, [%VS5.e#1]: any
// CHECK-NEXT:  %36 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %37 = BinaryAddInst (:any) %36: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %37: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst %BB7, %BB27
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %41 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %42 = BinaryAddInst (:any) %41: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %42: any, [%VS5.i]: any
// CHECK-NEXT:        TryStartInst %BB13, %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = CatchInst (:any)
// CHECK-NEXT:  %46 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %47 = BinaryAddInst (:any) %46: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %47: any, [%VS5.i]: any
// CHECK-NEXT:        ThrowInst %45: any, %BB10
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryEndInst %BB10, %BB26
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %52 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %52: any, [%VS5.e]: any
// CHECK-NEXT:  %54 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %55 = BinaryAddInst (:any) %54: any, 9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %55: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        TryEndInst %BB13, %BB25
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %59 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %60 = BinaryAddInst (:any) %59: any, 8: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %60: any, [%VS5.i]: any
// CHECK-NEXT:        TryEndInst %BB16, %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        TryEndInst %BB13, %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:  %64 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %65 = BinaryAddInst (:any) %64: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %65: any, [%VS5.i]: any
// CHECK-NEXT:        TryEndInst %BB10, %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        TryEndInst %BB7, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:  %69 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %70 = BinaryAddInst (:any) %69: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %70: any, [%VS5.i]: any
// CHECK-NEXT:        TryEndInst %BB4, %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        TryEndInst %BB1, %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %74 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %75 = BinaryAddInst (:any) %74: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %75: any, [%VS5.i]: any
// CHECK-NEXT:        ReturnInst "a": string
// CHECK-NEXT:%BB25:
// CHECK-NEXT:  %78 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %79 = BinaryAddInst (:any) %78: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %79: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %83 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %84 = BinaryAddInst (:any) %83: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %84: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %88 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %89 = BinaryAddInst (:any) %88: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %89: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [i: any, e: any, e#1: any, e#2: any]

// CHECK:function nested_catch_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS6.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS6.i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 8: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [%VS6.i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS6.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [%VS6.e]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [%VS6.i]: any
// CHECK-NEXT:        TryStartInst %BB8, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst %BB1, %BB32
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [%VS6.i]: any
// CHECK-NEXT:        TryEndInst %BB4, %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %28 = CatchInst (:any)
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %30 = BinaryAddInst (:any) %29: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: any, [%VS6.i]: any
// CHECK-NEXT:        TryStartInst %BB24, %BB26
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryStartInst %BB11, %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %35 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %35: any, [%VS6.e#1]: any
// CHECK-NEXT:  %37 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %38 = BinaryAddInst (:any) %37: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %38: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        TryEndInst %BB8, %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %42 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %43 = BinaryAddInst (:any) %42: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %43: any, [%VS6.i]: any
// CHECK-NEXT:        TryEndInst %BB11, %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %47 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %48 = BinaryAddInst (:any) %47: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %48: any, [%VS6.i]: any
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %51 = CatchInst (:any)
// CHECK-NEXT:  %52 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %53 = BinaryAddInst (:any) %52: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %53: any, [%VS6.i]: any
// CHECK-NEXT:        ThrowInst %51: any, %BB1
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        TryStartInst %BB19, %BB21
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %58 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %58: any, [%VS6.e#2]: any
// CHECK-NEXT:  %60 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %61 = BinaryAddInst (:any) %60: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %61: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        TryEndInst %BB16, %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %65 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %66 = BinaryAddInst (:any) %65: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %66: any, [%VS6.i]: any
// CHECK-NEXT:        TryEndInst %BB19, %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB23:
// CHECK-NEXT:  %70 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %71 = BinaryAddInst (:any) %70: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %71: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %74 = CatchInst (:any)
// CHECK-NEXT:  %75 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %76 = BinaryAddInst (:any) %75: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %76: any, [%VS6.i]: any
// CHECK-NEXT:        ThrowInst %74: any, %BB1
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        ThrowInst %28: any, %BB1
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryStartInst %BB27, %BB29
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %81 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %81: any, [%VS6.e#2]: any
// CHECK-NEXT:  %83 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %84 = BinaryAddInst (:any) %83: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %84: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        TryEndInst %BB24, %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %88 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %89 = BinaryAddInst (:any) %88: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %89: any, [%VS6.i]: any
// CHECK-NEXT:        TryEndInst %BB27, %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB31:
// CHECK-NEXT:  %93 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %94 = BinaryAddInst (:any) %93: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %94: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB32:
// CHECK-NEXT:  %97 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %98 = BinaryAddInst (:any) %97: any, 8: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %98: any, [%VS6.i]: any
// CHECK-NEXT:         BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [i: any, e: any]

// CHECK:function finally_with_break_continue_test(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS7.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS7.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [%VS7.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %12: any, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %16 = AsNumericInst (:number|bigint) %15: any
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: number|bigint, [%VS7.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = CatchInst (:any)
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %22 = BinaryAddInst (:any) %21: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: any, [%VS7.i]: any
// CHECK-NEXT:        ThrowInst %20: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %26 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: any, [%VS7.e]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: any, [%VS7.i]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %32 = BinaryEqualInst (:boolean) %31: any, 3: number
// CHECK-NEXT:        CondBranchInst %32: boolean, %BB11, %BB12
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %34 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %35 = AsNumericInst (:number|bigint) %34: any
// CHECK-NEXT:  %36 = UnaryIncInst (:number|bigint) %35: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: number|bigint, [%VS7.i]: any
// CHECK-NEXT:        TryEndInst %BB7, %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst %BB5, %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %40 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %41 = BinaryAddInst (:any) %40: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %41: any, [%VS7.i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        TryEndInst %BB5, %BB14
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        TryEndInst %BB5, %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:  %47 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %48 = BinaryAddInst (:any) %47: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %48: any, [%VS7.i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:  %51 = LoadFrameInst (:any) %1: environment, [%VS7.i]: any
// CHECK-NEXT:  %52 = BinaryAddInst (:any) %51: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %52: any, [%VS7.i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end
