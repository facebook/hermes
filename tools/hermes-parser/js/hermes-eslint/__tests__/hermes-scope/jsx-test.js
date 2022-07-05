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

import {DefinitionType, ScopeType} from '../../src';
import {verifyHasScopes} from '../../__test_utils__/verifyHasScopes';

describe('jsx', () => {
  describe('jsx pragma', () => {
    const code = `
      import React from 'react';
      import CustomReact from 'custom-react';
      import PragmaReact from 'pragma-react';
      <div />
    `;
    const parserOptions = {jsxPragma: 'CustomReact'};
    const pragmaComment = `\x40jsx PragmaReact.foo`;

    test('Defaults', () => {
      verifyHasScopes(
        `
          /**
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
            ],
          },
        ],
      );
    });
    test('Explicit Option', () => {
      verifyHasScopes(
        `
          /**
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
            ],
          },
        ],
        parserOptions,
      );
    });
    test('Comment pragma overrides defaults', () => {
      verifyHasScopes(
        `
          /**
           * ${pragmaComment}
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
            ],
          },
        ],
      );
    });
    test('Comment pragma overrides explicit option', () => {
      verifyHasScopes(
        `
          /**
           * ${pragmaComment}
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
            ],
          },
        ],
        parserOptions,
      );
    });
  });

  describe('jsx fragment pragma', () => {
    const code = `
      import React from 'react';
      import CustomReact from 'custom-react';
      import {CustomFragment} from 'custom-react';
      import PragmaReact from 'pragma-react';
      import {PragmaFragment} from 'pragma-react';
      <></>
    `;
    const parserOptions = {
      jsxPragma: 'CustomReact',
      jsxFragmentName: 'CustomFragment',
    };
    const pragmaComment = `\x40jsx PragmaReact.foo
      * \x40jsxFrag PragmaFragment.bar
    `;

    test('Defaults', () => {
      verifyHasScopes(
        `
          /**
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
            ],
          },
        ],
      );
    });
    test('Explicit Option', () => {
      verifyHasScopes(
        `
          /**
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'CustomFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
            ],
          },
        ],
        parserOptions,
      );
    });
    test('Comment pragma overrides defaults', () => {
      verifyHasScopes(
        `
          /**
           * ${pragmaComment}
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'PragmaFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
            ],
          },
        ],
      );
    });
    test('Comment pragma overrides explicit option', () => {
      verifyHasScopes(
        `
          /**
           * ${pragmaComment}
           */
          ${code}
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'CustomFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 0,
              },
              {
                name: 'PragmaReact',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'PragmaFragment',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
            ],
          },
        ],
        parserOptions,
      );
    });
  });

  describe('fbt', () => {
    describe('with option', () => {
      test('identifier', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 0,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: true,
          },
        );
      });
      test('identifier - fbs', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbs from 'fbs';
            <fbs />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 0,
                },
                {
                  name: 'fbs',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: true,
          },
        );
      });
      test('namespace', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt:foo />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 0,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: true,
          },
        );
      });
      test('member expr', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt.bar />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 0,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: true,
          },
        );
      });
    });
    describe('without option', () => {
      test('identifier', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 0,
                },
              ],
            },
          ],
          {
            fbt: false,
          },
        );
      });
      test('namespace', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt:foo />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  // follows the default rules and creates a reference
                  // due to the namespacing
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: false,
          },
        );
      });
      test('member expr', () => {
        verifyHasScopes(
          `
            import React from 'react';
            import fbt from 'fbt';
            <fbt.bar />;
          `,
          [
            {
              type: ScopeType.Module,
              variables: [
                {
                  name: 'React',
                  type: DefinitionType.ImportBinding,
                  referenceCount: 1,
                },
                {
                  name: 'fbt',
                  type: DefinitionType.ImportBinding,
                  // follows the default rules and creates a reference
                  // due to the namespacing
                  referenceCount: 1,
                },
              ],
            },
          ],
          {
            fbt: false,
          },
        );
      });
    });
  });

  describe('Component name references', () => {
    test('Upper-case references name', () => {
      verifyHasScopes(
        `
          import React from 'react';
          var Component;
          <Component />;
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'Component',
                type: DefinitionType.Variable,
                referenceCount: 1,
              },
            ],
          },
        ],
      );
    });

    test('Lower-case does not reference name', () => {
      verifyHasScopes(
        `
          import React from 'react';
          var component;
          <component />;
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'component',
                type: DefinitionType.Variable,
                referenceCount: 0,
              },
            ],
          },
        ],
      );
    });

    test('Member expressions are referenced regardless of casing', () => {
      verifyHasScopes(
        `
          import React from 'react';
          var lowerNamespacedName;
          <lowerNamespacedName.foo />;

          var UpperNamespacedName;
          <UpperNamespacedName.foo />;
        `,
        [
          {
            type: ScopeType.Module,
            variables: [
              {
                name: 'React',
                type: DefinitionType.ImportBinding,
                referenceCount: 1,
              },
              {
                name: 'lowerNamespacedName',
                type: DefinitionType.Variable,
                referenceCount: 1,
              },
              {
                name: 'UpperNamespacedName',
                type: DefinitionType.Variable,
                referenceCount: 1,
              },
            ],
          },
        ],
      );
    });
  });
});
