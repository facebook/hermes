var numIter = 15000;
var len = 10000;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

for (var i = 0; i < numIter; i++) {
  a.fill(i)
}

print('done');
