# Usage

In the hcdp directory:

1. Build `hcdp binary` using BUCK or CMake
2. Install npm packages
```
npm install
```
3. Run `hcdp.js`, passing it the path to the `hcdp binary`, and the script to be debugged.
e.g.:
```
node ./hcdp.js ~/hcdp ~/loop.js
```

# Implementation

The `hcdp` tool carries out Hermes debugging via CDP. There are
two main parts to the tool: a JavaScript component and a C++ component.

## C++

The C++ component uses the Hermes API to host the objects required to carry out a debugging session (Hermes Runtime, CDP Debug API, CDP Agents). It accepts IPC messages instructing it when to create/destroy a CDP Agent or handle a message. It emits IPC messages for responses and notifications emitted from CDP Agents.

## JavaScript

The JavaScript component hosts a websocket server to communicate with debug clients, prints CDP messages as they flow back and forth, and handles keyboard input from the user.

Incoming websocket messages from the debug client are converted to IPC messages to the C++ component. IPC messages from the C++ component are converted to websocket messages back to the debug client.

## IPC Messages

IPC messages carry a `type`, `agentId`, and optionally `message`. The following message `type`s are supported:

* `C` - Connect. `agentId` indicates the unique ID of the new agent to be created. `message` is unused.

* `M` - Message. `agentId` indicates the ID of the previously-created agent that should handle the `message`.

* `D` - Disconnect. `agentId` indicates the ID of the previously-created agent to be destroyed. `message` is unused.
