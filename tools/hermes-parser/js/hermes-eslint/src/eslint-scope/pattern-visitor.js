/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import type {
  ESNode,
  Identifier,
  AssignmentPattern,
  AssignmentExpression,
  RestElement,
  Property,
  ArrayPattern,
  ObjectPattern,
  MemberExpression,
  SpreadElement,
  ArrayExpression,
  CallExpression,
} from 'hermes-estree';
import type {VisitorOptions} from './Visitor';

const Visitor = require('./Visitor');

/**
 * Get last array element
 * @param {array} xs - array
 * @returns {any} Last elment
 */
function getLast<T>(xs: $ReadOnlyArray<T>): ?T {
  return xs[xs.length - 1] || null;
}

export type PatternVisitorCallback = (
  pattern: Identifier,
  info: {
    assignments: $ReadOnlyArray<AssignmentPattern | AssignmentExpression>,
    rest: boolean,
    topLevel: boolean,
  },
) => void;

function isPattern(node: ESNode): boolean %checks {
  return (
    node.type === 'Identifier' ||
    node.type === 'ObjectPattern' ||
    node.type === 'ArrayPattern' ||
    node.type === 'SpreadElement' ||
    node.type === 'RestElement' ||
    node.type === 'AssignmentPattern'
  );
}

class PatternVisitor extends Visitor {
  rootPattern: ESNode;
  callback: PatternVisitorCallback;
  assignments: Array<AssignmentPattern | AssignmentExpression>;
  extraNodesToVisit: Array<ESNode>;
  restElements: Array<RestElement>;

  constructor(
    options: VisitorOptions,
    rootPattern: ESNode,
    callback: PatternVisitorCallback,
  ) {
    super(null, options);
    this.rootPattern = rootPattern;
    this.callback = callback;
    this.assignments = [];
    this.extraNodesToVisit = [];
    this.restElements = [];
  }

  Identifier(pattern: Identifier) {
    const lastRestElement = getLast(this.restElements);

    if (pattern.typeAnnotation != null) {
      this.extraNodesToVisit.push(pattern.typeAnnotation);
    }

    this.callback(pattern, {
      topLevel: pattern === this.rootPattern,
      rest:
        lastRestElement !== null &&
        lastRestElement !== undefined &&
        lastRestElement.argument === pattern,
      assignments: this.assignments,
    });
  }

  Property(property: Property) {
    // Computed property's key is a right hand node.
    if (property.computed) {
      this.extraNodesToVisit.push(property.key);
    }

    // If it's shorthand, its key is same as its value.
    // If it's shorthand and has its default value, its key is same as its value.left (the value is AssignmentPattern).
    // If it's not shorthand, the name of new variable is its value's.
    this.visit(property.value);
  }

  ArrayPattern(pattern: ArrayPattern) {
    for (let i = 0, iz = pattern.elements.length; i < iz; ++i) {
      const element = pattern.elements[i];

      this.visit(element);
    }

    if (pattern.typeAnnotation != null) {
      this.extraNodesToVisit.push(pattern.typeAnnotation);
    }
  }

  ObjectPattern(pattern: ObjectPattern) {
    for (const property of pattern.properties) {
      this.visit(property);
    }

    if (pattern.typeAnnotation != null) {
      this.extraNodesToVisit.push(pattern.typeAnnotation);
    }
  }

  AssignmentPattern(pattern: AssignmentPattern) {
    this.assignments.push(pattern);
    this.visit(pattern.left);
    this.extraNodesToVisit.push(pattern.right);
    this.assignments.pop();
  }

  RestElement(pattern: RestElement) {
    this.restElements.push(pattern);
    this.visit(pattern.argument);
    this.restElements.pop();
  }

  MemberExpression(node: MemberExpression) {
    // Computed property's key is a right hand node.
    if (node.computed) {
      this.extraNodesToVisit.push(node.property);
    }

    // the object is only read, write to its property.
    this.extraNodesToVisit.push(node.object);
  }

  //
  // ForInStatement.left and AssignmentExpression.left are LeftHandSideExpression.
  // By spec, LeftHandSideExpression is Pattern or MemberExpression.
  //   (see also: https://github.com/estree/estree/pull/20#issuecomment-74584758)
  // But espree 2.0 parses to ArrayExpression, ObjectExpression, etc...
  //

  SpreadElement(node: SpreadElement) {
    this.visit(node.argument);
  }

  ArrayExpression(node: ArrayExpression) {
    node.elements.forEach(element => this.visit(element));
  }

  AssignmentExpression(node: AssignmentExpression) {
    this.assignments.push(node);
    this.visit(node.left);
    this.extraNodesToVisit.push(node.right);
    this.assignments.pop();
  }

  CallExpression(node: CallExpression) {
    // Arguments and type arguments may be visited later
    node.arguments.forEach(a => {
      this.extraNodesToVisit.push(a);
    });
    if (node.typeArguments != null) {
      this.extraNodesToVisit.push(node.typeArguments);
    }

    this.visit(node.callee);
  }
}

module.exports = {PatternVisitor, isPattern};
