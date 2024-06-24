/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 */

let BooleanTypeAnnotation: boolean;
let NumberTypeAnnotation: number;
let StringTypeAnnotation: string;
let MixedTypeAnnotation: mixed;
let EmptyTypeAnnotation: empty;
let VoidTypeAnnotation: void;
let NullLiteralTypeAnnotation: null;
let AnyTypeAnnotation: any;
let StringLiteralTypeAnnotation: '123';
let NumberLiteralTypeAnnotation: 123;
let SymbolTypeAnnotation: symbol;
let BigIntLiteralTypeAnnotation: 10n;
let ExistsTypeAnnotation: *;
let BooleanLiteralTypeAnnotation: true;
let GenericTypeAnnotation: T;
let FunctionTypeAnnotation: (this: User, str: string, bool?: boolean, ...nums: Array<number>) => void;
type NullableTypeAnnotation = ?string;
type QualifiedTypeIdentifier = Foo.Bar;
type TypeofTypeAnnotation = typeof Foo;
type QualifiedTypeofIdentifier = typeof Foo.Bar;
type QualifiedTypeofIdentifierNested = typeof Foo.Bar.Mark;
type KeyofTypeAnnotation = keyof T;
type UnionTypeAnnotation = "a" | "b" | "c";
type IntersectionTypeAnnotation = "a" & "b" & "c";
type IndexedAccessType = Account['account_id'];
type OptionalIndexedAccessType = Account?.['account_id'];
type TupleTypeAnnotation = [A,B]
