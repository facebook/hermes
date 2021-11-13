/**
 * Copyright (c) Facebook, Inc. and its affiliates.
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
        return this.mapSimpleLiteral(node);
      case 'RegExpLiteral':
        return this.mapRegExpLiteral(node);
      case 'Empty':
        return this.mapEmpty(node);
      case 'TemplateElement':
        return this.mapTemplateElement(node);
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
    node.type = 'Literal';
    node.raw = this.code.slice(node.range[0], node.range[1]);

    return node;
  }

  mapNullLiteral(node: HermesNode): HermesNode {
    node.type = 'Literal';
    node.value = null;
    node.raw = this.code.slice(node.range[0], node.range[1]);

    return node;
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
      type: 'Literal',
      loc: node.loc,
      range: node.range,
      value,
      raw: this.code.slice(node.range[0], node.range[1]),
      regex: {
        pattern,
        flags,
      },
    };
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
      node.typeParameters === null &&
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
