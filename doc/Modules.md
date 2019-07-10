---
id: modules
title: Modules
---


# Hermes Modules

In which we describe the module system and metadata input.

## Metadata Format

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

### Example Metadata File

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

## Require system

Modules can require each other using the `require` function provided as a local
variable by the runtime. The argument to `require` must be an absolute path
which root is the root of the ZIP file or input directory. For example:

```js
const Foo = require('/subdir/foo.js');

Foo.doSmth();
```

## Source maps

Each JavaScript file can optionally provide a corresponding source map, which is
the name of the source file with the suffix `.map`. For example `subdir/foo.js`
might provide a `subdir/foo.js.map` file.
