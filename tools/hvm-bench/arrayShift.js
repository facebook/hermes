var len = 10000;

var a = new Array(len);
for (var i = 0; i < len; i++) {
  a[i] = i;
}

for (var j = 0; j < len; j++) {
  a.shift();
}

print('done');
