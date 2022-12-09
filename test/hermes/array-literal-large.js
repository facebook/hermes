/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */


// RUN: %hermes %s > %t.js && %hermes -O %t.js -gc-sanitize-handles=0
// REQUIRES: !slow_debug

var numElems = 0;
var totalElems = 200;
var nextRecurse = 10;

function makeArrayStr(maxSz){
  var arrStr = '[';
  for (let i = 0; i < maxSz; i++) {
    if (numElems > totalElems){
      break;
    }
    numElems++;
    if (numElems % nextRecurse == 0){
      rnd = Math.floor(10 + Math.random() * 10);
      arrStr += makeArrayStr(rnd) + ',';
    }
    arrStr += Math.floor(1 + Math.random() * 200) + ',';
  }
  arrStr += ']';
  return arrStr;
}

function makeAssignVar(name){
  numElems = 0;
  code = 'var ' + name + ' = '
  code += makeArrayStr(totalElems);
  code += ';\n';
  return code
}

function makeTest(){
  var code = '';
  var N = 4000;
  for (var i = 0; i < N; i++){
    code += makeAssignVar('arr' + i);
  }
  return code;
}

print(makeTest())
