// RUN: %hermes %s | %FileCheck %s --match-full-lines

var global = Function("return this")();

function trycode(code) {
    try {
        global.eval(code);
    } catch (e) {
        print("caught:", e.message);
        return;
    }
    print("OK");
}

print("BEGIN")
//CHECK: BEGIN

trycode("var t1 = ();");
//CHECK-NEXT: caught: 1:10:invalid empty parentheses '( )'

trycode("var t1 = () + 5;");
//CHECK-NEXT: caught: 1:10:invalid empty parentheses '( )'

trycode("var t1 = (a, b, ) + 5;");
//CHECK-NEXT: caught: 1:15:expression expected after ','

trycode("var t1 = ((a)) => 1;");
//CHECK-NEXT: caught: 1:12:invalid arrow function parameter list

trycode("var t1 = (a,1) => 1;");
//CHECK-NEXT: caught: 1:13:identifier or pattern expected

trycode("var t1 = ((a),b) => 1;");
//CHECK-NEXT: caught: 1:12:parentheses are not allowed around parameters
