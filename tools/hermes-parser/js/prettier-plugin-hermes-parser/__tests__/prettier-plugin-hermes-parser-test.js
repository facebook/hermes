/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';

import * as prettier from 'prettier';
import * as prettierPluginHermesParser from '../src/index.js';

describe('prettier-plugin-hermes-parser', () => {
  it('uses plugin', async () => {
    const code = `const A: number = 1;`;
    const parseSpy = jest.spyOn(
      prettierPluginHermesParser.parsers.hermes,
      'parse',
    );
    const output = await prettier.format(code, {
      ...prettierConfig,
      parser: 'hermes',
      requirePragma: false,
      plugins: [prettierPluginHermesParser],
    });
    expect(parseSpy).toHaveBeenCalledTimes(1);
    expect(output).toBe(output);
  });
});
