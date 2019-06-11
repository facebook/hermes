// RUN: true

export var x = 42;

export function y() {
  return 182;
}

export default function() {
  return 352;
}

var myLongVariableName = 472;
var shortVar = 157;

export {myLongVariableName as z, shortVar};

export * from './esm-bar.js';
