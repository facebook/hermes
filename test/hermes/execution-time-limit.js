// RUN: (! %hermes -emit-async-break-check -time-limit=1000 %s 2>&1 ) | %FileCheck --match-full-lines %s

function entryPoint() {
  helper();
}

function helper() {
  var i = 0;
  while (true) {
    ++i;
  }
}

entryPoint();

//CHECK: Error: Javascript execution has timed out.
//CHECK: at helper ({{.*/execution-time-limit.js}}:10:5)
//CHECK-NEXT: at entryPoint ({{.*/execution-time-limit.js}}:4:9)
//CHECK-NEXT: at global ({{.*/execution-time-limit.js}}:14:11)
