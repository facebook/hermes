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

describe('CallExpression', () => {
  const testCase: AlignmentCase = {
    code: `
      one();
      two()();
      three.four();
    `,
    espree: {
      // TODO - ESTree spec is now ChainExpression + CallExpression
      expectToFail: 'ast-diff',
    },
    babel: {
      expectToFail: false,
    },
  };

  test('ESTree', () => {
    expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
      Object {
        "body": Array [
          Object {
            "directive": null,
            "expression": Object {
              "arguments": Array [],
              "callee": Object {
                "name": "one",
                "optional": false,
                "type": "Identifier",
                "typeAnnotation": null,
              },
              "type": "CallExpression",
              "typeArguments": null,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "arguments": Array [],
              "callee": Object {
                "arguments": Array [],
                "callee": Object {
                  "name": "two",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "CallExpression",
              "typeArguments": null,
            },
            "type": "ExpressionStatement",
          },
          Object {
            "directive": null,
            "expression": Object {
              "arguments": Array [],
              "callee": Object {
                "computed": false,
                "object": Object {
                  "name": "three",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "optional": false,
                "property": Object {
                  "name": "four",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "MemberExpression",
              },
              "type": "CallExpression",
              "typeArguments": null,
            },
            "type": "ExpressionStatement",
          },
        ],
        "type": "Program",
      }
    `);
    expectEspreeAlignment(testCase);
  });

  test('Babel', () => {
    expectBabelAlignment(testCase);
  });
});

// This is a special case to test because of the way parentheses
// short-circuit the optional chain
describe('OptionalCallExpression', () => {
  describe('Without parentheses', () => {
    const testCase: AlignmentCase = {
      code: `
        one?.fn();
        one?.two.fn();
        one.two?.fn();
        one.two?.three.fn();
        one.two?.three?.fn();

        one?.();
        one?.()();
        one?.()?.();

        one?.().two;
      `,
      espree: {
        // TODO - ESTree spec is now ChainExpression + CallExpression
        expectToFail: 'ast-diff',
      },
      babel: {
        expectToFail: false,
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": true,
                    "property": Object {
                      "name": "two",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": false,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": false,
                    "property": Object {
                      "name": "two",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "MemberExpression",
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "computed": false,
                      "object": Object {
                        "name": "one",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "property": Object {
                        "name": "two",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "three",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": false,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "computed": false,
                      "object": Object {
                        "name": "one",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "property": Object {
                        "name": "two",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "three",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "name": "one",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "optional": true,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "optional": true,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "computed": false,
                "object": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "optional": false,
                "property": Object {
                  "name": "two",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "OptionalMemberExpression",
              },
              "type": "ExpressionStatement",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expectBabelAlignment(testCase);
    });
  });

  // This is a special case to test because of the way parentheses
  // short-circuit the optional chain
  describe('With parentheses', () => {
    const testCase: AlignmentCase = {
      code: `
        (one?.fn());
        (one?.two).fn();
        (one.two?.fn());
        (one.two?.three).fn();
        (one.two?.three?.fn());

        (one?.());
        (one?.())();
        (one?.())?.();

        (one?.()).two;
      `,
      espree: {
        // TODO - ESTree spec is now ChainExpression + CallExpression
        expectToFail: 'ast-diff',
      },
      babel: {
        expectToFail: false,
      },
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": true,
                    "property": Object {
                      "name": "two",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": false,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "MemberExpression",
                },
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": false,
                    "property": Object {
                      "name": "two",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "MemberExpression",
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "computed": false,
                      "object": Object {
                        "name": "one",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "property": Object {
                        "name": "two",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "three",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": false,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "MemberExpression",
                },
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "computed": false,
                  "object": Object {
                    "computed": false,
                    "object": Object {
                      "computed": false,
                      "object": Object {
                        "name": "one",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "optional": false,
                      "property": Object {
                        "name": "two",
                        "optional": false,
                        "type": "Identifier",
                        "typeAnnotation": null,
                      },
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "three",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "OptionalMemberExpression",
                  },
                  "optional": true,
                  "property": Object {
                    "name": "fn",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "type": "OptionalMemberExpression",
                },
                "optional": false,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "name": "one",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "optional": true,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "optional": true,
                "type": "OptionalCallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "computed": false,
                "object": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "OptionalCallExpression",
                  "typeArguments": null,
                },
                "optional": false,
                "property": Object {
                  "name": "two",
                  "optional": false,
                  "type": "Identifier",
                  "typeAnnotation": null,
                },
                "type": "MemberExpression",
              },
              "type": "ExpressionStatement",
            },
          ],
          "type": "Program",
        }
      `);
      expectEspreeAlignment(testCase);
    });

    test('Babel', () => {
      expectBabelAlignment(testCase);
    });
  });
});
