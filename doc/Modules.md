---
id: modules
title: Modules
---

In which we describe the module system and metadata input.

### Metadata Format

In order to provide a directory or zipfile to Hermes,
it must contain a `metadata.json` file at the root.

The `metadata.json` file must be a JSON object, with the following fields:

- `segments`
  - An object with string keys representing integers, which are the segments
    IDs. The IDs are generally sequential but there can be gaps (ex. having IDs
    "6" and "8" and no "7"). ID zero has special meaning, this is the "main
    segment" that gets loaded on startup (ex. when React Native starts).
  - Each value must be an array containing file paths
    of the files to place into that segment, expressed relative to the root of
    the ZIP file (ex. `subdir/foo.js`). File paths may include `./` at the start,
    but they are not required to.
  - The first element of `segments["0"]` is the first module `require`d at run time.
- `resolutionTable` (Optional)
  - An object for which the keys are relative file names
    (the same file names as in `segments`).
  - Values are objects which map from strings given to `require()`
    to their actual resolved file path relative to the directory or zip file root.
    These relative paths should be the same as in `segments`, with leading `./`.

#### Example Metadata File

```js
{
  "segments": {
    "0": [
      "./cjs-subdir-main.js",
      "cjs-subdir-2.js",
      "bar/cjs-subdir-bar.js",
      "foo/cjs-subdir-foo.js"
    ]
  },
  "resolutionTable": {
    "./cjs-subdir-main.js": {
      "foo": "./foo/cjs-subdir-foo.js"
    },
    "./foo/cjs-subdir-foo.js": {
      "bar": "./bar/cjs-subdir-bar.js"
    }
  }
}
```

### Require system

Modules can require each other using the `require` function provided as a local
variable by the runtime. The argument to `require` must be an absolute path
which root is the root of the ZIP file or input directory. For example:

```js
const Foo = require('/subdir/foo.js');

Foo.doSmth();
```

### Source maps

Each JavaScript file can optionally provide a corresponding source map, which is
the name of the source file with the suffix `.map`. For example `subdir/foo.js`
might provide a `subdir/foo.js.map` file.

### Complete design

Module mode is currently activated via passing `-commonjs` to Hermes while compiling.
Hermes can then be given a directory and a `metadata.json` file,
or a simple list of files with the first being the entry point.

First, each file is parsed just like any other JS file.
Then, a call to `hermes::wrapCJSModule` puts each file in its own "module function",
which binds `exports, require, module` as parameters.

IR generation relies on this wrapping of the AST.
Every CJS module file generates IR into a shared `hermes::Module`
(it's called `M` in `generateIRForSourcesAsCJSModules`).
Each of these generated CJS modules are now `hermes::Function *`s.
These are registered in the `cjsModules_` field using the `hermes::Module::addCJSModule` function,
which stores information in the `CJSModule` struct regarding filename, id, and IR function.
This allows lookup of the CJS modules either via `hermes::Function *` or by string literal (file path).

If the caller of the Hermes CLI passes `-static-require`,
then we attempt to resolve all `require` calls at compilation time.
This occurs in `ResolveStaticRequire.cpp`, which is able to resolve files if they were povided
by the user at invocation time and if all `require` calls only take string literals as arguments.
If every `require` call is able to be resolved, every one of these `require` calls is replaced
with a call to `HermesBuiltin_requireFast` with an ID for the CJS module,
and that function does no string work and is therefore very fast.

After all CJS modules have generated IR and all the `require`s have been resolved,
we generate one or more bytecode files from our `IR::Module M`.
This requires two special bits of logic in `hbc::generateBytecodeModule`.
- We add a mapping from a CJS module to the HBC function ID.
  This allows us to actually run `require` when JS execution demands it.
  If `require`s have been resolved, we add to the `cjsModulesStatic_` table in the HBC file,
  else we add to to the `cjsModules_` table in the HBC file (which maps from strings instead of IDs).
- To accommodate bundle splitting, we also pass a `SegmentRange` to the function.
  This allows us to only compile the functions which are needed by the CJS modules in the segment.
  We then set a `cjsModuleOffset_` field in the HBC file,
  which allows us to know how far into the complete set of CJS modules this segment is.
  **Because every HBC file contains a contiguous set of CJS modules, each of which are unique,
  it is not possible for multiple segments to contain the same CJS module.**

Having built 1 or more HBC files in the compiler, we can now execute them.
The `vm::Domain` data structure is used for keeping track of all HBC files which were compiled together.
In particular, the `Domain` owns a CJS module table.
This table is an array of CJS modules, indexed by the CJS module's ID.
If `require`s were not resolved at compile time, there's also a mapping from file path to that ID.

The first HBC file to be loaded must have a `cjsModuleOffset_` of `0`.
CJSModule `0` is the entry point, and will be executed first.
Loading of other segments is then done via a `loadSegment` call in the runtime;
the user can call it via the ConsoleHost `loadSegment` or more commonly via the Hermes API.
`loadSegment` does need a `requireContext`, which allows us to determine which `Domain` to actually
load the new CJS modules from the segment into.
This is registered into `require.context` (recall that `require` was bound as a param in all CJS modules).

Finally, we can `require`.
When `require` is called, one of two things happens:
- If we DID resolve all static `require`s, then calls to `require` were turned into Hermes builtin
  calls, so the actual value of `require`s (the parameter) is not used.
  We simply call `HermesBuiltin_requireFast` via the `CallBuiltin` instruction.
- If we DID NOT resolve all static `require`s, we call the `require` function.
  The runtime will have set up that function to also pass along the base path (to allow relative `require`),
  so `require` is now a BoundFunction (as if we'd done `require = require.bind(currentPath)`).
  Note that this is transparent to the application developer, and doesn't change the way `require`
  is actually called in JS source.
In either case, we do `runRequireCall` as implemented in `require.cpp`.
It performs any necessary checks, calls the target CJS module, and caches and reads `exports` back.

Hermes also partially supports ECMAScript modules.
Currently, the interop of ESM and CJS modules is defined simply by turning `import` into `require` calls.
This precludes full correctness of the ESM module system, which requires _live bindings_;
when a value is changed in the imported module, the local binding to that value must also be updated.
Tests for the current emitted code can be found in `test/hermes/esm/`.
