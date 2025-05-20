var start = Date.now();
var val = (function () {
  var proto = {x: 7};
  var o = Object.create(proto);

  function access(o) {
    'noinline'
    // 50 accesses, all found in the proto.
    return o.x + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x
      + o.x  + o.x + o.x + o.x + o.x;
  }
  var numIter = 10000000;
  var sum = 0;
  for (var i = 0; i < numIter; i++) {
    sum += access(o);
  }
  return sum;
}());
var end = Date.now();
print("val: " + val);
print("Time: " + (end - start));
