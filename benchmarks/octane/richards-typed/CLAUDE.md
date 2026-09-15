# Richards Benchmark — Static Hermes Typed Performance Project

## Project Goal

Improve performance of the Richards benchmark by converting it to use Static Hermes sound typing.

- `richards.js` — original untyped version (do not modify)
- `richards-typed.js` — typed version targeting Static Hermes `--typed` mode

`richards.js` is taken from V8's Octane benchmark suite. The only
modification versus upstream is the harness at the bottom: instead of
running through the Octane `BenchmarkSuite` driver and reporting a
score, it loops the workload `COUNT` times, measures wall time with
`Date.now()`, and prints elapsed milliseconds. `richards-typed.js`
uses the same time-printing harness, so the two files are directly
comparable.

## Tools

The recipes below assume `$SH_BUILD` points to a release build of Static
Hermes. Set it once per shell, e.g.:

```bash
export SH_BUILD=$(pwd)/cmake-build-release   # default when configured per
                                             # the project root CLAUDE.md
```

### hermes (interpreter / JIT)

Binary: `$SH_BUILD/bin/hermes`

```bash
# Untyped (interpreter only)
hermes richards.js

# Typed (interpreter only)
hermes --typed richards-typed.js

# With JIT (add -Xjit)
hermes -Xjit richards.js
hermes --typed -Xjit richards-typed.js
```

### shermes (AOT native compiler)

Binary: `$SH_BUILD/bin/shermes`

Works like a C compiler: produces `a.out` by default, supports `-o`, `-c`, `-S`.

```bash
# Untyped
shermes -o richards-untyped richards.js

# Typed
shermes --typed -o richards-typed richards-typed.js
```

#### Static linking (`--static-link`)

`--static-link` avoids shared library indirection for better performance. There is currently a bug where `shermes` doesn't pass `asmjit` and `boost_context` libraries. Work around by passing `-L` and `-l` manually:

```bash
shermes --typed --static-link \
  -L$SH_BUILD/external/asmjit/asmjit -lasmjit \
  -L$SH_BUILD/external/boost/boost_1_86_0/libs/context -lboost_context \
  -o richards-typed-static richards-typed.js
```

### shermes compilation pipeline

`shermes -v` shows the commands executed. The pipeline is:
1. JS -> C (emitted to a temp file)
2. System C compiler (`/usr/bin/cc`) with `-O3` to produce the native binary
3. Links against `-lhermesvm` and `-lshermes_console`

`shermes` accepts `-L` and `-l` flags which are passed through to the C compiler. You can also use `-c` (compile to .o), `-S` (emit C source), or `-emit-asm` to inspect intermediate stages.

## Static Hermes Typing Rules

Static Hermes uses a subset of Flow (and TypeScript) with **sound typing** — types are enforced like C++, not just hints.

### Type annotations

Add annotations to all function/method parameters, return types, and class fields.

Local variable annotations can often be omitted when the compiler can infer the
type from the initializer. Inference works for:
- `var x = new Foo()` — inferred as `Foo`
- `var x = this.field` — inferred from the field's declared type
- `var x = someTypedExpr` — when the RHS has a known type

Annotations are still **required** for local variables when:
- The variable is **uninitialized** (`var v;` infers as `any`)
- The initializer is **nullable** but the variable should be non-null after a
  narrowing check (e.g., `var next = queue` after `if (queue === null) return`
  — inferred as `?T` not `T`)
- The initializer returns `any` (e.g., `Date.now()`)

Use `-dump-sema` and `-dump-ir` to verify that removing an annotation doesn't
change the inferred type.

```js
var x: number = 0;
function foo(a: number, b: string): boolean { ... }
```

### Arrays

Use `T[]` syntax for array types, not `Array<T>`:

```js
var arr: number[] = [0, 0, 0, 0];
// NOT: var arr: Array<number> = new Array<number>(4);
```

### Classes

Static Hermes requires ES6 classes, not constructor functions with `.prototype`. All fields must be declared with types in the class body:

```js
class Foo {
  x: number;
  next: Foo | null;

  constructor(x: number) {
    this.x = x;
    this.next = null;
  }

  bar(y: number): number {
    return this.x + y;
  }
}
```

### Inheritance

Use `extends` and `super()` for class hierarchies (virtual dispatch works):

```js
class Base {
  run(packet: Packet | null): TaskControlBlock | null {
    throw new Error("abstract");
  }
}

class Derived extends Base {
  run(packet: Packet | null): TaskControlBlock | null {
    // override implementation
  }
}
```

### Nullability

Nullable types use `T | null` or the shorthand `?T`:

```js
var x: Packet | null = null;
var x: ?Packet = null;        // equivalent, preferred
```

The type checker does **not** narrow `this.field` across statements — use local variables for narrowing:

```js
// WON'T work — this.field is not narrowed after the while condition
while (this.currentTcb !== null) {
  this.currentTcb.doSomething(); // type error: might be null
}

// DO THIS — local variable narrowing works
var tcb = this.list;
while (tcb !== null) {
  tcb.doSomething(); // OK: tcb narrowed to non-null
}
```

### Strict equality

Use `===` and `!==` instead of `==` and `!=`.

### No implicit string coercion

`string + number` is a type error. Convert explicitly:

```js
// Error:
"count = " + someNumber

// OK:
"count = " + String(someNumber)
```

### Known warnings

`console` is not declared in the type system — using `console.log()` produces a warning but works at runtime.

## Performance Baselines (400 iterations)

| Mode                        | Untyped | Typed |
|-----------------------------|---------|-------|
| Interpreter                 | 436ms   | 193ms |
| JIT (`-Xjit`)              | 207ms   | 83ms  |
| Native (shermes)            | 266ms   | 84ms  |
| Native static (`--static-link`) | 254ms | 81ms  |
