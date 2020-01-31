/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

importScripts('hermes.js');

const fileName = '/tmp/hermes-input.js';
let runArgs = [];
let runData = '';
let output = '';
let preheated = false;
let runtimeInitialized = false;
let runRequested = false;
let app;

resetApp();

onmessage = function(e) {
  switch (e.data[0]) {
    case 'run':
      onRun(e.data[1], e.data[2]);
      break;
  }
};

function resetApp() {
  app = createApp({
    print: handleStdout,
    printErr: handleStdout,
    onRuntimeInitialized,
  });
}

function reset() {
  runArgs = [];
  runData = '';
  output = '';
  runRequested = false;

  resetApp();
}

function preheat() {
  app.FS.writeFile(fileName, 'var x = 10;');
  app.callMain(['-O', fileName]);

  preheated = true;
  resetApp();
}

function handleStdout(txt) {
  output += txt;
  output += '\n';
}

function onRuntimeInitialized() {
  if (!preheated) {
    preheat();
    return;
  }

  runtimeInitialized = true;

  if (runRequested) {
    run();
  }
}

function onRun(args, data) {
  if (runRequested) {
    return;
  }

  runArgs = args;
  runData = data;
  runRequested = true;

  if (runtimeInitialized) {
    run();
  }
}

function run() {
  runArgs.push('--pretty-json');
  runArgs.push(fileName);

  app.FS.writeFile(fileName, runData);
  app.callMain(runArgs);

  postMessage(['result', { result: output }]);

  reset();
}
