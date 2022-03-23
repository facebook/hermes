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

import type {ESTreeJSON} from './utils/scriptUtils';

import {execSync} from 'child_process';
import fs from 'fs';
import mkdirp from 'mkdirp';
import path from 'path';

const OUTPUT_DIR = path.resolve(__dirname, 'dist');
const OUTPUT_FILE = path.resolve(OUTPUT_DIR, 'HermesESTreeJSON.json');
const TEMPLATE_FILE = path.resolve(
  __dirname,
  'templates',
  'HermesESTreeJSON.template',
);

// These nodes are not actually part of the ESTree spec
const NODES_TO_REMOVE = new Set([
  // EStree treats these as an "ExpressionStatement"
  'Directive',
  'DirectiveLiteral',
  // used by hermes to represent an "empty" array destructuring element (`let [,,] = [1,2,3]`)
  'Empty',
  // used by hermes to avoid adding 'raw' field to actual StringLiteral
  'JSXStringLiteral',
  // don't know what this is for...
  'Metadata',
]);

const rawJSON: ESTreeJSON = JSON.parse(
  execSync(`c++ -E -P -I"${process.argv[2]}" -x c "${TEMPLATE_FILE}"`, {
    encoding: 'utf8',
  }),
  (key: string, value: string) => {
    if (key === 'optional') {
      return value === 'true';
    }
    return value;
  },
).filter(Boolean);
const cleanedJSON = rawJSON
  .filter(n => !NODES_TO_REMOVE.has(n.name))
  .concat([
    // this is added by the ESTree adapter
    // https://www.internalfb.com/code/fbsource/[212aa6eff4441844b7a43e37e6cfc64932c348ae]/xplat/hermes/tools/hermes-parser/js/hermes-parser/src/HermesToESTreeAdapter.js?lines=133-143
    {
      name: 'ThisTypeAnnotation',
      base: 'Base',
      arguments: [],
    },
  ])
  .sort((a, b) => a.name.localeCompare(b.name));

// Format then sign file and write to disk
const formattedContents = execSync('prettier --parser=json', {
  input: JSON.stringify(cleanedJSON, null, 2),
}).toString();

mkdirp.sync(OUTPUT_DIR);
fs.writeFileSync(OUTPUT_FILE, formattedContents);
