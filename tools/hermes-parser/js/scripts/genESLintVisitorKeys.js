/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const fs = require('fs');
const path = require('path');

const {execSync} = require('child_process');

const OUTPUT_FILE = path.resolve(
  __dirname,
  '../hermes-eslint/dist/HermesESLintVisitorKeys.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesESLintVisitorKeys.template',
);

// Create visitor keys file
const visitorKeys = execSync(
  `c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`,
);
const visitorKeysFileContents = `/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

const VISITOR_KEYS = {};

// Add visitor keys for nodes produced by AST adapters that are not defined
// in Hermes AST definitions.
VISITOR_KEYS['Literal'] = [];
VISITOR_KEYS['ThisTypeAnnotation'] = [];

// Visitor keys generated from Hermes AST definitions
${visitorKeys}

// Override select visitor keys to set correct traversal order
VISITOR_KEYS['IfStatement'] = ['test', 'consequent', 'alternate'];
VISITOR_KEYS['ConditionalExpression'] = ['test', 'consequent', 'alternate'];
VISITOR_KEYS['WhileStatement'] = ['test', 'body'];

module.exports = VISITOR_KEYS;
`;

// Format then sign file and write to disk
const formattedContents = execSync('prettier --parser=flow', {
  input: visitorKeysFileContents,
}).toString();
fs.writeFileSync(OUTPUT_FILE, formattedContents);
