/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

/*
 * !!! THIS FILE WILL BE OVERWRITTEN BY CODEGEN !!!
 *
 * Statically it should only contain the minimal set of
 * definitions required to typecheck the code and execute
 * the transformation tests.
 */

import type {
  ESNode,
  BlockStatement as BlockStatementType,
  ExpressionStatement as ExpressionStatementType,
  NumberTypeAnnotation as NumberTypeAnnotationType,
  VariableDeclaration as VariableDeclarationType,
  VariableDeclarator as VariableDeclaratorType,
} from 'hermes-estree';
import type {DetachedNode} from '../detachedNode';

import {
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

export function BlockStatement({
  parent,
  ...props
}: {
  +body: $ReadOnlyArray<DetachedNode<BlockStatementType['body'][number]>>,
  +parent?: ESNode,
}): DetachedNode<BlockStatementType> {
  const node = detachedProps<BlockStatementType>(parent, {
    type: 'BlockStatement',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function ExpressionStatement({
  parent,
  ...props
}: {
  +expression: DetachedNode<ExpressionStatementType['expression']>,
  +directive?: ?ExpressionStatementType['directive'],
  +parent?: ESNode,
}): DetachedNode<ExpressionStatementType> {
  const node = detachedProps<ExpressionStatementType>(parent, {
    type: 'ExpressionStatement',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function NumberTypeAnnotation({
  parent,
}: {
  +parent?: ESNode,
} = {}): DetachedNode<NumberTypeAnnotationType> {
  return detachedProps<NumberTypeAnnotationType>(parent, {
    type: 'NumberTypeAnnotation',
  });
}

export function VariableDeclaration({
  parent,
  ...props
}: {
  +kind: VariableDeclarationType['kind'],
  +declarations: $ReadOnlyArray<
    DetachedNode<VariableDeclarationType['declarations'][number]>,
  >,
  +parent?: ESNode,
}): DetachedNode<VariableDeclarationType> {
  const node = detachedProps<VariableDeclarationType>(parent, {
    type: 'VariableDeclaration',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export function VariableDeclarator({
  parent,
  ...props
}: {
  +init?: ?DetachedNode<VariableDeclaratorType['init']>,
  +id: DetachedNode<VariableDeclaratorType['id']>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclaratorType> {
  const node = detachedProps<VariableDeclaratorType>(parent, {
    type: 'VariableDeclarator',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export * from './special-case-node-types';
