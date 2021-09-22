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
  '../hermes-parser/dist/types/generated/node-types.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserNodeTypes.template',
);
const ALIAS_DEFINITIONS = path.resolve(
  __dirname,
  '../hermes-parser/src/types/definitions/aliases.js',
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

`;

/**
 * Generate concrete node types
 */
fileContents += execSync(
  `c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`,
);

/**
 * Generate alias node types
 */
const aliasDefs = require(ALIAS_DEFINITIONS);
const FLIPPED_ALIAS_KEYS = Object.create(null);
for (let typeName of Object.keys(aliasDefs)) {
  for (let aliasName of aliasDefs[typeName]) {
    if (FLIPPED_ALIAS_KEYS[aliasName]) {
      FLIPPED_ALIAS_KEYS[aliasName].add(typeName);
    } else {
      FLIPPED_ALIAS_KEYS[aliasName] = new Set([typeName]);
    }
  }
}
for (let aliasKey of Object.keys(FLIPPED_ALIAS_KEYS)) {
  fileContents += `

export function is${aliasKey}(node, opts) {
  if (!node) return false;

  const nodeType = node.type;
  if (
    ${Array.from(FLIPPED_ALIAS_KEYS[aliasKey])
      .map(nodeType => `'${nodeType}' === nodeType`)
      .join(' ||\n')}
  ) {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}`;
}

/**
 * Generate custom temp defs
 */
const tempCustomASTDefs = require(TEMP_CUSTOM_AST_DEFINITIONS);
for (let typeName of Object.keys(tempCustomASTDefs)) {
  const builders = tempCustomASTDefs[typeName].builder;
  fileContents += `

export function ${typeName}(${builders.map(name => `arg_${name}`).join(', ')}) {
  return {
    type: '${typeName}',
    loc: null,
    ${builders.map(name => `${name}: arg_${name}`).join(',\n')}
  };
}
export function is${typeName}(node, opts) {
  if (!node) return false;

  const nodeType = node.type;
  if (nodeType === '${typeName}') {
    if (typeof opts === "undefined") {
      return true;
    } else {
      return shallowEqual(node, opts);
    }
  }

  return false;
}`;
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
