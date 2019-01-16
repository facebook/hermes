// RUN: true

print('foo: init');

var bar = require('../bar/cjs-subdir-bar.js');
print('foo: bar.y =', bar.y);

exports.x = bar.y;
