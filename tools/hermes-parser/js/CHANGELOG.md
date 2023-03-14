## Unreleased

## 0.10.0

### `hermes-parser`

- Add `SimpleTransform` API. This API is designed similar to the `SimpleTraversal` API but allows mutating the AST while traversing enabling the use of recursive transforms. The `hermes-transform` package by contrast only allows mutations at the end which is much easier to reason about but limits what can be achieved.
- Add support for `BigIntTypeAnnotation` parsing, e.g. `type T = bigint;`.
- Prevent docblock directives from being able to crash the parser.

### `hermes-transform`

- Add `stopTraversal` and `skipTraversal` API's. `stopTraversal` will abort the traversal all together where as `skipTraversal` will continue traversal but not traverse into the current node's child nodes.
- Export `MaybeDetachedNode` type and `asDetachedNode` function.
- Fix `ChainExpression` and `ObjectTypeProperty` node printing.
- Export new `parse`, `print` API's in addition to the `transform` API. The `parse` API will call the `hermes-parser` as well as prepare the AST for transform, by running comment attachment and setting docblock properties. The `print` API creates source text from the AST via `prettier`, it also does all work necessary to prepare the AST for prettier, for example reattaching the docblock as a simple comment.
- Stop the docblock from being attached to the first statement. It is now only accessible via the `docblock` property on `Program`. This ensures the docblock comment is not duplicated if the first statement is moved.
- Add `Program` node generator, e.g. `t.Program({...})`. This allows full AST's to be created.

### `hermes-eslint`

- Fix incorrect reference created by a JSX namespace name.
- Fix JSX above the React import not correctly marking React as used.
- Mark function variables as used if they have multiple defs and the declaration is exported.

### `hermes-estree`

- Improve AST Flow types.
- Add `isExpression` and `isStatement` predicate functions.

### `flow-api-translator`

- Created `flow-api-translator` package. This allows translating Flow code into either Flow definitions or TS definitions along with generating the non typed runtime code. This is designed to help library authors using Flow to more easily support the use of their code in TS or Flow codebases.

## 0.9.0

### `hermes-transform`

- Fix unnecessary newlines being added during printing.
- Fix handling of arrays with nullable elements in `shallowCloneArray` (like an array's `.elements` property).
- Add `modifyNodeInPlace` API for directly modifying a node with an implicit clone.
- Improve infra so that explicit cloning is mostly unnecessary.
    - Most of the API will automatically shallow-clone any node that's passed in.
    - The APIs still exist should you want to explicitly clone to control the depth, etc.

### `hermes-eslint`

- Add handling for FBT's `fbs` tags in scope analysis.
- Add handling for function type `this` param in scope analysis.

### `hermes-estree`

- Improve types for private brand checks (`#priv in this`)
- Add explicitly typed `parent` for nodes with small sets of known parent types
