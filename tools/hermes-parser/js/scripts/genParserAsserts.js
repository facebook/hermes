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
  '../hermes-parser/dist/types/generated/asserts.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserAsserts.template',
);
const ALIAS_DEFINITIONS = path.resolve(
  __dirname,
  '../hermes-parser/src/types/definitions/aliases.js',
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

export function assert${aliasKey}(node, opts) {
  if (
    (
      node &&
      (
        ${Array.from(FLIPPED_ALIAS_KEYS[aliasKey])
          .map(nodeType => `node.type === "${nodeType}"`)
          .join(' ||\n')}
      )
    ) &&
    (
      typeof opts === "undefined" ||
      shallowEqual(node, opts)
    )
  ) {
    return;
  }

  throw new Error(throwMessage("${aliasKey}", node, opts));
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
