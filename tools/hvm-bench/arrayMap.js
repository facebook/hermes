var USE_THISARG = true;

var numIter = 2000;
var len = 10000;
var thisArg;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

if (USE_THISARG) thisArg = a;

function addOne(value) {
  return value + 1;
}

for (var i = 0; i < numIter; i++) {
  a.map(addOne, thisArg);
}

print('done');
