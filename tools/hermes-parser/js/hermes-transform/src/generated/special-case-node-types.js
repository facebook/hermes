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
These are a number of special-case node creation functions that we can't auto-generate.
The list of exported functions here must be kept in sync with the `NODES_WITH_SPECIAL_HANDLING`
list in `scripts/genTransformNodeTypes` to ensure there's no duplicates
*/

import type {
  ESNode,
  ArrowFunctionExpression as ArrowFunctionExpressionType,
  BigIntLiteral as BigIntLiteralType,
  BlockComment as BlockCommentType,
  BooleanLiteral as BooleanLiteralType,
  ClassDeclaration as ClassDeclarationType,
  Identifier as IdentifierType,
  LineComment as LineCommentType,
  NullLiteral as NullLiteralType,
  NumericLiteral as NumericLiteralType,
  RegExpLiteral as RegExpLiteralType,
  StringLiteral as StringLiteralType,
  TemplateElement as TemplateElementType,
} from 'hermes-estree';
import type {DetachedNode, MaybeDetachedNode} from '../detachedNode';

import {
  asDetachedNode,
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

// hermes adds an `id` prop which is always null, and it adds an `expression`
// boolean which is true when the body isn't a BlockStatement.
// No need to make consumers set these
export type ArrowFunctionExpressionProps = {
  +params: $ReadOnlyArray<
    MaybeDetachedNode<ArrowFunctionExpressionType['params'][number]>,
  >,
  +body: MaybeDetachedNode<ArrowFunctionExpressionType['body']>,
  +typeParameters?: ?MaybeDetachedNode<
    ArrowFunctionExpressionType['typeParameters'],
  >,
  +returnType?: ?MaybeDetachedNode<ArrowFunctionExpressionType['returnType']>,
  +predicate?: ?MaybeDetachedNode<ArrowFunctionExpressionType['predicate']>,
  +async: ArrowFunctionExpressionType['async'],
};
export function ArrowFunctionExpression(props: {
  ...$ReadOnly<ArrowFunctionExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ArrowFunctionExpressionType> {
  const node = detachedProps<ArrowFunctionExpressionType>(props.parent, {
    type: 'ArrowFunctionExpression',
    id: null,
    // $FlowExpectedError[incompatible-use]
    expression: props.body.type !== 'BlockStatement',
    params: props.params.map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
    typeParameters: asDetachedNode(props.typeParameters),
    returnType: asDetachedNode(props.returnType),
    predicate: asDetachedNode(props.predicate),
    async: props.async,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type ClassDeclarationProps = {
  +id?: ?MaybeDetachedNode<ClassDeclarationType['id']>,
  +typeParameters?: ?MaybeDetachedNode<ClassDeclarationType['typeParameters']>,
  +superClass?: ?MaybeDetachedNode<ClassDeclarationType['superClass']>,
  +superTypeParameters?: ?MaybeDetachedNode<
    ClassDeclarationType['superTypeParameters'],
  >,
  // make this optional as it's rarer that people would want to include them
  +implements?: $ReadOnlyArray<
    MaybeDetachedNode<ClassDeclarationType['implements'][number]>,
  >,
  // make this optional as it's rarer that people would want to include them
  +decorators?: $ReadOnlyArray<
    MaybeDetachedNode<ClassDeclarationType['decorators'][number]>,
  >,
  +body: MaybeDetachedNode<ClassDeclarationType['body']>,
};
export function ClassDeclaration(props: {
  ...$ReadOnly<ClassDeclarationProps>,
  +parent?: ESNode,
}): DetachedNode<ClassDeclarationType> {
  const node = detachedProps<ClassDeclarationType>(props.parent, {
    type: 'ClassDeclaration',
    id: asDetachedNode(props.id),
    typeParameters: asDetachedNode(props.typeParameters),
    superClass: asDetachedNode(props.superClass),
    superTypeParameters: asDetachedNode(props.superTypeParameters),
    decorators: (props.decorators ?? []).map(n => asDetachedNode(n)),
    implements: (props.implements ?? []).map(n => asDetachedNode(n)),
    body: asDetachedNode(props.body),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

// pattern/flags are on a subobject in the estree spec, but are flat on the hermes types
// also the value is supposed to be a RegExp instance
export type RegExpLiteralProps = {
  +pattern: RegExpLiteralType['regex']['pattern'],
  +flags: RegExpLiteralType['regex']['flags'],
};
export function RegExpLiteral(props: {
  ...$ReadOnly<RegExpLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<RegExpLiteralType> {
  const value = new RegExp(props.pattern, props.flags);
  return detachedProps<RegExpLiteralType>(props.parent, {
    type: 'Literal',
    value,
    raw: value.toString(),
    regex: {
      pattern: props.pattern,
      flags: props.flags,
    },
  });
}

// raw/cooked are on a subobject in the estree spec, but are flat on the hermes types
export type TemplateElementProps = {
  +tail: TemplateElementType['tail'],
  +cooked: TemplateElementType['value']['cooked'],
  +raw: TemplateElementType['value']['raw'],
};
export function TemplateElement(props: {
  ...$ReadOnly<TemplateElementProps>,
  +parent?: ESNode,
}): DetachedNode<TemplateElementType> {
  return detachedProps<TemplateElementType>(props.parent, {
    type: 'TemplateElement',
    tail: props.tail,
    value: {
      cooked: props.cooked,
      raw: props.raw,
    },
  });
}

// Identifier has a bunch of stuff that usually you don't want to provide - so we have
// this manual def to allow us to default some values
export type IdentifierProps = {
  +name: IdentifierType['name'],
  +typeAnnotation?: ?MaybeDetachedNode<IdentifierType['typeAnnotation']>,
  +optional?: IdentifierType['optional'],
};
export function Identifier(props: {
  ...$ReadOnly<IdentifierProps>,
  +parent?: ESNode,
}): DetachedNode<IdentifierType> {
  const node = detachedProps<IdentifierType>(props.parent, {
    type: 'Identifier',
    name: props.name,
    optional: props.optional ?? false,
    typeAnnotation: asDetachedNode(props.typeAnnotation),
  });
  setParentPointersInDirectChildren(node);
  return node;
}

//
// Literals require a "raw" which is added by the estree transform, not hermes.
//

export type BigIntLiteralProps = {
  +value: $FlowFixMe /* bigint | null */,
  /**
   * Only set this if you want to use a source-code representation like 1_1n, etc.
   * By default "raw" will just be the exact number you've given.
   */
  +raw?: NumericLiteralType['raw'],
};
export function BigIntLiteral(props: {
  ...$ReadOnly<BigIntLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<BigIntLiteralType> {
  const node = detachedProps<BigIntLiteralType>(props.parent, {
    type: 'Literal',
    value: props.value,
    raw: props.raw ?? `${props.value}n`,
    bigint: `${props.value}`,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type BooleanLiteralProps = {
  +value: BooleanLiteralType['value'],
};
export function BooleanLiteral(props: {
  ...$ReadOnly<BooleanLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<BooleanLiteralType> {
  return detachedProps<BooleanLiteralType>(props.parent, {
    type: 'Literal',
    raw: props.value ? 'true' : 'false',
    value: props.value,
  });
}

export type NumericLiteralProps = {
  +value: NumericLiteralType['value'],
  /**
   * Only set this if you want to use a source-code representation like 1e100, 0x11, 1_1, etc.
   * By default "raw" will just be the exact number you've given.
   */
  +raw?: NumericLiteralType['raw'],
};
export function NumericLiteral(props: {
  ...$ReadOnly<NumericLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<NumericLiteralType> {
  return detachedProps<NumericLiteralType>(props.parent, {
    type: 'Literal',
    value: props.value,
    raw: props.raw ?? `${props.value}`,
  });
}

export type NullLiteralProps = {};
export function NullLiteral(
  props: {
    +parent?: ESNode,
  } = {...null},
): DetachedNode<NullLiteralType> {
  return detachedProps<NullLiteralType>(props.parent, {
    type: 'Literal',
    value: null,
    raw: 'null',
  });
}

export type StringLiteralProps = {
  +value: StringLiteralType['value'],
  +raw?: StringLiteralType['raw'],
};
export function StringLiteral(props: {
  ...$ReadOnly<StringLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<StringLiteralType> {
  const hasSingleQuote = props.value.includes('"');
  const hasDoubleQuote = props.value.includes("'");
  let raw = props.raw;
  if (raw == null) {
    if (hasSingleQuote && hasDoubleQuote) {
      raw = `'${props.value.replace(/'/g, "\\'")}'`;
    } else if (hasSingleQuote) {
      raw = `"${props.value}"`;
    } else {
      raw = `'${props.value}'`;
    }
  }
  return detachedProps<StringLiteralType>(props.parent, {
    type: 'Literal',
    raw,
    value: props.value,
  });
}

export type LineCommentProps = {+value: string};
export function LineComment(props: LineCommentProps): LineCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<LineCommentType>(undefined, {
    type: 'Line',
    value: props.value,
  });
}

export type BlockCommentProps = {+value: string};
export function BlockComment(props: BlockCommentProps): BlockCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<BlockCommentType>(undefined, {
    type: 'Block',
    value: props.value,
  });
}
