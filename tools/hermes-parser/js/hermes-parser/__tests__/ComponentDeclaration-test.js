/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parseForSnapshot} from '../__test_utils__/parse';

const parserOpts = {enableExperimentalComponentSyntax: true};

describe('ComponentDeclaration', () => {
  describe('Basic', () => {
    const testCase: AlignmentCase = {
      code: `
        component Foo() {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code, parserOpts))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [],
                "type": "BlockStatement",
              },
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [],
              "rendersType": null,
              "type": "ComponentDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "async": false,
              "body": {
                "body": [],
                "directives": [],
                "type": "BlockStatement",
              },
              "generator": false,
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [],
              "predicate": null,
              "returnType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "id": {
                      "name": "Node",
                      "type": "Identifier",
                    },
                    "qualification": {
                      "name": "React",
                      "type": "Identifier",
                    },
                    "type": "QualifiedTypeIdentifier",
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "FunctionDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });

  describe('Complex params', () => {
    const testCase: AlignmentCase = {
      code: `
        component Foo(bar: Bar, baz as boo?: Baz, 'data-bav' as bav: Bav) {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code, parserOpts))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [],
                "type": "BlockStatement",
              },
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "local": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": {
                      "type": "TypeAnnotation",
                      "typeAnnotation": {
                        "id": {
                          "name": "Bar",
                          "optional": false,
                          "type": "Identifier",
                          "typeAnnotation": null,
                        },
                        "type": "GenericTypeAnnotation",
                        "typeParameters": null,
                      },
                    },
                  },
                  "name": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "shorthand": true,
                  "type": "ComponentParameter",
                },
                {
                  "local": {
                    "name": "boo",
                    "optional": true,
                    "type": "Identifier",
                    "typeAnnotation": {
                      "type": "TypeAnnotation",
                      "typeAnnotation": {
                        "id": {
                          "name": "Baz",
                          "optional": false,
                          "type": "Identifier",
                          "typeAnnotation": null,
                        },
                        "type": "GenericTypeAnnotation",
                        "typeParameters": null,
                      },
                    },
                  },
                  "name": {
                    "name": "baz",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "shorthand": false,
                  "type": "ComponentParameter",
                },
                {
                  "local": {
                    "name": "bav",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": {
                      "type": "TypeAnnotation",
                      "typeAnnotation": {
                        "id": {
                          "name": "Bav",
                          "optional": false,
                          "type": "Identifier",
                          "typeAnnotation": null,
                        },
                        "type": "GenericTypeAnnotation",
                        "typeParameters": null,
                      },
                    },
                  },
                  "name": {
                    "literalType": "string",
                    "raw": "'data-bav'",
                    "type": "Literal",
                    "value": "data-bav",
                  },
                  "shorthand": false,
                  "type": "ComponentParameter",
                },
              ],
              "rendersType": null,
              "type": "ComponentDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "async": false,
              "body": {
                "body": [],
                "directives": [],
                "type": "BlockStatement",
              },
              "generator": false,
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "properties": [
                    {
                      "computed": false,
                      "key": {
                        "name": "bar",
                        "optional": false,
                        "type": "Identifier",
                      },
                      "method": false,
                      "shorthand": true,
                      "type": "ObjectProperty",
                      "value": {
                        "name": "bar",
                        "type": "Identifier",
                      },
                    },
                    {
                      "computed": false,
                      "key": {
                        "name": "baz",
                        "optional": false,
                        "type": "Identifier",
                      },
                      "method": false,
                      "shorthand": false,
                      "type": "ObjectProperty",
                      "value": {
                        "name": "boo",
                        "type": "Identifier",
                      },
                    },
                    {
                      "computed": false,
                      "key": {
                        "type": "StringLiteral",
                        "value": "data-bav",
                      },
                      "method": false,
                      "shorthand": false,
                      "type": "ObjectProperty",
                      "value": {
                        "name": "bav",
                        "type": "Identifier",
                      },
                    },
                  ],
                  "type": "ObjectPattern",
                  "typeAnnotation": {
                    "type": "TypeAnnotation",
                    "typeAnnotation": {
                      "id": {
                        "name": "$ReadOnly",
                        "type": "Identifier",
                      },
                      "type": "GenericTypeAnnotation",
                      "typeParameters": {
                        "params": [
                          {
                            "callProperties": [],
                            "exact": false,
                            "indexers": [],
                            "inexact": true,
                            "internalSlots": [],
                            "properties": [],
                            "type": "ObjectTypeAnnotation",
                          },
                        ],
                        "type": "TypeParameterInstantiation",
                      },
                    },
                  },
                },
              ],
              "predicate": null,
              "returnType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "id": {
                      "name": "Node",
                      "type": "Identifier",
                    },
                    "qualification": {
                      "name": "React",
                      "type": "Identifier",
                    },
                    "type": "QualifiedTypeIdentifier",
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "FunctionDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });

  describe('default params', () => {
    const testCase: AlignmentCase = {
      code: `
        component Foo(bar?: Bar = '') {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code, parserOpts))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [],
                "type": "BlockStatement",
              },
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "local": {
                    "left": {
                      "name": "bar",
                      "optional": true,
                      "type": "Identifier",
                      "typeAnnotation": {
                        "type": "TypeAnnotation",
                        "typeAnnotation": {
                          "id": {
                            "name": "Bar",
                            "optional": false,
                            "type": "Identifier",
                            "typeAnnotation": null,
                          },
                          "type": "GenericTypeAnnotation",
                          "typeParameters": null,
                        },
                      },
                    },
                    "right": {
                      "literalType": "string",
                      "raw": "''",
                      "type": "Literal",
                      "value": "",
                    },
                    "type": "AssignmentPattern",
                  },
                  "name": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "shorthand": true,
                  "type": "ComponentParameter",
                },
              ],
              "rendersType": null,
              "type": "ComponentDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "async": false,
              "body": {
                "body": [],
                "directives": [],
                "type": "BlockStatement",
              },
              "generator": false,
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "properties": [
                    {
                      "computed": false,
                      "key": {
                        "name": "bar",
                        "optional": false,
                        "type": "Identifier",
                      },
                      "method": false,
                      "shorthand": true,
                      "type": "ObjectProperty",
                      "value": {
                        "left": {
                          "name": "bar",
                          "type": "Identifier",
                        },
                        "right": {
                          "type": "StringLiteral",
                          "value": "",
                        },
                        "type": "AssignmentPattern",
                      },
                    },
                  ],
                  "type": "ObjectPattern",
                  "typeAnnotation": {
                    "type": "TypeAnnotation",
                    "typeAnnotation": {
                      "id": {
                        "name": "$ReadOnly",
                        "type": "Identifier",
                      },
                      "type": "GenericTypeAnnotation",
                      "typeParameters": {
                        "params": [
                          {
                            "callProperties": [],
                            "exact": false,
                            "indexers": [],
                            "inexact": true,
                            "internalSlots": [],
                            "properties": [],
                            "type": "ObjectTypeAnnotation",
                          },
                        ],
                        "type": "TypeParameterInstantiation",
                      },
                    },
                  },
                },
              ],
              "predicate": null,
              "returnType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "id": {
                      "name": "Node",
                      "type": "Identifier",
                    },
                    "qualification": {
                      "name": "React",
                      "type": "Identifier",
                    },
                    "type": "QualifiedTypeIdentifier",
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "FunctionDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });

  describe('return type', () => {
    const testCase: AlignmentCase = {
      code: `
        component Foo() renders SpecialType {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code, parserOpts))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [],
                "type": "BlockStatement",
              },
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [],
              "rendersType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "name": "SpecialType",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "ComponentDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "async": false,
              "body": {
                "body": [],
                "directives": [],
                "type": "BlockStatement",
              },
              "generator": false,
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [],
              "predicate": null,
              "returnType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "name": "SpecialType",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "FunctionDeclaration",
              "typeParameters": null,
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });

  describe('type parameters', () => {
    const testCase: AlignmentCase = {
      code: `
        component Foo<T1, T2>(bar: T1) renders T2 {}
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {
        expectToFail: 'babel-exception',
        expectedExceptionMessage: 'Unexpected token',
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code, parserOpts))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "body": {
                "body": [],
                "type": "BlockStatement",
              },
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "local": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": {
                      "type": "TypeAnnotation",
                      "typeAnnotation": {
                        "id": {
                          "name": "T1",
                          "optional": false,
                          "type": "Identifier",
                          "typeAnnotation": null,
                        },
                        "type": "GenericTypeAnnotation",
                        "typeParameters": null,
                      },
                    },
                  },
                  "name": {
                    "name": "bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "shorthand": true,
                  "type": "ComponentParameter",
                },
              ],
              "rendersType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "name": "T2",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "ComponentDeclaration",
              "typeParameters": {
                "params": [
                  {
                    "bound": null,
                    "default": null,
                    "name": "T1",
                    "type": "TypeParameter",
                    "usesExtendsBound": false,
                    "variance": null,
                  },
                  {
                    "bound": null,
                    "default": null,
                    "name": "T2",
                    "type": "TypeParameter",
                    "usesExtendsBound": false,
                    "variance": null,
                  },
                ],
                "type": "TypeParameterDeclaration",
              },
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parseForSnapshot(testCase.code, {babel: true, ...parserOpts}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "async": false,
              "body": {
                "body": [],
                "directives": [],
                "type": "BlockStatement",
              },
              "generator": false,
              "id": {
                "name": "Foo",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "params": [
                {
                  "properties": [
                    {
                      "computed": false,
                      "key": {
                        "name": "bar",
                        "optional": false,
                        "type": "Identifier",
                      },
                      "method": false,
                      "shorthand": true,
                      "type": "ObjectProperty",
                      "value": {
                        "name": "bar",
                        "type": "Identifier",
                      },
                    },
                  ],
                  "type": "ObjectPattern",
                  "typeAnnotation": {
                    "type": "TypeAnnotation",
                    "typeAnnotation": {
                      "id": {
                        "name": "$ReadOnly",
                        "type": "Identifier",
                      },
                      "type": "GenericTypeAnnotation",
                      "typeParameters": {
                        "params": [
                          {
                            "callProperties": [],
                            "exact": false,
                            "indexers": [],
                            "inexact": true,
                            "internalSlots": [],
                            "properties": [],
                            "type": "ObjectTypeAnnotation",
                          },
                        ],
                        "type": "TypeParameterInstantiation",
                      },
                    },
                  },
                },
              ],
              "predicate": null,
              "returnType": {
                "type": "TypeAnnotation",
                "typeAnnotation": {
                  "id": {
                    "name": "T2",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "GenericTypeAnnotation",
                  "typeParameters": null,
                },
              },
              "type": "FunctionDeclaration",
              "typeParameters": {
                "params": [
                  {
                    "bound": null,
                    "default": null,
                    "name": "T1",
                    "type": "TypeParameter",
                    "usesExtendsBound": false,
                    "variance": null,
                  },
                  {
                    "bound": null,
                    "default": null,
                    "name": "T2",
                    "type": "TypeParameter",
                    "usesExtendsBound": false,
                    "variance": null,
                  },
                ],
                "type": "TypeParameterDeclaration",
              },
            },
          ],
          "type": "Program",
        }
      `);
      expectBabelAlignment(testCase);
    });
  });
});
