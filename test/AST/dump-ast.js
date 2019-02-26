// RUN: %hermes -dump-ast %s | %FileCheck --match-full-lines %s

function foo() {
  return Math.random();
}

switch (foo()) {
  case 3:
    print('fizz');
    break;
  case 5:
    print('buzz');
    break;
  default:
    print(foo());
}

// CHECK: File <child>
// CHECK-NEXT:   Program <2 children>
// CHECK-NEXT:     FunctionDeclaration <child> <child> <> <>
// CHECK-NEXT:       Identifier "foo" <>
// CHECK-NEXT:       BlockStatement <1 children>
// CHECK-NEXT:         ReturnStatement <child>
// CHECK-NEXT:           CallExpression <child> <>
// CHECK-NEXT:             MemberExpression <child> <child> false
// CHECK-NEXT:               Identifier "Math" <>
// CHECK-NEXT:               Identifier "random" <>
// CHECK-NEXT:     SwitchStatement <child> <3 children>
// CHECK-NEXT:       CallExpression <child> <>
// CHECK-NEXT:         Identifier "foo" <>
// CHECK-NEXT:       SwitchCase <child> <2 children>
// CHECK-NEXT:         NumericLiteral 3.000000e+00
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <1 children>
// CHECK-NEXT:             Identifier "print" <>
// CHECK-NEXT:             StringLiteral "fizz"
// CHECK-NEXT:         BreakStatement <>
// CHECK-NEXT:       SwitchCase <child> <2 children>
// CHECK-NEXT:         NumericLiteral 5.000000e+00
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <1 children>
// CHECK-NEXT:             Identifier "print" <>
// CHECK-NEXT:             StringLiteral "buzz"
// CHECK-NEXT:         BreakStatement <>
// CHECK-NEXT:       SwitchCase <> <1 children>
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <1 children>
// CHECK-NEXT:             Identifier "print" <>
// CHECK-NEXT:             CallExpression <child> <>
// CHECK-NEXT:               Identifier "foo" <>
