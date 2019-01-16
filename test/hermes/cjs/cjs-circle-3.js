// RUN: true

print('3: init');

var mod2 = require('./cjs-circle-2.js');

// Require mod2 while it's initializing, get current exported object.
print('3: mod2.x =', mod2.x);
print('3: mod2.y =', mod2.y);
exports.z = 42;
