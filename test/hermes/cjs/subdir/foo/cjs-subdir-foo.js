// RUN: true

print('foo: init');

var bar = require('../bar/cjs-subdir-bar.js');
print('foo: bar.y =', bar.y);

var bar2 = require('/bar/cjs-subdir-bar.js');
print('foo: absolute bar.y =', bar2.y);

exports.x = bar.y;
