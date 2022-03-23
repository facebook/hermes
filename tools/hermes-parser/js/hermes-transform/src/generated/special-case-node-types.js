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
  RegExpLiteral as RegExpLiteralType,
  TemplateElement as TemplateElementType,
  Identifier as IdentifierType,
  BigIntLiteral as BigIntLiteralType,
  BooleanLiteral as BooleanLiteralType,
  NumericLiteral as NumericLiteralType,
  NullLiteral as NullLiteralType,
  StringLiteral as StringLiteralType,
  LineComment as LineCommentType,
  BlockComment as BlockCommentType,
} from 'hermes-estree';
import type {DetachedNode} from '../detachedNode';

import {
  detachedProps,
  setParentPointersInDirectChildren,
} from '../detachedNode';

// hermes adds an `id` prop which is always null, and it adds an `expression`
// boolean which is true when the body isn't a BlockStatement.
// No need to make consumers set these
export type ArrowFunctionExpressionProps = {
  +params: $ReadOnlyArray<
    DetachedNode<ArrowFunctionExpressionType['params'][number]>,
  >,
  +body: DetachedNode<ArrowFunctionExpressionType['body']>,
  +typeParameters?: ?DetachedNode<
    ArrowFunctionExpressionType['typeParameters'],
  >,
  +returnType?: ?DetachedNode<ArrowFunctionExpressionType['returnType']>,
  +predicate?: ?DetachedNode<ArrowFunctionExpressionType['predicate']>,
  +async: ArrowFunctionExpressionType['async'],
};
export function ArrowFunctionExpression({
  parent,
  ...props
}: {
  ...$ReadOnly<ArrowFunctionExpressionProps>,
  +parent?: ESNode,
}): DetachedNode<ArrowFunctionExpressionType> {
  const node = detachedProps<ArrowFunctionExpressionType>(parent, {
    type: 'ArrowFunctionExpression',
    id: null,
    // $FlowExpectedError[incompatible-use]
    expression: props.body.type !== 'BlockStatement',
    ...props,
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
export function RegExpLiteral({
  pattern,
  flags,
  parent,
}: {
  ...$ReadOnly<RegExpLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<RegExpLiteralType> {
  const value = new RegExp(pattern, flags);
  return detachedProps<RegExpLiteralType>(parent, {
    type: 'Literal',
    value,
    raw: value.toString(),
    regex: {
      pattern,
      flags,
    },
  });
}

// raw/cooked are on a subobject in the estree spec, but are flat on the hermes types
export type TemplateElementProps = {
  +tail: TemplateElementType['tail'],
  +cooked: TemplateElementType['value']['cooked'],
  +raw: TemplateElementType['value']['raw'],
};
export function TemplateElement({
  tail,
  parent,
  ...value
}: {
  ...$ReadOnly<TemplateElementProps>,
  +parent?: ESNode,
}): DetachedNode<TemplateElementType> {
  return detachedProps<TemplateElementType>(parent, {
    type: 'TemplateElement',
    tail,
    value,
  });
}

// Identifier has a bunch of stuff that usually you don't want to provide - so we have
// this manual def to allow us to default some values
export type IdentifierProps = {
  +name: IdentifierType['name'],
  +typeAnnotation?: ?DetachedNode<IdentifierType['typeAnnotation']>,
  +optional?: IdentifierType['optional'],
};
export function Identifier({
  parent,
  optional = false,
  typeAnnotation = null,
  ...props
}: {
  ...$ReadOnly<IdentifierProps>,
  +parent?: ESNode,
}): DetachedNode<IdentifierType> {
  const node = detachedProps<IdentifierType>(parent, {
    type: 'Identifier',
    optional,
    typeAnnotation,
    ...props,
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
export function BigIntLiteral({
  parent,
  ...props
}: {
  ...$ReadOnly<BigIntLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<BigIntLiteralType> {
  const node = detachedProps<BigIntLiteralType>(parent, {
    type: 'Literal',
    ...props,
    raw: props.raw ?? `${props.value}n`,
    bigint: `${props.value}`,
  });
  setParentPointersInDirectChildren(node);
  return node;
}

export type BooleanLiteralProps = {
  +value: BooleanLiteralType['value'],
};
export function BooleanLiteral({
  parent,
  value,
}: {
  ...$ReadOnly<BooleanLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<BooleanLiteralType> {
  return detachedProps<BooleanLiteralType>(parent, {
    type: 'Literal',
    raw: value ? 'true' : 'false',
    value,
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
export function NumericLiteral({
  parent,
  ...props
}: {
  ...$ReadOnly<NumericLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<NumericLiteralType> {
  return detachedProps<NumericLiteralType>(parent, {
    type: 'Literal',
    ...props,
    raw: props.raw ?? `${props.value}`,
  });
}

export type NullLiteralProps = {};
export function NullLiteral({
  parent,
}: {
  +parent?: ESNode,
} = {}): DetachedNode<NullLiteralType> {
  return detachedProps<NullLiteralType>(parent, {
    type: 'Literal',
    value: null,
    raw: 'null',
  });
}

export type StringLiteralProps = {
  +value: StringLiteralType['value'],
  +raw?: StringLiteralType['raw'],
};
export function StringLiteral({
  parent,
  raw: rawIn,
  value,
}: {
  ...$ReadOnly<StringLiteralProps>,
  +parent?: ESNode,
}): DetachedNode<StringLiteralType> {
  const hasSingleQuote = value.includes('"');
  const hasDoubleQuote = value.includes("'");
  let raw = rawIn;
  if (raw == null) {
    if (hasSingleQuote && hasDoubleQuote) {
      raw = `'${value.replace(/'/g, "\\'")}'`;
    } else if (hasSingleQuote) {
      raw = `"${value}"`;
    } else {
      raw = `'${value}'`;
    }
  }
  return detachedProps<StringLiteralType>(parent, {
    type: 'Literal',
    raw,
    value,
  });
}

export type LineCommentProps = {+value: string};
export function LineComment({value}: LineCommentProps): LineCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<LineCommentType>(undefined, {
    type: 'Line',
    value,
  });
}

export type BlockCommentProps = {+value: string};
export function BlockComment({value}: BlockCommentProps): BlockCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<BlockCommentType>(undefined, {
    type: 'Block',
    value,
  });
}
