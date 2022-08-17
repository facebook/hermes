/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict
 * @format
 */

// this script still uses a C++ template as it has to 1:1 match the ordering
// of the nodes in the parser as it's used as part of the deserialization of
// the WASM buffers

'use strict';

import {formatAndWriteDistArtifact} from './utils/scriptUtils';

const path = require('path');

const {execSync} = require('child_process');

const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates/HermesParserNodeDeserializers.template',
);

const deserializers = execSync(
  `c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`,
  {encoding: 'utf8'},
);

formatAndWriteDistArtifact({
  code: deserializers,
  package: 'hermes-parser',
  filename: 'HermesParserNodeDeserializers.js',
});
