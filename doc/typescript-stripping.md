# TypeScript Type Stripping

Hermes supports stripping erasable TypeScript syntax from source files via the
`--transform-ts` flag, similar to Node.js's `--strip-types`. This allows
running TypeScript files directly without a separate transpilation step, as long
as the code uses only erasable type syntax.

## Usage

```bash
# Run a TypeScript file directly
hermes --transform-ts file.ts

# Compile to bytecode
hermesc --transform-ts -emit-binary -out file.hbc file.ts

# With shermes
shermes --transform-ts file.ts
```

The `--transform-ts` flag implies `--parse-ts` (TypeScript parsing is enabled
automatically).

## Supported Erasable Syntax

The following TypeScript constructs are erased (removed or unwrapped) during
the AST transformation pass:

- **Type annotations** on variables, parameters, and return types
  (`let x: number`, `function f(a: string): void`)
- **Type aliases** (`type Foo = string`)
- **Interfaces** (`interface Bar { ... }`)
- **Generic type parameters** (`function f<T>(x: T): T`, `class C<T>`)
- **Type assertions** (`x as T`, `<T>x`)
- **Type-only imports** (`import type { Foo } from "bar"`)
- **Type-only exports** (`export type { Foo }`)
- **`implements` clauses** on classes (`class Foo implements Bar`)
- **Type arguments** on call/new expressions (`foo<T>(x)`, `new C<T>()`)
- **TS modifiers** on class properties (`readonly`, access modifiers when used
  only as type-level annotations on properties)
- **`superTypeArguments`** on classes (`class Foo extends Bar<T>`)

## Unsupported Non-Erasable Syntax

The following TypeScript constructs have runtime semantics and cannot be
simply erased. Using them with `--transform-ts` produces an error:

- **Enums** (`enum Color { Red, Green, Blue }`)
- **Namespaces / modules** (`namespace Foo { ... }`)
- **Parameter properties** (`constructor(public x: number)`)

## Restrictions

- `--transform-ts` is incompatible with `--typed` (typed mode). Using both
  together produces an error.

## How It Works

The stripping pass runs as an AST transformation before semantic analysis,
in the same pipeline slot as the async generator transform. It operates by:

1. Walking the AST using the recursive visitor framework.
2. Nulling out type annotation fields on nodes (identifiers, functions,
   classes, patterns, call expressions).
3. Removing type-only declaration nodes from statement lists (type aliases,
   interfaces, type-only imports/exports).
4. Unwrapping type assertion expressions (`x as T` becomes `x`).
5. Reporting errors for non-erasable constructs.

Deep TS type nodes (e.g., `TSAnyKeyword`, `TSFunctionType`, `TSUnionType`)
only appear as descendants of type annotation fields. Nulling those fields at
the parent level makes them unreachable, so no explicit handlers are needed.

## Comparison with Node.js

| Feature | Hermes `--transform-ts` | Node.js `--strip-types` |
|---|---|---|
| Type annotations | Stripped | Stripped |
| Interfaces / type aliases | Stripped | Stripped |
| Enums | Error | Error (unless `--experimental-transform-types`) |
| Namespaces | Error | Error (unless `--experimental-transform-types`) |
| Parameter properties | Error | Error (unless `--experimental-transform-types`) |
| `import type` | Stripped | Stripped |
| Per-specifier `import { type X }` | Not yet supported | Stripped |

## Future Work

- Per-specifier type import stripping (`import { type X, Y } from "bar"`)
- Enum lowering (transform enums to JavaScript objects)
- Namespace lowering (transform namespaces to IIFEs)
