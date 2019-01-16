// RUN: true

print('2: init');

module.exports = {x: 1};

var mod3 = require('./cjs-circle-3.js');

module.exports = {y: 2};
print('2: mod3.z =', mod3.z);
module.exports.z = mod3.z;
