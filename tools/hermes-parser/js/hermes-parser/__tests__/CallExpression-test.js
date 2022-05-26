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
    espree: {expectToFail: false},
    babel: {expectToFail: false},
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
              "optional": false,
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
                "optional": false,
                "type": "CallExpression",
                "typeArguments": null,
              },
              "optional": false,
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
              "optional": false,
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
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                      "type": "MemberExpression",
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
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                      "type": "MemberExpression",
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
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "fn",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "expression": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "optional": true,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "CallExpression",
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
                "type": "ChainExpression",
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

        (one?.()).two;
      `,
      espree: {expectToFail: false},
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
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
                    "expression": Object {
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
                      "type": "MemberExpression",
                    },
                    "type": "ChainExpression",
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
                "optional": false,
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
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
                    "expression": Object {
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
                      "type": "MemberExpression",
                    },
                    "type": "ChainExpression",
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
                "optional": false,
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                      "type": "MemberExpression",
                    },
                    "optional": true,
                    "property": Object {
                      "name": "fn",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "type": "MemberExpression",
                  },
                  "optional": false,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "expression": Object {
                  "arguments": Array [],
                  "callee": Object {
                    "name": "one",
                    "optional": false,
                    "type": "Identifier",
                    "typeAnnotation": null,
                  },
                  "optional": true,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "arguments": Array [],
                "callee": Object {
                  "expression": Object {
                    "arguments": Array [],
                    "callee": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": true,
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "type": "ChainExpression",
                },
                "optional": false,
                "type": "CallExpression",
                "typeArguments": null,
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
                "computed": false,
                "object": Object {
                  "expression": Object {
                    "arguments": Array [],
                    "callee": Object {
                      "name": "one",
                      "optional": false,
                      "type": "Identifier",
                      "typeAnnotation": null,
                    },
                    "optional": true,
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "type": "ChainExpression",
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

  describe('Tricky short-circuiting case', () => {
    const testCase: AlignmentCase = {
      code: `
        (one?.())?.();
        one?.()?.();
      `,
      espree: {
        // unfortunately the hermes AST doesn't let us differentiate between
        // these two code cases, so we can't create the proper ESTree AST here :(
        expectToFail: 'ast-diff',
      },
      babel: {expectToFail: false},
    };

    test('ESTree', () => {
      expect(parseForSnapshot(testCase.code)).toMatchInlineSnapshot(`
        Object {
          "body": Array [
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "optional": true,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
              },
              "type": "ExpressionStatement",
            },
            Object {
              "directive": null,
              "expression": Object {
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
                    "type": "CallExpression",
                    "typeArguments": null,
                  },
                  "optional": true,
                  "type": "CallExpression",
                  "typeArguments": null,
                },
                "type": "ChainExpression",
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
