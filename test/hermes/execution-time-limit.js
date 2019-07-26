// RUN: (! %hermes -emit-async-break-check -time-limit=1000 %s 2>&1 ) | %FileCheck --match-full-lines %s
// REQUIRES: timelimit

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

//CHECK: Javascript execution has timeout.
//CHECK: helper: {{.*/execution-time-limit.js}}:11:5
//CHECK-NEXT: entryPoint: {{.*/execution-time-limit.js}}:5:9
//CHECK-NEXT: global: {{.*/execution-time-limit.js}}:15:11
