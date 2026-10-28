/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror --typed --dump-sema %s | %FileCheck %s

type ArrayAlias<T> = T[];
type RecursiveArrayAlias =
  string | number | ArrayAlias<RecursiveArrayAlias>;
let recursiveArrayAlias: RecursiveArrayAlias;

type GenericArrayAlias<T> = Array<T>;
type RecursiveGenericArrayAlias =
  string | number | GenericArrayAlias<RecursiveGenericArrayAlias>;
let recursiveGenericArrayAlias: RecursiveGenericArrayAlias;

type ArrayAliasChain<T> = ArrayAlias<T>;
type RecursiveArrayAliasChain =
  string | number | ArrayAliasChain<RecursiveArrayAliasChain>;
let recursiveArrayAliasChain: RecursiveArrayAliasChain;

class Box<T> {
  value: T;
}
type BoxAlias<T> = Box<T>;
type RecursiveBoxAlias =
  string | number | BoxAlias<RecursiveBoxAlias>;
let recursiveBoxAlias: RecursiveBoxAlias;

type BoxAliasChain<T> = BoxAlias<T>;
type RecursiveBoxAliasChain =
  string | number | BoxAliasChain<RecursiveBoxAliasChain>;
let recursiveBoxAliasChain: RecursiveBoxAliasChain;

type MutualArrayAliasA<T> = MutualArrayAliasB<T>;
type MutualArrayAliasB<T> = Array<MutualArrayAliasA<T>>;
let mutualArrayAlias: MutualArrayAliasA<number>;

type MutualBoxAliasA<T> = MutualBoxAliasB<T>;
type MutualBoxAliasB<T> = Box<MutualBoxAliasA<T>>;
let mutualBoxAlias: MutualBoxAliasA<number>;

type Id<T> = T;
type Maybe<T> = ?T;

type DirectArrayAlias = ArrayAlias<DirectArrayAlias>;
let directArrayAlias: DirectArrayAlias;
type NestedArrayAlias = ArrayAlias<ArrayAlias<NestedArrayAlias>>;
let nestedArrayAlias: NestedArrayAlias;
type DirectBoxAlias = BoxAlias<DirectBoxAlias>;
let directBoxAlias: DirectBoxAlias;
type ReusedBoxAlias = BoxAlias<DirectBoxAlias>;
let reusedBoxAlias: ReusedBoxAlias;
type NestedBoxAlias = BoxAlias<BoxAlias<NestedBoxAlias>>;
let nestedBoxAlias: NestedBoxAlias;

type ArrayUnionAlias<T> = number | Array<T>;
type RecursiveArrayUnionAlias = ArrayUnionAlias<RecursiveArrayUnionAlias>;
let recursiveArrayUnionAlias: RecursiveArrayUnionAlias;
type BoxUnionAlias<T> = number | Box<T>;
type RecursiveBoxUnionAlias = BoxUnionAlias<RecursiveBoxUnionAlias>;
let recursiveBoxUnionAlias: RecursiveBoxUnionAlias;

// Recursion through a generic alias used as a generic class type argument.
type TypeArgArray = Id<TypeArgArray>[];
let typeArgArray: TypeArgArray;
type TypeArgBox = Box<Id<TypeArgBox>>;
let typeArgBox: TypeArgBox;
type MutualTypeArgBoxA = Box<Id<MutualTypeArgBoxB>>;
type MutualTypeArgBoxB = Box<Id<MutualTypeArgBoxA>>;
let mutualTypeArgBoxA: MutualTypeArgBoxA;
let mutualTypeArgBoxB: MutualTypeArgBoxB;

type MaybeArray = number | Array<Maybe<MaybeArray>>;
let maybeArray: MaybeArray;
type MaybeBox = number | Box<Maybe<MaybeBox>>;
let maybeBox: MaybeBox;

// Unions that collapse to the class that is still being completed.
type DuplicateUnionArray =
  Id<Array<DuplicateUnionArray>> | Id<Array<DuplicateUnionArray>>;
let duplicateUnionArray: DuplicateUnionArray;
type DuplicateUnionBox =
  Id<Box<DuplicateUnionBox>> | Id<Box<DuplicateUnionBox>>;
let duplicateUnionBox: DuplicateUnionBox;
type Duplicate<T> = T | T;
type DuplicateArgArray = Array<Duplicate<DuplicateArgArray>>;
let duplicateArgArray: DuplicateArgArray;
type DuplicateArgBox = Box<Duplicate<DuplicateArgBox>>;
let duplicateArgBox: DuplicateArgBox;

type ObjectAlias<T> = {value: T};
type NestedObjectAlias = ObjectAlias<ObjectAlias<NestedObjectAlias>>;
let nestedObjectAlias: NestedObjectAlias;
type FunctionAlias<T> = (value: T) => T;
type NestedFunctionAlias = FunctionAlias<FunctionAlias<NestedFunctionAlias>>;
let nestedFunctionAlias: NestedFunctionAlias;
type ObjectId<T> = {value: ObjectId<Id<T>>};
let objectId: ObjectId<number>;

// Type arguments that don't reach the result don't grow it.
type Erase<T> = string;
type Erased<T> = [Erased<Erase<T>>];
let erased: Erased<number>;

// Type arguments that repeat after a few expansions hit the cache.
type Converging<X, Y> = [Converging<Box<Y>, string>];
let converging: Converging<number, boolean>;
let convergingEqual: Converging<boolean, boolean>;
type ConvergingBoxes<X, Y> = [ConvergingBoxes<Box<Y>, Box<boolean>>];
let convergingBoxes: ConvergingBoxes<boolean, boolean>;

// CHECK: [[AU1:%union\.[0-9]+]] = union(string | number |
// CHECK-SAME: [[AC1:%class\.[0-9]+]])
// CHECK: [[AU2:%union\.[0-9]+]] = union(string | number |
// CHECK-SAME: [[AC2:%class\.[0-9]+]])
// CHECK: [[AU3:%union\.[0-9]+]] = union(string | number |
// CHECK-SAME: [[AC3:%class\.[0-9]+]])
// CHECK: [[BU1:%union\.[0-9]+]] = union(string | number |
// CHECK-SAME: [[BC1:%class\.[0-9]+]])
// CHECK: [[BU2:%union\.[0-9]+]] = union(string | number |
// CHECK-SAME: [[BC2:%class\.[0-9]+]])
// CHECK: [[DA:%class\.[0-9]+]] = class(Array<[[DA]]>)
// CHECK: [[NA:%class\.[0-9]+]] = class(Array<[[NAI:%class\.[0-9]+]]>)
// CHECK: [[DB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[DB]]
// CHECK: [[NB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[NBI:%class\.[0-9]+]]
// CHECK: [[EAU:%union\.[0-9]+]] = union(number | [[EAUC:%class\.[0-9]+]])
// CHECK: [[EBU:%union\.[0-9]+]] = union(number | [[EBUC:%class\.[0-9]+]])
// CHECK: [[TA:%class\.[0-9]+]] = class(Array<[[TA]]>)
// CHECK: [[TB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[TB]]
// CHECK: [[TMA:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[TMB:%class\.[0-9]+]]
// CHECK: [[TMB]] = class(Box {
// CHECK:   value: [[TMA]]
// CHECK: [[MAU:%union\.[0-9]+]] = union(number | [[MAC:%class\.[0-9]+]])
// CHECK: [[MBU:%union\.[0-9]+]] = union(number | [[MBC:%class\.[0-9]+]])
// CHECK: [[DUA:%class\.[0-9]+]] = class(Array<[[DUA]]>)
// CHECK: [[DUB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[DUB]]
// CHECK: [[DGA:%class\.[0-9]+]] = class(Array<[[DGA]]>)
// CHECK: [[DGB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[DGB]]
// CHECK: [[OBJ:%object\.[0-9]+]] = object({
// CHECK:   value: [[OBJ]]
// CHECK: [[FN:%function\.[0-9]+]] = function(value: [[FN]]): [[FN]]
// CHECK: [[NBI]] = class(Box {
// CHECK:   value: [[NB]]
// CHECK: [[MBI:%union\.[0-9]+]] = union(void | null | number | [[MBC]])
// CHECK: [[MA:%class\.[0-9]+]] = class(Array<[[MA]]>)
// CHECK: [[MB:%class\.[0-9]+]] = class(Box {
// CHECK:   value: [[MB]]
// CHECK: [[TUP:%tuple\.[0-9]+]] = tuple([[TUP]])
// CHECK: [[AC1]] = class(Array<[[AU1]]>)
// CHECK: [[AC2]] = class(Array<[[AU2]]>)
// CHECK: [[AC3]] = class(Array<[[AU3]]>)
// CHECK: [[BC1]] = class(Box {
// CHECK:   value: [[BU1]]
// CHECK: [[BC2]] = class(Box {
// CHECK:   value: [[BU2]]
// CHECK: [[NAI]] = class(Array<[[NA]]>)
// CHECK: [[EAUC]] = class(Array<[[EAU]]>)
// CHECK: [[EBUC]] = class(Box {
// CHECK:   value: [[EBU]]
// CHECK: [[MAC]] = class(Array<[[MAI:%union\.[0-9]+]]>)
// CHECK: [[MBC]] = class(Box {
// CHECK:   value: [[MBI]]
// CHECK: [[MAI]] = union(void | null | number | [[MAC]])
// CHECK: Decl {{.*}} 'recursiveArrayAlias' Let : [[AU1]]
// CHECK-NEXT: Decl {{.*}} 'recursiveGenericArrayAlias' Let : [[AU2]]
// CHECK-NEXT: Decl {{.*}} 'recursiveArrayAliasChain' Let : [[AU3]]
// CHECK: Decl {{.*}} 'recursiveBoxAlias' Let : [[BU1]]
// CHECK-NEXT: Decl {{.*}} 'recursiveBoxAliasChain' Let : [[BU2]]
// CHECK-NEXT: Decl {{.*}} 'mutualArrayAlias' Let : [[MA]]
// CHECK-NEXT: Decl {{.*}} 'mutualBoxAlias' Let : [[MB]]
// CHECK-NEXT: Decl {{.*}} 'directArrayAlias' Let : [[DA]]
// CHECK-NEXT: Decl {{.*}} 'nestedArrayAlias' Let : [[NA]]
// CHECK-NEXT: Decl {{.*}} 'directBoxAlias' Let : [[DB]]
// CHECK-NEXT: Decl {{.*}} 'reusedBoxAlias' Let : [[DB]]
// CHECK-NEXT: Decl {{.*}} 'nestedBoxAlias' Let : [[NB]]
// CHECK-NEXT: Decl {{.*}} 'recursiveArrayUnionAlias' Let : [[EAU]]
// CHECK-NEXT: Decl {{.*}} 'recursiveBoxUnionAlias' Let : [[EBU]]
// CHECK-NEXT: Decl {{.*}} 'typeArgArray' Let : [[TA]]
// CHECK-NEXT: Decl {{.*}} 'typeArgBox' Let : [[TB]]
// CHECK-NEXT: Decl {{.*}} 'mutualTypeArgBoxA' Let : [[TMA]]
// CHECK-NEXT: Decl {{.*}} 'mutualTypeArgBoxB' Let : [[TMB]]
// CHECK-NEXT: Decl {{.*}} 'maybeArray' Let : [[MAU]]
// CHECK-NEXT: Decl {{.*}} 'maybeBox' Let : [[MBU]]
// CHECK-NEXT: Decl {{.*}} 'duplicateUnionArray' Let : [[DUA]]
// CHECK-NEXT: Decl {{.*}} 'duplicateUnionBox' Let : [[DUB]]
// CHECK-NEXT: Decl {{.*}} 'duplicateArgArray' Let : [[DGA]]
// CHECK-NEXT: Decl {{.*}} 'duplicateArgBox' Let : [[DGB]]
// CHECK-NEXT: Decl {{.*}} 'nestedObjectAlias' Let : [[OBJ]]
// CHECK-NEXT: Decl {{.*}} 'nestedFunctionAlias' Let : [[FN]]
// CHECK-NEXT: Decl {{.*}} 'objectId' Let : [[OBJ]]
// CHECK-NEXT: Decl {{.*}} 'erased' Let : [[TUP]]
// CHECK-NEXT: Decl {{.*}} 'converging' Let : [[TUP]]
// CHECK-NEXT: Decl {{.*}} 'convergingEqual' Let : [[TUP]]
// CHECK-NEXT: Decl {{.*}} 'convergingBoxes' Let : [[TUP]]
