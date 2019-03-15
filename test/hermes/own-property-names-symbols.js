// RUN: %hermes -non-strict -O -target=HBC %s | %FileCheck --match-full-lines %s

function getOwnPropertySymbolsAsStrings(obj) {
  return Object.getOwnPropertySymbols(obj).map(function (sym) {
    return sym.toString();
  });
}

bar = Symbol.for("bar");
qux = Symbol.for("qux");

proto = { foo: 0 };
proto[bar] = 1;

obj = { __proto__: proto, baz: 2};
obj[qux] = 3;

print(Object.getOwnPropertyNames(obj));
//CHECK: baz

print(getOwnPropertySymbolsAsStrings(obj));
//CHECK: Symbol(qux)

// Strings have internal property slots which shouldn't be treated as property
// names.
print(Object.getOwnPropertyNames(""));
//Check: length
