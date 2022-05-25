/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

'use strict';

import {
  HermesESTreeJSON,
  formatAndWriteDistArtifact,
  LITERAL_TYPES,
} from './utils/scriptUtils';

const ALLOWED_ARG_TYPES = new Set(['NodePtr', 'NodeList']);

// $FlowExpectedError[incompatible-type]
const visitorKeys: {[string]: Array<string>} = Object.create(null);
for (const node of HermesESTreeJSON) {
  visitorKeys[node.name] = node.arguments
    .filter(arg => ALLOWED_ARG_TYPES.has(arg.type))
    .map(arg => arg.name);
}

// Override select visitor keys to set correct traversal order
visitorKeys['IfStatement'] = ['test', 'consequent', 'alternate'];
visitorKeys['ConditionalExpression'] = ['test', 'consequent', 'alternate'];
visitorKeys['WhileStatement'] = ['test', 'body'];
visitorKeys['VariableDeclarator'] = ['id', 'init'];

// hermes-parser uses babel's optional chaining representation, which does not emit this node
visitorKeys['ChainExpression'] = ['expression'];

// TODO - remove this once we migrate from old-school babel-style AST spec for properties
//        this exists to allow `SimpleTraverser` to traverse the ESTree spec for the alignment tests
visitorKeys['PropertyDefinition'] = visitorKeys['ClassProperty'];

// correct the "literal" types
// the base defs declare each literal as a separate node, but ESTree treats them as a single node
visitorKeys['Literal'] = [];
for (const type of LITERAL_TYPES) {
  delete visitorKeys[type];
}

const visitorKeysFileContents = `module.exports = ${JSON.stringify(
  visitorKeys,
  null,
  2,
)};`;

formatAndWriteDistArtifact({
  code: visitorKeysFileContents,
  package: 'hermes-eslint',
  filename: 'HermesESLintVisitorKeys.js',
});
