/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

import {printForSnapshot} from '../__test_utils__/parse';

const transform = (src: string) =>
  printForSnapshot(src, {babel: true, enableExperimentalFlowMatchSyntax: true});

function runMatchStmt(code: string, x: mixed): mixed {
  const f: $FlowFixMe = new Function(
    'x',
    'foo',
    'bar',
    'no',
    'yes',
    `let out; ${code}; return out;`,
  );
  return f(
    x,
    'foo',
    {a: 'bar'},
    () => false,
    () => true,
  );
}

describe('MatchStatement', () => {
  test('no cases', async () => {
    const code = `
      match (x()) {}
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        const $$gen$m1 = x();
        throw Error("Match: No case succesfully matched. Make exhaustive or add a wildcard case using '_'. Argument: " + $$gen$m1);
      }"
    `);

    expect(() => runMatchStmt(output, () => 'a')).toThrow(
      /Match: No case succesfully matched\. .* Argument: a/,
    );
  });

  test('simple and guards', async () => {
    const code = `
      match (x) {
        'a': {
          out = 0;
        }
        'b': {
          out = 1;
        }
        'c' if no(): {
          out = 2
        }
        'd' if yes(): {
          out = 3
        }
        _: {
          out = 4;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a') {
          out = 0;
          break $$gen$m0;
        }

        if (x === 'b') {
          out = 1;
          break $$gen$m0;
        }

        if (x === 'c') {
          if (no()) {
            out = 2;
            break $$gen$m0;
          }
        }

        if (x === 'd') {
          if (yes()) {
            out = 3;
            break $$gen$m0;
          }
        }

        {
          out = 4;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'b')).toBe(1);
    expect(runMatchStmt(output, 'c')).toBe(4);
    expect(runMatchStmt(output, 'd')).toBe(3);
    expect(runMatchStmt(output, 'xxx')).toBe(4);
  });

  test('simple or', async () => {
    const code = `
      match (x) {
        'a' | 'b': {
          out = 0;
        }
        _: {
          out = 1;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a' || x === 'b') {
          out = 0;
          break $$gen$m0;
        }

        {
          out = 1;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'b')).toBe(0);
    expect(runMatchStmt(output, 'xxx')).toBe(1);
  });

  test('no wildcard', async () => {
    const code = `
      match (x) {
        'a': {
          out = 0;
        }
        'b': {
          out = 1;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a') {
          out = 0;
          break $$gen$m0;
        }

        if (x === 'b') {
          out = 1;
          break $$gen$m0;
        }

        throw Error("Match: No case succesfully matched. Make exhaustive or add a wildcard case using '_'. Argument: " + x);
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'b')).toBe(1);
    expect(() => runMatchStmt(output, 'xxx')).toThrow(
      /Match: No case succesfully matched\. .* Argument: xxx/,
    );
  });

  test('only wildcard', async () => {
    const code = `
      match (x) {
        _: {
          out = 0;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        {
          out = 0;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'xxx')).toBe(0);
  });

  test('complex argument', async () => {
    const code = `
      match (x()) {
        'a': {
          out = 0;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        const $$gen$m1 = x();

        if ($$gen$m1 === 'a') {
          out = 0;
          break $$gen$m0;
        }

        throw Error("Match: No case succesfully matched. Make exhaustive or add a wildcard case using '_'. Argument: " + $$gen$m1);
      }"
    `);

    expect(runMatchStmt(output, () => 'a')).toBe(0);
  });

  test('NaN', async () => {
    const code = `
      match (x) {
        NaN: {
          out = 0;
        }
        _: {
          out = 1;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (Number.isNaN(x)) {
          out = 0;
          break $$gen$m0;
        }

        {
          out = 1;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, NaN)).toBe(0);
    expect(runMatchStmt(output, 'xxx')).toBe(1);
  });

  test('guard', async () => {
    const code = `
      match (x) {
        'a' if yes(): {
          out = 0;
        }
        _: {
          out = 1;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a') {
          if (yes()) {
            out = 0;
            break $$gen$m0;
          }
        }

        {
          out = 1;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'xxx')).toBe(1);
  });

  test('binding', async () => {
    const code = `
      match (x) {
        'a': {
          out = 0;
        }
        const a: {
          out = a;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a') {
          out = 0;
          break $$gen$m0;
        }

        {
          const a = x;
          out = a;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'xxx')).toBe('xxx');
  });

  test('object and array patterns', async () => {
    const code = `
      match (x) {
        ['a']: {
          out = 0;
        }
        {b: 'b'}: {
          out = 1;
        }
        _: {
          out = 2;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (Array.isArray(x) && x.length === 1 && x[0] === 'a') {
          out = 0;
          break $$gen$m0;
        }

        if (typeof x === "object" && x !== null && x.b === 'b') {
          out = 1;
          break $$gen$m0;
        }

        {
          out = 2;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, ['a'])).toBe(0);
    expect(runMatchStmt(output, {b: 'b'})).toBe(1);
    expect(runMatchStmt(output, 'xxx')).toBe(2);
  });

  test('object and array bindings', async () => {
    const code = `
      match (x) {
        [const a]: {
          out = a;
        }
        {const b}: {
          out = b;
        }
        const a if no(): {
          out = a;
        }
        _: {
          out = 3;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (Array.isArray(x) && x.length === 1) {
          const a = x[0];
          out = a;
          break $$gen$m0;
        }

        if (typeof x === "object" && x !== null && "b" in x) {
          const b = x.b;
          out = b;
          break $$gen$m0;
        }

        {
          const a = x;

          if (no()) {
            out = a;
            break $$gen$m0;
          }
        }
        {
          out = 3;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, ['a'])).toBe('a');
    expect(runMatchStmt(output, {b: 'b'})).toBe('b');
    expect(runMatchStmt(output, 'xxx')).toBe(3);
  });

  test('nested', async () => {
    const code = `
      match (x) {
        [const a]: {
          match (a) {
            foo: {
              out = 0;
            }
            bar.a: {
              out = 1;
            }
            _: {
              out = 2;
            }
          }
        }
        _: {
          out = 3;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (Array.isArray(x) && x.length === 1) {
          const a = x[0];

          $$gen$m1: {
            if (a === foo) {
              out = 0;
              break $$gen$m1;
            }

            if (a === bar.a) {
              out = 1;
              break $$gen$m1;
            }

            {
              out = 2;
              break $$gen$m1;
            }
          }

          break $$gen$m0;
        }

        {
          out = 3;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, ['foo'])).toBe(0);
    expect(runMatchStmt(output, ['bar'])).toBe(1);
    expect(runMatchStmt(output, ['xxx'])).toBe(2);
    expect(runMatchStmt(output, 'xxx')).toBe(3);
  });

  test('variable in body does not conflict', async () => {
    const code = `
      match (x) {
        'a': {
          const a = 0;
          out = a;
        }
        _: {
          const a = 1;
          out = a;
        }
      }
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "$$gen$m0: {
        if (x === 'a') {
          const a = 0;
          out = a;
          break $$gen$m0;
        }

        {
          const a = 1;
          out = a;
          break $$gen$m0;
        }
      }"
    `);

    expect(runMatchStmt(output, 'a')).toBe(0);
    expect(runMatchStmt(output, 'xxx')).toBe(1);
  });

  test('break through to loop', async () => {
    const code = `
      while (true) {
        match (x) {
          _: {
            break;
          }
        }
      }
      out = 0;
    `;
    const output = await transform(code);
    expect(output).toMatchInlineSnapshot(`
      "while (true) {
        $$gen$m0: {
          {
            break;
            break $$gen$m0;
          }
        }
      }

      out = 0;"
    `);

    expect(runMatchStmt(output, 'xxx')).toBe(0);
  });
});
