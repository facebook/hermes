/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import * as fs from 'fs';
import * as path from 'path';
import {translate} from './index';
// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';

const filePaths = process.argv
  .slice(2)
  .filter(filePath => path.extname(filePath) === '.js');

filePaths.forEach(filePath => {
  const fileContents = fs.readFileSync(filePath, {encoding: 'utf8'});

  let translatedContents = '';
  try {
    translatedContents = translate(fileContents, prettierConfig);
  } catch (e) {
    console.error(`Tranlation failed with file "${filePath}"`);
    console.error(e);
    return;
  }

  if (filePaths.length > 1) {
    console.log(`/**\n * TRANSLATED FILE: ${filePath}\n */`);
  }

  console.log(translatedContents);
});
