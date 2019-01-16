# Hermes Modules

In which we describe the module system and metadata input.

## Metadata Format

In order to provide a directory or zipfile to Hermes,
it must contain a `metadata.json` file at the root.

The `metadata.json` file must be a JSON object, with the following fields:

- `segments`
  - An object with string keys representing integers `0` to `n`,
    where `n` is the number of segments in the output.
  - Each value must be an array containing relative file names (with leading `./`)
    of the files to place into that segment.
  - The first element of `segments["0"]` is the first module `require`d at run time.
- `resolutionTable`
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
      "./cjs-subdir-2.js",
      "./bar/cjs-subdir-bar.js",
      "./foo/cjs-subdir-foo.js"
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
