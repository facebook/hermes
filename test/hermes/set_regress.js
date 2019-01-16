// RUN: %hermes -O -gc-sanitize-handles=0 -gc-max-heap=1K %s | %FileCheck --match-full-lines %s

function testCompact() {
  print("testCompact");
//CHECK-LABEL: testCompact
  var s = new Set();
  for (var i = 0; i < 100000; ++i) {
    s.add(i);
    s.delete(i);
  }
  print(s.size);
//CHECK-NEXT: 0
}

testCompact();
