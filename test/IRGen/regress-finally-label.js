// RUN: %hermes -dump-ir %s
//
// Test for a regression where we were asserting because bodies of fionally
// statements are visited more than once.

function foo(a, b) {
    try {
        if (foo())
            return;
    } finally {
        while (a < 10) {
            b[++a] = 0;
        }
    }
}


