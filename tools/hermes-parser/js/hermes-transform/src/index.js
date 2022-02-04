/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

export type {TraversalContext, Visitor} from './traverse/traverse';
export type {TransformVisitor} from './transform/transform';
export type {DetachedNode} from './detachedNode';

export {traverse, traverseWithContext} from './traverse/traverse';
export {transform} from './transform/transform';
export * as t from './generated/node-types';
