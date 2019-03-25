var mod2 = require('cjs-subdir-2.min.js');
function run() {
  try {
    mod2.throwError();
  } catch (e) {
    print(e.stack);
  }
}

exports.run = run;
