// RUN: true

print('foo: init');

var bar = require('../bar/cjs-subdir-bar.js');

function subFun() {
  print('foo_base: bar.y =', bar.y);
}
subFun();

exports.x = bar.y;
