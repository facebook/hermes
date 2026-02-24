# Synthetic benchmark replay tests
Each file in this directory is a single integration test for synthetic benchmark
parsing and replay.

## Adding a new test
Let each test file have a unique identifier, call it `testName`.
Each test file defines two functions:
```cpp
const char *testNameTrace();
const char *testNameSource();
```
`testNameTrace` should return a string of JSON that represents a synthetic trace
of a hypothetical native interaction with JS. This interaction can do things
like define native functions that are called from JS, or make native objects, or
simply call functions in JS.
NOTE: The object ids used in the trace string is only meant to be used as a
unique identifier, it doesn't have any special meaning. Most tests start at 10
just because of convention. You can use object ids like 3, 400000, or 1345, as
long as they are unique integers.
NOTE: We use these test traces for two tests each: one uses Strings to
represent property names, the other PropNameIDs.  To make this
possible, we have a "fake" CreatePropNameRecord type.  The string
substitution in Driver.cpp (where this convention is further
explained) leaves changes CreatePropNameRecord into either
CreateString or CreatePropNameID, depending on which test variant
we're doing.  Note that there are CreateStringRecords for strings that
are not used as property names; these are left unchanged.

`testNameSource` is some JS source code that interacts with the native
environment created by the trace. It should throw exceptions if the native code
does not behave as expected, this will cause the test to fail.

Make sure `testName` is added to the macro list in `TestFunctions.h`.
Then `Driver.cpp` will call these new functions for you!

## Debugging a broken test
Use gdb, and put a breakpoint on `TraceInterpreter::execFunction`. Then step
through each event that's executed by the interpreter. Currently it's not
particularly easy to also debug the JS, that would require a hybrid debugger.
