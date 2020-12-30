/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesASTAdapter = require('./HermesASTAdapter');

class HermesToESTreeAdapter extends HermesASTAdapter {
  fixSourceLocation(node) {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      source: this.sourceFilename,
      start: loc.start,
      end: loc.end,
    };

    node.range = [loc.rangeStart, loc.rangeEnd];
  }

  mapNode(node) {
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

  mapProgram(node) {
    node = this.mapNodeDefault(node);
    node.sourceType = this.sourceType;

    return node;
  }

  mapSimpleLiteral(node) {
    node.type = 'Literal';
    return node;
  }

  mapNullLiteral(node) {
    node.type = 'Literal';
    node.value = null;

    return node;
  }

  mapRegExpLiteral(node) {
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
      regex: {
        pattern,
        flags,
      },
    };
  }

  mapTemplateElement(node) {
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

  mapGenericTypeAnnotation(node) {
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

  mapComment(node) {
    if (node.type === 'CommentBlock') {
      node.type = 'Block';
    } else if (node.type === 'CommentLine') {
      node.type = 'Line';
    }

    return node;
  }
}

module.exports = HermesToESTreeAdapter;
