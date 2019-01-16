// RUN: %hermes -dump-ir %s
// Make sure we generate code that successfully verifies

function foo(n, r) {
    switch (n && r) {}
}
