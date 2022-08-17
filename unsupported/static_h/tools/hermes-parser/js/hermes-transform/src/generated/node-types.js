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
  CallExpression as CallExpressionType,
  ExpressionStatement as ExpressionStatementType,
  NumberTypeAnnotation as NumberTypeAnnotationType,
  PropertyDefinition as PropertyDefinitionType,
  VariableDeclaration as VariableDeclarationType,
  VariableDeclarator as VariableDeclaratorType,
} from 'hermes-estree';
import type {DetachedNode, MaybeDetachedNode} from '../detachedNode';

import {
  asDetachedNode,
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

export type BlockStatementProps = {
  +body: $ReadOnlyArray<MaybeDetachedNode<BlockStatementType['body'][number]>>,
};
export function BlockStatement(props: {
  ...$ReadOnly<BlockStatementProps>,
  +parent?: ESNode,
}): DetachedNode<BlockStatementType> {
  const node = detachedProps<BlockStatementType>(props.parent, {
    type: 'BlockStatement',
    body: props.body.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type CallExpressionProps = {
  +callee: MaybeDetachedNode<CallExpressionType['callee']>,
  +typeArguments?: ?MaybeDetachedNode<CallExpressionType['typeArguments']>,
  +arguments: $ReadOnlyArray<
    MaybeDetachedNode<CallExpressionType['arguments'][number]>,
  >,
};
export function CallExpression(props: {
  ...$ReadOnly<CallExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<CallExpressionType> {
  const node = detachedProps<CallExpressionType>(props.parent, {
    type: 'CallExpression',
    callee: asDetachedNode(props.callee),
    typeArguments: asDetachedNode(props.typeArguments),
    arguments: props.arguments.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type ExpressionStatementProps = {
  +expression: MaybeDetachedNode<ExpressionStatementType['expression']>,
  +directive?: ?ExpressionStatementType['directive'],
};
export function ExpressionStatement(props: {
  ...$ReadOnly<ExpressionStatementProps>,
  +parent?: ESNode,
}): DetachedNode<ExpressionStatementType> {
  const node = detachedProps<ExpressionStatementType>(props.parent, {
    type: 'ExpressionStatement',
    expression: asDetachedNode(props.expression),
    directive: props.directive,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type NumberTypeAnnotationProps = {};
export function NumberTypeAnnotation(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<NumberTypeAnnotationType> {
  return detachedProps<NumberTypeAnnotationType>(props.parent, {
    type: 'NumberTypeAnnotation',
  });
}

export type PropertyDefinitionProps = {
  +key: MaybeDetachedNode<PropertyDefinitionType['key']>,
  +value?: ?MaybeDetachedNode<PropertyDefinitionType['value']>,
  +computed: PropertyDefinitionType['computed'],
  +static: PropertyDefinitionType['static'],
  +declare: PropertyDefinitionType['declare'],
  +optional: PropertyDefinitionType['optional'],
  +variance?: ?MaybeDetachedNode<PropertyDefinitionType['variance']>,
  +typeAnnotation?: ?MaybeDetachedNode<
    PropertyDefinitionType['typeAnnotation'],
  >,
};
export function PropertyDefinition(props: {
  ...$ReadOnly<PropertyDefinitionProps>,
  +parent?: ESNode,
}): DetachedNode<PropertyDefinitionType> {
  const node = detachedProps<PropertyDefinitionType>(props.parent, {
    type: 'PropertyDefinition',
    key: asDetachedNode(props.key),
    value: asDetachedNode(props.value),
    computed: props.computed,
    static: props.static,
    declare: props.declare,
    optional: props.optional,
    variance: asDetachedNode(props.variance),
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type VariableDeclarationProps = {
  +kind: VariableDeclarationType['kind'],
  +declarations: $ReadOnlyArray<
    MaybeDetachedNode<VariableDeclarationType['declarations'][number]>,
  >,
};
export function VariableDeclaration(props: {
  ...$ReadOnly<VariableDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclarationType> {
  const node = detachedProps<VariableDeclarationType>(props.parent, {
    type: 'VariableDeclaration',
    kind: props.kind,
    declarations: props.declarations.map(n => asDetachedNode(n)),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type VariableDeclaratorProps = {
  +init?: ?MaybeDetachedNode<VariableDeclaratorType['init']>,
  +id: MaybeDetachedNode<VariableDeclaratorType['id']>,
};
export function VariableDeclarator(props: {
  ...$ReadOnly<VariableDeclaratorProps>,
  +parent?: ESNode,
}): DetachedNode<VariableDeclaratorType> {
  const node = detachedProps<VariableDeclaratorType>(props.parent, {
    type: 'VariableDeclarator',
    init: asDetachedNode(props.init),
    id: asDetachedNode(props.id),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export * from './special-case-node-types';
