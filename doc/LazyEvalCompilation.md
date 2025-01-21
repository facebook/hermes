---
id: lazy-eval-compilation
title: Lazy/Eval Compilation
---

This is a high level overview of how we handle compiling code while the VM is executing, using existing Environments. Used for lazy compilation (deferring full compilation of functions until they are executed) and debugger local `eval` (used by the debugger REPL). For more detailed explanations about each of the parts, look at comments in the code for each of the compiler pipeline phases.

The concepts of lazy and eval compilation have conceptual overlap, but use of one doesn't technically require use of the other. It's possible to run lazy compilation but disallow debugger local `eval` and vice versa. The code supplied to debugger local `eval` may also be lazily compiled, but it doesn't have to be.

NOTE: All of the below currently only works when executing from source, as it relies on `BCProviderFromSrc`. Lazy compilation doesn't make sense when running from an HBC file (everything's already compiled) and debugging HBC hasn't been implemented yet. We also rely on optimizations not running, to avoid deleting variables/promoting them to the stack, because we rely on everything being in Environments at runtime.

# Parsing

The parser has 3 phases when running lazy compilation:

* `PreParse`: Run the parser over the whole file, but discard the AST. The only information retained is `PreParsedData`, which has information on function locations and metadata (arrow functions, strict mode, etc.).
* `LazyParse`: Run the parser using the `PreParsedData` to skip over sufficiently large function bodies. The function bodies are stubbed out and the `isLazyFunctionBody` flag on their `BlockStatementNode` is set to `true`.
* `FullParse`: A regular parse. Produces a full AST for the input JS source.

In lazy compilation, the `PreParse` and `LazyParse` passes run before execution begins. Then `FullParse` is run when the program attempts to actually execute a lazy function.

# Data Storage

The data we need to run both lazy compilation and debugger `eval` is stored in `BCProviderFromSrc`'s `CompilationData`. It points to all the information needed to run semantic resolution, IRGen, and HBC BCGen.

`BCProviderFromSrc` uses `shared_ptr` to store the IR `Module` and `SemContext`, because it's possible for us to use the same `Module` and `SemContext` for multiple `BCProviderFromSrc`s. That happens during debugger `eval` when the `SemContext` and IR are reused (to be able to access captured variables) but the source is new so we use a new `BCProvider` (because one `BCProvider` corresponds to one `RuntimeModule`, which corresponds to a single JS source).

The fact that we persist `Module` and `SemContext` allows us to run the compiler with new code almost as if it existed the whole time.

We introduce two IR instructions: `LazyCompilationDataInst` and `EvalCompilationDataInst`. These store data needed for lazy/eval compilation respectively and ensure that relevant Variables are kept alive by storing every parent `VariableScope` as an operand (the `VariableScope`s have users, so they won't get destroyed).

We can optionally have a pointer to an IR `Function` from the `functionIR_` field of `BytecodeFunction`, which allows us to get to the relevant `CompilationDataInst`. The `functionIR_` will be destroyed when the `BytecodeFunction` is itself destroyed along with its `BytecodeModule` when the GC cleans up the corresponding `RuntimeModule`.

# Semantic Resolution

## First pass

The first time the resolver runs with lazy compilation or in preparation for allowing debugger `eval`, it stores extra data to the `FunctionInfo` by populating the `bindingTableScope` field. In lazy compilation mode, it doesn't have to enter lazy functions because they're empty stub `BlockStatementNode`s.

## Actual Resolution

`SemanticResolver::runLazy` is the entrypoint into `SemanticResolver` for lazy compilation. It uses an *existing* `SemContext` and adds new information to it. After resolving a lazy function, it cleans up binding table state unless we still need it for debugger `eval` in the future.

`SemanticResolver::runInScope` allows for local `eval`, and operates similarly, though it requires a `ProgramNode` as input. It doesn't free any preexisting `SemContext` data. It is called with a *new* `SemContext`, which is created as a child of the original `SemContext` to share the binding table but to allow it to be freed separately from the root `SemContext`.

These paths just run the `SemanticResolver` as usual after some setup, because they can restore the binding table state and internal `SemanticResolver` state based on their input.

# IRGen

## First pass

When IRGen encounters a lazy function body stub, it calls `setupLazyFunction`, which makes an IR `Function` with a `LazyCompilationDataInst` and an `UnreachableInst` terminator.

When IRGen actually compiles a function, if debugger `eval` is to be supported, it replaces the body of the IR `Function` with an `EvalCompilationDataInst`.

We also now have two lists of `Function`s in the `Module`: one ordinary list of functions to be compiled, and a `compiledFunctions_` list which only holds functions that have already passed through the whole pipeline once. This is a performance optimization to avoid reiterating through these functions needlessly every single time we rerun IRGen.

## Actual IRGen

Running IRGen on a lazy `Function` can produce many new IR `Function`s, because there may be many small functions inside the lazy function. These new functions will be some mix of lazy and eagerly compiled. They are not added to the `compiledFunctions_` list yet.

# BCGen

This part interacts heavily with the IR data structures.

## First pass

On trying to run BCGen for a lazy `Function`, assign it a `bcFunctionID`. This allows it to get a dummy `CodeBlock` created at runtime, and we can use the ID to replace the lazy bytecode with real bytecode. Then, assign it to the `functionIR_` field on the new `BytecodeFunction` so that we can find it later. Move the function to the `compiledFunctions_` list so that we no longer treat it as a member of the regular function list in the IR lowering pipeline. Generate a mostly empty function header and empty bytecode, so we have enough information to run actual compilation later. So an IR `Function` used in lazy compilation can be: not compiled yet (and containing a `LazyCompilationDataInst`), assigned a BC ID but not actually compiled (and in `compiledFunctions_`), or destroyed and replaced with a real `Function` with real instructions.

Similarly for functions with `EvalCompilationDataInst`. They're already in the `compiledFunctions_` list, but we store them in `functionIR_`.

## Actual compilation

When the function is called during lazy compilation, `generateLazyFunctions` is issued to recompile *every* `Function` in the `Module` that isn't a lazy/eval function. This assigns `bcFunctionID`s to all newly discovered functions. They are then generated as usual, and the `BCProviderFromSrc` is *modified* to point to the updated values for string tables, bytecode function tables, etc. by calling `setBytecodeModuleRefs`.

`generateForEval` works similarly but the key distinction is that `eval` creates a new `BCProvider`, so the existing one is not updated, and there's no existing `BytecodeFunction` with an ID that has to be replaced.

After either of these functions consumes the newly generated functions in the `Module`, we call `resetForMoreCompilation` which cleans upany `Function`s we don't need any more or replaces their body with just `EvalCompilationDataInst` if they're needed still for `eval`. It also deletes any now-unused `VariableScope`s.

# Running the pipeline

Lazy compilation is called via `CodeBlock::lazyCompile`, which detects whether there's no bytecode on a `CodeBlock` and calls `hbc::compileLazyFunction`. It modifies the existing `BCProviderFromSrc`, and `CodeBlock` replaces its own internal state with the new information.

Debugger `eval` runs through the standard `directEval` pipeline, which uses `evalInEnvironment`. If `evalInEnvironment` is provided a `Handle<Environment>` then it won't run the pipeline from scratch but rather go through `hbc::compileEvalModule`.
