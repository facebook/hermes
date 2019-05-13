// RUN: %hermes -O -emit-binary -target=HBC -out=%t %s && %hbcdump %t -c "disassemble;quit" | %FileCheck --match-full-lines %s

function func() {
  print("func");
}

//CHECK:   {{.*}}/sourcepath.js[3:1]
//CHECK:   {{.*}}/sourcepath.js[3:17]
