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
  '../hermes-estree/dist/HermesESTreeSelectorTypes.js.flow',
);
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesTransformSelectorTypes.template',
);

// Create visitor keys file
const types = execSync(
  `c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`,
);
const typesFileContents = `\
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

${types}
`;

// Format then sign file and write to disk
const formattedContents = execSync('prettier --parser=flow', {
  input: typesFileContents,
}).toString();

fs.writeFileSync(OUTPUT_FILE, formattedContents);
