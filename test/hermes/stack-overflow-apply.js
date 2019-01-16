//RUN: %hermes -O -target=HBC %s | %FileCheck --match-full-lines %s

function func() {
    print(arguments.length);
}

try {
    var a = []
    a[8*1024*1024] = 100;
    func.apply(null, a);
} catch(e) {
    print("caught:", e.name, e.message);
}
//CHECK: caught: RangeError {{.*}}
