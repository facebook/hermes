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
// CHECK-NEXT:   Program <children>
// CHECK-NEXT:     FunctionDeclaration <child> <child> <children> <child>
// CHECK-NEXT:       Identifier "foo" <child>
// CHECK-NEXT:       BlockStatement <children>
// CHECK-NEXT:         ReturnStatement <child>
// CHECK-NEXT:           CallExpression <child> <children>
// CHECK-NEXT:             MemberExpression <child> <child> 0
// CHECK-NEXT:               Identifier "Math" <child>
// CHECK-NEXT:               Identifier "random" <child>
// CHECK-NEXT:     SwitchStatement <child> <children>
// CHECK-NEXT:       CallExpression <child> <children>
// CHECK-NEXT:         Identifier "foo" <child>
// CHECK-NEXT:       SwitchCase <child> <children>
// CHECK-NEXT:         NumericLiteral 3.000000e+00
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <children>
// CHECK-NEXT:             Identifier "print" <child>
// CHECK-NEXT:             StringLiteral "fizz"
// CHECK-NEXT:         BreakStatement <child>
// CHECK-NEXT:       SwitchCase <child> <children>
// CHECK-NEXT:         NumericLiteral 5.000000e+00
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <children>
// CHECK-NEXT:             Identifier "print" <child>
// CHECK-NEXT:             StringLiteral "buzz"
// CHECK-NEXT:         BreakStatement <child>
// CHECK-NEXT:       SwitchCase <child> <children>
// CHECK-NEXT:         ExpressionStatement <child>
// CHECK-NEXT:           CallExpression <child> <children>
// CHECK-NEXT:             Identifier "print" <child>
// CHECK-NEXT:             CallExpression <child> <children>
// CHECK-NEXT:               Identifier "foo" <child>
