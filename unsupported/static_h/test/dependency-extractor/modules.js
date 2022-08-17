/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %dependency-extractor %s | %FileCheck --match-full-lines %s

import {x} from "foo";
// CHECK: ESM | foo
import {type x, y} from "bar";
// CHECK: ESM | bar
// CHECK: Type | bar
import "foo";
// CHECK: ESM | foo

export {x} from "foo";
// CHECK: ESM | foo
export {x} from "foo";
// CHECK: ESM | foo
export * from "foo";
// CHECK: ESM | foo
export type * from "foo";
// CHECK: Type | foo

import('foo');
// CHECK: Async | foo

require('foo');
// CHECK: Require | foo
(require.requireActual)('foo');
// CHECK: Require | foo
(require.requireMock)('foo');
// CHECK: Require | foo
(jest.requireMock)('foo');
// CHECK: Require | foo

__jsResource('foo');
// CHECK: Resource | foo
__jsResource('m#foo2');
// CHECK: Resource | foo2
__conditionallySplitJSResource('foo', {a: 'b'});
// CHECK: Resource | foo
JSResource('foo', {a: 'b'});
// CHECK: Resource | foo
ConditionallySplitJSResource('foo', {a: 'b'});
// CHECK: Resource | foo
PrefetchedJSResource('foo_prefetch', {a: 'b'});
// CHECK: PrefetchedResource | foo_prefetch
prefetchImport('foo', {a: 'b'});
// CHECK: Resource | foo
