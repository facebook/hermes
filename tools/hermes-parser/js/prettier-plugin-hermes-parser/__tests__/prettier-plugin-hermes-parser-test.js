/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

// $FlowExpectedError[cannot-resolve-module]
import prettierConfig from '../../.prettierrc.json';

import * as prettier from 'prettier';

function getOptions() {
  return {
    ...prettierConfig,
    parser: 'hermes',
    requirePragma: false,
    plugins: [require('../src/index.js')],
  };
}

describe('prettier-plugin-hermes-parser', () => {
  it('uses plugin', async () => {
    const code = `
      // Function with graphql embed
      function foo() {
        useMutation(
          graphql\`
            mutation FooMutation
            {foo}
          \`
        );
      }

      // Prettier ignore comments work
      const a = {
        ...foo.bar
          // prettier-ignore
          // $FlowFixMe[incompatible-use]
          .baz,
      }

      // Object with css embed
      const styles = {
        content: css\`
          column-gap:
          8px;
          display: grid;
          grid-template-columns: 1fr 3fr;
        \`,
        infoItem: css\`
          margin-bottom: 12px;
          overflow-wrap: break-anywhere;
        \`,
      };

      // TypePredicate
      function isString (x: mixed): x is string { return typeof x === "string"; }

      // ComponentDeclaration
      component Foo(
        bar: {
          // @deprecated
          baz?: Baz,
        },
      ) {}
    `;
    const output = prettier.format(code, getOptions());
    expect(output).toMatchInlineSnapshot(`
      "// Function with graphql embed
      function foo() {
        useMutation(graphql\`
          mutation FooMutation {
            foo
          }
        \`);
      }

      // Prettier ignore comments work
      const a = {
        ...foo.bar
                // prettier-ignore
                // $FlowFixMe[incompatible-use]
                .baz,
      };

      // Object with css embed
      const styles = {
        content: css\`
          column-gap: 8px;
          display: grid;
          grid-template-columns: 1fr 3fr;
        \`,
        infoItem: css\`
          margin-bottom: 12px;
          overflow-wrap: break-anywhere;
        \`,
      };

      // TypePredicate
      function isString(x: mixed): x is string {
        return typeof x === 'string';
      }

      // ComponentDeclaration
      component Foo(
        bar: {
          // @deprecated
          baz?: Baz,
        },
      ) {}
      "
    `);
  });
});
