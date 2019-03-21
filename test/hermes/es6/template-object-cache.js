// RUN: %hermes -Xflow-parser -O %s | %FileCheck --match-full-lines %s
// REQUIRES: flowparser

var previousObj = null;
function checkTemplateObject(templateObj) {
  print(templateObj instanceof Object, templateObj == previousObj);
  previousObj = templateObj;
}

checkTemplateObject``;
// CHECK: true false
checkTemplateObject`hello${1 + 1}world`;
// CHECK: true false
checkTemplateObject`hello${1 + 2}world`;
// CHECK: true true
checkTemplateObject`hello${1 + 1}world`;
// CHECK: true true
checkTemplateObject`hellowo${6}rld`;
// CHECK: true false
checkTemplateObject`hello\n${2}`;
// CHECK: true false
checkTemplateObject`hello\n${2}`;
// CHECK: true true
checkTemplateObject`hello\n${1}`;
// CHECK: true true
