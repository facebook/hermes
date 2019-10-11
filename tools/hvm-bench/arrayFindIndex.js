var USE_THISARG = false;

var numIter = 4000;
var len = 10000;
var thisArg;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

if (USE_THISARG) thisArg = a;

function nonFail(val) {
  return val > 0;
}

function allFail(val) {
  return val < 0;
}

function halfFail(val) {
  return val >= 5000;
}

for (var i = 0; i < numIter; i++) {
  a.findIndex(nonFail, thisArg);
  a.findIndex(allFail, thisArg);
  a.findIndex(halfFail, thisArg);
}

print('done');
