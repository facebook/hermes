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

// $FlowExpectedError[untyped-import]
import {transformSync} from '@babel/core';

import hermesParserPlugin from '../src';
import * as HermesParser from 'hermes-parser';

const MODULE_PREAMBLE = '"use strict";\n\n';

describe('babel-plugin-syntax-hermes-parser', () => {
  test('test basic parsing', () => {
    const parseSpy = jest.spyOn(HermesParser, 'parse');
    const code = MODULE_PREAMBLE + 'const a = 1;';
    const output = transformSync(code, {
      plugins: [hermesParserPlugin],
    });
    expect(output.code).toBe(code);
    expect(parseSpy).toBeCalledTimes(1);
  });

  test('test component syntax parsing', () => {
    const parseSpy = jest.spyOn(HermesParser, 'parse');
    const code = MODULE_PREAMBLE + 'component Foo() {}';
    const output = transformSync(code, {
      plugins: [hermesParserPlugin],
      parserOpts: {
        enableExperimentalComponentSyntax: true,
      },
    });
    expect(output.code).toMatchInlineSnapshot(`
      ""use strict";

      function Foo() {}"
    `);
    expect(parseSpy).toBeCalledTimes(1);
  });
});
