// RUN: %hdb < %s.debug %s | %FileCheck --match-full-lines %s
// REQUIRES: debugger

function funky() {
  print("funky");
}

debugger;
var x = [1,2,3];
x.forEach(function(i) {
  if (i == 2) {
    funky();
  }
  return 'myresult';
});
print("finish");

//CHECK: Break on 'debugger' statement in global: {{.*}}:8:1
//CHECK: Stepped to global: {{.*}}:9:9
//CHECK: Stepped to global: {{.*}}:10:1
//CHECK: Stepped to anonymous: {{.*}}:11:7
//CHECK: Stepped to anonymous: {{.*}}:11:3
//CHECK: Stepped to anonymous: {{.*}}:14:3
//CHECK: Stepped to anonymous: {{.*}}:10:23
//CHECK: Stepped to anonymous: {{.*}}:11:7
//CHECK: Stepped to anonymous: {{.*}}:12:5
//CHECK: Stepped to funky: {{.*}}:5:3
//CHECK: funky
//CHECK: Stepped to anonymous: {{.*}}:14:3
//CHECK: Stepped to anonymous: {{.*}}:10:23
//CHECK: Stepped to anonymous: {{.*}}:11:7
//CHECK: Stepped to global: {{.*}}:10:10
//CHECK: Stepped to global: {{.*}}:16:1
//CHECK: finish
