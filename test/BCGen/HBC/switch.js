// RUN: %hermes -target=HBC -dump-ra -fno-calln -O %s | %FileCheck --match-full-lines %s

//CHECK-LABEL:function f(x) : string
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:   {{.*}} %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:   {{.*}} %1 = HBCLoadConstInst "fall" : string
//CHECK-NEXT:   {{.*}} %2 = HBCLoadConstInst "" : string
//CHECK-NEXT:   {{.*}} %3 = BranchInst %BB1
//CHECK-NEXT: %BB2:
//CHECK-NEXT:   {{.*}} %4 = HBCLoadConstInst "regular" : string
//CHECK-NEXT:   {{.*}} %5 = ReturnInst %4 : string
//CHECK-NEXT: %BB3:
//CHECK-NEXT:   {{.*}} %6 = HBCLoadConstInst "multicase" : string
//CHECK-NEXT:   {{.*}} %7 = ReturnInst %6 : string
//CHECK-NEXT: %BB4:
//CHECK-NEXT:   {{.*}} %8 = MovInst %1 : string
//CHECK-NEXT:   {{.*}} %9 = BranchInst %BB5
//CHECK-NEXT: %BB5:
//CHECK-NEXT:   {{.*}} %10 = PhiInst %8 : string, %BB4, %17 : string, %BB6
//CHECK-NEXT:   {{.*}} %11 = HBCLoadConstInst "through" : string
//CHECK-NEXT:   {{.*}} %12 = BinaryOperatorInst '+', %10 : string, %11 : string
//CHECK-NEXT:   {{.*}} %13 = ReturnInst %12 : string
//CHECK-NEXT: %BB7:
//CHECK-NEXT:   {{.*}} %14 = HBCLoadConstInst "default" : string
//CHECK-NEXT:   {{.*}} %15 = ReturnInst %14 : string
//CHECK-NEXT: %BB6:
//CHECK-NEXT:   {{.*}} %16 = HBCLoadConstInst 4 : number
//CHECK-NEXT:   {{.*}} %17 = MovInst %2 : string
//CHECK-NEXT:   {{.*}} %18 = CompareBranchInst '===', %16 : number, %0, %BB5, %BB7
//CHECK-NEXT: %BB8:
//CHECK-NEXT:   {{.*}} %19 = HBCLoadConstInst 3 : number
//CHECK-NEXT:   {{.*}} %20 = CompareBranchInst '===', %19 : number, %0, %BB4, %BB6
//CHECK-NEXT: %BB9:
//CHECK-NEXT:   {{.*}}  %21 = HBCLoadConstInst 2 : number
//CHECK-NEXT:   {{.*}}  %22 = CompareBranchInst '===', %21 : number, %0, %BB3, %BB8
//CHECK-NEXT: %BB10:
//CHECK-NEXT:   {{.*}}  %23 = HBCLoadConstInst 1 : number
//CHECK-NEXT:   {{.*}}  %24 = CompareBranchInst '===', %23 : number, %0, %BB3, %BB9
//CHECK-NEXT: %BB1:
//CHECK-NEXT:   {{.*}}  %25 = HBCLoadConstInst 0 : number
//CHECK-NEXT:   {{.*}}  %26 = CompareBranchInst '===', %25 : number, %0, %BB2, %BB10
//CHECK-NEXT: function_end

function f(x) {
  var str = "";
  switch (x) {
    case 0:
      return "regular";
    case 1:
    case 2:
      return "multicase";
    case 3:
      str +="fall";
    case 4:
      str +="through";
      return str;
    default:
      return "default";
  }
}

//CHECK-LABEL:function regress1(w) : undefined
//CHECK-NEXT:frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}} %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}} %1 = HBCLoadConstInst 0 : number
//CHECK-NEXT:  {{.*}} %2 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}} %3 = HBCGetGlobalObjectInst
//CHECK-NEXT:  {{.*}} %4 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}} %5 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}} %6 = MovInst %1 : number
//CHECK-NEXT:  {{.*}} %7 = BranchInst %BB1
//CHECK-NEXT:%BB1:
//CHECK-NEXT:  {{.*}} %8 = PhiInst %6 : number, %BB0, %12 : number, %BB2, %14 : number, %BB3
//CHECK-NEXT:  {{.*}} %9 = TryLoadGlobalPropertyInst %3 : object, "print" : string
//CHECK-NEXT:  {{.*}} %10 = CallInst %9, %4 : undefined, %8 : number
//CHECK-NEXT:  {{.*}} %11 = BranchInst %BB4
//CHECK-NEXT:%BB2:
//CHECK-NEXT:  {{.*}} %12 = MovInst %2 : number
//CHECK-NEXT:  {{.*}} %13 = BranchInst %BB1
//CHECK-NEXT:%BB3:
//CHECK-NEXT:  {{.*}} %14 = MovInst %8 : number
//CHECK-NEXT:  {{.*}} %15 = CompareBranchInst '===', %1 : number, %0, %BB2, %BB1
//CHECK-NEXT:%BB4:
//CHECK-NEXT:  {{.*}} %16 = CompareBranchInst '===', %5 : number, %0, %BB2, %BB3
//CHECK-NEXT:function_end

function regress1(w) {
  // This structure caused an IR verification failure.
  var v=0;
  while(true) {
    print(v);
    switch (w) {
      case 1:
        v=1;
      case 0:
        v=2;
    }
  }
}


//CHECK-LABEL:function jump_table(x) : undefined|string
//CHECK-NEXT: frame = []
//CHECK-NEXT:%BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = SwitchImmInst %0, %BB1, 0 : number, 11 : number, 0 : number, %BB2, 1 : number, %BB3, 2 : number, %BB4, 3 : number, %BB5, 4 : number, %BB6, 5 : number, %BB7, 6 : number, %BB8, 8 : number, %BB9, 9 : number, %BB10, 10 : number, %BB11
//CHECK-NEXT: %BB1:
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %3 = ReturnInst %2 : undefined
//CHECK-NEXT: %BB2:
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst "foo" : string
//CHECK-NEXT:  {{.*}}  %5 = ReturnInst %4 : string
//CHECK-NEXT: %BB3:
//CHECK-NEXT:  {{.*}}  %6 = HBCLoadConstInst "bar" : string
//CHECK-NEXT:  {{.*}}  %7 = ReturnInst %6 : string
//CHECK-NEXT: %BB4:
//CHECK-NEXT:  {{.*}}  %8 = HBCLoadConstInst "baz" : string
//CHECK-NEXT:  {{.*}}  %9 = ReturnInst %8 : string
//CHECK-NEXT: %BB5:
//CHECK-NEXT:  {{.*}}  %10 = HBCLoadConstInst "foo1" : string
//CHECK-NEXT:  {{.*}}  %11 = ReturnInst %10 : string
//CHECK-NEXT: %BB6:
//CHECK-NEXT:  {{.*}}  %12 = HBCLoadConstInst "bar2" : string
//CHECK-NEXT:  {{.*}}  %13 = ReturnInst %12 : string
//CHECK-NEXT: %BB7:
//CHECK-NEXT:  {{.*}}  %14 = HBCLoadConstInst "baz3" : string
//CHECK-NEXT:  {{.*}}  %15 = ReturnInst %14 : string
//CHECK-NEXT: %BB8:
//CHECK-NEXT:  {{.*}}  %16 = HBCLoadConstInst "foo4" : string
//CHECK-NEXT:  {{.*}}  %17 = ReturnInst %16 : string
//CHECK-NEXT: %BB9:
//CHECK-NEXT:  {{.*}}  %18 = HBCLoadConstInst "bar5" : string
//CHECK-NEXT:  {{.*}}  %19 = ReturnInst %18 : string
//CHECK-NEXT: %BB10:
//CHECK-NEXT:  {{.*}}  %20 = HBCLoadConstInst "baz6" : string
//CHECK-NEXT:  {{.*}}  %21 = ReturnInst %20 : string
//CHECK-NEXT: %BB11:
//CHECK-NEXT:  {{.*}}  %22 = HBCLoadConstInst "baz9" : string
//CHECK-NEXT:  {{.*}}  %23 = ReturnInst %22 : string
//CHECK-NEXT: function_end

function jump_table(x) {
    switch (x) {
        case 0:
	        return "foo"
        case 1:
	        return "bar"
        case 2:
	        return "baz"
        case 3:
            return "foo1"
        case 4:
            return "bar2"
        case 5:
            return "baz3"
        case 6:
            return "foo4"
        case 8:
            return "bar5"
        case 9:
            return "baz6"
        case 10:
            return "baz9"
    }
}

//CHECK-LABEL:function string_switch(x) : undefined|number
//CHECK-NEXT: frame = []
//CHECK-NEXT: %BB0:
//CHECK-NEXT:  {{.*}}  %0 = HBCLoadParamInst 1 : number
//CHECK-NEXT:  {{.*}}  %1 = BranchInst %BB1
//CHECK-NEXT: %BB2:
//CHECK-NEXT:  {{.*}}  %2 = HBCLoadConstInst undefined : undefined
//CHECK-NEXT:  {{.*}}  %3 = ReturnInst %2 : undefined
//CHECK-NEXT: %BB3:
//CHECK-NEXT:  {{.*}}  %4 = HBCLoadConstInst 1 : number
//CHECK-NEXT:  {{.*}}  %5 = ReturnInst %4 : number
//CHECK-NEXT: %BB4:
//CHECK-NEXT:  {{.*}}  %6 = HBCLoadConstInst 2 : number
//CHECK-NEXT:  {{.*}}  %7 = ReturnInst %6 : number
//CHECK-NEXT: %BB5:
//CHECK-NEXT:  {{.*}}  %8 = HBCLoadConstInst 3 : number
//CHECK-NEXT:  {{.*}}  %9 = ReturnInst %8 : number
//CHECK-NEXT: %BB6:
//CHECK-NEXT:  {{.*}}  %10 = HBCLoadConstInst "c" : string
//CHECK-NEXT:  {{.*}}  %11 = CompareBranchInst '===', %10 : string, %0, %BB5, %BB2
//CHECK-NEXT: %BB7:
//CHECK-NEXT:  {{.*}}  %12 = HBCLoadConstInst "b" : string
//CHECK-NEXT:  {{.*}}  %13 = CompareBranchInst '===', %12 : string, %0, %BB4, %BB6
//CHECK-NEXT: %BB1:
//CHECK-NEXT:  {{.*}}  %14 = HBCLoadConstInst "a" : string
//CHECK-NEXT:  {{.*}}  %15 = CompareBranchInst '===', %14 : string, %0, %BB3, %BB7
//CHECK-NEXT: function_end

function string_switch(x) {
    switch (x) {
    case "a":
	return 1;
    case "b":
	return 2;
    case "c":
	return 3;
    }
}
