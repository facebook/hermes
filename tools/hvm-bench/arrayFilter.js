var USE_THISARG = false;

var numIter = 2000;
var len = 10000;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

if (USE_THISARG) thisArg = a;

function isEven(val) {
  return val % 2 === 0;
}

for (var i = 0; i < numIter; i++) {
  a.filter(isEven, thisArg);
}

print('done');
