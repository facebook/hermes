# Hermes Compilation and Runtime Modes

*November 2, 2025 · tmikov*

Hermes has the following compilation "modes":

- AOT compilation of typed or untyped JS to bytecode
- AOT compilation of typed or untyped JS to native code

Plus the following runtime modes:

- Native execution
- Bytecode interpreter
- Lazy compilation (source to bytecode) of each function the first time it is called
- Baseline JIT (bytecode to native) of frequently executed functions

All of these modes can be mixed and exist in the same runtime simultaneously. An app could use all of these:

1. Some code compiled to native
2. Some code compiled to bytecode (to save space)
3. Some source (perhaps generated at runtime)

If the JIT is enabled, parts of (2) and (3) will be JIT-ed to native if they become hot enough.
