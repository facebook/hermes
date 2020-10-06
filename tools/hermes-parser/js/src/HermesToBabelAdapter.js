/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

const HermesASTAdapter = require('./HermesASTAdapter');

class HermesToBabelAdapter extends HermesASTAdapter {
  fixSourceLocation(node) {
    const loc = node.loc;
    if (loc == null) {
      return;
    }

    node.loc = {
      start: loc.start,
      end: loc.end,
    };

    node.start = loc.rangeStart;
    node.end = loc.rangeEnd;
  }

  mapNode(node) {
    this.fixSourceLocation(node);
    switch (node.type) {
      case 'Program':
      case 'BlockStatement':
        return this.mapNodeWithDirectives(node);
      default:
        return this.mapNodeDefault(node);
    }
  }

  mapNodeWithDirectives(node) {
    const directives = [];
    for (const child of node.body) {
      if (child.type === 'ExpressionStatement' && child.directive != null) {
        // Visit directive children
        const directiveChild = this.mapNode(child);

        // Modify string literal node to be DirectiveLiteral node
        directiveChild.expression.type = 'DirectiveLiteral';

        // Construct Directive node with DirectiveLiteral value
        directives.push({
          type: 'Directive',
          loc: directiveChild.loc,
          start: directiveChild.start,
          end: directiveChild.end,
          value: directiveChild.expression,
        });
      } else {
        // Once we have found the first non-directive node we know there cannot be any more directives
        break;
      }
    }

    // Move directives from body to new directives array
    node.directives = directives;
    if (directives.length !== 0) {
      node.body = node.body.slice(directives.length);
    }

    // Visit expression statement children
    const body = node.body;
    for (let i = 0; i < body.length; i++) {
      const child = body[i];
      if (child != null) {
        body[i] = this.mapNode(child);
      }
    }

    return node;
  }
}

module.exports = HermesToBabelAdapter;
