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

import {parseForSnapshot} from '../__test_utils__/parse';

describe('ClassProperty', () => {
  const source = `
    class C {
      foo;
      foo = 1;
      foo: F = 1;
      declare foo: 1;
    }
  `;

  test('ESTree', () => {
    expect(parseForSnapshot(source)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "body": Object {
              "body": Array [
                Object {
                  "computed": false,
                  "declare": false,
                  "key": Object {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "type": "ClassProperty",
                  "typeAnnotation": null,
                  "value": null,
                  "variance": null,
                },
                Object {
                  "computed": false,
                  "declare": false,
                  "key": Object {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "type": "ClassProperty",
                  "typeAnnotation": null,
                  "value": Object {
                    "literalType": "numeric",
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                  "variance": null,
                },
                Object {
                  "computed": false,
                  "declare": false,
                  "key": Object {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "type": "ClassProperty",
                  "typeAnnotation": Object {
                    "type": "TypeAnnotation",
                    "typeAnnotation": Object {
                      "id": Object {
                        "name": "F",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "GenericTypeAnnotation",
                      "typeParameters": null,
                    },
                  },
                  "value": Object {
                    "literalType": "numeric",
                    "raw": "1",
                    "type": "Literal",
                    "value": 1,
                  },
                  "variance": null,
                },
                Object {
                  "computed": false,
                  "declare": true,
                  "key": Object {
                    "name": "foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": false,
                  "static": false,
                  "type": "ClassProperty",
                  "typeAnnotation": Object {
                    "type": "TypeAnnotation",
                    "typeAnnotation": Object {
                      "raw": "1",
                      "type": "NumberLiteralTypeAnnotation",
                      "value": 1,
                    },
                  },
                  "value": null,
                  "variance": null,
                },
              ],
              "type": "ClassBody",
            },
            "decorators": Array [],
            "id": Object {
              "name": "C",
              "optional": false,
              "type": "Identifier",
              "typeAnnotation": null,
            },
            "implements": Array [],
            "superClass": null,
            "superTypeParameters": null,
            "type": "ClassDeclaration",
            "typeParameters": null,
          },
        ],
        "type": "Program",
      }
    `);
  });
});
