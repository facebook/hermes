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
  '../build/HermesParserNodeDeserializers.js',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserNodeDeserializers.template',
);

const deserializers = execSync(
  `cpp -P -I "${process.argv[2]}" "${TEMPLATE_FILE}"`,
);
const deserializersFileContents = `/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

'use strict';

${deserializers}
`;

// Format then sign file and write to disk
const formattedContents = execSync('prettier', {
  input: deserializersFileContents,
}).toString();
fs.writeFileSync(OUTPUT_FILE, formattedContents);
