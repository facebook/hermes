/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

importScripts('hermes.js');

var output = '';
var runRequested = 0;
var runArgs, runData;
var firstInitialization = true;
var runtimeInitialized;
var app;
var counter = 0;

startRuntimeInitialization(true);

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

function startRuntimeInitialization(first) {
  console.log(ellapsed(), 'starting runtime initialization');
  runtimeInitialized = false;
  app = createApp({
    print: handleStdout,
    printErr: handleStdout,
    onRuntimeInitialized: first
      ? onFirstRuntimeInitialization
      : onRuntimeInitialized,
  });
}

function handleStdout(txt) {
  output += txt;
  output += '\n';
}

function onFirstRuntimeInitialization() {
  console.log(ellapsed(), 'first runtime initialized');
  console.log(ellapsed(), 'preheating hermes...');

  // Warm up Hermes with a basic input. This will force compile all of asm.js.
  var fileName = '/tmp/hermes-input.js';
  app.FS.writeFile(fileName, 'var x = 10;');
  app.callMain(['-O', fileName]);

  output = '';
  console.log(ellapsed(), 'hermes is preheated');
  startRuntimeInitialization();
}

function onRuntimeInitialized() {
  console.log(ellapsed(), 'runtime initialized');
  runtimeInitialized = true;

  if (runRequested) runHermes();
}

onmessage = function(e) {
  console.log(ellapsed(), 'received a message');
  switch (e.data[0]) {
    case 'run':
      onRunHermes(e.data[1], e.data[2]);
      break;
  }
};

function onRunHermes(args, data) {
  if (runRequested) return;
  runArgs = args;
  runData = data;
  runRequested = 1;
  console.log(ellapsed(), 'run requested');

  if (runtimeInitialized) runHermes();
}

function runHermes() {
  console.log(ellapsed(), 'running hermes');
  runRequested = 2;

  var fileName = '/tmp/hermes-input.js';
  app.FS.writeFile(fileName, runData);

  runArgs.push('--pretty-json');
  runArgs.push(fileName);

  app.callMain(runArgs);

  sendRunResult(output);

  runArgs = undefined;
  runData = undefined;
  output = '';
  runRequested = 0;
  startRuntimeInitialization();
}

function sendRunResult(result) {
  console.log(ellapsed(), 'sending result');
  postMessage(['runResult', result]);
}
