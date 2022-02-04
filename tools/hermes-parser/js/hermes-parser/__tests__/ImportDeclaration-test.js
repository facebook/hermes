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

describe('ImportDeclaration', () => {
  describe('Value importKind converted to null', () => {
    const source = `import {Foo, type Bar, typeof Baz} from 'Foo'`;

    test('ESTree', () => {
      expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "assertions": Array [],
              "importKind": "value",
              "source": Object {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": Array [
                Object {
                  "importKind": null,
                  "imported": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportSpecifier",
                },
                Object {
                  "importKind": "type",
                  "imported": Object {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": Object {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportSpecifier",
                },
                Object {
                  "importKind": "typeof",
                  "imported": Object {
                    "name": "Baz",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": Object {
                    "name": "Baz",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportSpecifier",
                },
              ],
              "type": "ImportDeclaration",
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
              type: 'ImportDeclaration',
              importKind: 'value',
              specifiers: [
                {
                  type: 'ImportSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                  importKind: null,
                },
                {
                  type: 'ImportSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Bar',
                  },
                  importKind: 'type',
                },
                {
                  type: 'ImportSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Baz',
                  },
                  importKind: 'typeof',
                },
              ],
            },
          ],
        },
      });
    });
  });
});
