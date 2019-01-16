// RUN: %hermes -non-strict %s

// Assertion failure because parser incorrectly recognized this as a directive
// but the AST validator did not.
"use strict"+
1;
"use strict";

x = 1;
