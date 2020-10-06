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
      start: loc.start,
      end: loc.end,
    };

    node.range = [loc.rangeStart, loc.rangeEnd];
  }

  mapNode(node) {
    this.fixSourceLocation(node);
    switch (node.type) {
      case 'NullLiteral':
        return this.mapNullLiteral(node);
      case 'BooleanLiteral':
      case 'StringLiteral':
      case 'NumericLiteral':
        return this.mapSimpleLiteral(node);
      case 'RegExpLiteral':
        return this.mapRegExpLiteral(node);
      default:
        return this.mapNodeDefault(node);
    }
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
}

module.exports = HermesToESTreeAdapter;
