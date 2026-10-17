// RUN: %shermes -typed -O0 -dump-ir  %s | %FileCheckOrRegen %s

'use strict';

class IRGetterBase {
  static _storageVal: string = "Moth man";
  static field: number = 0;
  @Hermes.final
  static get answer(): string {
    return IRGetterBase._storageVal;
  }
  @Hermes.final
  static set answer(value: string) {
    IRGetterBase._storageVal = value;
  }
}

class IRGetterChild extends IRGetterBase {
  static readGetter(): string {
    return super.answer;
  }
  static writeSetter(val: string): void {
    super.answer = val;
  }

  static compound(): string {
    return super.answer += '!';
  }

  static writeField(): void {
    // Preserve the current frame-storage implementation. This is not a
    // runtime test claiming receiver-based property shadowing is implemented.
    super.field = 42;
  }
}

IRGetterChild.writeField();
IRGetterChild.writeSetter("Mummy");
IRGetterChild.compound();
print(IRGetterChild.answer);

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:       StoreStackInst %4: any, %1: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [__SameValueZero: any, __ToIntegerOrInfinity: any, charAt: any, at: any, charCodeAt: any, codePointAt: any, concat: any, endsWith: any, includes: any, indexOf: any, lastIndexOf: any, padEnd: any, padStart: any, repeat: any, slice: any, split: any, startsWith: any, substring: any, toLowerCase: any, toUpperCase: any, trim: any, trimStart: any, trimEnd: any, toFixed: any, toPrecision: any, toExponential: any, toString: any]

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS1: any, %__SameValueZero(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS1.__SameValueZero]: any
// CHECK-NEXT:  %4 = CreateFunctionInst (:object) %1: environment, %VS1: any, %__ToIntegerOrInfinity(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: object, [%VS1.__ToIntegerOrInfinity]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %charAt(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.charAt]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %at(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS1.at]: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %VS1: any, %charCodeAt(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS1.charCodeAt]: any
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %1: environment, %VS1: any, %codePointAt(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS1.codePointAt]: any
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %VS1: any, %concat(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.concat]: any
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %1: environment, %VS1: any, %endsWith(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: object, [%VS1.endsWith]: any
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %1: environment, %VS1: any, %includes(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [%VS1.includes]: any
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %1: environment, %VS1: any, %indexOf(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: object, [%VS1.indexOf]: any
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %1: environment, %VS1: any, %lastIndexOf(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %22: object, [%VS1.lastIndexOf]: any
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %1: environment, %VS1: any, %padEnd(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %24: object, [%VS1.padEnd]: any
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %1: environment, %VS1: any, %padStart(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %26: object, [%VS1.padStart]: any
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %1: environment, %VS1: any, %repeat(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %28: object, [%VS1.repeat]: any
// CHECK-NEXT:  %30 = CreateFunctionInst (:object) %1: environment, %VS1: any, %slice(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: object, [%VS1.slice]: any
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %1: environment, %VS1: any, %split(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %32: object, [%VS1.split]: any
// CHECK-NEXT:  %34 = CreateFunctionInst (:object) %1: environment, %VS1: any, %startsWith(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %34: object, [%VS1.startsWith]: any
// CHECK-NEXT:  %36 = CreateFunctionInst (:object) %1: environment, %VS1: any, %substring(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %36: object, [%VS1.substring]: any
// CHECK-NEXT:  %38 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toLowerCase(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %38: object, [%VS1.toLowerCase]: any
// CHECK-NEXT:  %40 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toUpperCase(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %40: object, [%VS1.toUpperCase]: any
// CHECK-NEXT:  %42 = CreateFunctionInst (:object) %1: environment, %VS1: any, %trim(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %42: object, [%VS1.trim]: any
// CHECK-NEXT:  %44 = CreateFunctionInst (:object) %1: environment, %VS1: any, %trimStart(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %44: object, [%VS1.trimStart]: any
// CHECK-NEXT:  %46 = CreateFunctionInst (:object) %1: environment, %VS1: any, %trimEnd(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %46: object, [%VS1.trimEnd]: any
// CHECK-NEXT:  %48 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toFixed(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %48: object, [%VS1.toFixed]: any
// CHECK-NEXT:  %50 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toPrecision(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %50: object, [%VS1.toPrecision]: any
// CHECK-NEXT:  %52 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toExponential(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %52: object, [%VS1.toExponential]: any
// CHECK-NEXT:  %54 = CreateFunctionInst (:object) %1: environment, %VS1: any, %toString(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %54: object, [%VS1.toString]: any
// CHECK-NEXT:  %56 = CreateFunctionInst (:object) %1: environment, %VS1: any, %" 1#"(): functionCode
// CHECK-NEXT:  %57 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %58 = CallInst [njsf] (:any) %56: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %57: object
// CHECK-NEXT:        ReturnInst %58: any
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any, y: any]

// CHECK:function __SameValueZero(x: any, y: any): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.x]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %y: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS2.y]: any
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %8 = BinaryStrictlyNotEqualInst (:boolean) %6: any, %7: any
// CHECK-NEXT:       CondBranchInst %8: boolean, %BB4, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst true: boolean
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS2.x]: any
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS2.y]: any
// CHECK-NEXT:  %14 = BinaryStrictlyEqualInst (:boolean) %12: any, %13: any
// CHECK-NEXT:        ReturnInst %14: boolean
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS2.y]: any
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS2.y]: any
// CHECK-NEXT:  %18 = BinaryStrictlyNotEqualInst (:boolean) %16: any, %17: any
// CHECK-NEXT:        CondBranchInst %18: boolean, %BB1, %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [n: any, trunc: any]

// CHECK:function __ToIntegerOrInfinity(n: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:number) %n: number
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: number, [%VS3.n]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.trunc]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS3.n]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:  %7 = LoadFrameInst (:any) %1: environment, [%VS3.n]: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:number) %7: any, type(number)
// CHECK-NEXT:  %9 = BinaryStrictlyNotEqualInst (:boolean) %6: number, %8: number
// CHECK-NEXT:        CondBranchInst %9: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst 0: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS3.n]: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:number) %13: any, type(number)
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS3.n]: any
// CHECK-NEXT:  %16 = CheckedTypeCastInst (:number) %15: any, type(number)
// CHECK-NEXT:  %17 = BinaryModuloInst (:any) %16: number, 1: number
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:number) %17: any, type(number)
// CHECK-NEXT:  %19 = BinarySubtractInst (:any) %14: number, %18: number
// CHECK-NEXT:  %20 = CheckedTypeCastInst (:number) %19: any, type(number)
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: number, [%VS3.trunc]: any
// CHECK-NEXT:  %22 = LoadFrameInst (:any) %1: environment, [%VS3.trunc]: any
// CHECK-NEXT:  %23 = CheckedTypeCastInst (:number) %22: any, type(number)
// CHECK-NEXT:  %24 = LoadFrameInst (:any) %1: environment, [%VS3.trunc]: any
// CHECK-NEXT:  %25 = CheckedTypeCastInst (:number) %24: any, type(number)
// CHECK-NEXT:  %26 = BinaryStrictlyNotEqualInst (:boolean) %23: number, %25: number
// CHECK-NEXT:        CondBranchInst %26: boolean, %BB4, %BB5
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %28 = LoadFrameInst (:any) %1: environment, [%VS3.n]: any
// CHECK-NEXT:  %29 = CheckedTypeCastInst (:number) %28: any, type(number)
// CHECK-NEXT:        ReturnInst %29: number
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB6
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %32 = LoadFrameInst (:any) %1: environment, [%VS3.trunc]: any
// CHECK-NEXT:  %33 = CheckedTypeCastInst (:number) %32: any, type(number)
// CHECK-NEXT:        ReturnInst %33: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [pos: any]

// CHECK:function charAt(pos: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %pos: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS4.pos]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS4.pos]: any
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %0: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [index: any]

// CHECK:function at(index: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %index: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS5.index]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "String": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "at": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS5.index]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [pos: any]

// CHECK:function charCodeAt(pos: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS6: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %pos: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS6.pos]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "String": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "charCodeAt": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS6.pos]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [pos: any]

// CHECK:function codePointAt(pos: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS7: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %pos: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS7.pos]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "String": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "codePointAt": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS7.pos]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS8 [other: any]

// CHECK:function concat(other: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS8: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %other: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS8.other]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS8.other]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) %0: any, %5: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS9 [searchString: any, endPosition: any]

// CHECK:function endsWith(searchString: any, endPosition: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS9: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %searchString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS9.searchString]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %endPosition: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS9.endPosition]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "endsWith": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS9.searchString]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS9.endPosition]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS10 [searchString: any, position: any]

// CHECK:function includes(searchString: any, position: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS10: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %searchString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS10.searchString]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %position: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS10.position]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "includes": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS10.searchString]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS10.position]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS11 [searchString: any, position: any]

// CHECK:function indexOf(searchString: any, position: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS11: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %searchString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS11.searchString]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %position: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS11.position]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "indexOf": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS11.searchString]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS11.position]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS12 [searchString: any, position: any]

// CHECK:function lastIndexOf(searchString: any, position: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS12: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %searchString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS12.searchString]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %position: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS12.position]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "lastIndexOf": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS12.searchString]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS12.position]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS13 [targetLength: any, padString: any]

// CHECK:function padEnd(targetLength: any, padString: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS13: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %targetLength: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS13.targetLength]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %padString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS13.padString]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "padEnd": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS13.targetLength]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS13.padString]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS14 [targetLength: any, padString: any]

// CHECK:function padStart(targetLength: any, padString: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS14: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %targetLength: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS14.targetLength]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %padString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS14.padString]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "padStart": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS14.targetLength]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS14.padString]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS15 [count: any]

// CHECK:function repeat(count: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS15: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %count: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS15.count]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "String": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "repeat": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS15.count]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS16 [start: any, end: any]

// CHECK:function slice(start: any, end: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS16: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %start: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS16.start]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %end: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS16.end]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "slice": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS16.start]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS16.end]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS17 [separator: any, limit: any]

// CHECK:function split(separator: any, limit: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS17: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %separator: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS17.separator]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %limit: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS17.limit]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "split": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS17.separator]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS17.limit]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:        ReturnInst %13: any
// CHECK-NEXT:function_end

// CHECK:scope %VS18 [searchString: any, position: any]

// CHECK:function startsWith(searchString: any, position: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS18: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %searchString: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS18.searchString]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %position: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS18.position]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "startsWith": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS18.searchString]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS18.position]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS19 [start: any, end: any]

// CHECK:function substring(start: any, end: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS19: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %start: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS19.start]: any
// CHECK-NEXT:  %5 = LoadParamInst (:any) %end: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %5: any, [%VS19.end]: any
// CHECK-NEXT:  %7 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "String": string
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "prototype": string
// CHECK-NEXT:  %10 = LoadPropertyInst (:any) %9: any, "substring": string
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %2: environment, [%VS19.start]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %2: environment, [%VS19.end]: any
// CHECK-NEXT:  %13 = CallInst (:any) %10: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %11: any, %12: any
// CHECK-NEXT:  %14 = CheckedTypeCastInst (:any) %13: any, type(any)
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS20 []

// CHECK:function toLowerCase(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS20: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "String": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "toLowerCase": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:any) %7: any, type(any)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS21 []

// CHECK:function toUpperCase(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS21: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "String": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "toUpperCase": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:any) %7: any, type(any)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS22 []

// CHECK:function trim(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS22: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "String": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "trim": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:any) %7: any, type(any)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS23 []

// CHECK:function trimStart(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS23: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "String": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "trimStart": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:any) %7: any, type(any)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS24 []

// CHECK:function trimEnd(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS24: any, %1: environment
// CHECK-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) %3: any, "String": string
// CHECK-NEXT:  %5 = LoadPropertyInst (:any) %4: any, "prototype": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "trimEnd": string
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:any) %7: any, type(any)
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS25 [digits: any]

// CHECK:function toFixed(digits: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS25: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %digits: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS25.digits]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "Number": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "toFixed": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS25.digits]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS26 [precision: any]

// CHECK:function toPrecision(precision: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS26: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %precision: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS26.precision]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "Number": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "toPrecision": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS26.precision]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS27 [digits: any]

// CHECK:function toExponential(digits: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS27: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %digits: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS27.digits]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "Number": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "toExponential": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS27.digits]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS28 [radix: any]

// CHECK:function toString(radix: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS28: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:any) %radix: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: any, [%VS28.radix]: any
// CHECK-NEXT:  %5 = TryLoadGlobalPropertyInst (:any) globalObject: object, "globalThis": string
// CHECK-NEXT:  %6 = LoadPropertyInst (:any) %5: any, "Number": string
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) %6: any, "prototype": string
// CHECK-NEXT:  %8 = LoadPropertyInst (:any) %7: any, "toString": string
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %2: environment, [%VS28.radix]: any
// CHECK-NEXT:  %10 = CallInst (:any) %8: any, empty: any, false: boolean, empty: any, undefined: undefined, %0: any, %9: any
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:any) %10: any, type(any)
// CHECK-NEXT:        ReturnInst %11: any
// CHECK-NEXT:function_end

// CHECK:scope %VS29 [exports: any, IRGetterBase: any, IRGetterChild: any, _storageVal: string, field: number, answer: object, answer#1: object, ?IRGetterBase.prototype: object, readGetter: object, writeSetter: object, compound: object, writeField: object, ?IRGetterChild.prototype: object]

// CHECK:function " 1#"(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS29: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS29.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS29.IRGetterBase]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS29.IRGetterChild]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, "Moth man": string, [%VS29._storageVal]: string
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS29.field]: number
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS29: any, %answer(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: object, [%VS29.answer]: object
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %1: environment, %VS29: any, %"answer 1#"(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: object, [%VS29.answer#1]: object
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %1: environment, %VS29: any, %IRGetterBase(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: object, [%VS29.IRGetterBase]: any
// CHECK-NEXT:  %14 = AllocTypedNonEnumObjectInst (:object) null: null
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS29.?IRGetterBase.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %14: object, %12: object, "prototype": string
// CHECK-NEXT:  %17 = LoadFrameInst (:any) %1: environment, [%VS29.IRGetterBase]: any
// CHECK-NEXT:  %18 = CheckedTypeCastInst (:object) %17: any, type(object)
// CHECK-NEXT:  %19 = CreateFunctionInst (:object) %1: environment, %VS29: any, %readGetter(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %19: object, [%VS29.readGetter]: object
// CHECK-NEXT:  %21 = CreateFunctionInst (:object) %1: environment, %VS29: any, %writeSetter(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: object, [%VS29.writeSetter]: object
// CHECK-NEXT:  %23 = CreateFunctionInst (:object) %1: environment, %VS29: any, %compound(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %23: object, [%VS29.compound]: object
// CHECK-NEXT:  %25 = CreateFunctionInst (:object) %1: environment, %VS29: any, %writeField(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %25: object, [%VS29.writeField]: object
// CHECK-NEXT:  %27 = CreateFunctionInst (:object) %1: environment, %VS29: any, %IRGetterChild(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %27: object, [%VS29.IRGetterChild]: any
// CHECK-NEXT:  %29 = LoadFrameInst (:object) %1: environment, [%VS29.?IRGetterBase.prototype]: object
// CHECK-NEXT:  %30 = AllocTypedNonEnumObjectInst (:object) %29: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %30: object, [%VS29.?IRGetterChild.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %30: object, %27: object, "prototype": string
// CHECK-NEXT:  %33 = LoadFrameInst (:any) %1: environment, [%VS29.IRGetterChild]: any
// CHECK-NEXT:  %34 = CheckedTypeCastInst (:object) %33: any, type(object)
// CHECK-NEXT:  %35 = LoadFrameInst (:object) %1: environment, [%VS29.writeField]: object
// CHECK-NEXT:  %36 = CallInst [njsf] (:undefined) %35: object, %writeField(): functionCode, true: boolean, empty: any, undefined: undefined, %34: object
// CHECK-NEXT:  %37 = LoadFrameInst (:any) %1: environment, [%VS29.IRGetterChild]: any
// CHECK-NEXT:  %38 = CheckedTypeCastInst (:object) %37: any, type(object)
// CHECK-NEXT:  %39 = LoadFrameInst (:object) %1: environment, [%VS29.writeSetter]: object
// CHECK-NEXT:  %40 = CallInst [njsf] (:undefined) %39: object, %writeSetter(): functionCode, true: boolean, empty: any, undefined: undefined, %38: object, "Mummy": string
// CHECK-NEXT:  %41 = LoadFrameInst (:any) %1: environment, [%VS29.IRGetterChild]: any
// CHECK-NEXT:  %42 = CheckedTypeCastInst (:object) %41: any, type(object)
// CHECK-NEXT:  %43 = LoadFrameInst (:object) %1: environment, [%VS29.compound]: object
// CHECK-NEXT:  %44 = CallInst [njsf] (:string) %43: object, %compound(): functionCode, true: boolean, empty: any, undefined: undefined, %42: object
// CHECK-NEXT:  %45 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %46 = LoadFrameInst (:any) %1: environment, [%VS29.IRGetterChild]: any
// CHECK-NEXT:  %47 = CheckedTypeCastInst (:object) %46: any, type(object)
// CHECK-NEXT:  %48 = LoadFrameInst (:object) %1: environment, [%VS29.answer]: object
// CHECK-NEXT:  %49 = CallInst [njsf] (:string) %48: object, %answer(): functionCode, true: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %50 = CallInst (:any) %45: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %49: string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS30 []

// CHECK:function answer(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS30: any, %0: environment
// CHECK-NEXT:  %2 = LoadFrameInst (:any) %0: environment, [%VS29.IRGetterBase]: any
// CHECK-NEXT:  %3 = CheckedTypeCastInst (:object) %2: any, type(object)
// CHECK-NEXT:  %4 = LoadFrameInst (:string) %0: environment, [%VS29._storageVal]: string
// CHECK-NEXT:       ReturnInst %4: string
// CHECK-NEXT:function_end

// CHECK:scope %VS31 [value: any]

// CHECK:function "answer 1#"(value: string): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS31: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:string) %value: string
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: string, [%VS31.value]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %0: environment, [%VS29.IRGetterBase]: any
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:object) %4: any, type(object)
// CHECK-NEXT:  %6 = LoadFrameInst (:any) %1: environment, [%VS31.value]: any
// CHECK-NEXT:  %7 = CheckedTypeCastInst (:string) %6: any, type(string)
// CHECK-NEXT:       StoreFrameInst %0: environment, %7: string, [%VS29._storageVal]: string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS32 []

// CHECK:function IRGetterBase(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS32: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS33 []

// CHECK:function readGetter(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS33: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS29.answer]: object
// CHECK-NEXT:  %4 = CallInst [njsf] (:string) %3: object, %answer(): functionCode, true: boolean, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:       ReturnInst %4: string
// CHECK-NEXT:function_end

// CHECK:scope %VS34 [val: any]

// CHECK:function writeSetter(val: string): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS34: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:string) %val: string
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: string, [%VS34.val]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS34.val]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:string) %5: any, type(string)
// CHECK-NEXT:  %7 = LoadFrameInst (:object) %1: environment, [%VS29.answer#1]: object
// CHECK-NEXT:  %8 = CallInst [njsf] (:undefined) %7: object, %"answer 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %0: object, %6: string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS35 []

// CHECK:function compound(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS35: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS29.answer]: object
// CHECK-NEXT:  %4 = CallInst [njsf] (:string) %3: object, %answer(): functionCode, true: boolean, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %5 = BinaryAddInst (:any) %4: string, "!": string
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:string) %5: any, type(string)
// CHECK-NEXT:  %7 = LoadFrameInst (:object) %1: environment, [%VS29.answer#1]: object
// CHECK-NEXT:  %8 = CallInst [njsf] (:undefined) %7: object, %"answer 1#"(): functionCode, true: boolean, empty: any, undefined: undefined, %0: object, %6: string
// CHECK-NEXT:       ReturnInst %6: string
// CHECK-NEXT:function_end

// CHECK:scope %VS36 []

// CHECK:function writeField(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS36: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %0: environment, 42: number, [%VS29.field]: number
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS37 []

// CHECK:function IRGetterChild(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS29: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS37: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
