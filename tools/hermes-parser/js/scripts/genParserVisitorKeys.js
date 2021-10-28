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
  '../hermes-parser/dist/types/generated/visitor-keys.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserVisitorKeys.template',
);
const TEMP_CUSTOM_AST_DEFINITIONS = path.resolve(
  __dirname,
  '../hermes-parser/src/types/definitions/tempCustomASTDefs.js',
);

let fileContents = `/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

export const HERMES_AST_VISITOR_KEYS = {};
export const NODE_CHILD = 'Node';
export const NODE_LIST_CHILD = 'NodeList';

`;

/**
 * Create visitor keys
 */
fileContents += execSync(
  `c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`,
);

/**
 * Generate custom temp defs
 */
const tempCustomASTDefs = require(TEMP_CUSTOM_AST_DEFINITIONS).default;
for (let typeName of Object.keys(tempCustomASTDefs)) {
  const visitors = tempCustomASTDefs[typeName].visitor;
  fileContents += `

HERMES_AST_VISITOR_KEYS['${typeName}'] = {
  ${Object.keys(visitors)
    .map(name => `${name}: ${visitors[name]}`)
    .join(',\n')}
};`;
}

// Format then sign file and write to disk
const formattedContents = execSync('prettier --parser=flow', {
  input: fileContents,
}).toString();

const outputDir = path.dirname(OUTPUT_FILE);
if (!fs.existsSync(outputDir)) {
  fs.mkdirSync(outputDir, {recursive: true});
}

fs.writeFileSync(OUTPUT_FILE, formattedContents);
