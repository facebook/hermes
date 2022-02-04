/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

export type BlockStatementProps = {
  +body: $ReadOnlyArray<DetachedNode<BlockStatementType['body'][number]>>,
};
export function BlockStatement({
  parent,
  ...props
}: {
  ...$ReadOnly<BlockStatementProps>,
  +parent?: ESNode,
}): DetachedNode<BlockStatementType> {
  const node = detachedProps<BlockStatementType>(parent, {
    type: 'BlockStatement',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type ExpressionStatementProps = {
  +expression: DetachedNode<ExpressionStatementType['expression']>,
  +directive?: ?ExpressionStatementType['directive'],
};
export function ExpressionStatement({
  parent,
  ...props
}: {
  ...$ReadOnly<ExpressionStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ExpressionStatementType> {
  const node = detachedProps<ExpressionStatementType>(parent, {
    type: 'ExpressionStatement',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type NumberTypeAnnotationProps = {};
export function NumberTypeAnnotation({
  parent,
}: {
  +parent?: ESNode,
} = {}): DetachedNode<NumberTypeAnnotationType> {
  return detachedProps<NumberTypeAnnotationType>(parent, {
    type: 'NumberTypeAnnotation',
  });
}

export type VariableDeclarationProps = {
  +kind: VariableDeclarationType['kind'],
  +declarations: $ReadOnlyArray<
    DetachedNode<VariableDeclarationType['declarations'][number]>,
  >,
};
export function VariableDeclaration({
  parent,
  ...props
}: {
  ...$ReadOnly<VariableDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclarationType> {
  const node = detachedProps<VariableDeclarationType>(parent, {
    type: 'VariableDeclaration',
    ...props,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type VariableDeclaratorProps = {
  +init?: ?DetachedNode<VariableDeclaratorType['init']>,
  +id: DetachedNode<VariableDeclaratorType['id']>,
};
export function VariableDeclarator({
  parent,
  ...props
}: {
  ...$ReadOnly<VariableDeclaratorProps>,
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
