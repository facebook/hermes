# Merge Input Source Map with debug info Tests

To update these tests, first update the `.js` file that you want to update.
Note: make sure each function occupies a separate line so that line numbers
in the symbolicated traces will match the original trace.
Then, run `uglify` (can be installed via `npm install -g uglify-js`)
For example:
```
uglifyjs throw.js --source-map -o throw.min.js
```
This will minify the files and place a source map which can be used for testing
in the directory.
