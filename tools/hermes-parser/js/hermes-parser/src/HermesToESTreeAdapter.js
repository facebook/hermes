/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow
 * @format
 */

/*
This class does some very "javascripty" things in the name of
performance which are ultimately impossible to soundly type.

So instead of adding strict types and a large number of suppression
comments, instead it is left untyped and subclasses are strictly
typed via a separate flow declaration file.
*/

import type {HermesNode} from './HermesAST';
import type {ParserOptions} from './ParserOptions';

import HermesASTAdapter from './HermesASTAdapter';

declare var BigInt: ?(value: $FlowFixMe) => mixed;

export default class HermesToESTreeAdapter extends HermesASTAdapter {
  +code: string;

  constructor(options: ParserOptions, code: string) {
    super(options);
    this.code = code;
  }

  fixSourceLocation(node: HermesNode): void {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      source: this.sourceFilename ?? null,
      start: loc.start,
      end: loc.end,
    };

    node.range = [loc.rangeStart, loc.rangeEnd];
  }

  mapNode(node: HermesNode): HermesNode {
    this.fixSourceLocation(node);
    switch (node.type) {
      case 'Program':
        return this.mapProgram(node);
      case 'NullLiteral':
        return this.mapNullLiteral(node);
      case 'BooleanLiteral':
      case 'StringLiteral':
      case 'NumericLiteral':
      case 'JSXStringLiteral':
        return this.mapSimpleLiteral(node);
      case 'BigIntLiteral':
        return this.mapBigIntLiteral(node);
      case 'RegExpLiteral':
        return this.mapRegExpLiteral(node);
      case 'Empty':
        return this.mapEmpty(node);
      case 'TemplateElement':
        return this.mapTemplateElement(node);
      case 'BigIntLiteralTypeAnnotation':
        return this.mapBigIntLiteralTypeAnnotation(node);
      case 'GenericTypeAnnotation':
        return this.mapGenericTypeAnnotation(node);
      case 'ImportDeclaration':
        return this.mapImportDeclaration(node);
      case 'ImportSpecifier':
        return this.mapImportSpecifier(node);
      case 'ExportDefaultDeclaration':
        return this.mapExportDefaultDeclaration(node);
      case 'ExportNamedDeclaration':
        return this.mapExportNamedDeclaration(node);
      case 'ExportAllDeclaration':
        return this.mapExportAllDeclaration(node);
      case 'PrivateName':
      case 'ClassPrivateProperty':
        return this.mapPrivateProperty(node);
      default:
        return this.mapNodeDefault(node);
    }
  }

  mapProgram(node: HermesNode): HermesNode {
    node = this.mapNodeDefault(node);
    node.sourceType = this.getSourceType();

    return node;
  }

  mapSimpleLiteral(node: HermesNode): HermesNode {
    return {
      type: 'Literal',
      loc: node.loc,
      range: node.range,
      value: node.value,
      raw: this.code.slice(node.range[0], node.range[1]),
      literalType: (() => {
        switch (node.type) {
          case 'NullLiteral':
            return 'null';

          case 'BooleanLiteral':
            return 'boolean';

          case 'StringLiteral':
          case 'JSXStringLiteral':
            return 'string';

          case 'NumericLiteral':
            return 'numeric';

          case 'BigIntLiteral':
            return 'bigint';

          case 'RegExpLiteral':
            return 'regexp';
        }
        return null;
      })(),
    };
  }

  mapBigIntLiteral(node: HermesNode): HermesNode {
    const newNode = this.mapSimpleLiteral(node);
    const bigint = node.bigint
      // estree spec is to not have a trailing `n` on this property
      // https://github.com/estree/estree/blob/db962bb417a97effcfe9892f87fbb93c81a68584/es2020.md#bigintliteral
      .replace(/n$/, '')
      // `BigInt` doesn't accept numeric separator and `bigint` property should not include numeric separator
      .replace(/_/, '');
    return {
      ...newNode,
      // coerce the string to a bigint value if supported by the environment
      value: typeof BigInt === 'function' ? BigInt(bigint) : null,
      bigint,
    };
  }

  mapNullLiteral(node: HermesNode): HermesNode {
    return {
      ...this.mapSimpleLiteral(node),
      value: null,
    };
  }

  mapRegExpLiteral(node: HermesNode): HermesNode {
    const {pattern, flags} = node;

    // Create RegExp value if possible. This can fail when the flags are invalid.
    let value;
    try {
      value = new RegExp(pattern, flags);
    } catch (e) {
      value = null;
    }

    return {
      ...this.mapSimpleLiteral(node),
      value,
      regex: {
        pattern,
        flags,
      },
    };
  }

  mapBigIntLiteralTypeAnnotation(node: HermesNode): HermesNode {
    node.value = null;
    return node;
  }

  mapTemplateElement(node: HermesNode): HermesNode {
    return {
      type: 'TemplateElement',
      loc: node.loc,
      range: node.range,
      tail: node.tail,
      value: {
        cooked: node.cooked,
        raw: node.raw,
      },
    };
  }

  mapGenericTypeAnnotation(node: HermesNode): HermesNode {
    // Convert simple `this` generic type to ThisTypeAnnotation
    if (
      node.typeParameters == null &&
      node.id.type === 'Identifier' &&
      node.id.name === 'this'
    ) {
      return {
        type: 'ThisTypeAnnotation',
        loc: node.loc,
        range: node.range,
      };
    }

    return this.mapNodeDefault(node);
  }

  mapComment(node: HermesNode): HermesNode {
    if (node.type === 'CommentBlock') {
      node.type = 'Block';
    } else if (node.type === 'CommentLine') {
      node.type = 'Line';
    }

    return node;
  }
}
