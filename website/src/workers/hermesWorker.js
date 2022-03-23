/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

importScripts('hermes.js');

var lastTime;
function ellapsed() {
  if (!lastTime) {
    lastTime = new Date();
    return 0 + ' ms';
  }
  var newTime = new Date();
  var res = newTime - lastTime;
  lastTime = newTime;
  return res + ' ms';
}

async function initHermes(handleOutput) {
  ellapsed();
  let hermesModule = await createHermes({
    print: handleOutput,
    printErr: handleOutput,
  });
  console.log(ellapsed(), 'initializing runtime for hermes');
  return hermesModule;
}

async function runHermes(args, source) {
  ellapsed();

  let output = '';
  let handleOutput = txt => {
    output += `${txt}\n`;
  };

  const {callMain, FS} = await initHermes(handleOutput);

  const fileName = '/tmp/hermes-input.js';
  FS.writeFile(fileName, source);

  callMain([...args, fileName]);

  console.log(ellapsed(), 'running hermes');
  return output;
}

onmessage = async function(e) {
  const [tag, args, source] = e.data;
  switch (tag) {
    case 'run':
      const result = await runHermes(args, source);
      postMessage(['runResult', result]);
      break;
  }
};
