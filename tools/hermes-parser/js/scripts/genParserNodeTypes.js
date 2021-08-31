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
  '../hermes-parser/dist/types/HermesParserNodeTypes.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserNodeTypes.template',
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

const nodeBases = {};

${visitorKeys}
`;

// Format then sign file and write to disk
const formattedContents = execSync('prettier --parser=flow', {
  input: visitorKeysFileContents,
}).toString();
fs.writeFileSync(OUTPUT_FILE, formattedContents);
