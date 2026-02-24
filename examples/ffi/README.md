# FFI Examples

This directory contains examples of using the FFI to call C functions from Static Hermes.

These examples are not meant to be production quality, but rather to demonstrate
the basic usage of the FFI. They don't necessarily have robust error checking.
Only ASCII strings are supported (no UTF-8, though it would not be difficult to add).

For now these examples have only been tested on MacOS, but they should work on Linux.

## getenv.js

A very simple example reading the `PATH` environment variable and printing it.
To run it:

```shell
shermes -typed -exec getenv.js
```

## fopen.js

An example demonstrating `fopen()`, `fread()` and `fclose()`. To run use:

```shell
shermes -typed -exec fopen.js
```

## sqlite.js

Demonstrates using libsqlite3. Opens a databas, executes a query, and prints the results.

This example requires that libsqlite3 be installed on your system. On MacOS, it is already installed.
On Linux you may need to install it. You may need to supply `-L` and `-Wc,-I` flags to `shermes`.

```shell
# First create the database
./init_db.sh
# Then run the example
shermes -typed -exec sqlite.js -lsqlite3
```
