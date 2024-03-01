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
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %global(): any, empty: any
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

// CHECK:function simple_try_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_try_catch_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = CatchInst (:any)
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [e]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [e]: any
// CHECK-NEXT:  %7 = AsNumericInst (:number|bigint) %6: any
// CHECK-NEXT:  %8 = UnaryIncInst (:number|bigint) %7: number|bigint
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: number|bigint, [e]: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %11 = BinarySubtractInst (:any) %10: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %18 = BinaryLessThanInst (:boolean) %17: any, 10: number
// CHECK-NEXT:        CondBranchInst %18: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %20: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:        ThrowInst %24: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %26 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %27 = BinaryLessThanInst (:boolean) %26: any, 10: number
// CHECK-NEXT:        CondBranchInst %27: boolean, %BB4, %BB5
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %29 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %30 = AsNumericInst (:number|bigint) %29: any
// CHECK-NEXT:  %31 = UnaryIncInst (:number|bigint) %30: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:function_end

// CHECK:function simple_try_catch_finally_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_try_catch_finally_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [e]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %31 = BinaryAddInst (:any) %30: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: any, [i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function simple_try_finally_test(): any
// CHECK-NEXT:frame = [i: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %simple_try_finally_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 2: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %15 = AsNumericInst (:number|bigint) %14: any
// CHECK-NEXT:  %16 = UnaryIncInst (:number|bigint) %15: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %20: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function try_catch_finally_with_return_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %try_catch_finally_with_return_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [i]: any
// CHECK-NEXT:       ReturnInst "c": string
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        TryStartInst %BB3, %BB4
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %11 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [e]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) %13: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %18 = AsNumericInst (:number|bigint) %17: any
// CHECK-NEXT:  %19 = UnaryIncInst (:number|bigint) %18: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %25 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %26 = BinaryAddInst (:any) %25: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: any, [i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %31 = BinaryAddInst (:any) %30: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: any, [i]: any
// CHECK-NEXT:        ReturnInst "c": string
// CHECK-NEXT:function_end

// CHECK:function nested_try_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %nested_try_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 3: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [e#2]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [i]: any
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB29
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [i]: any
// CHECK-NEXT:        TryStartInst %BB7, %BB9
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %27 = CatchInst (:any)
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: any, [i]: any
// CHECK-NEXT:        ThrowInst %27: any
// CHECK-NEXT:%BB8:
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryStartInst %BB10, %BB12
// CHECK-NEXT:%BB10:
// CHECK-NEXT:  %34 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: any, [e#1]: any
// CHECK-NEXT:  %36 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %37 = BinaryAddInst (:any) %36: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %37: any, [i]: any
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB27
// CHECK-NEXT:%BB12:
// CHECK-NEXT:  %41 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %42 = BinaryAddInst (:any) %41: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %42: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB13, %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %45 = CatchInst (:any)
// CHECK-NEXT:  %46 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %47 = BinaryAddInst (:any) %46: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %47: any, [i]: any
// CHECK-NEXT:        ThrowInst %45: any
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        BranchInst %BB26
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %52 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %52: any, [e]: any
// CHECK-NEXT:  %54 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %55 = BinaryAddInst (:any) %54: any, 9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %55: any, [i]: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        BranchInst %BB25
// CHECK-NEXT:%BB18:
// CHECK-NEXT:  %59 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %60 = BinaryAddInst (:any) %59: any, 8: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %60: any, [i]: any
// CHECK-NEXT:        BranchInst %BB19
// CHECK-NEXT:%BB19:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %66 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %67 = BinaryAddInst (:any) %66: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %67: any, [i]: any
// CHECK-NEXT:        BranchInst %BB21
// CHECK-NEXT:%BB21:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %73 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %74 = BinaryAddInst (:any) %73: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %74: any, [i]: any
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB24
// CHECK-NEXT:%BB24:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %80 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %81 = BinaryAddInst (:any) %80: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %81: any, [i]: any
// CHECK-NEXT:        ReturnInst "a": string
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %85 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %86 = BinaryAddInst (:any) %85: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %86: any, [i]: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:%BB27:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %92 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %93 = BinaryAddInst (:any) %92: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %93: any, [i]: any
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB29:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %99 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %100 = BinaryAddInst (:any) %99: any, 3: number
// CHECK-NEXT:         StoreFrameInst %1: environment, %100: any, [i]: any
// CHECK-NEXT:         BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function nested_catch_test(): any
// CHECK-NEXT:frame = [i: any, e: any, e#1: any, e#2: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %nested_catch_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:       TryStartInst %BB1, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %5 = CatchInst (:any)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: any, 8: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: any, [i]: any
// CHECK-NEXT:       ThrowInst %5: any
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %11 = BinaryAddInst (:any) %10: any, 10: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        TryStartInst %BB4, %BB6
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [e]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %18 = BinaryAddInst (:any) %17: any, 9: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB8, %BB10
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB32
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %23 = AsNumericInst (:number|bigint) %22: any
// CHECK-NEXT:  %24 = UnaryIncInst (:number|bigint) %23: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %29 = CatchInst (:any)
// CHECK-NEXT:  %30 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %31 = BinaryAddInst (:any) %30: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %31: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB24, %BB26
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryStartInst %BB11, %BB13
// CHECK-NEXT:%BB11:
// CHECK-NEXT:  %36 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: any, [e#1]: any
// CHECK-NEXT:  %38 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %39 = BinaryAddInst (:any) %38: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %39: any, [i]: any
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB13:
// CHECK-NEXT:  %43 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %44 = BinaryAddInst (:any) %43: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %44: any, [i]: any
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %50 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %51 = BinaryAddInst (:any) %50: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %51: any, [i]: any
// CHECK-NEXT:        TryStartInst %BB16, %BB18
// CHECK-NEXT:%BB16:
// CHECK-NEXT:  %54 = CatchInst (:any)
// CHECK-NEXT:  %55 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %56 = BinaryAddInst (:any) %55: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %56: any, [i]: any
// CHECK-NEXT:        ThrowInst %54: any
// CHECK-NEXT:%BB17:
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB18:
// CHECK-NEXT:        TryStartInst %BB19, %BB21
// CHECK-NEXT:%BB19:
// CHECK-NEXT:  %61 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %61: any, [e#2]: any
// CHECK-NEXT:  %63 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %64 = BinaryAddInst (:any) %63: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %64: any, [i]: any
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB20:
// CHECK-NEXT:        BranchInst %BB23
// CHECK-NEXT:%BB21:
// CHECK-NEXT:  %68 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %69 = BinaryAddInst (:any) %68: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %69: any, [i]: any
// CHECK-NEXT:        BranchInst %BB22
// CHECK-NEXT:%BB22:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB20
// CHECK-NEXT:%BB23:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %75 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %76 = BinaryAddInst (:any) %75: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %76: any, [i]: any
// CHECK-NEXT:        BranchInst %BB17
// CHECK-NEXT:%BB24:
// CHECK-NEXT:  %79 = CatchInst (:any)
// CHECK-NEXT:  %80 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %81 = BinaryAddInst (:any) %80: any, 7: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %81: any, [i]: any
// CHECK-NEXT:        ThrowInst %79: any
// CHECK-NEXT:%BB25:
// CHECK-NEXT:        ThrowInst %29: any
// CHECK-NEXT:%BB26:
// CHECK-NEXT:        TryStartInst %BB27, %BB29
// CHECK-NEXT:%BB27:
// CHECK-NEXT:  %86 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %86: any, [e#2]: any
// CHECK-NEXT:  %88 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %89 = BinaryAddInst (:any) %88: any, 6: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %89: any, [i]: any
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB28:
// CHECK-NEXT:        BranchInst %BB31
// CHECK-NEXT:%BB29:
// CHECK-NEXT:  %93 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %94 = BinaryAddInst (:any) %93: any, 5: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %94: any, [i]: any
// CHECK-NEXT:        BranchInst %BB30
// CHECK-NEXT:%BB30:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB28
// CHECK-NEXT:%BB31:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %100 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %101 = BinaryAddInst (:any) %100: any, 7: number
// CHECK-NEXT:         StoreFrameInst %1: environment, %101: any, [i]: any
// CHECK-NEXT:         BranchInst %BB25
// CHECK-NEXT:%BB32:
// CHECK-NEXT:         TryEndInst
// CHECK-NEXT:  %105 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %106 = BinaryAddInst (:any) %105: any, 8: number
// CHECK-NEXT:         StoreFrameInst %1: environment, %106: any, [i]: any
// CHECK-NEXT:         BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:function finally_with_break_continue_test(): any
// CHECK-NEXT:frame = [i: any, e: any]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %global(): any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %finally_with_break_continue_test(): any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       TryStartInst %BB5, %BB6
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %9 = BinaryAddInst (:any) %8: any, 4: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %12: any, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %16 = AsNumericInst (:number|bigint) %15: any
// CHECK-NEXT:  %17 = UnaryIncInst (:number|bigint) %16: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %20 = CatchInst (:any)
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %22 = BinaryAddInst (:any) %21: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: any, [i]: any
// CHECK-NEXT:        ThrowInst %20: any
// CHECK-NEXT:%BB6:
// CHECK-NEXT:        TryStartInst %BB7, %BB8
// CHECK-NEXT:%BB7:
// CHECK-NEXT:  %26 = CatchInst (:any)
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: any, [e]: any
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %29 = BinaryAddInst (:any) %28: any, 2: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %29: any, [i]: any
// CHECK-NEXT:  %31 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %32 = BinaryEqualInst (:boolean) %31: any, 3: number
// CHECK-NEXT:        CondBranchInst %32: boolean, %BB11, %BB12
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %34 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %35 = AsNumericInst (:number|bigint) %34: any
// CHECK-NEXT:  %36 = UnaryIncInst (:number|bigint) %35: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: number|bigint, [i]: any
// CHECK-NEXT:        BranchInst %BB9
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:        BranchInst %BB10
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %42 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %43 = BinaryAddInst (:any) %42: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %43: any, [i]: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        BranchInst %BB14
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB13
// CHECK-NEXT:%BB13:
// CHECK-NEXT:        BranchInst %BB15
// CHECK-NEXT:%BB14:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %50 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %51 = BinaryAddInst (:any) %50: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %51: any, [i]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB15:
// CHECK-NEXT:        TryEndInst
// CHECK-NEXT:  %55 = LoadFrameInst (:any) %1: environment, [i]: any
// CHECK-NEXT:  %56 = BinaryAddInst (:any) %55: any, 3: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %56: any, [i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end
