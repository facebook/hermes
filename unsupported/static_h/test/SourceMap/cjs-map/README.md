# CommonJS Segment SourceMap Tests

To update these tests, first update the `.js` file that you want to update.
Then, run `uglify` (can be installed via `npm install -g uglify-js`)
For example:
```
uglifyjs cjs-subdir-main.js --source-map -o cjs-subdir-main.min.js
```
This will minify the files and place a source map which can be used for testing
in the directory.

Note that some files have manually crafted source maps, e.g.
`cjs-subdir-partially-mapped.js`.
