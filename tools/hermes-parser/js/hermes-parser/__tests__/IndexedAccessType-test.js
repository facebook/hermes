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

describe('IndexedAccessType', () => {
  describe('ESTree', () => {
    test('Basic Indexed Access Type', () => {
      const source = `type T = O[k]`;
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
                "indexType": Object {
                  "id": Object {
                    "name": "k",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "objectType": Object {
                  "id": Object {
                    "name": "O",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "type": "IndexedAccessType",
              },
              "type": "TypeAlias",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
    });

    test('Optional Indexed Access Type', () => {
      const source = `type T = O?.[k]`;
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
                "indexType": Object {
                  "id": Object {
                    "name": "k",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "objectType": Object {
                  "id": Object {
                    "name": "O",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
                "optional": true,
                "type": "OptionalIndexedAccessType",
              },
              "type": "TypeAlias",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
    });
  });

  describe('Babel', () => {
    test('Basic Indexed Access Type', () => {
      expect(parse(`type T = O[k]`, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'TypeAlias',
              right: {
                type: 'AnyTypeAnnotation',
              },
              typeParameters: null,
            },
          ],
        },
      });
    });

    test('Optional Indexed Access Type', () => {
      expect(parse(`type T = O?.[k]`, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'TypeAlias',
              right: {
                type: 'AnyTypeAnnotation',
              },
              typeParameters: null,
            },
          ],
        },
      });
    });
  });
});
