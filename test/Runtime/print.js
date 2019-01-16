// RUN: %hermes -hermes-parser -dump-ra %s


// Make sure that we are not crashing on this one:
print("hello world")
parseInt(10)

