var numIter = 3000;
var len = 10000;
var a = Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

function add(prevSum, value) {
  return prevSum + value;
}

for (var i = 0; i < numIter; i++) {
  a.reduceRight(add, 0);
}

print('done');
