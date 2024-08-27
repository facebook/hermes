## 0.23.1

### `hermes-transform`
* Fixed an issue that caused an error when trying to print a `Program` with an empty `body` array and a docblock comment.

### `flow-api-translator`
* Added support for `as` expressions in parent classes like `class A extends (Foo as Bar)`.

## 0.23.0

### `hermes-estree`
* Adopted one-sided type guards in predicate functions (`implies x is T`)

### `hermes-parser`
* Added `reactRuntimeTarget` config option (defaulting to `'18'`) that when set to `'19'` no longer adds the forwardRef wrapper as React 19 treats refs on function components as regular props.

### `hermes-transform`
* Allow `context.modifyInPlace` API to modify the Program node. Previously, this relied upon `context.replaceNode` internally which meant modifications to the Program node itself were not possible.

### `flow-api-translator`
* Added support for parsing member expressions in parent classes like `class A extends Foo.Bar` and type casts such as `class A extends (Foo: Bar)`.
* Added support for the `StringPrefix` and `StringSuffix` Flow types, turning them into a TS string literal type. For example, `StringPrefix<foo>` is turned into `foo${string}` and `StringPrefix<foo, T> is turned into foo${T}`.
* Added support for inexact tuple types in TS by spreading an array of unknown (mixed) to the end of the array. This will allow for additional arbitrary elements in the tuple.

## 0.22.0

### `hermes-eslint`
* Fixed analysis of mapped types' scopes

### `prettier-plugin-hermes-parser`
* Added support for printing inexact tuple type annotations `[...]`
* Added support for printing Flow Enums with bigint members
* Added support for printing the one-sided variant of type guards `implies x is T`

## 0.21.1

### `flow-api-translator`
* Added support for translating `React.AbstractComponent` with 3 type arguments into TS. The third type argument containing render type information will be ignored.

## 0.21.0

### `hermes-parser`
* Added support for `as const` expressions
* Added support for inexact tuple type annotations `[...]`
* Added support for parsing Flow Enums with bigint members
* Added support for a one-sided variant to type guards `implies x is T`

## 0.20.1

### `hermes-eslint`
* Scope analysis support for typeof nodes with type arguments

### `babel-plugin-syntax-hermes-parser`
* Support for use in beta releases of Babel 8

## 0.20.0

### `prettier-plugin-hermes-parser`
* Added support for newer unreleased Flow syntax

## 0.19.2

### `hermes-parser`

* Bug fixes in preparation of new feature rollout

## 0.19.1

### `hermes-eslint`
* Scope analysis support for `DeclareNamespace` nodes

## 0.19.0

### `flow-api-translator`
* `$ReadOnlyMap` and `$ReadOnlySet` have been updated to expect two type arguments instead of one, aligning with the flow API.

## 0.18.2

### `hermes-parser`

* Retain more type annotations when lowering to babel AST.

## 0.18.1

### `flow-api-translator`

* Update codegen to output type guards instead of `%checks`
* handle Literal properties in flow-to-flowdef conversion

### `babel-plugin-syntax-hermes-parser`

* Do not apply the plugin for TS files

## 0.18.0

### `hermes-eslint`
* Include JSX closing elements in scope/bindings references

## 0.17.1

### `hermes-estree`
* Fix type of `typeAnnotation` property of `AsExpression`

### `hermes-parser`

### `hermes-eslint`
* Ensure types cast to in `AsExpression`s are considered as referenced

### `hermes-transform`
* Fix an issue with preserving comments of optional chaining nodes

### `flow-api-translator`
* Add support for `AsExpression`s in Flow to Flow def

### `prettier-plugin-hermes-parser`
* Patch prettier#15514, fixing edge case of `AsExpression` printing

### `babel-plugin-syntax-hermes-parser`

## 0.17.0

### `hermes-estree`

### `hermes-parser`
* Parse `AsExpressions` in Flow.
* Fix `ObjectTypeMappedTypeProperty` babel lowering to output valid code.

### `hermes-eslint`

### `hermes-transform`

### `flow-api-translator`

### `prettier-plugin-hermes-parser`
* Update internal version of prettier to version `3.0.3`.

### `babel-plugin-syntax-hermes-parser`

## 0.16.0

### `hermes-estree`
* Improve types of `DestructuringObjectPropertyWithShorthandStaticName`, `ExportNamedDeclarationWithSpecifiers`, `ObjectTypeAnnotation` and `BigIntLiteral`.

### `hermes-parser`
* Major refactor of hermes parser babel support infra to allow safer/easier transforms and more closely match babel output.
* Correct `FunctionExpression` property range.
* Correct `DeclareEnum` babel output to correctly wrap the `TypeAnnotationType` in a `TypeAnnotation`.
* Upgrade to latest `emscripten` (3.1.44 from 3.1.3).

### `hermes-eslint`

### `hermes-transform`

### `flow-api-translator`
* Correctly handle converting identifier references via `typeof identifier`.
* Support conditional types, type guards, `infer`, mapped object types, `$ReadOnlyMap` and `$ReadOnlySet`.

### `prettier-plugin-hermes-parser`
* Update internal version of prettier to version `3.0.2`.

### `babel-plugin-syntax-hermes-parser`

## 0.15.1

### `hermes-estree`

### `hermes-parser`
* Ensure `ExportNamespaceSpecifier` has location information.

### `hermes-eslint`

### `hermes-transform`
* Update `prettier-plugin-hermes-parser` peer dependency version.

### `flow-api-translator`

### `prettier-plugin-hermes-parser`

### `babel-plugin-syntax-hermes-parser`

## 0.15.0

### `hermes-estree`

### `hermes-parser`
* Correctly convert `MethodDefinition` to `ClassPrivateMethod` when `babel: true` is set and the key is `PrivateName`.

### `hermes-eslint`

### `hermes-transform`
* Ensure the prettier `print` cache key is unique even if there is more than one instance of `hermes-transform`.
* Add peer dependency on `prettier-plugin-hermes-parser`.

### `flow-api-translator`
* Remove support for now removed `$Shape` and `$Partial` Flow utilities.
* Add support for tuple labeled and spread elements.
* Support translating `React.ElementProps` from Flow to TS.
* Support `ExportAllDeclaration` in TS translation.

### `prettier-plugin-hermes-parser`

### `babel-plugin-syntax-hermes-parser`

## 0.14.0

### `hermes-estree`

### `hermes-parser`
* Added support for Flow's new type guards

### `hermes-eslint`

### `hermes-transform`

### `flow-api-translator`

### `prettier-plugin-hermes-parser`

### `babel-plugin-syntax-hermes-parser`

## 0.13.1

### `hermes-estree`

### `hermes-parser`

### `hermes-eslint`

### `hermes-transform`

### `flow-api-translator`

### `prettier-plugin-hermes-parser`
- The previous release did not build properly for release. This release fixes the issue.

### `babel-plugin-syntax-hermes-parser`
- The previous release did not build properly for release. This release fixes the issue.

## 0.13.0

### `hermes-estree`

### `hermes-parser`

- Strip TS only `tsModifiers` property from `PropertyDefinition` nodes
- Fix issue with `ConditionalTypeAnnotation` not being correctly stripped with `babel: true` mode.

### `hermes-eslint`

### `hermes-transform`

- Switched the printer to always use `prettier-plugin-hermes-parser` to ensure it can support the latest Flow syntax.

### `flow-api-translator`

- Switched the printer to always use `prettier-plugin-hermes-parser` to ensure it can support the latest Flow syntax.

### `prettier-plugin-hermes-parser`

- Patched issue in prettier that caused infinite recursion when printing array produced by `hermes-transform`.
  - See https://github.com/prettier/prettier/pull/14963 for upstream patch.

### `babel-plugin-syntax-hermes-parser`

- Created new package which creates a Hermes parser plugin for [Babel](https://babeljs.io/). This plugin switches Babel to use `hermes-parser` instead of the `@babel/parser`. Since Hermes parser uses C++ compiled to WASM it is significantly faster and provides full syntax support for Flow.

## 0.12.1

### `prettier-plugin-hermes-parser`

- Lazy load the plugin so it does not effect performance if its included but not used.
- Strip Flow parser from embedded prettier v3 bundle to improve plugin initialization time.

## 0.12.0

### `hermes-parser`

- Fix `process.exitCode` being overridden when initializing WASM.
- Add support fo parsing type parameter bounds separated by `extends` keyword in Flow.

### `hermes-transform`

- Add support for Prettier v3.

### `hermes-eslint`

- Add support for Flow AST nodes as parsed by [`hermes-parser@0.12.0`](#`hermes-parser`).

### `hermes-estree`

- Add support for Flow AST nodes as parsed by [`hermes-parser@0.12.0`](#`hermes-parser`).

### `flow-api-translator`

- Add support for Flow AST nodes as parsed by [`hermes-parser@0.12.0`](#`hermes-parser`).

### `prettier-plugin-hermes-parser`

- Created `prettier-plugin-hermes-parser` package. This is a plugin for Prettier to enable the use of Hermes parser. It supports Prettier v3 and v2 but always prints via the latest prettier v3 printing logic, to enable full support for all Hermes parser features.

## 0.11.1

### `flow-api-translator`

- Fix dependency versions.

## 0.11.0

### `hermes-parser`

- Update Flow AST representation for `typeof` types. `TypeofTypeAnnotation` now can have `QualifiedTypeofIdentifier` and `Identifiers` as arguments directly.
- Add support for parsing `TupleTypeSpreadElement` and `TupleTypeLabeledElement` in Flow.
- `DeclareVariable` in Flow now has a `kind` property, which can be `var`, `let`, or `const`.
- Add support for parsing `DeclareEnum` in Flow.
- Add support for parsing type arguments in `JSXElement` in Flow.
- Fixes typo in parsed TypeScript `TSConditionalType` nodes. The `falseTYpe` property has been renamed to `falseType`.
- Add support for parsing `keyof` types in Flow.
- Add support for parsing `ConditionalTypeAnnotation` in Flow.
- Add support for parsing `InferTypeAnnotation` in Flow.
- Add support for parsing `ObjectTypeMappedTypeProperty` in Flow.
- Add support for parsing `TypePredicate` in Flow.

### `hermes-transform`
- Add support for Flow AST nodes as parsed by [`hermes-parser@0.11.0`](#`hermes-parser`).
- Exports `makeCommentOwnLine` function which makes added comments print on their own line.
- Add support for corrected `MemberExpression` representation with `optional` property from [`hermes-estree@0.11.0`](#`hermes-estree`).

### `hermes-eslint`
- Add support for Flow AST nodes as parsed by [`hermes-parser@0.11.0`](#`hermes-parser`).

### `hermes-estree`
- Add support for Flow AST nodes as parsed by [`hermes-parser@0.11.0`](#`hermes-parser`).
- Fix `MemberExpression` to properly expose `optional` property in Flow.

### `flow-api-translator`
- Add handling for updated `typeof` type representation.
- Add handling for `DeclareVariable` with `kind` property.
- Add handling for `DeclareEnum`.
- Adds error recovery for unsupported Flow syntax in TypeScript translation. Instead of exiting, most errors will now be printed as comments in the output TypeScript code with appropriate type fallbacks.
- Add handling for Flow `typeof` imports.
- Improve scope resolution of `React` imports.
- Add `React.ElementConfig` handling.
- Add `React.Key` handling.
- Add `React.Ref` handling.
- Add `React.Component` handling.
- Add `React.ElementType` handling.
- Add `React.ChildrenArray` handling.
- Improve `React.ComponentType` handling.
- Improve `React.AbstractComponent` handling.

## 0.10.1

### `hermes-parser`

- Remove unlisted dependency on the `hermes-eslint` package as this caused module not found errors if you only installed the `hermes-parser` package.

### `flow-api-translator`

- Support global React type annotations, e.g. `React$Node`.
- Support `export default` of global type annotations.
- Fix issue with older version of prettier incorrectly printing TS value import statements.
- Add `hermes-eslint` as explict dependency.

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
