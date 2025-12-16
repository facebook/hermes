'use strict';
// Flags: --expose-gc --force-node-api-uncaught-exceptions-policy

const common = require('../../common');
const binding = require(`./build/${common.buildType}/test_finalizer`);
const assert = require('assert');
const { gcUntil } = require('../../common/gc');

process.on('uncaughtException', common.mustCall((err) => {
  assert.throws(() => { throw err; }, /finalizer error/);
}));

(async function() {
  (() => {
    binding.createExternalWithJsFinalize(
      common.mustCall(() => {
        throw new Error('finalizer error');
      })
    );
  })();
  await gcUntil('External value calls finalizer',
             () => (binding.finalizeCount === 1));

})().then(common.mustCall());
