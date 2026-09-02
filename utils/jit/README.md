# JIT dump tools

Tools for checking that a change to the arm64 JIT emitter did not change the
code it emits.

- `jit-dump.sh` — capture a canonicalized dump of all JIT-emitted code from
  one `hermes` binary.
- `jit-diff.sh` — capture from two binaries and compare, reporting comment
  changes separately from instruction changes.

## Why not just diff `-Xdump-jitcode` output?

Because two runs of an **unchanged** binary produce different dumps.

`Emitter::loadBits64InGp` asks `isCheapConst()` whether a 64-bit value has at
most two non-zero 16-bit halfwords. If it does, the value is materialized
inline with `mov`/`movk`; if not, it is spilled to the read-only data section
and loaded with `ldr`. JIT-emitted code bakes in runtime pointers, and ASLR
moves those every run. So the emitter genuinely selects *different
instructions* from one run to the next, and every subsequent RO-data offset
shifts along with them.

You can see this for yourself:

```sh
utils/jit/jit-dump.sh --raw -o /tmp/a.txt build/bin/hermes
utils/jit/jit-dump.sh --raw -o /tmp/b.txt build/bin/hermes
diff -u /tmp/a.txt /tmp/b.txt | grep -cE '^[-+]'
```

That reports roughly 3,500 changed lines out of about 90,000 — from the same
binary, twice, with no code change at all. Disabling ASLR (running under lldb)
cuts it to a few dozen but does not eliminate it, because the contiguous
heap's base address comes from a separate randomized mmap.

`jit-dump.sh` therefore collapses constant materialization to a `CONST` token,
drops the RO-data contents, and normalizes any remaining wide hex literal to
`ADDR`. What survives is compared verbatim: instructions, registers, branches,
labels, ordering, and the emitter's own comments. In practice that is about
92% of the dump, and the same command without `--raw` reports zero
differences.

## Usage

Both tools take **binaries**, not a build directory, so they work regardless
of how you built.

```sh
# Build the "before" binary and stash it somewhere.
cmake --build build --target hermes
cp build/bin/hermes /tmp/hermes-before

# Make your change, rebuild, and compare.
utils/jit/jit-diff.sh /tmp/hermes-before build/bin/hermes
```

Typical output for a refactor that should be behaviour-preserving:

```
jit-diff: dumps are identical
```

and for one that touches only `Emitter::comment()` strings:

```
jit-diff: 20 changed lines (+10 / -10)
  comment lines:     20
  instruction lines: 0
RESULT: comments only, no instruction changed
```

`--comments-ok` makes that second case exit 0, which is useful when a change
deliberately edits a comment string.

If the "before" binary is gone but you kept its capture, compare the dumps
directly:

```sh
utils/jit/jit-diff.sh --dumps /tmp/before.txt /tmp/after.txt
```

To capture a single dump, for example to read one function by hand:

```sh
utils/jit/jit-dump.sh --raw -c test/jit/binops.js build/bin/hermes | less
```

`--raw` skips canonicalization. Use it for reading, never for comparing.

## Corpus

By default the corpus is every `test/jit/*.js` plus three typed tests that
exercise paths the JIT tests do not:

- `test/shermes/array-typed.js`
- `test/hermes/flow/array-for-of.js`
- `test/hermes/flow/nbody.js`

Each `test/jit` file is run typed if and only if its own lit `RUN:` line
passes `-typed`. That matters more than it sounds: running a typed test
untyped does not just lose coverage, it fails to compile, so the file
contributes a syntax error to the dump instead of any JIT code.

Override with `-c FILE` (untyped) and `-t FILE` (compiled with `-typed`),
both repeatable. Supplying either replaces the whole default corpus.

A corpus file that exits nonzero aborts the capture. Its diagnostics would
otherwise land in the dump, where they are indistinguishable from emitted
code and will happily compare equal between two runs.

## Limitations

**Canonicalized lines are not compared.** A change that alters *which*
instruction form is chosen for a constant is invisible to these tools, because
that is exactly what gets collapsed to `CONST`. If your change touches
`isCheapConst`, `loadBits64InGp`, `uint64Const`, or RO-data layout, these tools
cannot verify it.

**Coverage is only as good as the corpus.** An emitter path no test reaches
will not appear in the dump. Notably, every function in the default corpus has
well under 4 KB of bytecode, so paths gated on large bytecode offsets — such as
the two-instruction `add` in `Emitter::getBytecodeIP` — are never exercised.
Add a targeted file with `-c` when working on one of those.

**A clean diff is not a correctness proof.** It shows the emitted code did not
change, which is the right check for a refactor. It says nothing about whether
the code was correct to begin with, and nothing about the emitter's own runtime
behaviour. Run the lit suite too.

**The JIT must be enabled** in the binaries under test
(`-DHERMESVM_ALLOW_JIT=1`); the tools force it on per-run with `-Xjit=force
-Xjit-threshold=1`, but cannot enable a compile-time-disabled JIT. If a binary
was built without it, `jit-dump.sh` fails with a message saying so rather than
silently producing an empty dump.
