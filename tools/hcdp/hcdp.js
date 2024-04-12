/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

const WebSocket = require('ws');
const { spawn } = require('child_process');
const fs = require('fs');

const port = 9999;
const devtoolsVersion = "60127beb442528082b3f6eff7392267e145262c3";
const url = `https://chrome-devtools-frontend.appspot.com/serve_file/@${devtoolsVersion}/js_app.html?ws=127.0.0.1%3A${port}`;

const exitKey = 'x';

const connectIcon = '⚡';
const commandIcon = '→';
const responseIcon = '←';
const disconnectIcon = '✕';

function logEvent(icon, clientId, message) {
  if (!message) {
    message = '';
  }
  console.log(`${icon} ${clientId} ${message}`);
}

function logConnect(clientId) {
  logEvent(connectIcon, clientId);
}

function logCommand(clientId, message) {
  logEvent(commandIcon, clientId, message);
}

function logResponse(clientId, message) {
  logEvent(responseIcon, clientId, message);
}

function logDisconnect(clientId) {
  logEvent(disconnectIcon, clientId);
}

const connectIPCType = 'C';
const messageIPCType = 'M';
const disconnectIPCType = 'D';

function sendIPC(type, clientId, message) {
  if (!message) {
    message = '';
  }
  childProcess.stdin.write(`${type}${clientId}${message}\n`);
}

function sendConnectIPC(clientId) {
  sendIPC(connectIPCType, clientId);
}

function sendMessageIPC(clientId, message) {
  sendIPC(messageIPCType, clientId, message);
}

function sendDisconnectIPC(clientId) {
  sendIPC(disconnectIPCType, clientId);
}

function fileExists(path) {
  try {
    return fs.statSync(path).isFile();
  } catch (err) {
    return false;
  }
}

// Start Hermes
const usage = "Usage: node <path to hcdp binary> <path to script to debug>";
const binaryPath = process.argv[2];
const scriptPath = process.argv[3];
if (!binaryPath || !scriptPath) {
  console.error(usage);
  process.exit(1);
}
if (!fileExists(binaryPath)) {
  console.error(`Binary not found at ${binaryPath}`);
  process.exit(1);
}
if (!fileExists(scriptPath)) {
  console.error(`Script not found at ${scriptPath}`);
  process.exit(1);
}
const childProcess = spawn(binaryPath, [scriptPath]);

// Start a websocket that forwards messages between the debug client
// and the child process.
const wss = new WebSocket.Server({ port: port });

let clientIdCounter = 0;
const clients = {};

wss.on('connection', (ws) => {
  // Assign a unique ID to this client
  const clientId = clientIdCounter++;
  clients[clientId] = ws;

  // Handle connect
  sendConnectIPC(clientId);
  logConnect(clientId);

  // Handle commands from the client
  ws.on('message', (message) => {
    sendMessageIPC(clientId, message);
    logCommand(clientId, message);
  });

  // Handle disconnect
  ws.on('close', () => {
    sendDisconnectIPC(clientId);
    logDisconnect(clientId);
    delete clients[clientId];
  });
});

// Forward responses/events from Hermes to the client
let lineBuffer = '';
childProcess.stdout.on('data', (data) => {
  // Each line is an IPC element; wait for a newline to indicate the command
  // has been fully received.
  lineBuffer += data.toString();
  const lines = lineBuffer.split('\n');

  // Don't process the last item; if the buffer ended with a newline, the last
  // item is an empty string. If the buffer didn't end with a newline, the
  // last item is a partial line that shouldn't be processed yet.
  for (let i = 0; i < lines.length - 1; i++) {
    const line = lines[i].trim();

    // Find the message
    const messagePosition = line.indexOf('{');
    if (messagePosition === -1) {
      throw new Error(`IPC missing message: ${line}`);
    }

    // Parse the type from the first character
    const typeLength = 1;
    const type = line.substring(0, typeLength);
    if (type !== messageIPCType) {
      throw new Error(`Unexpected IPC command type: ${type}`);
    }

    // Parse the client ID from after the type, until the message
    const clientIdString = line.substring(typeLength, messagePosition);
    const clientId = parseInt(clientIdString, 10);
    if (isNaN(clientId)) {
        throw new Error(`"${clientIdString}" is not a valid number`);
    }
    if (clientId < 0 || clientId >= clientIdCounter) {
      throw new Error(`Unrecognized client ID: ${clientId}`);
    }
    if (!clients[clientId]) {
      // It's possible to receive a message after a client has disconnected.
      // e.g. if the client disconnects at the same time as Hermes CDP emitting
      // a message, we could receive the disconnect notification via the
      // websocket before receiving the message from Hermes via stdout.
      continue;
    }

    // The remainder of the line is the message
    const message = line.substring(messagePosition);
    try {
      JSON.parse(message);
    } catch (error) {
      throw new Error(`Malformed message: ${message}`);
    }

    clients[clientId].send(message);
    logResponse(clientId, message);
  }

  // Save any partial line
  lineBuffer = lines[lines.length - 1];
});

childProcess.on('close', () => {
  console.log('Hermes process exited.');
  process.exit();
});

// Process keyboard commands
process.stdin.setRawMode(true);
process.stdin.setEncoding('utf8');
process.stdin.on('data', (key) => {
  if (key === exitKey) {
    childProcess.kill();
    process.exit();
  }
});

console.log(`Running. Visit ${url} to start debugging.

Commands:
${exitKey} exit

Event format: <event type> <client ID> <message>

Event types:
${connectIcon} connect
${commandIcon} command from client
${responseIcon} response/notification to client
${disconnectIcon} disconnect

`);
