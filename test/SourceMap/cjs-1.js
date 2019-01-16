var mod2 = require('./cjs-2.js');

function throws1() {
  mod2.throws2();
}

try {
  throws1();
} catch(err) {
  print(err.stack);
}
