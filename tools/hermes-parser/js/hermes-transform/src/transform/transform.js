/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

'use strict';

import type {Visitor} from '../traverse/traverse';
import type {TransformContext} from './TransformContext';

import * as prettier from 'prettier';
import {getTransformedAST} from './getTransformedAST';

export function transform(
  code: string,
  visitors: Visitor<TransformContext>,
): string {
  const {ast, astWasMutated} = getTransformedAST(code, visitors);

  if (!astWasMutated) {
    return code;
  }

  return prettier.format(code, {
    parser(code, _, options) {
      return ast;
    },
  });
}
