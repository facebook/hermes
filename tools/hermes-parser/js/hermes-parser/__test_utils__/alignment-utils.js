/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {parse as parseBabelOriginal} from '@babel/parser';
import {parse as parseEspreeOriginal} from 'espree';
import {VisitorKeys} from 'hermes-eslint';
import {parse as parseHermesOriginal} from './parse';

function isNode(thing: mixed): boolean %checks {
  return (
    typeof thing === 'object' && thing != null && typeof thing.type === 'string'
  );
}
function getVisitorKeys(
  thing: $ReadOnly<{[string]: mixed}>,
): $ReadOnlyArray<string> {
  // $FlowExpectedError[incompatible-type]
  const keys = VisitorKeys[thing.type];
  if (keys == null) {
    return Object.keys(thing).filter(
      k =>
        k !== 'parent' &&
        k !== 'range' &&
        k !== 'loc' &&
        k !== 'leadingComments' &&
        k !== 'trailingComments' &&
        !k.startsWith('_'),
    );
  }

  return keys;
}
function traverse(obj: mixed, cb: (node: {[string]: mixed}) => void): void {
  if (!isNode(obj)) {
    return;
  }

  // $FlowExpectedError[incompatible-variance]
  cb(obj);

  const keys = getVisitorKeys(obj);
  for (const key of keys) {
    const child = obj[key];

    if (Array.isArray(child)) {
      for (let j = 0; j < child.length; ++j) {
        traverse(child[j], cb);
      }
    } else {
      traverse(child, cb);
    }
  }
}

function cleanAst(ast: mixed): mixed {
  // $FlowExpectedError[incompatible-use]
  delete ast.comments;
  // $FlowExpectedError[incompatible-use]
  delete ast.tokens;
  // $FlowExpectedError[incompatible-use]
  delete ast.errors;
  traverse(ast, node => {
    delete node.parent;
    delete node.start;
    delete node.end;

    // this is babel-specific
    delete node.extra;
    // $FlowExpectedError[incompatible-use]
    delete node.loc?.identifierName;
    // babel does some inconsistent stuff with the program loc so just ignore it
    if (node.type === 'Program' || node.type === 'File') {
      delete node.loc;
    }
  });

  return JSON.parse(
    // $FlowExpectedError[incompatible-call]
    JSON.stringify(ast, (_, value) => {
      // $FlowExpectedError[illegal-typeof]
      if (typeof value === 'bigint') {
        return value.toString();
      }
      return value;
    }),
  );
}

export function parseBabel(source: string): mixed {
  const ast = parseBabelOriginal(source, {
    attachComment: false,
    errorRecovery: false,
    plugins: [
      'bigInt',
      'dynamicImport',
      'classPrivateMethods',
      'classPrivateProperties',
      'privateIn',

      // from WWW config
      'jsx',
      ['flow', {enums: true}],
      'objectRestSpread',
      'classProperties',
      'numericSeparator',
      'optionalChaining',
      'optionalCatchBinding',
      'nullishCoalescingOperator',
    ],
    ranges: false,
    sourceType: 'module',
    tokens: false,
  });
  return cleanAst(ast);
}

export function parseEspree(source: string): mixed {
  const ast = parseEspreeOriginal(source, {
    comment: false,
    ecmaVersion: 'latest',
    ecmaFeatures: {
      jsx: true,
    },
    loc: false,
    range: false,
    sourceType: 'module',
    tokens: false,
  });
  return cleanAst(ast);
}

export function parseHermes(source: string, style: 'babel' | 'estree'): mixed {
  // $FlowExpectedError[incompatible-call] - the overloads confuse flow
  const ast = parseHermesOriginal(source, {
    babel: style === 'babel',
    sourceType: 'module',
    tokens: false,
  });
  return cleanAst(ast);
}

export type AlignmentExpectation = $ReadOnly<
  | {
      expectToFail: false,
    }
  | {
      expectToFail: 'ast-diff',
    }
  | {
      expectToFail: 'hermes-exception' | 'espree-exception' | 'babel-exception',
      expectedExceptionMessage: string,
    },
>;
export type AlignmentCase = $ReadOnly<{
  code: string,
  espree: AlignmentExpectation,
  babel: AlignmentExpectation,
}>;

function expectAlignment(
  hermesAst: () => mixed,
  otherAst: () => mixed,
  expectation: AlignmentExpectation,
): void {
  switch (expectation.expectToFail) {
    case false:
      // Received = Hermes, Expected = Babel/ESPree
      expect(hermesAst()).toMatchObject(otherAst());
      break;

    case 'ast-diff':
      // Received = Hermes, Expected = Babel/ESPree
      expect(hermesAst()).not.toMatchObject(otherAst());
      break;

    case 'hermes-exception':
      expect(hermesAst).toThrowError(expectation.expectedExceptionMessage);
      break;

    case 'espree-exception':
    case 'babel-exception':
      expect(otherAst).toThrowError(expectation.expectedExceptionMessage);
      break;
  }
}

export function expectEspreeAlignment(testCase: AlignmentCase): void {
  const hermesAst = () => parseHermes(testCase.code, 'estree');
  const espreeAst = () => parseEspree(testCase.code);

  expectAlignment(hermesAst, espreeAst, testCase.espree);
}

export function expectBabelAlignment(testCase: AlignmentCase): void {
  const hermesAst = () => parseHermes(testCase.code, 'babel');
  const babelAst = () => parseBabel(testCase.code);

  expectAlignment(hermesAst, babelAst, testCase.babel);
}
