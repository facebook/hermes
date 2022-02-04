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

import {parse, parseForSnapshot} from '../__test_utils__/parse';

describe('Symbol type annotation', () => {
  const source = `type T = symbol`;

  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "id": Object {
              "name": "T",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "right": Object {
              "type": "SymbolTypeAnnotation",
            },
            "type": "TypeAlias",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
  });

  test('Babel', () => {
    expect(parse(source, {babel: true})).toMatchObject({
      type: 'File',
      program: {
        type: 'Program',
        body: [
          {
            type: 'TypeAlias',
            right: {
              type: 'GenericTypeAnnotation',
              id: {
                type: 'Identifier',
                name: 'symbol',
              },
            },
            typeParameters: null,
          },
        ],
      },
    });
  });
});
