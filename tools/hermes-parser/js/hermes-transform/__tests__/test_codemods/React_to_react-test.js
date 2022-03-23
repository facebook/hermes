/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {isStringLiteral} from 'hermes-estree';
import {t, transform} from './test-utils';

function codemod(code: string) {
  return transform(code, context => ({
    ImportDeclaration(node) {
      if (node.source.value !== 'React') {
        return;
      }

      context.replaceNode(node.source, t.StringLiteral({value: 'react'}));
    },
    CallExpression(node) {
      if (
        node.callee.type !== 'Identifier' ||
        node.callee.name !== 'require' ||
        node.arguments.length !== 1 ||
        !isStringLiteral(node.arguments[0])
      ) {
        return;
      }

      context.replaceNode(node.arguments[0], t.StringLiteral({value: 'react'}));
    },
  }));
}

describe('React to react', () => {
  it('should transform files correctly', () => {
    const result = codemod(`\
/**
 * LICENCE GOES HERE
 *
 * @flow strict-local
 * @format
 */

import React from 'React';
import type React from 'React';
import React, {useRef} from 'React';
import type React, {useRef} from 'React';
import {useRef} from 'React';

const React = require('React');
const {useRef} = require('React');

function foo() {
  const state = require('React').useState();
}
`);

    expect(result).toBe(`\
/**
 * LICENCE GOES HERE
 *
 * @flow strict-local
 * @format
 */

import React from 'react';
import type React from 'react';
import React, {useRef} from 'react';
import type React, {useRef} from 'react';
import {useRef} from 'react';

const React = require('react');
const {useRef} = require('react');

function foo() {
  const state = require('react').useState();
}
`);
  });
});
