// RUN: %hermes -hermes-parser -dump-ir %s | %FileCheck %s --match-full-lines
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
//CHECK-NEXT:    %5 = AsNumberInst %4
//CHECK-NEXT:    %6 = BinaryOperatorInst '+', %5 : number, 1 : number
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
//CHECK-NEXT:    %29 = AsNumberInst %28
//CHECK-NEXT:    %30 = BinaryOperatorInst '+', %29 : number, 1 : number
//CHECK-NEXT:    %31 = StoreFrameInst %30, [i]
//CHECK-NEXT:    %32 = BranchInst %BB8
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %33 = LoadFrameInst [i]
//CHECK-NEXT:    %34 = AsNumberInst %33
//CHECK-NEXT:    %35 = BinaryOperatorInst '-', %34 : number, 1 : number
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
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = CatchInst
//CHECK-NEXT:    %4 = StoreFrameInst %3, [?anon_0_e]
//CHECK-NEXT:    %5 = TryStartInst %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '+', %6, 4 : number
//CHECK-NEXT:    %8 = StoreFrameInst %7, [i]
//CHECK-NEXT:    %9 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %10 = CatchInst
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 3 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = ThrowInst %10
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %15 = LoadFrameInst [i]
//CHECK-NEXT:    %16 = AsNumberInst %15
//CHECK-NEXT:    %17 = BinaryOperatorInst '+', %16 : number, 1 : number
//CHECK-NEXT:    %18 = StoreFrameInst %17, [i]
//CHECK-NEXT:    %19 = BranchInst %BB6
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %20 = TryEndInst
//CHECK-NEXT:    %21 = LoadFrameInst [i]
//CHECK-NEXT:    %22 = BinaryOperatorInst '+', %21, 3 : number
//CHECK-NEXT:    %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:    %24 = BranchInst %BB5
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %25 = LoadFrameInst [i]
//CHECK-NEXT:    %26 = BinaryOperatorInst '+', %25, 2 : number
//CHECK-NEXT:    %27 = StoreFrameInst %26, [i]
//CHECK-NEXT:    %28 = BranchInst %BB7
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %29 = TryEndInst
//CHECK-NEXT:    %30 = LoadFrameInst [i]
//CHECK-NEXT:    %31 = BinaryOperatorInst '+', %30, 3 : number
//CHECK-NEXT:    %32 = StoreFrameInst %31, [i]
//CHECK-NEXT:    %33 = BranchInst %BB5
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
//CHECK-NEXT:    %13 = AsNumberInst %12
//CHECK-NEXT:    %14 = BinaryOperatorInst '+', %13 : number, 1 : number
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
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = CatchInst
//CHECK-NEXT:    %4 = StoreFrameInst %3, [?anon_0_e]
//CHECK-NEXT:    %5 = TryStartInst %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '+', %6, 4 : number
//CHECK-NEXT:    %8 = StoreFrameInst %7, [i]
//CHECK-NEXT:    %9 = ReturnInst "d" : string
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %10 = CatchInst
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 3 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = ReturnInst "c" : string
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %15 = LoadFrameInst [i]
//CHECK-NEXT:    %16 = AsNumberInst %15
//CHECK-NEXT:    %17 = BinaryOperatorInst '+', %16 : number, 1 : number
//CHECK-NEXT:    %18 = StoreFrameInst %17, [i]
//CHECK-NEXT:    %19 = BranchInst %BB6
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %20 = TryEndInst
//CHECK-NEXT:    %21 = LoadFrameInst [i]
//CHECK-NEXT:    %22 = BinaryOperatorInst '+', %21, 3 : number
//CHECK-NEXT:    %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:    %24 = ReturnInst "c" : string
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %25 = ReturnInst "a" : string
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %26 = BranchInst %BB9
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %27 = TryEndInst
//CHECK-NEXT:    %28 = LoadFrameInst [i]
//CHECK-NEXT:    %29 = BinaryOperatorInst '+', %28, 3 : number
//CHECK-NEXT:    %30 = StoreFrameInst %29, [i]
//CHECK-NEXT:    %31 = ReturnInst "c" : string
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %32 = BranchInst %BB5
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %33 = LoadFrameInst [i]
//CHECK-NEXT:    %34 = BinaryOperatorInst '+', %33, 2 : number
//CHECK-NEXT:    %35 = StoreFrameInst %34, [i]
//CHECK-NEXT:    %36 = BranchInst %BB11
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %37 = TryEndInst
//CHECK-NEXT:    %38 = LoadFrameInst [i]
//CHECK-NEXT:    %39 = BinaryOperatorInst '+', %38, 3 : number
//CHECK-NEXT:    %40 = StoreFrameInst %39, [i]
//CHECK-NEXT:    %41 = ReturnInst "c" : string
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %42 = ReturnInst "b" : string
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %43 = BranchInst %BB14
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %44 = TryEndInst
//CHECK-NEXT:    %45 = LoadFrameInst [i]
//CHECK-NEXT:    %46 = BinaryOperatorInst '+', %45, 3 : number
//CHECK-NEXT:    %47 = StoreFrameInst %46, [i]
//CHECK-NEXT:    %48 = ReturnInst "c" : string
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %49 = BranchInst %BB5
//CHECK-NEXT:  %BB16:
//CHECK-NEXT:    %50 = ThrowInst %10
//CHECK-NEXT:  %BB17:
//CHECK-NEXT:    %51 = ReturnInst undefined : undefined
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
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = CatchInst
//CHECK-NEXT:    %4 = StoreFrameInst %3, [?anon_2_e]
//CHECK-NEXT:    %5 = TryStartInst %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '+', %6, 4 : number
//CHECK-NEXT:    %8 = StoreFrameInst %7, [i]
//CHECK-NEXT:    %9 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %10 = CatchInst
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 3 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = ThrowInst %10
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %15 = LoadFrameInst [i]
//CHECK-NEXT:    %16 = AsNumberInst %15
//CHECK-NEXT:    %17 = BinaryOperatorInst '+', %16 : number, 1 : number
//CHECK-NEXT:    %18 = StoreFrameInst %17, [i]
//CHECK-NEXT:    %19 = TryStartInst %BB6, %BB7
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %20 = CatchInst
//CHECK-NEXT:    %21 = StoreFrameInst %20, [?anon_1_e]
//CHECK-NEXT:    %22 = TryStartInst %BB8, %BB9
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %23 = BranchInst %BB11
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %24 = CatchInst
//CHECK-NEXT:    %25 = LoadFrameInst [i]
//CHECK-NEXT:    %26 = BinaryOperatorInst '+', %25, 7 : number
//CHECK-NEXT:    %27 = StoreFrameInst %26, [i]
//CHECK-NEXT:    %28 = ThrowInst %24
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %29 = LoadFrameInst [i]
//CHECK-NEXT:    %30 = BinaryOperatorInst '+', %29, 5 : number
//CHECK-NEXT:    %31 = StoreFrameInst %30, [i]
//CHECK-NEXT:    %32 = TryStartInst %BB12, %BB13
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %33 = CatchInst
//CHECK-NEXT:    %34 = StoreFrameInst %33, [?anon_0_e]
//CHECK-NEXT:    %35 = TryStartInst %BB14, %BB15
//CHECK-NEXT:  %BB16:
//CHECK-NEXT:    %36 = BranchInst %BB17
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %37 = CatchInst
//CHECK-NEXT:    %38 = LoadFrameInst [i]
//CHECK-NEXT:    %39 = BinaryOperatorInst '+', %38, 10 : number
//CHECK-NEXT:    %40 = StoreFrameInst %39, [i]
//CHECK-NEXT:    %41 = ThrowInst %37
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %42 = LoadFrameInst [i]
//CHECK-NEXT:    %43 = BinaryOperatorInst '+', %42, 8 : number
//CHECK-NEXT:    %44 = StoreFrameInst %43, [i]
//CHECK-NEXT:    %45 = BranchInst %BB18
//CHECK-NEXT:  %BB18:
//CHECK-NEXT:    %46 = TryEndInst
//CHECK-NEXT:    %47 = LoadFrameInst [i]
//CHECK-NEXT:    %48 = BinaryOperatorInst '+', %47, 10 : number
//CHECK-NEXT:    %49 = StoreFrameInst %48, [i]
//CHECK-NEXT:    %50 = BranchInst %BB19
//CHECK-NEXT:  %BB19:
//CHECK-NEXT:    %51 = TryEndInst
//CHECK-NEXT:    %52 = LoadFrameInst [i]
//CHECK-NEXT:    %53 = BinaryOperatorInst '+', %52, 7 : number
//CHECK-NEXT:    %54 = StoreFrameInst %53, [i]
//CHECK-NEXT:    %55 = BranchInst %BB20
//CHECK-NEXT:  %BB20:
//CHECK-NEXT:    %56 = TryEndInst
//CHECK-NEXT:    %57 = LoadFrameInst [i]
//CHECK-NEXT:    %58 = BinaryOperatorInst '+', %57, 3 : number
//CHECK-NEXT:    %59 = StoreFrameInst %58, [i]
//CHECK-NEXT:    %60 = ReturnInst "a" : string
//CHECK-NEXT:  %BB21:
//CHECK-NEXT:    %61 = BranchInst %BB22
//CHECK-NEXT:  %BB22:
//CHECK-NEXT:    %62 = TryEndInst
//CHECK-NEXT:    %63 = LoadFrameInst [i]
//CHECK-NEXT:    %64 = BinaryOperatorInst '+', %63, 10 : number
//CHECK-NEXT:    %65 = StoreFrameInst %64, [i]
//CHECK-NEXT:    %66 = BranchInst %BB16
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %67 = LoadFrameInst [i]
//CHECK-NEXT:    %68 = BinaryOperatorInst '+', %67, 9 : number
//CHECK-NEXT:    %69 = StoreFrameInst %68, [i]
//CHECK-NEXT:    %70 = BranchInst %BB23
//CHECK-NEXT:  %BB23:
//CHECK-NEXT:    %71 = TryEndInst
//CHECK-NEXT:    %72 = LoadFrameInst [i]
//CHECK-NEXT:    %73 = BinaryOperatorInst '+', %72, 10 : number
//CHECK-NEXT:    %74 = StoreFrameInst %73, [i]
//CHECK-NEXT:    %75 = BranchInst %BB16
//CHECK-NEXT:  %BB17:
//CHECK-NEXT:    %76 = TryEndInst
//CHECK-NEXT:    %77 = LoadFrameInst [i]
//CHECK-NEXT:    %78 = BinaryOperatorInst '+', %77, 7 : number
//CHECK-NEXT:    %79 = StoreFrameInst %78, [i]
//CHECK-NEXT:    %80 = BranchInst %BB10
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %81 = LoadFrameInst [i]
//CHECK-NEXT:    %82 = BinaryOperatorInst '+', %81, 6 : number
//CHECK-NEXT:    %83 = StoreFrameInst %82, [i]
//CHECK-NEXT:    %84 = BranchInst %BB24
//CHECK-NEXT:  %BB24:
//CHECK-NEXT:    %85 = TryEndInst
//CHECK-NEXT:    %86 = LoadFrameInst [i]
//CHECK-NEXT:    %87 = BinaryOperatorInst '+', %86, 7 : number
//CHECK-NEXT:    %88 = StoreFrameInst %87, [i]
//CHECK-NEXT:    %89 = BranchInst %BB10
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %90 = TryEndInst
//CHECK-NEXT:    %91 = LoadFrameInst [i]
//CHECK-NEXT:    %92 = BinaryOperatorInst '+', %91, 3 : number
//CHECK-NEXT:    %93 = StoreFrameInst %92, [i]
//CHECK-NEXT:    %94 = BranchInst %BB5
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %95 = LoadFrameInst [i]
//CHECK-NEXT:    %96 = BinaryOperatorInst '+', %95, 2 : number
//CHECK-NEXT:    %97 = StoreFrameInst %96, [i]
//CHECK-NEXT:    %98 = BranchInst %BB25
//CHECK-NEXT:  %BB25:
//CHECK-NEXT:    %99 = TryEndInst
//CHECK-NEXT:    %100 = LoadFrameInst [i]
//CHECK-NEXT:    %101 = BinaryOperatorInst '+', %100, 3 : number
//CHECK-NEXT:    %102 = StoreFrameInst %101, [i]
//CHECK-NEXT:    %103 = BranchInst %BB5
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
//CHECK-NEXT:frame = [i, ?anon_0_e, ?anon_1_e, ?anon_2_e, ?anon_3_e, ?anon_4_e]
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = TryStartInst %BB1, %BB2
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %3 = CatchInst
//CHECK-NEXT:    %4 = StoreFrameInst %3, [?anon_0_e]
//CHECK-NEXT:    %5 = TryStartInst %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %6 = LoadFrameInst [i]
//CHECK-NEXT:    %7 = BinaryOperatorInst '+', %6, 10 : number
//CHECK-NEXT:    %8 = StoreFrameInst %7, [i]
//CHECK-NEXT:    %9 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %10 = CatchInst
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '+', %11, 8 : number
//CHECK-NEXT:    %13 = StoreFrameInst %12, [i]
//CHECK-NEXT:    %14 = ThrowInst %10
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %15 = LoadFrameInst [i]
//CHECK-NEXT:    %16 = AsNumberInst %15
//CHECK-NEXT:    %17 = BinaryOperatorInst '+', %16 : number, 1 : number
//CHECK-NEXT:    %18 = StoreFrameInst %17, [i]
//CHECK-NEXT:    %19 = BranchInst %BB6
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %20 = TryEndInst
//CHECK-NEXT:    %21 = LoadFrameInst [i]
//CHECK-NEXT:    %22 = BinaryOperatorInst '+', %21, 8 : number
//CHECK-NEXT:    %23 = StoreFrameInst %22, [i]
//CHECK-NEXT:    %24 = BranchInst %BB5
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %25 = LoadFrameInst [i]
//CHECK-NEXT:    %26 = BinaryOperatorInst '+', %25, 9 : number
//CHECK-NEXT:    %27 = StoreFrameInst %26, [i]
//CHECK-NEXT:    %28 = TryStartInst %BB7, %BB8
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %29 = CatchInst
//CHECK-NEXT:    %30 = StoreFrameInst %29, [?anon_2_e]
//CHECK-NEXT:    %31 = TryStartInst %BB9, %BB10
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %32 = BranchInst %BB12
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %33 = CatchInst
//CHECK-NEXT:    %34 = LoadFrameInst [i]
//CHECK-NEXT:    %35 = BinaryOperatorInst '+', %34, 4 : number
//CHECK-NEXT:    %36 = StoreFrameInst %35, [i]
//CHECK-NEXT:    %37 = TryStartInst %BB13, %BB14
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %38 = LoadFrameInst [i]
//CHECK-NEXT:    %39 = BinaryOperatorInst '+', %38, 2 : number
//CHECK-NEXT:    %40 = StoreFrameInst %39, [i]
//CHECK-NEXT:    %41 = BranchInst %BB15
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %42 = TryEndInst
//CHECK-NEXT:    %43 = LoadFrameInst [i]
//CHECK-NEXT:    %44 = BinaryOperatorInst '+', %43, 4 : number
//CHECK-NEXT:    %45 = StoreFrameInst %44, [i]
//CHECK-NEXT:    %46 = TryStartInst %BB16, %BB17
//CHECK-NEXT:  %BB16:
//CHECK-NEXT:    %47 = CatchInst
//CHECK-NEXT:    %48 = StoreFrameInst %47, [?anon_1_e]
//CHECK-NEXT:    %49 = TryStartInst %BB18, %BB19
//CHECK-NEXT:  %BB20:
//CHECK-NEXT:    %50 = BranchInst %BB11
//CHECK-NEXT:  %BB18:
//CHECK-NEXT:    %51 = CatchInst
//CHECK-NEXT:    %52 = LoadFrameInst [i]
//CHECK-NEXT:    %53 = BinaryOperatorInst '+', %52, 7 : number
//CHECK-NEXT:    %54 = StoreFrameInst %53, [i]
//CHECK-NEXT:    %55 = ThrowInst %51
//CHECK-NEXT:  %BB17:
//CHECK-NEXT:    %56 = LoadFrameInst [i]
//CHECK-NEXT:    %57 = BinaryOperatorInst '+', %56, 5 : number
//CHECK-NEXT:    %58 = StoreFrameInst %57, [i]
//CHECK-NEXT:    %59 = BranchInst %BB21
//CHECK-NEXT:  %BB21:
//CHECK-NEXT:    %60 = TryEndInst
//CHECK-NEXT:    %61 = LoadFrameInst [i]
//CHECK-NEXT:    %62 = BinaryOperatorInst '+', %61, 7 : number
//CHECK-NEXT:    %63 = StoreFrameInst %62, [i]
//CHECK-NEXT:    %64 = BranchInst %BB20
//CHECK-NEXT:  %BB19:
//CHECK-NEXT:    %65 = LoadFrameInst [i]
//CHECK-NEXT:    %66 = BinaryOperatorInst '+', %65, 6 : number
//CHECK-NEXT:    %67 = StoreFrameInst %66, [i]
//CHECK-NEXT:    %68 = BranchInst %BB22
//CHECK-NEXT:  %BB22:
//CHECK-NEXT:    %69 = TryEndInst
//CHECK-NEXT:    %70 = LoadFrameInst [i]
//CHECK-NEXT:    %71 = BinaryOperatorInst '+', %70, 7 : number
//CHECK-NEXT:    %72 = StoreFrameInst %71, [i]
//CHECK-NEXT:    %73 = BranchInst %BB20
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %74 = LoadFrameInst [i]
//CHECK-NEXT:    %75 = BinaryOperatorInst '+', %74, 3 : number
//CHECK-NEXT:    %76 = StoreFrameInst %75, [i]
//CHECK-NEXT:    %77 = BranchInst %BB23
//CHECK-NEXT:  %BB23:
//CHECK-NEXT:    %78 = TryEndInst
//CHECK-NEXT:    %79 = LoadFrameInst [i]
//CHECK-NEXT:    %80 = BinaryOperatorInst '+', %79, 4 : number
//CHECK-NEXT:    %81 = StoreFrameInst %80, [i]
//CHECK-NEXT:    %82 = TryStartInst %BB24, %BB25
//CHECK-NEXT:  %BB24:
//CHECK-NEXT:    %83 = CatchInst
//CHECK-NEXT:    %84 = StoreFrameInst %83, [?anon_3_e]
//CHECK-NEXT:    %85 = TryStartInst %BB26, %BB27
//CHECK-NEXT:  %BB28:
//CHECK-NEXT:    %86 = BranchInst %BB11
//CHECK-NEXT:  %BB26:
//CHECK-NEXT:    %87 = CatchInst
//CHECK-NEXT:    %88 = LoadFrameInst [i]
//CHECK-NEXT:    %89 = BinaryOperatorInst '+', %88, 7 : number
//CHECK-NEXT:    %90 = StoreFrameInst %89, [i]
//CHECK-NEXT:    %91 = ThrowInst %87
//CHECK-NEXT:  %BB25:
//CHECK-NEXT:    %92 = LoadFrameInst [i]
//CHECK-NEXT:    %93 = BinaryOperatorInst '+', %92, 5 : number
//CHECK-NEXT:    %94 = StoreFrameInst %93, [i]
//CHECK-NEXT:    %95 = BranchInst %BB29
//CHECK-NEXT:  %BB29:
//CHECK-NEXT:    %96 = TryEndInst
//CHECK-NEXT:    %97 = LoadFrameInst [i]
//CHECK-NEXT:    %98 = BinaryOperatorInst '+', %97, 7 : number
//CHECK-NEXT:    %99 = StoreFrameInst %98, [i]
//CHECK-NEXT:    %100 = BranchInst %BB28
//CHECK-NEXT:  %BB27:
//CHECK-NEXT:    %101 = LoadFrameInst [i]
//CHECK-NEXT:    %102 = BinaryOperatorInst '+', %101, 6 : number
//CHECK-NEXT:    %103 = StoreFrameInst %102, [i]
//CHECK-NEXT:    %104 = BranchInst %BB30
//CHECK-NEXT:  %BB30:
//CHECK-NEXT:    %105 = TryEndInst
//CHECK-NEXT:    %106 = LoadFrameInst [i]
//CHECK-NEXT:    %107 = BinaryOperatorInst '+', %106, 7 : number
//CHECK-NEXT:    %108 = StoreFrameInst %107, [i]
//CHECK-NEXT:    %109 = BranchInst %BB28
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %110 = CatchInst
//CHECK-NEXT:    %111 = StoreFrameInst %110, [?anon_4_e]
//CHECK-NEXT:    %112 = TryStartInst %BB31, %BB32
//CHECK-NEXT:  %BB33:
//CHECK-NEXT:    %113 = ThrowInst %33
//CHECK-NEXT:  %BB31:
//CHECK-NEXT:    %114 = CatchInst
//CHECK-NEXT:    %115 = LoadFrameInst [i]
//CHECK-NEXT:    %116 = BinaryOperatorInst '+', %115, 7 : number
//CHECK-NEXT:    %117 = StoreFrameInst %116, [i]
//CHECK-NEXT:    %118 = ThrowInst %114
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %119 = LoadFrameInst [i]
//CHECK-NEXT:    %120 = BinaryOperatorInst '+', %119, 5 : number
//CHECK-NEXT:    %121 = StoreFrameInst %120, [i]
//CHECK-NEXT:    %122 = BranchInst %BB34
//CHECK-NEXT:  %BB34:
//CHECK-NEXT:    %123 = TryEndInst
//CHECK-NEXT:    %124 = LoadFrameInst [i]
//CHECK-NEXT:    %125 = BinaryOperatorInst '+', %124, 7 : number
//CHECK-NEXT:    %126 = StoreFrameInst %125, [i]
//CHECK-NEXT:    %127 = BranchInst %BB33
//CHECK-NEXT:  %BB32:
//CHECK-NEXT:    %128 = LoadFrameInst [i]
//CHECK-NEXT:    %129 = BinaryOperatorInst '+', %128, 6 : number
//CHECK-NEXT:    %130 = StoreFrameInst %129, [i]
//CHECK-NEXT:    %131 = BranchInst %BB35
//CHECK-NEXT:  %BB35:
//CHECK-NEXT:    %132 = TryEndInst
//CHECK-NEXT:    %133 = LoadFrameInst [i]
//CHECK-NEXT:    %134 = BinaryOperatorInst '+', %133, 7 : number
//CHECK-NEXT:    %135 = StoreFrameInst %134, [i]
//CHECK-NEXT:    %136 = BranchInst %BB33
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %137 = TryEndInst
//CHECK-NEXT:    %138 = LoadFrameInst [i]
//CHECK-NEXT:    %139 = BinaryOperatorInst '+', %138, 8 : number
//CHECK-NEXT:    %140 = StoreFrameInst %139, [i]
//CHECK-NEXT:    %141 = BranchInst %BB5
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
//CHECK-NEXT:  %BB0:
//CHECK-NEXT:    %0 = StoreFrameInst undefined : undefined, [i]
//CHECK-NEXT:    %1 = StoreFrameInst 0 : number, [i]
//CHECK-NEXT:    %2 = BranchInst %BB1
//CHECK-NEXT:  %BB2:
//CHECK-NEXT:    %3 = TryStartInst %BB3, %BB4
//CHECK-NEXT:  %BB5:
//CHECK-NEXT:    %4 = LoadFrameInst [i]
//CHECK-NEXT:    %5 = BinaryOperatorInst '+', %4, 4 : number
//CHECK-NEXT:    %6 = StoreFrameInst %5, [i]
//CHECK-NEXT:    %7 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB1:
//CHECK-NEXT:    %8 = LoadFrameInst [i]
//CHECK-NEXT:    %9 = BinaryOperatorInst '<', %8, 10 : number
//CHECK-NEXT:    %10 = CondBranchInst %9, %BB2, %BB5
//CHECK-NEXT:  %BB6:
//CHECK-NEXT:    %11 = LoadFrameInst [i]
//CHECK-NEXT:    %12 = BinaryOperatorInst '<', %11, 10 : number
//CHECK-NEXT:    %13 = CondBranchInst %12, %BB2, %BB5
//CHECK-NEXT:  %BB7:
//CHECK-NEXT:    %14 = LoadFrameInst [i]
//CHECK-NEXT:    %15 = AsNumberInst %14
//CHECK-NEXT:    %16 = BinaryOperatorInst '+', %15 : number, 1 : number
//CHECK-NEXT:    %17 = StoreFrameInst %16, [i]
//CHECK-NEXT:    %18 = BranchInst %BB6
//CHECK-NEXT:  %BB3:
//CHECK-NEXT:    %19 = CatchInst
//CHECK-NEXT:    %20 = StoreFrameInst %19, [?anon_0_e]
//CHECK-NEXT:    %21 = TryStartInst %BB8, %BB9
//CHECK-NEXT:  %BB10:
//CHECK-NEXT:    %22 = BranchInst %BB7
//CHECK-NEXT:  %BB8:
//CHECK-NEXT:    %23 = CatchInst
//CHECK-NEXT:    %24 = LoadFrameInst [i]
//CHECK-NEXT:    %25 = BinaryOperatorInst '+', %24, 3 : number
//CHECK-NEXT:    %26 = StoreFrameInst %25, [i]
//CHECK-NEXT:    %27 = ThrowInst %23
//CHECK-NEXT:  %BB4:
//CHECK-NEXT:    %28 = LoadFrameInst [i]
//CHECK-NEXT:    %29 = AsNumberInst %28
//CHECK-NEXT:    %30 = BinaryOperatorInst '+', %29 : number, 1 : number
//CHECK-NEXT:    %31 = StoreFrameInst %30, [i]
//CHECK-NEXT:    %32 = BranchInst %BB11
//CHECK-NEXT:  %BB11:
//CHECK-NEXT:    %33 = TryEndInst
//CHECK-NEXT:    %34 = LoadFrameInst [i]
//CHECK-NEXT:    %35 = BinaryOperatorInst '+', %34, 3 : number
//CHECK-NEXT:    %36 = StoreFrameInst %35, [i]
//CHECK-NEXT:    %37 = BranchInst %BB5
//CHECK-NEXT:  %BB12:
//CHECK-NEXT:    %38 = BranchInst %BB13
//CHECK-NEXT:  %BB13:
//CHECK-NEXT:    %39 = TryEndInst
//CHECK-NEXT:    %40 = LoadFrameInst [i]
//CHECK-NEXT:    %41 = BinaryOperatorInst '+', %40, 3 : number
//CHECK-NEXT:    %42 = StoreFrameInst %41, [i]
//CHECK-NEXT:    %43 = BranchInst %BB10
//CHECK-NEXT:  %BB9:
//CHECK-NEXT:    %44 = LoadFrameInst [i]
//CHECK-NEXT:    %45 = BinaryOperatorInst '+', %44, 2 : number
//CHECK-NEXT:    %46 = StoreFrameInst %45, [i]
//CHECK-NEXT:    %47 = LoadFrameInst [i]
//CHECK-NEXT:    %48 = BinaryOperatorInst '==', %47, 3 : number
//CHECK-NEXT:    %49 = CondBranchInst %48, %BB14, %BB15
//CHECK-NEXT:  %BB14:
//CHECK-NEXT:    %50 = BranchInst %BB16
//CHECK-NEXT:  %BB15:
//CHECK-NEXT:    %51 = BranchInst %BB17
//CHECK-NEXT:  %BB17:
//CHECK-NEXT:    %52 = BranchInst %BB18
//CHECK-NEXT:  %BB16:
//CHECK-NEXT:    %53 = TryEndInst
//CHECK-NEXT:    %54 = LoadFrameInst [i]
//CHECK-NEXT:    %55 = BinaryOperatorInst '+', %54, 3 : number
//CHECK-NEXT:    %56 = StoreFrameInst %55, [i]
//CHECK-NEXT:    %57 = ReturnInst undefined : undefined
//CHECK-NEXT:  %BB19:
//CHECK-NEXT:    %58 = BranchInst %BB17
//CHECK-NEXT:  %BB18:
//CHECK-NEXT:    %59 = TryEndInst
//CHECK-NEXT:    %60 = LoadFrameInst [i]
//CHECK-NEXT:    %61 = BinaryOperatorInst '+', %60, 3 : number
//CHECK-NEXT:    %62 = StoreFrameInst %61, [i]
//CHECK-NEXT:    %63 = BranchInst %BB7
//CHECK-NEXT:  %BB20:
//CHECK-NEXT:    %64 = BranchInst %BB21
//CHECK-NEXT:  %BB21:
//CHECK-NEXT:    %65 = TryEndInst
//CHECK-NEXT:    %66 = LoadFrameInst [i]
//CHECK-NEXT:    %67 = BinaryOperatorInst '+', %66, 3 : number
//CHECK-NEXT:    %68 = StoreFrameInst %67, [i]
//CHECK-NEXT:    %69 = BranchInst %BB10
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


