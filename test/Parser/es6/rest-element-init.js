// RUN: %hermes -O %s | %FileCheck --match-full-lines %s

function trycode(code) {
    try {
        (0,eval)(code);
    } catch (e) {
        print("caught:", e.message);
        return;
    }
    print("OK");
}

print("BEGIN")
//CHECK: BEGIN

trycode("function foo(a, ...b = 10) {}");
//CHECK-NEXT: caught: 1:20:rest elemenent may not have a default initializer

trycode("var [a, ...b = []] = []");
//CHECK-NEXT: caught: 1:12:rest elemenent may not have a default initializer

trycode("[a, ...b = []] = []");
//CHECK-NEXT: caught: 1:8:invalid assignment left-hand side

trycode("var {a, ...b = {}} = {}");
//CHECK-NEXT: caught: 1:14:'}' expected at end of object binding pattern '{...'

trycode("({a, ...b = {}} = {})");
//CHECK-NEXT: caught: 1:9:invalid assignment left-hand side
