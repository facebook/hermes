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
import {parseForSnapshot} from '../__test_utils__/parse';

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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "importKind": null,
                  "imported": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "importKind": null,
                  "imported": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                {
                  "importKind": null,
                  "imported": {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                {
                  "importKind": null,
                  "imported": {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                {
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportDefaultSpecifier",
                },
                {
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "importKind": "type",
                  "imported": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportSpecifier",
                },
                {
                  "importKind": "typeof",
                  "imported": {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "value",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "importKind": "type",
                  "imported": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
                    "name": "Foo",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "ImportSpecifier",
                },
                {
                  "importKind": "typeof",
                  "imported": {
                    "name": "Bar",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "type",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "type",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
        {
          "body": [
            {
              "assertions": [],
              "importKind": "typeof",
              "source": {
                "literalType": "string",
                "raw": "'Foo'",
                "type": "Literal",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
      expect(parseForSnapshot(testCase.code, {babel: true}))
        .toMatchInlineSnapshot(`
        {
          "body": [
            {
              "assertions": [],
              "importKind": "typeof",
              "source": {
                "type": "StringLiteral",
                "value": "Foo",
              },
              "specifiers": [
                {
                  "local": {
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
      expectBabelAlignment(testCase);
    });
  });
});
