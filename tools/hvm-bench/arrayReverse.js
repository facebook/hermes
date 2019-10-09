var numIter = 10000;
var len = 5000;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

for (var i = 0; i < numIter; i++) {
  a.reverse();
}

print('done');
