// Verify that hermes and hermesc produce the same output.
// RUN: diff <(%hermes -target=HBC -dump-bytecode %s) <(%hermesc -target=HBC -dump-bytecode %s)
print(42);
