This directory contains tests for the synth parser.

They are formatted like this:

$name.js is the source file of some original JS that has native interactions,
$name.json is the recording of native interactions that occurred during the first execution (this is written by hand).
The test runs the parser over $name.json, and outputs $name.synth.js, which is
then executed by hermes. The source file has extra checks to ensure it gets
the right values.

Each $name.js file contains an explanation of the path that is under test.
