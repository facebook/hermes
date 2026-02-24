# widgets benchmark

Features used:
* classes (inheritance, methods, field lookups)
* array methods and `Map`

Directories:
- `original/`: the original Flow annotated benchmark.
- `single-file/`: packaged in a single file.
- `single-file/cpp/`: C++ version.
- `simple-classes/`: uses only Flow classes, no other complex types.

The `/stripped/` subdirectory in every case contains the JS without Flow type
annotations.

The `/es5/` subdirectory, where present, contains a version of the code lowered
to ES5. When lowering a single file only Babel is used, but when lowering
multuple modules we use Webpack.
