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

function format(code: string) {
  const options = {
    ...prettierConfig,
    parser: 'hermes',
    requirePragma: false,
    plugins: [require.resolve('../index.mjs')],
  };
  return prettier.format(code, options);
}

// TODO(marcoww): These tests are expected to fail until the prettier plugin
// is rebuilt with hermes-parser >= 0.35.0 which supports readonly variance.
// Remove the .failing and update snapshots once the plugin is rebuilt.
/* eslint-disable jest/no-standalone-expect */
describe(`'readonly' variance annotation`, () => {
  // $FlowFixMe[prop-missing]
  test.failing('class property', async () => {
    expect(
      await format(`
        class ReadonlyRoute {
            static   readonly   param: T;
        }
      `),
    ).toMatchInlineSnapshot(`
      "class ReadonlyRoute {
        static readonly param: T;
      }
      "
    `);
  });

  // $FlowFixMe[prop-missing]
  test.failing('object type property', async () => {
    expect(
      await format(`
        type ReadonlyObj = {
            readonly   foo:   string,
          readonly [string]:   unknown
        };
      `),
    ).toMatchInlineSnapshot(`
      "type ReadonlyObj = {
        readonly foo: string,
        readonly [string]: unknown,
      };
      "
    `);
  });

  // $FlowFixMe[prop-missing]
  test.failing('tuple label', async () => {
    expect(
      await format(`
        type ReadonlyTuple = [  readonly   label:   number  ];
      `),
    ).toMatchInlineSnapshot(`
      "type ReadonlyTuple = [readonly label: number];
      "
    `);
  });

  // $FlowFixMe[prop-missing]
  test.failing('interface', async () => {
    expect(
      await format(`
        type ReadonlyInterface = interface {
              readonly   prop:   string
        };
      `),
    ).toMatchInlineSnapshot(`
      "type ReadonlyInterface = interface {
        readonly prop: string,
      };
      "
    `);
  });
});
/* eslint-enable jest/no-standalone-expect */
