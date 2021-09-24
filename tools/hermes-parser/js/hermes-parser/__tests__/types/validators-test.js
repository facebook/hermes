/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

import {types as t, parse} from 'hermes-parser';

describe('validators', function () {
  describe('isCompatTag', function () {
    it('should handle lowercase tag names', function () {
      expect(t.react.isCompatTag('div')).toBe(true);
      expect(t.react.isCompatTag('a')).toBe(true); // one letter
      expect(t.react.isCompatTag('h3')).toBe(true); // letters and numbers
    });

    it('should handle custom element tag names', function () {
      expect(t.react.isCompatTag('plastic-button')).toBe(true); // ascii letters
      expect(t.react.isCompatTag('math-α')).toBe(true); // non-latin chars
      expect(t.react.isCompatTag('img-viewer2')).toBe(true); // numbers
      expect(t.react.isCompatTag('emotion-😍')).toBe(true); // emoji
    });

    it("accepts trailing dash '-' in custom element tag names", function () {
      expect(t.react.isCompatTag('div-')).toBe(true);
      expect(t.react.isCompatTag('a-')).toBe(true);
      expect(t.react.isCompatTag('h3-')).toBe(true);
    });

    it('rejects empty or null tag names', function () {
      expect(t.react.isCompatTag(null)).toBe(false);
      expect(t.react.isCompatTag()).toBe(false);
      expect(t.react.isCompatTag(undefined)).toBe(false);
      expect(t.react.isCompatTag('')).toBe(false);
    });

    it('rejects tag names starting with an uppercase letter', function () {
      expect(t.react.isCompatTag('Div')).toBe(false);
      expect(t.react.isCompatTag('A')).toBe(false);
      expect(t.react.isCompatTag('H3')).toBe(false);
    });

    it('rejects all uppercase tag names', function () {
      expect(t.react.isCompatTag('DIV')).toBe(false);
      expect(t.react.isCompatTag('A')).toBe(false);
      expect(t.react.isCompatTag('H3')).toBe(false);
    });

    it("rejects leading dash '-'", function () {
      expect(t.react.isCompatTag('-div')).toBe(false);
      expect(t.react.isCompatTag('-a')).toBe(false);
      expect(t.react.isCompatTag('-h3')).toBe(false);
    });
  });

  describe('isReferenced', function () {
    it('returns false if node is a key of ObjectTypeProperty', function () {
      const node = t.Identifier('a');
      const parent = t.ObjectTypeProperty(node, t.NumberTypeAnnotation());

      expect(t.isReferenced(node, parent)).toBe(false);
    });

    it('returns true if node is a value of ObjectTypeProperty', function () {
      const node = t.Identifier('a');
      const parent = t.ObjectTypeProperty(
        t.Identifier('someKey'),
        t.GenericTypeAnnotation(node),
      );

      expect(t.isReferenced(node, parent)).toBe(true);
    });

    describe('ObjectProperty', () => {
      it('returns true if node is a value of ObjectProperty of an expression', function () {
        const node = t.Identifier('a');
        const parent = t.ObjectProperty(t.Identifier('key'), node);
        const grandparent = t.ObjectExpression([parent]);

        expect(t.isReferenced(node, parent, grandparent)).toBe(true);
      });

      it('returns false if node is a value of ObjectProperty of a pattern', function () {
        const node = t.Identifier('a');
        const parent = t.ObjectProperty(t.Identifier('key'), node);
        const grandparent = t.ObjectPattern([parent]);

        expect(t.isReferenced(node, parent, grandparent)).toBe(false);
      });

      it('returns true if node is computed property key of an expression', function () {
        const node = t.Identifier('a');
        const parent = t.ObjectProperty(node, t.Identifier('value'), true);
        const grandparent = t.ObjectExpression([parent]);

        expect(t.isReferenced(node, parent, grandparent)).toBe(true);
      });

      it('returns true if node is computed property key of a pattern', function () {
        const node = t.Identifier('a');
        const parent = t.ObjectProperty(node, t.Identifier('value'), true);
        const grandparent = t.ObjectPattern([parent]);

        expect(t.isReferenced(node, parent, grandparent)).toBe(true);
      });
    });

    describe('ObjectMethod', function () {
      it('returns false if node is method key', function () {
        const node = t.Identifier('A');
        const parent = t.ObjectMethod('method', node, [], t.BlockStatement([]));

        expect(t.isReferenced(node, parent)).toBe(false);
      });

      it('returns true if node is computed method key', function () {
        const node = t.Identifier('A');
        const parent = t.ObjectMethod(
          'method',
          node,
          [],
          t.BlockStatement([]),
          true,
        );

        expect(t.isReferenced(node, parent)).toBe(true);
      });

      it('returns false if node is method param', function () {
        const node = t.Identifier('A');
        const parent = t.ObjectMethod(
          'method',
          t.Identifier('foo'),
          [node],
          t.BlockStatement([]),
        );

        expect(t.isReferenced(node, parent)).toBe(false);
      });
    });

    describe('ClassMethod', function () {
      it('returns false if node is method key', function () {
        const node = t.Identifier('A');
        const parent = t.ClassMethod('method', node, [], t.BlockStatement([]));

        expect(t.isReferenced(node, parent)).toBe(false);
      });

      it('returns true if node is computed method key', function () {
        const node = t.Identifier('A');
        const parent = t.ClassMethod(
          'method',
          node,
          [],
          t.BlockStatement([]),
          true,
        );

        expect(t.isReferenced(node, parent)).toBe(true);
      });

      it('returns false if node is method param', function () {
        const node = t.Identifier('A');
        const parent = t.ClassMethod(
          'method',
          t.Identifier('foo'),
          [node],
          t.BlockStatement([]),
        );

        expect(t.isReferenced(node, parent)).toBe(false);
      });
    });

    describe('exports', function () {
      it('returns false for re-exports', function () {
        const node = t.Identifier('foo');
        const parent = t.ExportSpecifier(t.Identifier('bar'), node);
        const grandparent = t.ExportNamedDeclaration(
          null,
          [parent],
          t.StringLiteral('library'),
        );

        expect(t.isReferenced(node, parent, grandparent)).toBe(false);
      });

      it('returns true for local exports', function () {
        const node = t.Identifier('foo');
        const parent = t.ExportSpecifier(t.Identifier('bar'), node);
        const grandparent = t.ExportNamedDeclaration(null, [parent]);

        expect(t.isReferenced(node, parent, grandparent)).toBe(true);
      });
    });

    describe('import attributes', function () {
      it('returns false for import attributes', function () {
        const node = t.Identifier('foo');
        const parent = t.ImportAttribute(node, t.StringLiteral('bar'));

        expect(t.isReferenced(node, parent)).toBe(false);
      });
    });
  });

  describe('isBinding', function () {
    it('returns false if node id a value of ObjectProperty of an expression', function () {
      const node = t.Identifier('a');
      const parent = t.ObjectProperty(t.Identifier('key'), node);
      const grandparent = t.ObjectExpression([parent]);

      expect(t.isBinding(node, parent, grandparent)).toBe(false);
    });

    it('returns true if node id a value of ObjectProperty of a pattern', function () {
      const node = t.Identifier('a');
      const parent = t.ObjectProperty(t.Identifier('key'), node);
      const grandparent = t.ObjectPattern([parent]);

      expect(t.isBinding(node, parent, grandparent)).toBe(true);
    });
  });

  describe('isType', function () {
    it('returns true if nodeType equals targetType', function () {
      expect(t.isType('Identifier', 'Identifier')).toBe(true);
    });
    it('returns false if targetType is a primary node type', function () {
      expect(t.isType('Expression', 'ArrayExpression')).toBe(false);
    });
    it('returns true if targetType is an alias of nodeType', function () {
      expect(t.isType('ArrayExpression', 'Expression')).toBe(true);
    });
    it('returns false if nodeType and targetType are unrelated', function () {
      expect(t.isType('ArrayExpression', 'ClassBody')).toBe(false);
    });
    it('returns false if nodeType is undefined', function () {
      expect(t.isType(undefined, 'Expression')).toBe(false);
    });
  });
});
