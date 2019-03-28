// RUN: %hermes -dump-ast --pretty-json %s | %FileCheck %s --match-full-lines
// XFAIL: *

for(a1 of b);
for(var a2 of b);
for(let a3 of b);
for(const a4 of b);

let a5, a6;
const a7 = 1;
