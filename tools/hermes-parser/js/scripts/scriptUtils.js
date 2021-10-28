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

import {execSync} from 'child_process';
import fs from 'fs';
import path from 'path';

export type ESTreeJSON = $ReadOnlyArray<
  $ReadOnly<{
    name: string,
    base: string,
    arguments: $ReadOnlyArray<
      $ReadOnly<{
        type:
          | 'NodeLabel'
          | 'NodeString'
          | 'NodeBoolean'
          | 'NodeNumber'
          | 'NodePtr'
          | 'NodeList',
        name: string,
        optional: boolean,
      }>,
    >,
  }>,
>;

// $FlowExpectedError[cannot-resolve-module]
export const HermesESTreeJSON: ESTreeJSON = require('./dist/HermesESTreeJSON.json');
