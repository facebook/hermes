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

import type {AlignmentCase} from '../__test_utils__/alignment-utils';

import {
  expectBabelAlignment,
  expectEspreeAlignment,
} from '../__test_utils__/alignment-utils';
import {parse, parseForSnapshot} from '../__test_utils__/parse';

describe('ImportDeclaration', () => {
  describe('Named', () => {
    const testCase: AlignmentCase = {
      code: `
        import {Foo} from 'Foo';
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Default', () => {
    const testCase: AlignmentCase = {
      code: `
        import Foo from 'Foo';
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'value',
              specifiers: [
                {
                  type: 'ImportDefaultSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Namespace', () => {
    const testCase: AlignmentCase = {
      code: `
        import * as Foo from 'Foo';
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportNamespaceSpecifier",
                },
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'value',
              specifiers: [
                {
                  type: 'ImportNamespaceSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Default + Named', () => {
    const testCase: AlignmentCase = {
      code: `
        import Foo, {Bar} from 'Foo';
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                Object {
                  "importKind": null,
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
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'value',
              specifiers: [
                {
                  type: 'ImportDefaultSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
                {
                  type: 'ImportSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Bar',
                  },
                  importKind: null,
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Default + Namespace', () => {
    const testCase: AlignmentCase = {
      code: `
        import Foo, * as Bar from 'Foo';
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                Object {
                  "local": Object {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportNamespaceSpecifier",
                },
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'value',
              specifiers: [
                {
                  type: 'ImportDefaultSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
                {
                  type: 'ImportNamespaceSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Bar',
                  },
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Named type / typeof', () => {
    const testCase: AlignmentCase = {
      code: `
        import {type Foo, typeof Bar} from 'Foo';
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
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
                  "importKind": "type",
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
                  "importKind": "typeof",
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
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
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
                  importKind: 'type',
                },
                {
                  type: 'ImportSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Bar',
                  },
                  importKind: 'typeof',
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Default type', () => {
    const testCase: AlignmentCase = {
      code: `
        import type Foo from 'Foo';
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token Foo',
      },
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "assertions": Array [],
              "importKind": "type",
              "source": Object {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": Array [
                Object {
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'type',
              specifiers: [
                {
                  type: 'ImportDefaultSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });

  describe('Default typeof', () => {
    const testCase: AlignmentCase = {
      code: `
        import typeof Foo from 'Foo';
      `,
      espree: {
        expectToFail: 'espree-exception',
        expectedExceptionMessage: 'Unexpected token typeof',
      },
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "assertions": Array [],
              "importKind": "typeof",
              "source": Object {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": Array [
                Object {
                  "local": Object {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
              ],
              "type": "ImportDeclaration",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expect(parse(testCase.code, {babel: true})).toMatchObject({
        type: 'File',
        program: {
          type: 'Program',
          body: [
            {
              type: 'ImportDeclaration',
              importKind: 'typeof',
              specifiers: [
                {
                  type: 'ImportDefaultSpecifier',
                  local: {
                    type: 'Identifier',
                    name: 'Foo',
                  },
                },
              ],
            },
          ],
        },
      });
      expectBabelAlignment(testCase);
    });
  });
});
