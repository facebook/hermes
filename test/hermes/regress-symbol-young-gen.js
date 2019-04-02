// RUN: %hermes %s | %FileCheck --match-full-lines %s

// There was a bug that would cause a crash if Symbols were allocated in the
// young gen because the IdentifierTable is only marked during full GCs.

var obj = {};
var sym = Symbol(1);

// Allocate a lot of objects.
// Optimizations should be off, so this actually happens.
for (var i = 0; i < 1000; ++i) {
  obj = {};
}

// Force a GC to move the string backing the Symbol description.
gc();

print(sym.toString());
// CHECK: Symbol(1)
