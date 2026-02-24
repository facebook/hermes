# Octane

Obtained from [this fork of Octane](https://github.com/tmikov/octane) at 2bfcf3dcec9353fe836028964569eca1c3dd876c.

The files in this directory are modified by putting the related source files together so that each file is individually complete and executable. Each file is prefixed with the base.js that was originally available in the repository.

The following files are are further concatenated:

1. gbemu.js includes both gbemu-part1.js and gbemu-part2.js
2. typescript.js includes all of typescript.js, typescript-input.js and typescript-compiler.js
3. zlib.js include both zlib.js and zlib-data.js.

Furthermore, original zlib benchmark's code is entirely contained in a single string that is then passed to eval in zlib-data.js. The original content in zlib-data.js has been modified with the following transformation:

1. Get the body of the benchmark by replacing eval() with print ().
2. Apply prettier.
3. Expose a dummy function read(), which isn't used but is referred to.

Finally, at the end of each file, a call to BenchmarkSuite.RunSuites is invoked with the corresponding Print functions.
