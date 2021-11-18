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
export function ArrowFunctionExpression({
  parent,
  ...props
}: {
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
export function RegExpLiteral({
  pattern,
  flags,
  parent,
}: {
  +pattern: RegExpLiteralType['regex']['pattern'],
  +flags: RegExpLiteralType['regex']['flags'],
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
export function TemplateElement({
  tail,
  parent,
  ...value
}: {
  +tail: TemplateElementType['tail'],
  +cooked: TemplateElementType['value']['cooked'],
  +raw: TemplateElementType['value']['raw'],
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
export function Identifier({
  parent,
  optional = false,
  typeAnnotation = null,
  ...props
}: {
  +name: IdentifierType['name'],
  +typeAnnotation?: ?DetachedNode<IdentifierType['typeAnnotation']>,
  +optional?: IdentifierType['optional'],
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

export function BooleanLiteral({
  parent,
  value,
}: {
  +value: BooleanLiteralType['value'],
  +parent?: ESNode,
}): DetachedNode<BooleanLiteralType> {
  return detachedProps<BooleanLiteralType>(parent, {
    type: 'Literal',
    raw: value ? 'true' : 'false',
    value,
  });
}

export function NumericLiteral({
  parent,
  ...props
}: {
  +value: NumericLiteralType['value'],
  /**
   * Only set this if you want to use a source-code representation like 1e100, 0x11, etc.
   * By default "raw" will just be the exact number you've given.
   */
  +raw?: NumericLiteralType['raw'],
  +parent?: ESNode,
}): DetachedNode<NumericLiteralType> {
  return detachedProps<NumericLiteralType>(parent, {
    type: 'Literal',
    ...props,
    raw: props.raw ?? `${props.value}`,
  });
}

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

export function StringLiteral({
  parent,
  raw: rawIn,
  value,
}: {
  +value: StringLiteralType['value'],
  +raw?: StringLiteralType['raw'],
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

export function LineComment({value}: {+value: string}): LineCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<LineCommentType>(undefined, {
    type: 'Line',
    value,
  });
}

export function BlockComment({value}: {+value: string}): BlockCommentType {
  // $FlowExpectedError[prop-missing]
  // $FlowExpectedError[incompatible-return]
  return detachedProps<BlockCommentType>(undefined, {
    type: 'Block',
    value,
  });
}
