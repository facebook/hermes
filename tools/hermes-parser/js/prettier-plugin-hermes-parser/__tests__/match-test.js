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

function format(code: string) {
  const options = {
    ...prettierConfig,
    parser: 'hermes',
    requirePragma: false,
    plugins: [require('../src/index.js')],
  };
  return prettier.format(code, options);
}

describe('Match expression', () => {
  test('empty', async () => {
    expect(
      format(`
       const e = match (x) {};
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
      };
      "
    `);
  });

  test('body', async () => {
    expect(
      format(`
       const e = match (x) {
        0: f(),
        1: (f(), 1),
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        0: f(),
        1: (f(), 1),
      };
      "
    `);
  });

  test('guards', async () => {
    expect(
      format(`
       const e = match (x) {
         1 if b: true,
         'foo' if f(): true,
         2 if x < y: true,
         3 if (f(), x): true,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        1 if b: true,
        'foo' if f(): true,
        2 if x < y: true,
        3 if (f(), x): true,
      };
      "
    `);
  });

  test('sequence expressions: always with parens', async () => {
    expect(
      format(`
       const e = match (x, y) {
         1: (x, y),
         2 if (x, y): 0,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match ((x, y)) {
        1: (x, y),
        2 if (x, y): 0,
      };
      "
    `);
  });

  test('patterns: core', async () => {
    expect(
      format(`
       const e = match (x) {
         "s": 1,
         true: 1,
         null: 1,
         3: 1,
         4n: 1,
         +5: 1,
         -6: 1,
         +7n: 1,
         -8n: 1,
         y: 1,
         const y: y,
         let y: y,
         var y: y,
         ('s'): 1,
         _: 1,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        's': 1,
        true: 1,
        null: 1,
        3: 1,
        4n: 1,
        +5: 1,
        -6: 1,
        +7n: 1,
        -8n: 1,
        y: 1,
        const y: y,
        let y: y,
        var y: y,
        's': 1,
        _: 1,
      };
      "
    `);
  });

  test('patterns: member', async () => {
    expect(
      format(`
       const e = match (x) {
         foo.bar: true,
         foo[1]: true,
         foo["bar"]: true,
         foo.bar[1]: true,
         foo[1].bar["baz"]: true,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        foo.bar: true,
        foo[1]: true,
        foo['bar']: true,
        foo.bar[1]: true,
        foo[1].bar['baz']: true,
      };
      "
    `);
  });

  test('patterns: object', async () => {
    expect(
      format(`
       const e = match (x) {
         {foo: 1, bar: 2}: 1,
         {'foo': 1}: 1,
         {111: true}: 1,
         {foo: const y}: y,
         {const x, let y, var z}: y,
         {const x, ...const y}: y,
         {const x, ...let y}: y,
         {const x, ...var z}: y,
         {const x, ...}: 1,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        {foo: 1, bar: 2}: 1,
        {'foo': 1}: 1,
        {111: true}: 1,
        {foo: const y}: y,
        {const x, let y, var z}: y,
        {const x, ...const y}: y,
        {const x, ...let y}: y,
        {const x, ...var z}: y,
        {const x, ...}: 1,
      };
      "
    `);
  });

  test('patterns: array', async () => {
    expect(
      format(`
       const e = match (x) {
         [10]: 1,
         [const y, 1]: y,
         [1, ...]: 1,
         [1, 2, ...const rest]: rest,
         [...let rest]: rest,
         [...var rest]: rest,
         [{nested: [1, const x]}]: x,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        [10]: 1,
        [const y, 1]: y,
        [1, ...]: 1,
        [1, 2, ...const rest]: rest,
        [...let rest]: rest,
        [...var rest]: rest,
        [{nested: [1, const x]}]: x,
      };
      "
    `);
  });

  test('patterns: or & as', async () => {
    expect(
      format(`
       const e = match (x) {
         "s" | true | null: 1,
         {foo: 1 | 2}: 2,
         {foo: [1] as y}: y,
         {foo: 1 | 2 | 3 as y}: y,
         {foo: (1 | 2 | 3) as y}: y,
         {foo: [1] as const y}: y,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        's' | true | null: 1,
        {foo: 1 | 2}: 2,
        {foo: [1] as y}: y,
        {foo: (1 | 2 | 3) as y}: y,
        {foo: (1 | 2 | 3) as y}: y,
        {foo: [1] as const y}: y,
      };
      "
    `);
  });

  test('long lines', async () => {
    expect(
      format(`
       const e = match (x) {
         2 if f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23): true,
         fooooooooooooooooooooooooooooo: fooooooooooooooooooooooooooooooooooooooooooooooooooooooo,
         foo.loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong: true,
         foo["loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong"]: true,
         foo.loooooooooooooooooooooooooooooooooooooooooooooooooooooong[3333333333333333333333333333333333333333333333333333333333333333333333]: true,
         {foo: 1, bar: 2, loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 3}: 1,
         {foo: 1, bar: 2, loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 3, ...}: 1,
         {foo: 1, bar: 2, loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 3333333333333333333333333333333333333333333333333333333333333333333333}: 1,
         [1, 2, 333333333333333333333333333333333333333333333333333333333333333333333]: 1,
         [1, 2, 333333333333333333333333333333333333333333333333333333333333333333333, ...]: 1,
         [{foo: 1, bar: 2, loooooooooooooooooooooooooooooooooooooooooooooooooooooong: [33333333333333333333]}]: 1,
        fooooooooooooooooooooooooo | loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 1,
        (fooooooooooooooooooooooooo | loooooooooooooooooooooooooooooooooooooooooooooooooooooong) as foo: 1,
       }
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        2 if f(
          1,
          2,
          3,
          4,
          5,
          6,
          7,
          8,
          9,
          10,
          11,
          12,
          13,
          14,
          15,
          16,
          17,
          18,
          19,
          20,
          21,
          22,
          23,
        ): true,
        fooooooooooooooooooooooooooooo:
          fooooooooooooooooooooooooooooooooooooooooooooooooooooooo,
        foo.loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong: true,
        foo[
          'loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong'
        ]: true,
        foo.loooooooooooooooooooooooooooooooooooooooooooooooooooooong[
          3333333333333333333333333333333333333333333333333333333333333333333333
        ]: true,
        {
          foo: 1,
          bar: 2,
          loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 3,
        }: 1,
        {
          foo: 1,
          bar: 2,
          loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 3,
          ...
        }: 1,
        {
          foo: 1,
          bar: 2,
          loooooooooooooooooooooooooooooooooooooooooooooooooooooong:
            3333333333333333333333333333333333333333333333333333333333333333333333,
        }: 1,
        [1, 2, 333333333333333333333333333333333333333333333333333333333333333333333]:
          1,
        [
          1,
          2,
          333333333333333333333333333333333333333333333333333333333333333333333,
          ...
        ]: 1,
        [
          {
            foo: 1,
            bar: 2,
            loooooooooooooooooooooooooooooooooooooooooooooooooooooong:
              [33333333333333333333],
          },
        ]: 1,
        fooooooooooooooooooooooooo |
        loooooooooooooooooooooooooooooooooooooooooooooooooooooong: 1,
        (fooooooooooooooooooooooooo |
        loooooooooooooooooooooooooooooooooooooooooooooooooooooong) as foo: 1,
      };
      "
    `);
  });

  test('comments', async () => {
    expect(
      format(`
       const e = match (x) {
         // bork
         [
           // bork bork
           1, 2,
         ]:
           // bork bork bork
           true,
       };
      `),
    ).toMatchInlineSnapshot(`
      "const e = match (x) {
        // bork
        [
          // bork bork
          1,
          2,
        ]:
          // bork bork bork
          true,
      };
      "
    `);
  });
});

describe('Match statement', () => {
  test('empty', async () => {
    expect(
      format(`
       match (x) {};
      `),
    ).toMatchInlineSnapshot(`
      "match (x) {
      }
      "
    `);
  });

  test('guards', async () => {
    expect(
      format(`
       match (a) {
         1 if b: {
           const x = 1;
         }
         'foo' if f(): {
           const x = 2;
         }
         2 if x < y: {
           const x = 3;
         }
       }
      `),
    ).toMatchInlineSnapshot(`
      "match (a) {
        1 if b: {
          const x = 1;
        }
        'foo' if f(): {
          const x = 2;
        }
        2 if x < y: {
          const x = 3;
        }
      }
      "
    `);
  });

  test('long lines', async () => {
    expect(
      format(`
       match (x) {
         fooooooooooooooooooooooooooooo: {
           fooooooooooooooooooooooooooooooooooooooooooooooooooooooo();
         }
       };
      `),
    ).toMatchInlineSnapshot(`
      "match (x) {
        fooooooooooooooooooooooooooooo: {
          fooooooooooooooooooooooooooooooooooooooooooooooooooooooo();
        }
      }
      "
    `);
  });

  test('big example', async () => {
    expect(
      format(`
       match (node) {
         {
           type: 'GenericTypeAnnotation',
           id:
             {type: 'Identifier', name: 'React$Element' } |
             {
               type: 'QualifiedTypeIdentifier',
               qualification: {type: 'Identifier', name: 'React'},
               name: 'Element',
             },
           typeParameters: {
             params: [{type: 'TypeofTypeAnnotation', argument: 'Identifier', const name}],
           }
         } if name.startsWith(designSystem): {
           doStuffWithName(name);
         }
       }
      `),
    ).toMatchInlineSnapshot(`
      "match (node) {
        {
          type: 'GenericTypeAnnotation',
          id:
            {type: 'Identifier', name: 'React$Element'} |
            {
              type: 'QualifiedTypeIdentifier',
              qualification: {type: 'Identifier', name: 'React'},
              name: 'Element',
            },
          typeParameters:
            {
              params:
                [{type: 'TypeofTypeAnnotation', argument: 'Identifier', const name}],
            },
        } if name.startsWith(designSystem): {
          doStuffWithName(name);
        }
      }
      "
    `);
  });
});
