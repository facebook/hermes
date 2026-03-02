// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// The JavaScript code in this file is adopted from the Node.js project.
// See the src\napi\Readme.md about the Node.js copyright notice.
"use strict";

const { mustCall, mustCallAtLeast, mustNotCall } = require("assert");
const { gcUntil } = require("gc");

const buildType = process.target_config;
const isWindows = process.platform === 'win32';

// Returns true if the exit code "exitCode" and/or signal name "signal"
// represent the exit code and/or signal name of a node process that aborted,
// false otherwise.
function nodeProcessAborted(exitCode, signal) {
  // Depending on the compiler used, node will exit with either
  // exit code 132 (SIGILL), 133 (SIGTRAP) or 134 (SIGABRT).
  let expectedExitCodes = [132, 133, 134];

  // On platforms using KSH as the default shell (like SmartOS),
  // when a process aborts, KSH exits with an exit code that is
  // greater than 256, and thus the exit code emitted with the 'exit'
  // event is null and the signal is set to either SIGILL, SIGTRAP,
  // or SIGABRT (depending on the compiler).
  const expectedSignals = ['SIGILL', 'SIGTRAP', 'SIGABRT'];

  // On Windows, 'aborts' are of 2 types, depending on the context:
  // (i) Exception breakpoint, if --abort-on-uncaught-exception is on
  // which corresponds to exit code 2147483651 (0x80000003)
  // (ii) Otherwise, _exit(134) which is called in place of abort() due to
  // raising SIGABRT exiting with ambiguous exit code '3' by default
  if (isWindows)
    expectedExitCodes = [0x80000003, 134];

  // When using --abort-on-uncaught-exception, V8 will use
  // base::OS::Abort to terminate the process.
  // Depending on the compiler used, the shell or other aspects of
  // the platform used to build the node binary, this will actually
  // make V8 exit by aborting or by raising a signal. In any case,
  // one of them (exit code or signal) needs to be set to one of
  // the expected exit codes or signals.
  if (signal !== null) {
    return expectedSignals.includes(signal);
  }
  return expectedExitCodes.includes(exitCode);
}

Object.assign(module.exports, {
  buildType,
  gcUntil,
  mustCall,
  mustCallAtLeast,
  mustNotCall,
  nodeProcessAborted,
});