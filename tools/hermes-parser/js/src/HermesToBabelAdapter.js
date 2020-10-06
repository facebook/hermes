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
        return this.mapProgram(node);
      case 'BlockStatement':
        return this.mapNodeWithDirectives(node);
      case 'Empty':
        return this.mapEmpty(node);
      case 'TemplateElement':
        return this.mapTemplateElement(node);
      default:
        return this.mapNodeDefault(node);
    }
  }

  mapProgram(node) {
    // Visit child nodes and convert to directives
    const {comments, ...program} = this.mapNodeWithDirectives(node);

    // Rename root node to File node and move Program node under program property
    return {
      type: 'File',
      loc: program.loc,
      start: program.start,
      end: program.end,
      program,
      comments,
    };
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

  mapTemplateElement(node) {
    // Adjust start loc to exclude "`" at beginning of template literal if this is the first quasi,
    // otherwise exclude "}" from previous expression.
    const startCharsToExclude = 1;

    // Adjust end loc to exclude "`" at end of template literal if this is the last quasi,
    // otherwise exclude "${" from next expression.
    const endCharsToExclude = node.tail ? 1 : 2;

    return {
      type: 'TemplateElement',
      loc: {
        start: {
          line: node.loc.start.line,
          column: node.loc.start.column + startCharsToExclude,
        },
        end: {
          line: node.loc.end.line,
          column: node.loc.end.column - endCharsToExclude,
        },
      },
      start: node.start + startCharsToExclude,
      end: node.end - endCharsToExclude,
      tail: node.tail,
      value: {
        cooked: node.cooked,
        raw: node.raw,
      },
    };
  }
}

module.exports = HermesToBabelAdapter;
