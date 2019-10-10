var numIter = 20000;
var len = 100;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

// The purpose of this variable is so that the return value from indexOf
// is used, thus the compiler wouldn't optimize out the lastIndexOf call
var sum = 0;

for (var i = 0; i < numIter; i++) {
  for (var j = 0; j < len; j++) {
    sum += a.lastIndexOf(j);
  }
}

print('done, sum =', sum);
