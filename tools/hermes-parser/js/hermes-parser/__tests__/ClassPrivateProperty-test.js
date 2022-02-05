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

import {parse} from '../__test_utils__/parse';

describe('Private properties', () => {
  test('ESTree', () => {
    // Private property uses are not supported
    expect(() => parse(`foo.#private`)).toThrow(
      new SyntaxError('Private properties are not supported (1:4)'),
    );

    // Private property brand checks are not supported
    expect(() => parse(`#private in foo`)).toThrow(
      new SyntaxError(`\
invalid expression (1:0)
#private in foo
^`),
    );

    // Private property definitions are not supported
    expect(() => parse(`class C { #private }`)).toThrow(
      new SyntaxError('Private properties are not supported (1:10)'),
    );
  });
});
