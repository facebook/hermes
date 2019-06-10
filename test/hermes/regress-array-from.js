// RUN: %hermes -target=HBC %s | %FileCheck --match-full-lines %s

try {
  Array.from("1", RegExp);
} catch (e) {
  print (e);
}
// CHECK: SyntaxError: Invalid flags passed to RegExp
